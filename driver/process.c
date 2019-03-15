/**
 * Odin Driver
 * Copyright (c) 2018 Archetype Entertainment Private Limited. All rights reserved.
 *
 * @file process.c
 * @author Aidan Khoury (dude719)
 * @date 8/27/2018  - Initial implementation
 */

#include "process.h"
#include "loader.h"

#include "driver.h"
#include "util.h"
#include "mm.h"

#include "log.h"

static LONG PsThreadPreviousModeOffset = 0;

typedef
NTSTATUS
(NTAPI *PNT_PROTECT_VIRTUAL_MEMORY)(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T NumberOfBytesToProtect,
    IN ULONG NewAccessProtection,
    OUT PULONG OldAccessProtection
    );
static PNT_PROTECT_VIRTUAL_MEMORY NtProtectVirtualMemory = NULL;

typedef
NTSTATUS
(NTAPI *PNT_CREATE_THREAD_EX)(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN HANDLE ProcessHandle,
    IN PVOID StartRoutine,
    IN PVOID Parameter,
    IN ULONG Flags,
    IN SIZE_T StackZeroBits,
    IN SIZE_T SizeOfStackCommit,
    IN SIZE_T SizeOfStackReserve,
    IN OUT PPS_ATTRIBUTE_LIST AttributeList
    );
static PNT_CREATE_THREAD_EX NtCreateThreadEx = NULL;

typedef
NTSTATUS
(NTAPI *PPSP_GET_CONTEXT_THREAD_INTERNAL)(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode,
    IN KPROCESSOR_MODE Mode1,
    IN KPROCESSOR_MODE Mode2
    );
static PPSP_GET_CONTEXT_THREAD_INTERNAL PspGetContextThreadInternal = NULL;

typedef
NTSTATUS
(NTAPI *PPSP_SET_CONTEXT_THREAD_INTERNAL)(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode,
    IN KPROCESSOR_MODE Mode1,
    IN KPROCESSOR_MODE Mode2
    );
static PPSP_SET_CONTEXT_THREAD_INTERNAL PspSetContextThreadInternal = NULL;

static
DEFINE_KE_SUSPEND_THREAD(
    KeSuspendThread
    );

static
DEFINE_KE_RESUME_THREAD(
    KeResumeThread
    );


//
// Private Implementation
//

static
NTSTATUS
PspFindPreviousModeOffset(
    IN PVOID KernelBase,
    OUT PLONG FoundOffset
)
{
    NTSTATUS Status;
    PUCHAR Found;
    LONG PreviousModeOffset;
    CONST UNICODE_STRING ExGetPreviousModeUnicode = RTL_CONSTANT_STRING(L"ExGetPreviousMode");

    if (!ARGUMENT_PRESENT(FoundOffset)) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Get the pointer to the exported ExGetPreviousMode routine.
    //
    Status = LdrFindExportAddressUnicode(KernelBase, &ExGetPreviousModeUnicode, &Found);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Skip the 9 bytes of the first instruction, then grab the offset from the mov instruction.
    //
    //      Since >= Windows 7 build 7600:
    //
    // .text:140093F60                            ExGetPreviousMode proc near
    // .text:140093F60 65 48 8B 04 25 88 01 00 00     mov     rax, gs:188h ; <----- Skip this instruction
    // .text:140093F69 8A 80 F6 01 00 00              mov     al, [rax+1F6h]
    // .text:140093F6F C3                             retn
    // .text:140093F6F                            ; -----------------------------------------------------
    // .text:140093F70 90 90 90 90 90 90 90 90 90+    align 20h
    // .text:140093F70 90 90 90 90 90 90          ExGetPreviousMode endp
    //
    //      Since >= Windows 8 build 9200:
    //
    // .text:140081350                            ExGetPreviousMode proc near
    // .text:140081350 65 48 8B 04 25 88 01 00 00     mov     rax, gs:188h ; <----- Skip this instruction
    // .text:140081359 0F B6 80 32 02 00 00           movzx   eax, byte ptr [rax+232h]
    // .text:140081360 C3                             retn
    // .text:140081360                            ExGetPreviousMode endp
    //
    Found += 9;
    switch (Found[0]) {
    case 0x0F:
        PreviousModeOffset = *((LONG*)(Found + 3));
        break;
    case 0x8A:
        PreviousModeOffset = *((LONG*)(Found + 2));
        break;
    default:
        return STATUS_NOT_FOUND;
    }

    *FoundOffset = PreviousModeOffset;

    return STATUS_SUCCESS;
}

static
NTSTATUS
PspFindNtProtectVirtualMemory(
    IN PVOID KernelBase,
    OUT PVOID *FoundProtectVirtualMemory
)
{
    NTSTATUS Status;
    PKSERVICE_TABLE_DESCRIPTOR SystemServiceDescriptorTable;
    PVOID Found;
    ULONG SsdtIndex;
    USHORT NtKernelBuild;
    CONST UNICODE_STRING ProtectVirtualMemoryUstr = RTL_CONSTANT_STRING(L"ZwProtectVirtualMemory");

    if (!ARGUMENT_PRESENT(FoundProtectVirtualMemory)) {
        return STATUS_INVALID_PARAMETER;
    }

    SystemServiceDescriptorTable = UtlGetServiceDescriptorTable(SYSTEM_SERVICE_INDEX);
    if (!SystemServiceDescriptorTable) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = LdrFindExportAddressUnicode(KernelBase, &ProtectVirtualMemoryUstr, &Found);
    if (NT_SUCCESS(Status)) {
        SsdtIndex = UtlGetServiceDescriptorTableEntryIndex(Found);
    } else {
        //
        // If the export simply wasn't found, continue to get it a different way.
        //
        if (Status != STATUS_NOT_FOUND && Status != STATUS_PROCEDURE_NOT_FOUND) {
            return Status;
        }

        NtKernelBuild = UtlGetNtKernelBuild(NULL);
        if (!NtKernelBuild) {
            return STATUS_UNSUCCESSFUL;
        }

        if (NtKernelBuild >= NT_BUILD_7 && NtKernelBuild < NT_BUILD_8) {
            SsdtIndex = 0x004D;
        } else if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_BLUE) {
            SsdtIndex = 0x004E;
        } else if (NtKernelBuild >= NT_BUILD_BLUE && NtKernelBuild < NT_BUILD_10_1507) {
            SsdtIndex = 0x004F;
        } else if (NtKernelBuild >= NT_BUILD_10_1507) {
            SsdtIndex = 0x0050;
        } else {
            return STATUS_NOT_SUPPORTED;
        }
    }

    LOG_INFO("NtProtectVirtualMemory SSDT Index = 0x%X", SsdtIndex);

    if (SsdtIndex >= SystemServiceDescriptorTable->Limit) {
        return STATUS_ARRAY_BOUNDS_EXCEEDED;
    }

    *FoundProtectVirtualMemory = UtlGetServiceDescriptorTableEntry(
                                        SystemServiceDescriptorTable,
                                        SsdtIndex
                                        );
    if (!*FoundProtectVirtualMemory) {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
PspFindNtCreateThreadEx(
    IN PVOID KernelBase,
    OUT PVOID *FoundCreateThreadEx
)
{
    PKSERVICE_TABLE_DESCRIPTOR SystemServiceDescriptorTable;
    ULONG SsdtIndex;
    USHORT NtKernelBuild;

    UNREFERENCED_PARAMETER(KernelBase);

    if (!ARGUMENT_PRESENT(FoundCreateThreadEx)) {
        return STATUS_INVALID_PARAMETER;
    }

    SystemServiceDescriptorTable = UtlGetServiceDescriptorTable(SYSTEM_SERVICE_INDEX);
    if (!SystemServiceDescriptorTable) {
        return STATUS_UNSUCCESSFUL;
    }

    NtKernelBuild = UtlGetNtKernelBuild(NULL);
    if (!NtKernelBuild) {
        return STATUS_UNSUCCESSFUL;
    }

    if (NtKernelBuild >= NT_BUILD_7 && NtKernelBuild < NT_BUILD_8) {
        SsdtIndex = 0x00A5;
    } else if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_BLUE) {
        SsdtIndex = 0x00AF;
    } else if (NtKernelBuild >= NT_BUILD_BLUE && NtKernelBuild < NT_BUILD_10_1507) {
        SsdtIndex = 0x00B0;
    } else if (NtKernelBuild >= NT_BUILD_10_1507 && NtKernelBuild < NT_BUILD_10_1511) {
        SsdtIndex = 0x00B3;
    } else if (NtKernelBuild >= NT_BUILD_10_1511 && NtKernelBuild < NT_BUILD_10_1607) {
        SsdtIndex = 0x00B4;
    } else if (NtKernelBuild >= NT_BUILD_10_1607 && NtKernelBuild < NT_BUILD_10_1703) {
        SsdtIndex = 0x00B6;
    } else if (NtKernelBuild >= NT_BUILD_10_1703 && NtKernelBuild < NT_BUILD_10_1709) {
        SsdtIndex = 0x00B9;
    } else if (NtKernelBuild >= NT_BUILD_10_1709 && NtKernelBuild < NT_BUILD_10_1803) {
        SsdtIndex = 0x00BA;
    } else if (NtKernelBuild >= NT_BUILD_10_1803 && NtKernelBuild < NT_BUILD_10_1809) {
        SsdtIndex = 0x00BB;
    } else if (NtKernelBuild >= NT_BUILD_10_1809 && NtKernelBuild < NT_BUILD_10_BUILD_18272) {
        SsdtIndex = 0x00BC;
    } else if (NtKernelBuild >= NT_BUILD_10_BUILD_18272 && NtKernelBuild <= NT_BUILD_10_BUILD_18348) {
        SsdtIndex = 0x00BD;
    } else {
        return STATUS_NOT_SUPPORTED;
    }

    LOG_INFO("NtCreateThreadEx SSDT Index = 0x%X", SsdtIndex);

    if (SsdtIndex >= SystemServiceDescriptorTable->Limit) {
        return STATUS_ARRAY_BOUNDS_EXCEEDED;
    }

    *FoundCreateThreadEx = UtlGetServiceDescriptorTableEntry(SystemServiceDescriptorTable,
                                                             SsdtIndex);
    if (!*FoundCreateThreadEx) {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
PspFindKeSuspendThread(
    IN PVOID KernelBase,
    OUT PVOID *FoundKeSuspendThread
)
{
    NTSTATUS Status;
    PUCHAR Found;

    if (!ARGUMENT_PRESENT(FoundKeSuspendThread)) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = UtlFindPatternInSection(KernelBase,
                                     SECTION_PAGELK,
                                     0xCC,
                                     NTSIG(KeSuspendThread),
                                     sizeof(NTSIG(KeSuspendThread)),
                                     (PVOID *)&Found
                                     );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *FoundKeSuspendThread = UtlCallTargetAddress(Found + 9);
    if (!*FoundKeSuspendThread) {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
PspFindKeResumeThread(
    IN PVOID KernelBase,
    OUT PVOID *FoundKeResumeThread
)
{
    NTSTATUS Status;
    PUCHAR Found;

    if (!ARGUMENT_PRESENT(FoundKeResumeThread)) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = UtlFindPatternInSection(KernelBase,
                                     SECTION_TEXT,
                                     0xCC,
                                     NTSIG(KeResumeThread),
                                     sizeof(NTSIG(KeResumeThread)),
                                     (PVOID *)&Found
                                     );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    return UtlFindFunctionStartFromPtr(Found, 1024, FoundKeResumeThread);
}

static
NTSTATUS
PspFindGetSetContextThreadInternal(
    IN PVOID KernelBase,
    OUT PVOID *FoundGetContextThreadInternal,
    OUT PVOID *FoundSetContextThreadInternal
)
{
    NTSTATUS Status;
    PUCHAR FoundGetContextThread;
    PUCHAR FoundSetContextThread;
    ULONG Offset;
    CONST UNICODE_STRING PsGetContextThreadUstr = RTL_CONSTANT_STRING(L"PsGetContextThread");
    CONST UNICODE_STRING PsSetContextThreadUstr = RTL_CONSTANT_STRING(L"PsSetContextThread");

    Status = LdrFindExportAddressUnicode(KernelBase,
                                         &PsGetContextThreadUstr,
                                         &FoundGetContextThread
                                         );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = LdrFindExportAddressUnicode(KernelBase,
                                         &PsSetContextThreadUstr,
                                         &FoundSetContextThread
                                         );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Offset = 0;
    while (Offset < 32 && FoundGetContextThread[Offset] != 0xE8) {
        ++Offset;
    }

    if (Offset >= 32) {
        return STATUS_NOT_FOUND;
    }

    *FoundGetContextThreadInternal = UtlCallTargetAddress(FoundGetContextThread + Offset);
    *FoundSetContextThreadInternal = UtlCallTargetAddress(FoundSetContextThread + Offset);
    if (!*FoundGetContextThreadInternal || !*FoundSetContextThreadInternal) {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}


//
// Public Implementation
//

NTSTATUS
PSAPI
PsInitialize(
    IN PVOID KernelBase
)
{
    NTSTATUS Status;

    //BO_DBG_BREAK();

    //
    // Find the previous mode offset.
    //
    Status = PspFindPreviousModeOffset(KernelBase, &PsThreadPreviousModeOffset);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find ETHREAD PreviousMode offset");
        return Status;
    }

    //
    // Find the NtProtectVirtualMemory function address.
    //
    Status = PspFindNtProtectVirtualMemory(KernelBase, (PVOID*)&NtProtectVirtualMemory);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find NtProtectVirtualMemory");
        return Status;
    }

    //
    // Find the NtCreateThreadEx function address.
    //
    Status = PspFindNtCreateThreadEx(KernelBase, (PVOID*)&NtCreateThreadEx);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find NtCreateThreadEx");
        return Status;
    }

    ////
    //// Find the address of the PsSuspendThread routine.
    ////
    //Status = PspFindKeSuspendThread(KernelBase, (PVOID*)&KeSuspendThread);
    //if (!NT_SUCCESS(Status)) {
    //    LOG_ERROR("Failed to find KeSuspendThread");
    //    return Status;
    //}
    //
    ////
    //// Find the address of the KeResumeThread routine.
    ////
    //Status = PspFindKeResumeThread(KernelBase, (PVOID*)&KeResumeThread);
    //if (!NT_SUCCESS(Status)) {
    //    LOG_ERROR("Failed to find KeResumeThread");
    //    return Status;
    //}

    //
    // Find the address of the PspGetContextThreadInternal and PspSetContextThreadInternal routines.
    //
    Status = PspFindGetSetContextThreadInternal(KernelBase,
                                                (PVOID*)&PspGetContextThreadInternal,
                                                (PVOID*)&PspSetContextThreadInternal);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find PsGetContextThread or PsSetContextThread");
        return Status;
    }


    return STATUS_SUCCESS;
}

KPROCESSOR_MODE
PSAPI
PsSetPreviousMode(
    IN KPROCESSOR_MODE NewMode
)
{
    KPROCESSOR_MODE OldPreviousMode;
    KPROCESSOR_MODE *PreviousModePtr;

    PreviousModePtr = (KPROCESSOR_MODE*)((ULONG_PTR)KeGetCurrentThread() +
                                                        PsThreadPreviousModeOffset);

    OldPreviousMode = *PreviousModePtr;
    *PreviousModePtr = NewMode;

    return OldPreviousMode;
}

NTSTATUS
PSAPI
PsOpenProcessHandle(
    IN PEPROCESS Process,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ProcessHandle
)
{
    NTSTATUS Status;

    //
    // Open a handle to the process.
    //
    Status = ObOpenObjectByPointer(Process,
                                   OBJ_KERNEL_HANDLE,
                                   NULL,
                                   DesiredAccess,
                                   *PsProcessType,
                                   KernelMode,
                                   ProcessHandle
                                   );
    return Status;
}

NTSTATUS
PSAPI
PsCloseProcessHandle(
    IN HANDLE ProcessHandle
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
    Status = NtClose(ProcessHandle);
    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsCloseThreadHandle(
    IN HANDLE ThreadHandle
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtClose(ThreadHandle);

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsGetProcessFullImageName(
    IN PEPROCESS Process,
    OUT WCHAR *ResultImageName,
    IN USHORT MaxImageNameLength,
    OUT USHORT *ImageNameLengthInBytes OPTIONAL
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    ULONG ResultLength;
    HANDLE ProcessHandle;
    PUNICODE_STRING ImageNameUnicode;
    CHAR *ImageFileNameAscii;
    UINT8 ImageFileNameInformation[sizeof(UNICODE_STRING) +
                                   (MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR)) +
                                   sizeof(UNICODE_NULL)];

    if (!Process || !ResultImageName || !MaxImageNameLength) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Need to make sure we are running <= APC_LEVEL because of the call to
    // the ObOpenObjectByPointer API.
    //
    if (KeGetCurrentIrql() > APC_LEVEL) {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Open a handle to the process.
    //
    Status = ObOpenObjectByPointer(Process,
                                   OBJ_KERNEL_HANDLE,
                                   NULL,
                                   PROCESS_QUERY_INFORMATION,
                                   *PsProcessType,
                                   KernelMode,
                                   &ProcessHandle
                                   );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Query for the process image file name.
    //
    // NOTE: We must indicate we are coming from KernelMode in order
    //       to use our kernel handle created from ObOpenObjectByPointer.
    //
    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtQueryInformationProcess(ProcessHandle,
                                       ProcessImageFileName,
                                       ImageFileNameInformation,
                                       sizeof(ImageFileNameInformation),
                                       &ResultLength
                                       );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (ResultLength >= sizeof(UNICODE_STRING)) {
        ImageNameUnicode = (PUNICODE_STRING)ImageFileNameInformation;

        //
        // Make sure the results are valid.
        //
        if (ImageNameUnicode->Buffer && ImageNameUnicode->Length > 0) {

            //
            // Check for overflow!
            // NOTE: The UNICODE_STRING.Length field includes the null-terminating character!
            //
            if (ImageNameUnicode->Length > (MaxImageNameLength - 1) * sizeof(WCHAR)) {

                //
                // Return overflow status
                //
                Status = STATUS_BUFFER_OVERFLOW;

                //
                // Copy what we can into the result image name buffer.
                //
                RtlCopyMemory(ResultImageName, ImageNameUnicode->Buffer, (MaxImageNameLength - 1) * sizeof(WCHAR));

                //
                // Null-terminate for safety.
                //
                ResultImageName[MaxImageNameLength - 1] = L'\0';

            } else {

                //
                // Copy entire string into the result image name buffer.
                //
                RtlCopyMemory(ResultImageName, ImageNameUnicode->Buffer, ImageNameUnicode->Length);

                //
                // Null-terminate for safety.
                //
                ResultImageName[ImageNameUnicode->Length / sizeof(WCHAR)] = L'\0';
            }

            if (ARGUMENT_PRESENT(ImageNameLengthInBytes)) {
                *ImageNameLengthInBytes = ImageNameUnicode->Length;
            }

        } else {

            //
            // Reference the process object by handle.
            //
            Status = ObReferenceObjectByHandle(ProcessHandle,
                                               0,
                                               *PsProcessType,
                                               KernelMode,
                                               (PVOID*)&Process,
                                               NULL
                                               );
            if (NT_SUCCESS(Status)) {

                //
                // Get a pointer to the 15 character process image file name.
                //
                ImageFileNameAscii = PsGetProcessImageFileName(Process);

                //
                // Copy the image file name into our result. 
                //
                Status = RtlMultiByteToUnicodeN(ResultImageName,
                                                MaxImageNameLength,
                                                &ResultLength,
                                                ImageFileNameAscii,
                                                15
                                                );
                if (NT_SUCCESS(Status)) {

                    //
                    // Null-terminate for safety.
                    //
                    ResultImageName[ResultLength / sizeof(WCHAR)] = L'\0';
                    if (ARGUMENT_PRESENT(ImageNameLengthInBytes)) {
                        *ImageNameLengthInBytes = (USHORT)ResultLength;
                    }
                }

                //
                // Dereference the object since ObReferenceObjectByHandle increments 
                // the reference count.
                //
                ObDereferenceObject(Process);
            }
        }
    }

Exit:
    //
    // Don't forget to close the process handle!
    //
    PsCloseProcessHandle(ProcessHandle);

    return Status;
}

NTSTATUS
PSAPI
PsGetProcessFullImageNameByProcessId(
    IN HANDLE ProcessId,
    OUT WCHAR *ResultImageName,
    IN USHORT MaxImageNameLength,
    OUT USHORT *ImageNameLengthInBytes OPTIONAL
)
{
    NTSTATUS Status;
    PEPROCESS Process;

    //
    // Get the process object for the given process id.
    //
    Status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Call the routine using the Process object.
    //
    Status = PsGetProcessFullImageName(Process,
                                       ResultImageName,
                                       MaxImageNameLength,
                                       ImageNameLengthInBytes
                                       );

    //
    // Dereference the process object referenced by PsLookupProcessByProcessId.
    // 
    ObDereferenceObject(Process);

    return Status;
}

NTSTATUS
PSAPI
PsQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtQueryInformationProcess(ProcessHandle,
                                       ProcessInformationClass,
                                       ProcessInformation,
                                       ProcessInformationLength,
                                       ReturnLength
                                       );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtAllocateVirtualMemory(ProcessHandle,
                                     BaseAddress,
                                     ZeroBits,
                                     RegionSize,
                                     AllocationType,
                                     Protect
                                     );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    //
    // !!IMPORTANT!!
    //
    // If the MEM_RELEASE flag is set in the FreeType parameter, the variable
    // pointed to by RegionSize *MUST* be zero. NtFreeVirtualMemory frees the
    // entire region that was reserved in the initial allocation call to the
    // NtAllocateVirtualMemory routine.
    //
    // If the MEM_DECOMMIT flag is set in the FreeType parameter, the call to
    // NtFreeVirtualMemory decommits all memory pages that contain one or more
    // bytes in the range from the BaseAddress parameter to the end of the
    // range (BaseAddress + *RegionSize). This means, for example, that if a
    // two byte region of memory straddles a page boundary, both pages are
    // decommitted.
    //

    Status = NtFreeVirtualMemory(ProcessHandle,
                                 BaseAddress,
                                 RegionSize,
                                 FreeType
                                 );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsProtectVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T NumberOfBytesToProtect,
    IN ULONG NewAccessProtection,
    OUT PULONG OldAccessProtection
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtProtectVirtualMemory(ProcessHandle,
                                    BaseAddress,
                                    NumberOfBytesToProtect,
                                    NewAccessProtection,
                                    OldAccessProtection
                                    );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsCreateThreadEx(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN HANDLE ProcessHandle,
    IN PUSER_THREAD_START_ROUTINE StartRoutine,
    IN PVOID Parameter,
    IN ULONG Flags,
    IN SIZE_T StackZeroBits,
    IN SIZE_T SizeOfStackCommit,
    IN SIZE_T SizeOfStackReserve,
    IN OUT PPS_ATTRIBUTE_LIST AttributeList
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtCreateThreadEx(ThreadHandle,
                              DesiredAccess,
                              ObjectAttributes,
                              ProcessHandle,
                              (PVOID)StartRoutine,
                              Parameter,
                              Flags,
                              StackZeroBits,
                              SizeOfStackCommit,
                              SizeOfStackReserve,
                              AttributeList
                              );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsGetThreadContext(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode
)
{
    NTSTATUS Status;

    //KPROCESSOR_MODE PreviousMode;
    //PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = PspGetContextThreadInternal(Thread, ThreadContext, PreviousMode, UserMode, KernelMode);

    //PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsSetThreadContext(
    IN PETHREAD Thread,
    IN PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode
)
{
    NTSTATUS Status;

    //KPROCESSOR_MODE PreviousMode;
    //PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = PspSetContextThreadInternal(Thread, ThreadContext, PreviousMode, UserMode, KernelMode);

    //PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
PSAPI
PsSuspendThread(
    IN PETHREAD Thread,
    OUT PULONG SuspendCount OPTIONAL
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Count;

    //KPROCESSOR_MODE PreviousMode;
    //PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Count = KeSuspendThread(Thread);

    //PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (ARGUMENT_PRESENT(SuspendCount)) {
        *SuspendCount = Count;
    }

    return Status;
}

NTSTATUS
PSAPI
PsResumeThread(
    IN PETHREAD Thread,
    OUT PULONG SuspendCount OPTIONAL
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Count;

    //KPROCESSOR_MODE PreviousMode;
    //PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Count = KeResumeThread(Thread);

    //PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (ARGUMENT_PRESENT(SuspendCount)) {
        *SuspendCount = Count;
    }

    return Status;
}

NTSTATUS
PSAPI
PsSetProcessProtection(
    IN HANDLE ProcessId,
    IN ULONG ProtectDisableOptions,
    IN ULONG ProtectEnableOptions
)
{
    NTSTATUS Status;
    PEPROCESS Process;
    ULONG ProtectionOffset;
    ULONG FlagsOffset;
    USHORT KernelBuild;

    Status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("PsLookupProcessByProcessId failed with status %08x", Status);
        return Status;
    }

    KernelBuild = BoRuntimeData.KernelBuild;
    if (KernelBuild >= NT_BUILD_7 && KernelBuild < NT_BUILD_8) {
    
        //
        // Bit 11 of the Flags2 field.
        //
        if (ProtectEnableOptions & PS_PROTECT_POLICY_PROTECTION) {
            *((ULONG *)((ULONG_PTR)Process + 0x43C)) |= (1 << 11);
        } else if (ProtectDisableOptions & PS_PROTECT_POLICY_PROTECTION) {
            *((ULONG *)((ULONG_PTR)Process + 0x43C)) &= ~(1 << 11);
        }
    }
    else if (KernelBuild >= NT_BUILD_8 && KernelBuild < NT_BUILD_8_1) {

        //
        // Signing levels indicated by the SignatureLevel field (SE_SIGNING_LEVEL_*).
        //
        if (ProtectEnableOptions & PS_PROTECT_POLICY_PROTECTION) {
            *((SE_SIGNING_LEVEL *)((ULONG_PTR)Process + 0x648)) = SE_SIGNING_LEVEL_MICROSOFT;
        } else if (ProtectDisableOptions & PS_PROTECT_POLICY_PROTECTION) {
            *((SE_SIGNING_LEVEL *)((ULONG_PTR)Process + 0x648)) = SE_SIGNING_LEVEL_UNCHECKED;
        }
    }
    else if (KernelBuild >= NT_BUILD_8_1) {

        //
        // Process protection indicated by the Protection field of type PS_PROTECTION.
        //
        if (KernelBuild < NT_BUILD_10_1507_9841) {
            ProtectionOffset = 0x67A;
            FlagsOffset = 0x2F8;
        } else if (KernelBuild >= NT_BUILD_10_1507_9841 && KernelBuild < NT_BUILD_10_1507_10041) {
            ProtectionOffset = 0x692;
            FlagsOffset = 0x2F8;
        } else if (KernelBuild >= NT_BUILD_10_1507_10041 && KernelBuild < NT_BUILD_10_1507) {
            ProtectionOffset = 0x6AA;
            FlagsOffset = 0x300;
        } else if (KernelBuild >= NT_BUILD_10_1507 && KernelBuild < NT_BUILD_10_1511) {
            ProtectionOffset = 0x6AA;
            FlagsOffset = 0x300;
        } else if (KernelBuild >= NT_BUILD_10_1511 && KernelBuild < NT_BUILD_10_1607) {
            ProtectionOffset = 0x6B2;
            FlagsOffset = 0x300;
        } else if (KernelBuild >= NT_BUILD_10_1607 && KernelBuild < NT_BUILD_10_1703) {
            ProtectionOffset = 0x6CA;
            FlagsOffset = 0x300;
        } else if (KernelBuild >= NT_BUILD_10_1703 && KernelBuild < NT_BUILD_10_1709) {
            ProtectionOffset = 0x6CA;
            FlagsOffset = 0x300;
        } else if (KernelBuild >= NT_BUILD_10_1709 && KernelBuild < NT_BUILD_10_1803) {
            ProtectionOffset = 0x6CA;
            FlagsOffset = 0x828; // MitigationFlagsValues
        } else if (KernelBuild >= NT_BUILD_10_1803 && KernelBuild < NT_BUILD_10_1809) {
            ProtectionOffset = 0x6CA;
            FlagsOffset = 0x828; // MitigationFlagsValues
        } else if (KernelBuild >= NT_BUILD_10_1809 && KernelBuild < NT_BUILD_10_BUILD_18272) {
            ProtectionOffset = 0x6CA;
            FlagsOffset = 0x820; // MitigationFlagsValues
        } else {
            return STATUS_NOT_SUPPORTED;
        }

        //
        // Set process protection.
        //
        if (ProtectEnableOptions & PS_PROTECT_POLICY_PROTECTION) {
            ((PS_PROTECTION *)((ULONG_PTR)Process + ProtectionOffset))->Type = PsProtectedTypeProtected;
            ((PS_PROTECTION *)((ULONG_PTR)Process + ProtectionOffset))->Signer = PsProtectedSignerWinTcb;
        } else if (ProtectDisableOptions & PS_PROTECT_POLICY_PROTECTION) {
            ((PS_PROTECTION *)((ULONG_PTR)Process + ProtectionOffset))->Level = 0;
        }

        //
        // Set process Dynamic Code.
        //
        if (ProtectEnableOptions & PS_PROTECT_POLICY_DYNAMICODE) {
            if (KernelBuild >= NT_BUILD_10_1709) {
                //
                // Indicated by the DisableDynamicCode bit in the MitigationFlagsValues field.
                //
                *((ULONG *)((ULONG_PTR)Process + FlagsOffset)) &= ~(1 << 8); // DisableDynamicCode  
            } else {
                //
                // Indicated by the DisableDynamicCode bit in the Flags2 field.
                //
                *((ULONG *)((ULONG_PTR)Process + FlagsOffset)) &= ~(1 << 10); // DisableDynamicCode
           }
        } else if (ProtectDisableOptions & PS_PROTECT_POLICY_DYNAMICODE) {
            if (KernelBuild >= NT_BUILD_10_1709) {
                //
                // Indicated by the DisableDynamicCode bit in the MitigationFlagsValues field.
                //
                *((ULONG *)((ULONG_PTR)Process + FlagsOffset)) |= (1 << 8); // DisableDynamicCode
            } else {
                //
                // Indicated by the DisableDynamicCode bit in the Flags2 field.
                //
                *((ULONG *)((ULONG_PTR)Process + FlagsOffset)) |= (1 << 10); // DisableDynamicCode
            }
        }

        //
        // Set process Binary Signature.
        //
        PSE_SIGNING_LEVEL SignLevel = (PSE_SIGNING_LEVEL)Process + ProtectionOffset - 2;
        PSE_SIGNING_LEVEL SignLevelSection = (PSE_SIGNING_LEVEL)Process + ProtectionOffset - 1;
        if (ProtectEnableOptions & PS_PROTECT_POLICY_SIGNATURE) {
            *SignLevel = *SignLevelSection = SE_SIGNING_LEVEL_MICROSOFT;
        } else if (ProtectDisableOptions & PS_PROTECT_POLICY_SIGNATURE) {    
            *SignLevel = *SignLevelSection = SE_SIGNING_LEVEL_UNCHECKED;
        }
    }
    else
    {
        return STATUS_NOT_SUPPORTED;
    }

    ObDereferenceObject(Process);

    return Status;
}

NTSTATUS
PSAPI
PsFindHijackableThread(
    IN PEPROCESS Process,
    IN BOOLEAN AcceptMainThread,
    OUT PHANDLE HijackableThreadId
)
{
    NTSTATUS Status;
    PVOID Buffer;
    ULONG BufferSize;
    ULONG NextEntryOffset;
    HANDLE ProcessId;
    PSYSTEM_PROCESS_INFORMATION ProcessInformation;
    PSYSTEM_THREAD_INFORMATION ThreadInformation;
    PSYSTEM_THREAD_INFORMATION ThreadInformationEnd;
    HANDLE CurrentThreadId;
    HANDLE ThreadId;
    HANDLE FoundThreadId;
    PETHREAD Thread;
    KPRIORITY Priority, HighestThreadPriority;
    KPROCESSOR_MODE PreviousMode;

    Buffer = NULL;
    BufferSize = 0;

    //
    // Get the intial size of the process information buffer.
    //
    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
    Status = NtQuerySystemInformation(SystemProcessInformation,
                                      Buffer,
                                      BufferSize,
                                      &BufferSize
                                      );
    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if ((!NT_SUCCESS(Status) &&
         Status != STATUS_INFO_LENGTH_MISMATCH &&
         Status != STATUS_BUFFER_TOO_SMALL) || !BufferSize) {

        //
        // Something weird happened!
        //
        return Status;
    }

    //
    // Keep increasing buffer size until we can fit the entire process 
    // information data buffer.
    //
    for (; Status == STATUS_INFO_LENGTH_MISMATCH ||
           Status == STATUS_BUFFER_TOO_SMALL
         ; BufferSize <<= 1) {

        if (Buffer) {
            MmFreePaged(Buffer);
        }

        //
        // Allocate new size buffer.
        //
        Buffer = MmAllocatePaged(BufferSize);
        if (!Buffer) {

            //
            // Ran out of memory!
            //
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // Try again.
        //
        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
        Status = NtQuerySystemInformation(SystemProcessInformation,
                                          Buffer,
                                          BufferSize,
                                          &BufferSize
                                          );
        PS_PREVIOUS_MODE_RESTORE(PreviousMode);
    }

    //
    // Make sure a failure did not occur.
    //
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    //
    // Search for a hijackable thread.
    //
    ProcessInformation = (PSYSTEM_PROCESS_INFORMATION)Buffer;
    ProcessId = PsGetProcessId(Process);
    CurrentThreadId = PsGetCurrentThreadId();
    FoundThreadId = NULL;
    HighestThreadPriority = LOW_PRIORITY;

    //
    // Iterate all processes, looking for the target 
    //
    do {
        NextEntryOffset = ProcessInformation->NextEntryOffset;

        //CONST UNICODE_STRING CsgoNameString = RTL_CONSTANT_STRING(L"csgo.exe");
        //if (RtlEqualUnicodeString(&ProcessInformation->ImageName, &CsgoNameString, TRUE)) {
        //    BO_DBG_BREAK();
        //}

        //
        // Make sure this is the correct process.
        //
        if (ProcessInformation->UniqueProcessId != ProcessId) {
            goto Next;
        }

        //
        // Get thread information for this process.
        //
        ThreadInformation = (PSYSTEM_THREAD_INFORMATION)((ULONG_PTR)ProcessInformation +
                                                         sizeof(SYSTEM_PROCESS_INFORMATION));
        ThreadInformationEnd = (PSYSTEM_THREAD_INFORMATION)((ULONG_PTR)ThreadInformation +
                                                    ((ProcessInformation->NumberOfThreads - 1) *
                                                        sizeof(SYSTEM_THREAD_INFORMATION)));
        //
        // If this is not a multi-thread process, we'll have to execute on the main thread.
        //
        if (ProcessInformation->NumberOfThreads < 2) {

            //
            // If we cannot accept the main thread, return not found here.
            //
            if (!AcceptMainThread) {
                Status = STATUS_NOT_FOUND;
                goto Exit;
            }

            //
            // Get the main thread's unique id.
            //
            ThreadId = ThreadInformation->ClientId.UniqueThread;

            //
            // Make sure to exclude the current running thread!
            //
            if (ThreadId == CurrentThreadId) {
                Status = STATUS_NOT_FOUND;
                goto Exit;
            }

            //
            // Make sure the main thread is valid.
            //
            if (NT_SUCCESS(PsLookupThreadByThreadId(ThreadId, &Thread))) {

                //
                // Immediately dereference the thread acquired from PsLookupThreadByThreadId.
                //
                ObDereferenceObject(Thread);

                //
                // Looks like we are good!
                //
                FoundThreadId = ThreadId;
            }

        } else {

            if (AcceptMainThread) {

                //
                // If we accept the main thread, decrement the ThreadInformation
                // iterator so that the main thread is included during iteration.
                //
                --ThreadInformation;
            }

            //
            // Iterate all threads, skipping the main thread.
            //
            while (ThreadInformation < ThreadInformationEnd) {

                //
                // Iterate to the next thread. We do this at the beginning of the loop
                // to skip the main thread.
                //
                ++ThreadInformation;

                //
                // Get this thread's unique id.
                //
                ThreadId = ThreadInformation->ClientId.UniqueThread;

                //
                // Make sure to exclude the current running thread!
                //
                if (ThreadId == CurrentThreadId) {
                    continue;
                }

                //
                // Make sure this thread is valid.
                //
                if (PsLookupThreadByThreadId(ThreadId, &Thread) != STATUS_SUCCESS) {
                    continue;
                }

                //
                // Immediately dereference the thread acquired from PsLookupThreadByThreadId.
                //
                ObDereferenceObject(Thread);

                //
                // Check if this thread is suitable to hijack.
                //
                Priority = ThreadInformation->Priority;

                //
                // Check if the thread is in the ideal state for hijacking.
                //
                if (ThreadInformation->ThreadState == Waiting &&
                    (ThreadInformation->WaitReason == UserRequest ||
                     ThreadInformation->WaitReason == WrUserRequest)) {

                    #if defined(ENABLE_EXTENDED_LOGGING)
                    LOG_DEBUG("Found an ideal thread %d in target process:", ThreadId);
                    LOG_DEBUG("    Priority = %d", ThreadInformation->Priority);
                    LOG_DEBUG("    ThreadState = %d", ThreadInformation->ThreadState);
                    LOG_DEBUG("    WaitReason = %d", ThreadInformation->WaitReason);
                    LOG_DEBUG("    Base priority = %d", ThreadInformation->BasePriority);
                    LOG_DEBUG("    Ctx switches = %d", ThreadInformation->ContextSwitches);
                    #endif // ENABLE_EXTENDED_LOGGING

                    FoundThreadId = ThreadId;
                    break;
                }

                //
                // Otherwise, check let's keep track of the thread with the highest priority.
                //
                if ((Priority > LOW_PRIORITY && Priority > HighestThreadPriority) &&
                    (Priority <= LOW_REALTIME_PRIORITY)) {

                    #if defined(ENABLE_EXTENDED_LOGGING)
                    LOG_DEBUG("Found an eligible thread %d in target process:", ThreadId);
                    LOG_DEBUG("    Priority = %d", ThreadInformation->Priority);
                    LOG_DEBUG("    ThreadState = %d", ThreadInformation->ThreadState);
                    LOG_DEBUG("    WaitReason = %d", ThreadInformation->WaitReason);
                    LOG_DEBUG("    Base priority = %d", ThreadInformation->BasePriority);
                    LOG_DEBUG("    Ctx switches = %d", ThreadInformation->ContextSwitches);
                    #endif // ENABLE_EXTENDED_LOGGING

                    FoundThreadId = ThreadId;
                    HighestThreadPriority = Priority;
                }
            }
        }

    Next:
        ProcessInformation = (PSYSTEM_PROCESS_INFORMATION)((ULONG_PTR)ProcessInformation +
                                                                            NextEntryOffset);
    } while (NextEntryOffset);

    //
    // Check if a suitable thread was found.
    //
    if (!FoundThreadId) {
        Status = STATUS_NOT_FOUND;
        goto Exit;
    }

    *HijackableThreadId = FoundThreadId;

Exit:
    if (Buffer) {
        MmFreePaged(Buffer);
    }

    return Status;
}