/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file inject.c
 * @author Aidan Khoury (ajkhoury)
 * @date 2/19/2019
 */

#include "inject.h"
#include "driver.h"
#include "mm.h"
#include "process.h"
#include "loader.h"
#include "util.h"
#include "image.h"
#include "codegen.h"

#include "log.h"

typedef LONG INJ_THUNK_STATE;
typedef LONG INJ_THUNK_RESULT;

typedef struct _INJ_THUNK {
    LIST_ENTRY Link;
    INJ_THUNK_RESULT Result;
    BOOLEAN Queued;
    PUCHAR Code;
    SIZE_T CodeSize;
    PUCHAR Data;
    SIZE_T DataSize;
} INJ_THUNK, *PINJ_THUNK;

#define INJ_THUNK_RESULT_INVALID (INJ_THUNK_RESULT)-1
#define INJ_THUNK_RESULT_SUCCESS (INJ_THUNK_RESULT)TRUE
#define INJ_THUNK_RESULT_FAILURE (INJ_THUNK_RESULT)FALSE
#define INJ_THUNK_STATE_WAITING  (INJ_THUNK_STATE)STATUS_MORE_PROCESSING_REQUIRED

#define INJ_THUNK_IS_FINALIZED(THUNK) \
    (BOOLEAN)((THUNK)->Data == NULL)

#define INJ_THUNK_IS_WAITING(THUNK) \
    (BOOLEAN)(((THUNK)->Data != NULL) && \
              (*((INJ_THUNK_STATE *)((THUNK)->Data)) == INJ_THUNK_STATE_WAITING))

#define INJ_THUNK_GET_RESULT(THUNK) \
    (INJ_THUNK_RESULT)(((THUNK)->Result != INJ_THUNK_RESULT_INVALID) ? (THUNK)->Result : \
                        ((THUNK)->Result = *(INJ_THUNK_RESULT *)((THUNK)->Data)))


typedef struct _INJ_MODULE_INFO {
    WCHAR ModuleName[MAXIMUM_FILENAME_LENGTH];
    PVOID ModuleBase;
    PVOID ModuleEntry;
    ULONG ModuleSize;
    BOOLEAN Injected;
    LIST_ENTRY ThunkList;
    HANDLE ProcessId;
} INJ_MODULE_INFO, *PINJ_MODULE_INFO;

typedef struct _INJ_INFO {
    HANDLE ProcessId;
    ULONGLONG LoadedMask;
    BOOLEAN ForceUserApc;
    RTL_AVL_TABLE ModuleTable;
} INJ_INFO, *PINJ_INFO;

//
// The global gneric AVL table of injection information.
//
static RTL_AVL_TABLE InjpInfoTable;

//
// Injection Source Identifier.
//
// N.B. The Base and Path fields are mutually exclusive when being identified.
//      If the Base field is NULL, then the Path field is used to identify
//      the source.
//
typedef struct _INJ_SOURCE {
    PVOID Base;                           // The source module buffer address.
    WCHAR Path[MAXIMUM_FILENAME_LENGTH];  // The source module path.
} INJ_SOURCE, *PINJ_SOURCE;

//
// Injection Target Identifier.
//
// N.B. The ProcessId and Name fields are mutually exclusive when being
//      identified. If the ProcessId field is NULL, then the Name field is
//      used to identify the target.
//
typedef struct _INJ_TARGET {
    HANDLE ProcessId;                     // The target process id.
    WCHAR Name[MAXIMUM_FILENAME_LENGTH];  // The target process name.
} INJ_TARGET, *PINJ_TARGET;

//
// Injection Source-to-Target Mapping.
//
typedef struct _INJ_MAPPING {
    LIST_ENTRY Link;                      // The link into the InjpInjectionList.
    INJ_SOURCE Source;                    // The injection source module.
    INJ_TARGET Target;                    // The injection target process.
    PINJ_MODULE_INFO ModuleInfo;          // The corresponding module info element for the mapping.
} INJ_MAPPING, *PINJ_MAPPING;

// The maximum allowed number of mappings per process.
#define INJ_MAXIMUM_MAPPINGS 64

//
// The global list of Injection Source-to-Target Mappings.
//
static LIST_ENTRY InjpInjectionListHead;
static KSPIN_LOCK InjpInjectionListLock;


#define INJ_SYSTEM32_NTDLL_INDEX 0
#define INJ_SYSWOW64_NTDLL_INDEX 1
#define INJ_SYSTEM32_WOW64_INDEX 2
#define INJ_SYSTEM32_WOW64WIN_INDEX 3
#define INJ_SYSTEM32_WOW64CPU_INDEX 4
#define INJ_SYSTEM32_KERNEL32_INDEX 5
#define INJ_SYSWOW64_KERNEL32_INDEX 6
#define INJ_SYSTEM32_USER32_INDEX 7
#define INJ_SYSWOW64_USER32_INDEX 8

#define INJ_SYSWOW64_NTDLL (1ULL << INJ_SYSWOW64_NTDLL_INDEX)
#define INJ_SYSTEM32_NTDLL (1ULL << INJ_SYSTEM32_NTDLL_INDEX)
#define INJ_SYSTEM32_WOW64 (1ULL << INJ_SYSTEM32_WOW64_INDEX)
#define INJ_SYSTEM32_WOW64WIN (1ULL << INJ_SYSTEM32_WOW64WIN_INDEX)
#define INJ_SYSTEM32_WOW64CPU (1ULL << INJ_SYSTEM32_WOW64CPU_INDEX)
#define INJ_SYSTEM32_KERNEL32 (1ULL << INJ_SYSTEM32_KERNEL32_INDEX)
#define INJ_SYSWOW64_KERNEL32 (1ULL << INJ_SYSWOW64_KERNEL32_INDEX)
#define INJ_SYSTEM32_USER32 (1ULL << INJ_SYSTEM32_USER32_INDEX)
#define INJ_SYSWOW64_USER32 (1ULL << INJ_SYSWOW64_USER32_INDEX)

//
// Array of dependency system DLLs.
//
// Paths can have format "\Device\HarddiskVolume3\Windows\System32\NTDLL.DLL",
// so only the end of the string is compared.
//

static CONST UNICODE_STRING InjpSystemDlls[] = {
    [INJ_SYSTEM32_NTDLL_INDEX] = RTL_CONSTANT_STRING(L"\\System32\\ntdll.dll"),
    [INJ_SYSWOW64_NTDLL_INDEX] = RTL_CONSTANT_STRING(L"\\SysWOW64\\ntdll.dll"),
    [INJ_SYSTEM32_WOW64_INDEX] = RTL_CONSTANT_STRING(L"\\System32\\wow64.dll"),
    [INJ_SYSTEM32_WOW64WIN_INDEX] = RTL_CONSTANT_STRING(L"\\System32\\wow64win.dll"),
    [INJ_SYSTEM32_WOW64CPU_INDEX] = RTL_CONSTANT_STRING(L"\\System32\\wow64cpu.dll"),
    [INJ_SYSTEM32_KERNEL32_INDEX] = RTL_CONSTANT_STRING(L"\\System32\\kernel32.dll"),
    [INJ_SYSWOW64_KERNEL32_INDEX] = RTL_CONSTANT_STRING(L"\\SysWOW64\\kernel32.dll"),
    [INJ_SYSTEM32_USER32_INDEX] = RTL_CONSTANT_STRING(L"\\System32\\user32.dll"),
    [INJ_SYSWOW64_USER32_INDEX] = RTL_CONSTANT_STRING(L"\\SysWOW64\\user32.dll"),
};


//
// Forward Declarations
//

static
VOID
NTAPI
InjpCreateProcessNotifyRoutineEx(
    IN OUT PEPROCESS Process,
    IN HANDLE ProcessId,
    IN OUT PPS_CREATE_NOTIFY_INFO CreateInfo OPTIONAL
    );

static
VOID
NTAPI
InjpLoadImageNotifyRoutine(
    IN PUNICODE_STRING FullImageName OPTIONAL,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
    );

static
NTSTATUS
InjpDestroyModuleInfoThunkList(
    IN HANDLE ProcessHandle,
    IN OUT PINJ_MODULE_INFO ModuleInfo
    );

static
NTSTATUS
InjpDestroyInjectionInfo(
    IN HANDLE ProcessHandle,
    IN OUT PINJ_INFO InjectInfo
    );


//
// Implementation
//

static
RTL_GENERIC_COMPARE_RESULTS
NTAPI
InjpInjectionInfoCompareRoutine(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
)
{
    UNREFERENCED_PARAMETER(Table);

    //
    // Compare process IDs.
    //
    if (*((HANDLE *)FirstStruct) < *((HANDLE *)SecondStruct)) {
        return GenericLessThan;
    } else if (*((HANDLE *)FirstStruct) > *((HANDLE *)SecondStruct)) {
        return GenericGreaterThan;
    } else {
        return GenericEqual;
    }
}

static
RTL_GENERIC_COMPARE_RESULTS
NTAPI
InjpModuleInfoCompareRoutine(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
)
{
    UNREFERENCED_PARAMETER(Table);

    int Result = _wcsicmp((const WCHAR *)FirstStruct, (const WCHAR *)SecondStruct);
    if (Result < 0) {
        return GenericLessThan;
    } else if (Result > 0) {
        return GenericGreaterThan;
    } else {
        return GenericEqual;
    }
}

static
PVOID
InjpAllocate(
    IN CLONG ByteSize
)
{
    PVOID Buffer;

    //
    // Allocate a non paged no-execute buffer.
    //
    Buffer = MmAllocateNonPagedNx((SIZE_T)ByteSize);
    if (!Buffer) {

        //
        // If allocation was unsuccessful, then just bugcheck.
        //
        KeBugCheckEx(MEMORY_MANAGEMENT, ByteSize, 0, 0, 0);
    }

    //
    // Zero out the newly allocated buffer.
    //
    RtlZeroMemory(Buffer, ByteSize);

    return Buffer;
}

static
PVOID
NTAPI
InjpAllocateRoutine(
    IN struct _RTL_AVL_TABLE *Table,
    IN CLONG ByteSize
)
{
    UNREFERENCED_PARAMETER(Table);
    return InjpAllocate(ByteSize);
}

static
VOID
InjpFree(
    IN PVOID Buffer
)
{
    BO_ASSERT(Buffer != NULL);
    MmFreeNonPaged(Buffer);
}

static
VOID
NTAPI
InjpFreeRoutine(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(Table);
    InjpFree(Buffer);
}

static
PINJ_INFO
InjpFindInjectionInfoForProcessId(
    IN HANDLE ProcessId
)
{
    PINJ_INFO InjectInfo = (PINJ_INFO)RtlLookupElementGenericTableAvl(
                                        &InjpInfoTable,
                                        (PVOID)&ProcessId
                                        );
    return InjectInfo;
}

static
PINJ_MODULE_INFO
InjpFindModuleInfoForInjectionMapping(
    IN PINJ_MAPPING Mapping
)
{
    PINJ_INFO InjectInfo;
    UNICODE_STRING ImageName;
    PINJ_MODULE_INFO ModuleInfo = Mapping->ModuleInfo;

    //
    // If a corresponding module info element has been set in the mapping
    // already, then just return it.
    //
    if (ModuleInfo) {
        return ModuleInfo;
    }

    //
    // Lookup the injection info element for the target process id.
    //
    InjectInfo = InjpFindInjectionInfoForProcessId(Mapping->Target.ProcessId);
    if (!InjectInfo) {
        return NULL;
    }

    //
    // Get the source image name (make one if the source Base field was used).
    //
    if (Mapping->Source.Base) {

        RtlInitEmptyUnicodeString(&ImageName, Mapping->Source.Path, sizeof(Mapping->Source.Path));
        RtlInt64ToUnicodeString((ULONGLONG)Mapping->Source.Base, 16, &ImageName);

    } else {

        RtlInitUnicodeString(&ImageName, Mapping->Source.Path);
    }

    //
    // Lookup a matching element for the source module name.
    //
    ModuleInfo = (PINJ_MODULE_INFO)RtlLookupElementGenericTableAvl(
                                        &InjectInfo->ModuleTable,
                                        (PVOID)ImageName.Buffer
                                        );
    return ModuleInfo;
}

static
ULONG
InjpFindInjectionMappingsForProcessId(
    IN HANDLE ProcessId,
    OUT PINJ_MAPPING *Mappings,
    IN ULONG MaximumMappings
)
{
    PLIST_ENTRY Next;
    USHORT ProcessFileNameLength;
    WCHAR ProcessFileNameBuffer[MAXIMUM_FILENAME_LENGTH];
    UNICODE_STRING ProcessFileName;
    UNICODE_STRING TargetProcessName;
    PINJ_MAPPING Mapping;
    PINJ_MAPPING *MappingArray;
    ULONG MappingCount;

    //
    // Get the process file name.
    //
    if (!NT_SUCCESS(PsGetProcessFullImageNameByProcessId(ProcessId,
                                                         ProcessFileNameBuffer,
                                                         RTL_NUMBER_OF(ProcessFileNameBuffer),
                                                         &ProcessFileNameLength))) {
        return 0;
    }
    ProcessFileName.Length = ProcessFileNameLength;
    ProcessFileName.MaximumLength = sizeof(ProcessFileNameBuffer);
    ProcessFileName.Buffer = ProcessFileNameBuffer;

    MappingArray = Mappings;
    MappingCount = 0;

    //
    // Check if the given process id matches one of the target processes
    // in the registered injection mappings.
    //
    Next = InjpInjectionListHead.Flink;
    while (Next != &InjpInjectionListHead) {
        Mapping = CONTAINING_RECORD(Next, INJ_MAPPING, Link);

        //
        // If the mapping was validated it will contain a process id.
        // 
        // N.B. Targets that use process ids for indentification should
        //      validate the process id before registering the injection!
        //
        if (Mapping->Target.ProcessId) {

            //
            // The target in this mapping is identified by the process id.
            // Check to see if the given process id matches the target 
            // process id of the mapping.
            //
            if (ProcessId == Mapping->Target.ProcessId) {

                *MappingArray++ = Mapping;
                if (++MappingCount == MaximumMappings) {
                    return MappingCount;
                }
            }

        } else {
            
            //
            // The target in this mapping is identified by the process file
            // name. Check to see if the suffixed part of the process name of
            // the given process id matches this mapping's target process name.
            //
            RtlInitUnicodeString(&TargetProcessName, Mapping->Target.Name);
            if (UtlSuffixUnicodeString(&TargetProcessName, &ProcessFileName, TRUE)) {

                //
                // Update the mapping with the process ID of the process that
                // matched the target name. This will set the mapping in a
                // validated state, and must be set back to NULL to invalidate
                // once the target process terminates.
                //
                Mapping->Target.ProcessId = ProcessId;

                *MappingArray++ = Mapping;
                if (++MappingCount == MaximumMappings) {
                    return MappingCount;
                }
            }
        }

        Next = Next->Flink;
    }

    return MappingCount;
}

static
VOID
InjpInvalidateInjectionMappingsForProcessId(
    IN HANDLE ProcessId
)
{
    PLIST_ENTRY Next;
    PINJ_MAPPING Mapping;

    //
    // Check for inection mappings that match the supplied process id of
    // the target process.
    //
    Next = InjpInjectionListHead.Flink;
    while (Next != &InjpInjectionListHead) {
        Mapping = CONTAINING_RECORD(Next, INJ_MAPPING, Link);

        if (Mapping->Target.ProcessId == ProcessId) {

            //
            // Invalidate the mapping.
            //
            Mapping->Target.ProcessId = NULL;
            Mapping->ModuleInfo = NULL;
        }

        Next = Next->Flink;
    }
}

typedef struct _INJ_FORCE_APC_CONTEXT {
    KAPC Apc;
    PVOID SystemArgument2;
    KPROCESSOR_MODE ApcMode;
} INJ_FORCE_APC_CONTEXT, *PINJ_FORCE_APC_CONTEXT;

static
VOID
NTAPI
InjpApcKernelRoutine(
    IN PRKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
)
{
    //
    // Common kernel routine for both user-mode and kernel-mode APCs queued
    // by the InjQueueApc/InjForceApc routine. Just release the memory of the
    // APC object and return.
    //

    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    //BO_DBG_BREAK();

    //
    // If the current thread is terminating (due to an exception or something)
    // then skip executing the NormalRoutine.
    //
    if (PsIsThreadTerminating(PsGetCurrentThread())) {
        *NormalRoutine = NULL;
    }

    //
    // Determine if this is a wow64 process or not.
    //
    if (PsGetCurrentProcessWow64Process()) {

        //
        // PsWrapApcWow64Thread essentially assigns wow64.dll!Wow64ApcRoutine
        // to the NormalRoutine. This Wow64ApcRoutine (which is 64-bit code)
        // in turn calls KiUserApcDispatcher (in 32-bit ntdll.dll) which finally
        // calls our provided APC routine.
        //
        PsWrapApcWow64Thread(NormalContext, (PVOID *)NormalRoutine);
    }

    //
    // Just release the memory of the APC context and return.
    //
    InjpFree((PVOID)Apc);
}

static
VOID
NTAPI
InjpForceApcKernelRoutine(
    IN PRKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
)
{
    PKAPC ActualApc;
    PINJ_FORCE_APC_CONTEXT ForceContext;
    KPROCESSOR_MODE ApcMode;
    PVOID Context, Argument1, Argument2;
    LARGE_INTEGER AlertableTimeout;

    UNREFERENCED_PARAMETER(NormalRoutine);

    ForceContext = (PINJ_FORCE_APC_CONTEXT)Apc;

    ApcMode = ForceContext->ApcMode;
    Context = *SystemArgument1;
    Argument1 = *SystemArgument2;
    Argument2 = ForceContext->SystemArgument2;

    //BO_DBG_BREAK();
    LOG_DEBUG("Forcing User-Mode APC execution...");
    LOG_DEBUG("NormalRoutine = %p", *NormalRoutine);
    LOG_DEBUG("NormalContext = %p", *NormalContext);
    LOG_DEBUG("Context = %p", Context);
    LOG_DEBUG("Argument1 = %p", Argument1);
    LOG_DEBUG("Argument2 = %p", Argument2);

    //
    // Free the old force APC context.
    //
    InjpFree(ForceContext);

    //
    // Allocate the actual APC object.
    //
    // N.B. This should NEVER fail because we free space in the statement
    //      directly before this!
    //
    ActualApc = InjpAllocate(sizeof(KAPC));
    BO_ASSERT(ActualApc != NULL);

    //
    // Initialize the actual APC object we are forcing.
    //
    KeInitializeApc(ActualApc,
                    PsGetCurrentThread(),
                    OriginalApcEnvironment,
                    InjpApcKernelRoutine,
                    NULL,
                    (PKNORMAL_ROUTINE)*NormalContext,
                    ApcMode,
                    Context
                    );
    //
    // Insert the actual APC.
    //
    if (!KeInsertQueueApc(ActualApc, Argument1, Argument2, IO_NO_INCREMENT)) {
    
        //
        // If a failure occured trying to insert the actual APC, free the
        // actual APC here.
        //
        InjpFree(ActualApc);
    }

    //
    // Small wait in user mode to cause the thread to become alertable by
    // user mode APC, essentially forcing it to the top of the queue!
    //
    //KeTestAlertThread(UserMode);
    AlertableTimeout.QuadPart = RELATIVE(MILLISECONDS(1));
    KeDelayExecutionThread(UserMode, TRUE, &AlertableTimeout);
}

static
VOID
NTAPI
InjpForceApcNormalRoutine(
    IN PVOID NormalContext OPTIONAL,
    IN PVOID SystemArgument1 OPTIONAL,
    IN PVOID SystemArgument2 OPTIONAL
)
{
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);
    return;
}

NTSTATUS
INJAPI
InjForceApc(
    IN PETHREAD Thread,
    IN KPROCESSOR_MODE ApcMode,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
)
{
    PINJ_FORCE_APC_CONTEXT ForceContext;
    BOOLEAN Inserted;

    //
    // Allocate memory for the APC object.
    //
    ForceContext = (PINJ_FORCE_APC_CONTEXT)InjpAllocate(sizeof(INJ_FORCE_APC_CONTEXT));
    if (!ForceContext) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ForceContext->SystemArgument2 = SystemArgument2;
    ForceContext->ApcMode = ApcMode;

    //
    // Initialize and queue the kernel mode APC used to force the actual APC.
    //
    KeInitializeApc(&ForceContext->Apc,             // Apc
                    Thread,                         // Thread
                    OriginalApcEnvironment,         // Environment
                    InjpForceApcKernelRoutine,      // KernelRoutine
                    NULL,                           // RundownRoutine
                    InjpForceApcNormalRoutine,      // NormalRoutine
                    KernelMode,                     // ApcMode
                    (PVOID)NormalRoutine            // NormalContext
                    );

    Inserted = KeInsertQueueApc(&ForceContext->Apc, // Apc
                                NormalContext,      // SystemArgument1
                                SystemArgument1,    // SystemArgument2
                                IO_NO_INCREMENT     // Increment
                                );
    if (!Inserted) {
        InjpFree((PVOID)ForceContext);
        return STATUS_NOT_CAPABLE;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
INJAPI
InjQueueApc(
    IN PETHREAD Thread,
    IN KPROCESSOR_MODE ApcMode,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
)
{
    PKAPC Apc;
    BOOLEAN Inserted;

    //
    // Allocate memory for the APC object.
    //
    Apc = (PKAPC)InjpAllocate(sizeof(KAPC));
    if (!Apc) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initialize and queue the APC.
    //
    KeInitializeApc(Apc,                            // Apc
                    Thread,                         // Thread
                    OriginalApcEnvironment,         // Environment
                    InjpApcKernelRoutine,           // KernelRoutine
                    NULL,                           // RundownRoutine
                    NormalRoutine,                  // NormalRoutine
                    ApcMode,                        // ApcMode
                    NormalContext                   // NormalContext
                    );

    Inserted = KeInsertQueueApc(Apc,                // Apc
                                SystemArgument1,    // SystemArgument1
                                SystemArgument2,    // SystemArgument2
                                IO_NO_INCREMENT     // Increment
                                );
    if (!Inserted) {
        InjpFree(Apc);
        return STATUS_NOT_CAPABLE;
    }

    return STATUS_SUCCESS;
}

FORCEINLINE
BOOLEAN
InjpReadyToInject(
    IN PEPROCESS Process,
    IN PINJ_INFO InjectInfo
)
{
    ULONG RequiredMask;

    //
    // DLLs that need to be loaded in the native process (i.e.: x64 process on
    // x64 Windows, x86 process on x86 Windows) before we can safely load our
    // image.
    //
    RequiredMask = INJ_SYSTEM32_NTDLL;

    if (PsGetProcessWow64Process(Process)) {

        //
        // DLLs that need to be loaded in the Wow64 process before we can safely
        // load our module.
        //
        RequiredMask |= INJ_SYSTEM32_WOW64;
        RequiredMask |= INJ_SYSTEM32_WOW64WIN;
        RequiredMask |= INJ_SYSTEM32_WOW64CPU;
        RequiredMask |= INJ_SYSWOW64_NTDLL;

        if (BoRuntimeData.IsWindows7) {

            //
            // On Windows 7, if we're injecting DLL into Wow64 process using,
            // we have to additionaly postpone the load after these system DLLs.
            //
            // This is because on Windows 7, these DLLs are loaded as part of
            // the wow64!ProcessInit routine, therefore the Wow64 subsystem
            // is not fully initialized to execute our injected Wow64ApcRoutine.
            //
            RequiredMask |= INJ_SYSTEM32_KERNEL32;
            RequiredMask |= INJ_SYSWOW64_KERNEL32;
            RequiredMask |= INJ_SYSTEM32_USER32;
            RequiredMask |= INJ_SYSWOW64_USER32;
        }

    } else {

        RequiredMask |= INJ_SYSTEM32_KERNEL32;
        RequiredMask |= INJ_SYSTEM32_USER32;
    }

    return (InjectInfo->LoadedMask & RequiredMask) == RequiredMask;
}

static
NTSTATUS
InjpFreeThunk(
    IN HANDLE ProcessHandle,
    IN PINJ_THUNK Thunk
)
{
    NTSTATUS Status, FinalStatus = STATUS_SUCCESS;

    if (Thunk->Code) {
    
        //
        // Free the thunk code user mode process virtual memory.
        //
        Thunk->CodeSize = 0;
        Status = PsFreeVirtualMemory(ProcessHandle,
                                     &Thunk->Code,
                                     &Thunk->CodeSize,
                                     MEM_RELEASE
                                     );
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to free thunk code with status %08x", Status);
            FinalStatus = Status;
        }

        LOG_INFO("Thunk code %p memory released successfully", Thunk->Code);
    }

    if (Thunk->Data) {
    
        //
        // Free the thunk data user mode process virtual memory.
        //
        Thunk->DataSize = 0;
        Status = PsFreeVirtualMemory(ProcessHandle,
                                     &Thunk->Data,
                                     &Thunk->DataSize,
                                     MEM_RELEASE
                                     );
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to free thunk data with status %08x", Status);
            FinalStatus = Status;
        }

        LOG_INFO("Thunk code %p memory released successfully", Thunk->Data);
    }

    //
    // Free the thunk object kernel memory.
    //
    InjpFree(Thunk);

    return FinalStatus;
}

static
NTSTATUS
InjpAllocateThunk(
    IN PEPROCESS Process, 
    IN PVOID InitializerRoutine,
    IN PVOID ImageBase,
    IN ULONG Reason,
    OUT PINJ_THUNK *Thunk
)
{
    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T RegionSize;
    ULONG OldProtection;
    PINJ_THUNK NewThunk;
    HCODEGEN Generator = NULL;
    BOOLEAN Wow64 = (BOOLEAN)(PsGetProcessWow64Process(Process) != NULL);

    //
    // Allocate kernel memory for new thunk object.
    //
    NewThunk = (PINJ_THUNK)InjpAllocate(sizeof(INJ_THUNK));
    if (!NewThunk) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Allocate thunk code.
    //
    BaseAddress = NULL;
    RegionSize = PAGE_SIZE;
    Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                     &BaseAddress,
                                     0,
                                     &RegionSize,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {
        goto FailExit;
    }
    NewThunk->Code = (PUCHAR)BaseAddress;
    NewThunk->CodeSize = RegionSize;

    //
    // Allocate thunk data.
    //
    BaseAddress = NULL;
    RegionSize = PAGE_SIZE;
    Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                     &BaseAddress,
                                     0,
                                     &RegionSize,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {
        goto FailExit;
    }
    NewThunk->Data = (PUCHAR)BaseAddress;
    NewThunk->DataSize = RegionSize;

    //
    // Initialize the code generator.
    //
    Status = CgInitializeGenerator(&Generator, NewThunk->Code, NewThunk->CodeSize, !Wow64);
    if (!NT_SUCCESS(Status)) {
        goto FailExit;
    }

    //
    // Initialize the thunk to a waiting state.
    //
    *(INJ_THUNK_STATE *)NewThunk->Data = INJ_THUNK_STATE_WAITING;
    NewThunk->Result = INJ_THUNK_RESULT_INVALID;

    //
    // Generate the thunk code.
    //
    CgBeginCode(Generator, FALSE);                          // backup non-volatile registers  
    if (!Wow64) CgAddUInt8(Generator, 0x48);                // mov    rbx, rcx
    CgAddUInt16(Generator, 0xCB89);                         //
    
    //
    // BOOL
    // WINAPI
    // Initializer(
    //     IN HANDLE Instance,
    //     IN ULONG Reason,
    //     IN PVOID Context
    //     );
    //
    CgPushPointer(Generator, ImageBase);                    // ModuleHandle,
    CgPushUInt32(Generator, Reason);                        // Reason,
    CgPushPointer(Generator, NewThunk->Data);               // Context
    CgCall(Generator, CC_STDCALL, InitializerRoutine);      // Initializer(Instance, DLL_PROCESS_ATTACH, Data);
    CgAddUInt16(Generator, 0x0389);                         // mov    dword ptr[rbx], eax  
    CgEndCode(Generator);                                   // restore non-volatile registers
    CgReturn(Generator, 3 * (!Wow64 ? 0 : sizeof(ULONG)));  // ret

    //
    // Destroy the code generator object kernel memory.
    //
    CgDestroyGenerator(Generator);

    //
    // Properly protect the thunk code.
    //
    BaseAddress = NewThunk->Code;
    RegionSize = NewThunk->CodeSize;
    Status = PsProtectVirtualMemory(NtCurrentProcess(),
                                    &BaseAddress,
                                    &RegionSize,
                                    PAGE_EXECUTE_READ,
                                    &OldProtection
                                    );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to protect thunk code with status %08x", Status);
        goto FailExit;
    }

    //
    // If this doesn't match, something has fucked up majorly!
    //
    BO_ASSERT(OldProtection == PAGE_EXECUTE_READWRITE);

    *Thunk = NewThunk;
    return Status;

FailExit:
    if (Generator) {
        CgDestroyGenerator(Generator);
    }

    InjpFreeThunk(NtCurrentProcess(), NewThunk);

    *Thunk = NULL;
    return Status;
}

NTSTATUS
NTAPI
InjpInsertThunkInitializer(
    IN PEPROCESS Process,
    IN PLDR_INIT_ROUTINE InitializerRoutine,
    IN PLDR_MAPPED_IMAGE_INFO ImageInfo,
    IN PINJ_MODULE_INFO ModuleInfo OPTIONAL
)
{
    NTSTATUS Status;
    PINJ_THUNK Thunk;

    BO_ASSERT(ModuleInfo != NULL);

    //
    // Allocate new thunk object.
    //
    Status = InjpAllocateThunk(Process,
                               (PVOID)InitializerRoutine,
                               ImageInfo->ImageBase,
                               LDR_DLL_PROCESS_ATTACH,
                               &Thunk
                               );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to allocate thunk with status %08x", Status);
        goto Exit;
    }

    //
    // Insert entry at the end of the thunk list.
    //
    InsertTailList(&ModuleInfo->ThunkList, &Thunk->Link);

Exit:
    return Status;
}

static
NTSTATUS
InjpDestroyModuleInfoThunkList(
    IN HANDLE ProcessHandle,
    IN OUT PINJ_MODULE_INFO ModuleInfo
)
{
    NTSTATUS Status, FinalStatus = STATUS_SUCCESS;
    PINJ_THUNK Thunk;

    //
    // Clear out all entries in the thunk list.
    //
    while (!IsListEmpty(&ModuleInfo->ThunkList)) {
        Thunk = CONTAINING_RECORD(ModuleInfo->ThunkList.Flink, INJ_THUNK, Link);
        RemoveHeadList(&ModuleInfo->ThunkList);

        //
        // Free the thunk user mode process virtual memory.
        //
        // N.B. This also frees the thunk entry kernel memory!
        //
        Status = InjpFreeThunk(ProcessHandle, Thunk);
        if (!NT_SUCCESS(Status)) {
            FinalStatus = Status;
            LOG_ERROR("Failed to free thunk with status %08x", Status);
        }
    }

    return FinalStatus;
}

static
NTSTATUS
InjpDestroyInjectionInfo(
    IN HANDLE ProcessHandle,
    IN OUT PINJ_INFO InjectInfo
)
{
    NTSTATUS SubStatus, Status = STATUS_SUCCESS;
    PVOID P;
    PINJ_MODULE_INFO ModuleInfo;

    //
    // Clear out all entries in the module info list.
    //
    for (P = RtlEnumerateGenericTableAvl(&InjectInfo->ModuleTable, TRUE);
         P != NULL;
         P = RtlEnumerateGenericTableAvl(&InjectInfo->ModuleTable, FALSE)) {

        ModuleInfo = (PINJ_MODULE_INFO)P;

        //
        // Destroy the thunk list.
        //
        SubStatus = InjpDestroyModuleInfoThunkList(ProcessHandle, ModuleInfo);
        if (!NT_SUCCESS(SubStatus)) {
            Status = SubStatus;
            LOG_ERROR("Failed to destroy thunk list with status %08x", Status);
        }

        //
        // Delete the module info element from the AVL table.
        //
        RtlDeleteElementGenericTableAvl(&InjectInfo->ModuleTable, P);
    }

    return Status;
}

static
NTSTATUS
InjpInject(
    IN OUT PINJ_INFO InjectInfo,
    IN PEPROCESS Process,
    IN PVOID ImageBase OPTIONAL,
    IN PUNICODE_STRING ImagePath,
    OUT PINJ_MODULE_INFO *ResultModuleInfo
)
{
    NTSTATUS Status;
    LDR_MAPPED_IMAGE_INFO MappedImageInfo;
    INJ_MODULE_INFO NewModuleInfo;
    PINJ_MODULE_INFO ModuleInfo;
    BOOLEAN NewElement;
    PINJ_THUNK Thunk;

    BO_DBG_BREAK();

    //
    // Let 'er rip
    //
    Status = LdrFindOrMapImage(Process,
                               ImageBase,
                               ImagePath,
                               FALSE,
                               &MappedImageInfo
                               );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to map image with status %08x", Status);
        return Status;
    }

    //
    // Setup the new injected module info and insert a the new module info
    // element into the generic AVL table.
    //
    RtlZeroMemory(NewModuleInfo.ModuleName, sizeof(NewModuleInfo.ModuleName));
    RtlCopyMemory(NewModuleInfo.ModuleName,
                  MappedImageInfo.ImageName.Buffer, MappedImageInfo.ImageName.Length);
    NewModuleInfo.ModuleBase = MappedImageInfo.ImageBase;
    NewModuleInfo.ModuleEntry = MappedImageInfo.EntryPoint;
    NewModuleInfo.ModuleSize = MappedImageInfo.SizeOfImage;
    NewModuleInfo.ProcessId = PsGetProcessId(Process);
    NewModuleInfo.Injected = FALSE;
    ModuleInfo = RtlInsertElementGenericTableAvl(&InjectInfo->ModuleTable,
                                                 &NewModuleInfo,
                                                 sizeof(NewModuleInfo),
                                                 &NewElement
                                                 );
    BO_ASSERT(NewElement == TRUE);

    //
    // Initialize the thunk list head after the new module info element is
    // added to the generic AVL table.
    //
    // N.B. We *MUST* initialize the thunk list head AFTER we insert the new
    //      element as the generic AVL table implementation simply allocates
    //      a new element and copies our given data over to the new element.
    //      This means if we initialized the list head before inserting the
    //      new element into the generic AVL table, the list head field would
    //      be pointing to the old address of the element data we initialized
    //      on the stack, and therefore be invalid after leaving this scope.
    //      This is also the reason we queue the initializers after the new
    //      module info element is added to the table, so that we dont link
    //      the new initializer thunk entries to a list head on the stack!
    //
    InitializeListHead(&ModuleInfo->ThunkList);

    //
    // Insert the image TLS initializers into the thunk list.
    //
    Status = LdrForEachTlsInitializer(Process,
                                      &MappedImageInfo,
                                      InjpInsertThunkInitializer,
                                      ModuleInfo
                                      );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to insert TLS initializers with status %08x", Status);
        goto FailExit;
    }
    
    //
    // Insert the image entry point initializer into the thunk list.
    //
    if (MappedImageInfo.EntryPoint != NULL) {
    
        Status = InjpInsertThunkInitializer(Process,
                                            (PLDR_INIT_ROUTINE)MappedImageInfo.EntryPoint,
                                            &MappedImageInfo,
                                            ModuleInfo
                                            );
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to insert entry point initializer with status %08x", Status);
            goto FailExit;
        }
    }

    BO_DBG_BREAK();

    //
    // Check if there are any initializer thunks for this module that need
    // to be queued.
    //
    if (!IsListEmpty(&ModuleInfo->ThunkList)) {
    
        //
        // Queue an APC for the first initializer thunk.
        //
        Thunk = CONTAINING_RECORD(ModuleInfo->ThunkList.Flink, INJ_THUNK, Link);
        Status = InjForceApc(PsGetCurrentThread(),
                             UserMode,
                             (PKNORMAL_ROUTINE)Thunk->Code,
                             Thunk->Data,
                             NULL,
                             NULL
                             );
    
        if (NT_SUCCESS(Status)) {

            //
            // Mark this thunk as queued for execution.
            //
            Thunk->Queued = TRUE;

        } else {
        
            LOG_ERROR("Failed to queue APC for thunk with status %08x", Status);

            //
            // We DO NOT fail here since we have another chance to queue the
            // initializer for execution later.
            //
        }
    }

    *ResultModuleInfo = ModuleInfo;

    //
    // Return here to skip the failure cleanup. 
    //
    return Status;

FailExit:
    //
    // Destroy the module info thunk list and delete the element from the
    // generic AVL table.
    //
    InjpDestroyModuleInfoThunkList(NtCurrentProcess(), ModuleInfo);
    RtlDeleteElementGenericTableAvl(&InjectInfo->ModuleTable, ModuleInfo);

    //
    // Free the mapped image.
    //
    LdrFreeImageMemory(&MappedImageInfo);

    return Status;
}

static
NTSTATUS
InjpFinalizeInjection(
    IN PINJ_INFO InjectInfo,
    IN PINJ_MODULE_INFO ModuleInfo
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PINJ_THUNK Thunk = NULL;
    INJ_THUNK_RESULT ThunkResult;

    UNREFERENCED_PARAMETER(InjectInfo);

    //
    // Check if all the initializer thunks have completed execution yet.
    //
    while (!IsListEmpty(&ModuleInfo->ThunkList)) {

        //
        // Get the first entry of the list from the head.
        //
        Thunk = CONTAINING_RECORD(ModuleInfo->ThunkList.Flink, INJ_THUNK, Link);

        //
        // Check if this thunk was queued for execution yet.
        //
        if (Thunk->Queued) {

            //
            // Looks like this initializer thunk was queued for execution, so
            // check the thunk state and result status to determine if we are
            // done with it or not.
            //
            // If the thunk status is still in a waiting state, then the APC
            // has not executed yet, so keep waiting. The state will be updated
            // after execution of the thunk initializer.
            //
            if (INJ_THUNK_IS_WAITING(Thunk)) {

                //
                // We break here, because we cannot queue the next initializer
                // thunk until this one has completed.
                //
                Status = STATUS_MORE_PROCESSING_REQUIRED;
                break;
            }

            //BO_DBG_BREAK();

            //
            // Read the the thunk initializer result.
            //
            ThunkResult = *(INJ_THUNK_RESULT *)Thunk->Data;

            //
            // Check if the resulting status is a failure.
            //
            if (ThunkResult == INJ_THUNK_RESULT_FAILURE) {

                LOG_ERROR("Thunk %p execution resulted in a failure - aborting!", Thunk->Code);
                Status = STATUS_UNSUCCESSFUL;
                break;
            }

            LOG_INFO("Thunk %p executed successfully with result %d", Thunk->Code, ThunkResult);

            //
            // Looks like the initializer thunk executed without failure,
            // therefore we are remove it from the list head and
            // free its resources.
            //
            RemoveHeadList(&ModuleInfo->ThunkList);
            InjpFreeThunk(NtCurrentProcess(), Thunk);

            if (IsListEmpty(&ModuleInfo->ThunkList)) {
            
                //
                // If the thunk list is empty after removing the last head
                // element, then indicate that this module was injected so
                // we dont do any further processing for this module!
                //
                ModuleInfo->Injected = TRUE;
                break;
            }

            //
            // Continue onto handling the next thunk initializer in queue.
            //
            continue;
        }

        //
        // This next thunk has not bee queued yet, so lets queue it for
        // execution and check back later after it has executed.
        //
        Status = InjForceApc(PsGetCurrentThread(),
                             UserMode,
                             (PKNORMAL_ROUTINE)Thunk->Code,
                             Thunk->Data,
                             NULL,
                             NULL
                             );

        if (NT_SUCCESS(Status)) {

            //
            // Mark this thunk as queued for execution.
            //
            Thunk->Queued = TRUE;

        } else {
        
            //
            // We DO NOT fail here because we still have a chance to queue
            // this thunk initializer for execution the next time we are
            // notified.
            //
            LOG_ERROR("Failed to queue thunk with status %08x", Status);
        }

        //
        // Break here and check back later after this thunk initializer
        // has executed.
        //
        break;
    }

    return Status;
}

static
VOID
NTAPI
InjpLoadImageNotifyRoutine(
    IN PUNICODE_STRING FullImageName OPTIONAL,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
)
{
    NTSTATUS Status;
    PLIST_ENTRY Next;
    PINJ_MAPPING Mapping;
    ULONG Index;
    PINJ_INFO InjectInfo;
    PEPROCESS Process;
    UNICODE_STRING ImageName;
    //PVOID LdrLoadDllRoutine;
    //CONST UNICODE_STRING LdrLoadDllUnicode = RTL_CONSTANT_STRING(L"LdrLoadDll");

    PINJ_MODULE_INFO ModuleInfo;

    UNREFERENCED_PARAMETER(ImageInfo);

    //
    // Get the inject info for the current process.
    //
    InjectInfo = InjpFindInjectionInfoForProcessId(ProcessId);
    if (!InjectInfo) {
        return;
    }

    //
    // Get the current process object.
    //
    Process = IoGetCurrentProcess();

    //
    // Check if we are good to inject yet.
    //
    if (!InjpReadyToInject(Process, InjectInfo)) {
    
        //
        // This process is still in an early stage of initialization,
        // important DLLs (such as ntdll.dll - or wow64.dll in case of
        // WoW64 process) are not loaded yet. We can't inject our module
        // until we know they are initialized.
        //
        // Check if any of the system modules we're interested in is being
        // currently loaded. If so, mark that information down into the
        // LoadedMask field.
        //
        for (Index = 0; Index < RTL_NUMBER_OF(InjpSystemDlls); Index++) {
            if (UtlSuffixUnicodeString(&InjpSystemDlls[Index], FullImageName, TRUE)) {
            
                //if (Index == INJ_SYSWOW64_NTDLL_INDEX || Index == INJ_SYSTEM32_NTDLL_INDEX) {
                //
                //    //
                //    // Capture the address of the LdrLoadDll routine from the NTDLL
                //    // module (which is of the same architecture as the process).
                //    //
                //    if (NT_SUCCESS(LdrFindExportAddressForProcessUnicode(Process, 
                //                                                         ImageInfo->ImageBase,
                //                                                         &LdrLoadDllUnicode, 
                //                                                         &LdrLoadDllRoutine))) {
                //        InjectInfo->LdrLoadDllRoutine = LdrLoadDllRoutine;
                //    }
                //}

                //
                // Mark this dependency as loaded in the LoadedMask.
                //
                InjectInfo->LoadedMask |= (1ULL << Index);
                break;
            }
        }

    } else {

        //
        // All necessary modules are loaded. Perform the injections.
        //
        Next = InjpInjectionListHead.Flink;
        while (Next != &InjpInjectionListHead) {
            Mapping = CONTAINING_RECORD(Next, INJ_MAPPING, Link);

            if (Mapping->Source.Base) {

                RtlInitEmptyUnicodeString(&ImageName,
                                          Mapping->Source.Path, sizeof(Mapping->Source.Path));
                RtlInt64ToUnicodeString((ULONGLONG)Mapping->Source.Base, 16, &ImageName);
            
            } else {
            
                RtlInitUnicodeString(&ImageName, Mapping->Source.Path);
            }

            //
            // Lookup a matching element for the hook module name.
            //
            ModuleInfo = (PINJ_MODULE_INFO)RtlLookupElementGenericTableAvl(
                                                &InjectInfo->ModuleTable,
                                                (PVOID)ImageName.Buffer
                                                );
            if (ModuleInfo) {

                //
                // If a module info element exists then its already been
                // at least queued for injected, so let's check if we should
                // handle any of the thunks, and finalize the injection.
                // Injection is only totally complete if the Injected field
                // is set in the module info element.
                //

                if (!ModuleInfo->Injected) {

                    Status = InjpFinalizeInjection(InjectInfo, ModuleInfo);

                    if (!NT_SUCCESS(Status) && Status != STATUS_MORE_PROCESSING_REQUIRED) {

                        LOG_ERROR("Failed to check/finalize injection with status %08x", Status);

                        //
                        // Since the injection seems to have failed, destroy
                        // the remaining resources and remove the element from
                        // the generic AVL table.
                        //
                        InjpDestroyModuleInfoThunkList(NtCurrentProcess(), ModuleInfo);
                        RtlDeleteElementGenericTableAvl(&InjectInfo->ModuleTable, ModuleInfo);
                    }
                }

            } else {

                //
                // A module info element does not exist yet for the
                // corresponding injection mapping. So inject it!
                //

                Status = InjpInject(InjectInfo,
                                    Process,
                                    Mapping->Source.Base,
                                    &ImageName,
                                    &ModuleInfo
                                    );

                if (NT_SUCCESS(Status)) {
                
                    //
                    // Update the module name to the inputted image name.
                    //
                    RtlCopyMemory(ModuleInfo->ModuleName, ImageName.Buffer, ImageName.Length);

                    //
                    // Update the mapping module info element pointer.
                    //
                    Mapping->ModuleInfo = ModuleInfo;
                }
            }

            Next = Next->Flink;
        }
    }
}

static
VOID
NTAPI
InjpCreateProcessNotifyRoutineEx(
    IN OUT PEPROCESS Process,
    IN HANDLE ProcessId,
    IN OUT PPS_CREATE_NOTIFY_INFO CreateInfo OPTIONAL
)
{
    PINJ_MAPPING Mappings[INJ_MAXIMUM_MAPPINGS];
    ULONG MappingCount;
    INJ_INFO NewInjectInfo;
    PINJ_INFO InjectInfo;
    BOOLEAN NewElement;

    if (CreateInfo) {

        //
        // Check if this process is a target in any of the injection mappings.
        //
        MappingCount = InjpFindInjectionMappingsForProcessId(ProcessId,
                                                             Mappings,
                                                             RTL_NUMBER_OF(Mappings));
        if (!MappingCount) {
            return;
        }

        BO_DBG_BREAK();

        //
        // Setup a new inject info element and insert it into the generic AVL table.
        //
        NewInjectInfo.ProcessId = ProcessId;
        NewInjectInfo.LoadedMask = 0;
        NewInjectInfo.ForceUserApc = TRUE;
        InjectInfo = RtlInsertElementGenericTableAvl(&InjpInfoTable,
                                                     &NewInjectInfo,
                                                     sizeof(INJ_INFO),
                                                     &NewElement
                                                     );
        if (!InjectInfo) {

            //
            // If we can't allocate memory for the inject info, then just bugcheck.
            //
            KeBugCheckEx(MEMORY_MANAGEMENT,
                         (ULONG_PTR)Process,
                         (ULONG_PTR)ProcessId,
                         (ULONG_PTR)CreateInfo,
                         sizeof(INJ_INFO)
                         );
        }

        //
        // Make sure there is no duplicate element!
        //
        BO_ASSERT(NewElement == TRUE);

        //
        // Initialize the generic AVL table of injection modules.
        //
        // N.B. We *MUST* initialize the generic AVL table AFTER we insert the
        //      new element as the generic AVL table implementation simply
        //      allocates a new element and copies our given data over to the
        //      new element. This means if we initialized the table head before
        //      inserting the new element into the generic AVL table, the table
        //      field would be pointing to the old address of the element data
        //      we initialized on the stack!
        //
        RtlInitializeGenericTableAvl(&InjectInfo->ModuleTable,
                                     InjpModuleInfoCompareRoutine,
                                     InjpAllocateRoutine,
                                     InjpFreeRoutine,
                                     NULL
                                     );
    } else {

        //
        // Invalidate the mappings associated with the supplied process id
        // so that we can reinject if a new instance of the process starts
        // again.
        //
        InjpInvalidateInjectionMappingsForProcessId(ProcessId);

        //
        // If the process is being terminated, see if we cant find an already
        // existing inject info element for the process ID.
        //
        InjectInfo = InjpFindInjectionInfoForProcessId(ProcessId);
        if (InjectInfo) {

            BO_DBG_BREAK();
            InjpDestroyInjectionInfo(NtCurrentProcess(), InjectInfo);
            RtlDeleteElementGenericTableAvl(&InjpInfoTable, InjectInfo);
        }
    }
}

NTSTATUS
INJAPI
InjInitialize(
    VOID
)
{
    NTSTATUS Status;

    //
    // Initialize the injection mappings list.
    //
    InitializeListHead(&InjpInjectionListHead);
    KeInitializeSpinLock(&InjpInjectionListLock);

    //
    // Initialize the inject info AVL table.
    //
    RtlInitializeGenericTableAvl(&InjpInfoTable,
                                 InjpInjectionInfoCompareRoutine,
                                 InjpAllocateRoutine,
                                 InjpFreeRoutine,
                                 NULL
                                 );
    //
    // Register the create process notify routine for injection.
    //
    Status = PsSetCreateProcessNotifyRoutineEx(InjpCreateProcessNotifyRoutineEx, FALSE);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to set create process notify routine with status %08x", Status);
        return Status;
    }

    //
    // Register the load image notify routine for injection.
    //
    Status = PsSetLoadImageNotifyRoutine(InjpLoadImageNotifyRoutine);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to set load image notify routine with status %08x", Status);
        PsSetCreateProcessNotifyRoutineEx(InjpCreateProcessNotifyRoutineEx, TRUE);
    }

    return Status;
}

VOID
INJAPI
InjDestroy(
    VOID
)
{
    NTSTATUS Status;
    PVOID P;
    PEPROCESS Process;
    HANDLE ProcessHandle;
    PLIST_ENTRY Next;
    PINJ_INFO InjectInfo;
    PINJ_MAPPING Mapping;
    
    BO_DBG_BREAK();

    //
    // Remove the notify callback routines.
    //
    PsRemoveLoadImageNotifyRoutine(InjpLoadImageNotifyRoutine);
    PsSetCreateProcessNotifyRoutineEx(InjpCreateProcessNotifyRoutineEx, TRUE);

    //
    // Clear our all elements in the inject info AVL table.
    //
    for (P = RtlEnumerateGenericTableAvl(&InjpInfoTable, TRUE);
         P != NULL;
         P = RtlEnumerateGenericTableAvl(&InjpInfoTable, FALSE)) {

        InjectInfo = (PINJ_INFO)P;
        Process = NULL;

        //
        // Lookup the process object.
        //
        Status = PsLookupProcessByProcessId(InjectInfo->ProcessId, &Process);
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Corrupt inject info detected - failed to lookup process for"
                      "process id %d with status %08x",
                      (ULONG)(ULONG_PTR)InjectInfo->ProcessId, Status);
            goto doNext;
        }
        
        //
        // Open a handle to the process.
        //
        Status = ObOpenObjectByPointer(Process,
                                       OBJ_KERNEL_HANDLE,
                                       NULL,
                                       PROCESS_VM_OPERATION,
                                       *PsProcessType,
                                       KernelMode,
                                       &ProcessHandle
                                       );
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to open process handle with status %08x", Status);
            goto doNext;
        }

        //
        // Destroy the injection info.
        //
        InjpDestroyInjectionInfo(ProcessHandle, InjectInfo);

        //
        // Close the handle opened by ObOpenObjectByPointer.
        //
        PsCloseProcessHandle(ProcessHandle);

    doNext:
        //
        // Do not forget to decrement the reference count of the process object.
        //
        if (Process) {
            ObDereferenceObject(Process);
        }

        //
        // Delete the inject info element from the AVL table.
        //
        RtlDeleteElementGenericTableAvl(&InjpInfoTable, P);
    }

    //
    // Free all of the registered injection mappings.
    //
    Next = ExInterlockedRemoveHeadList(&InjpInjectionListHead, &InjpInjectionListLock);
    while(Next != NULL) {
        Mapping = CONTAINING_RECORD(Next, INJ_MAPPING, Link);
        InjpFree(Mapping);
        Next = ExInterlockedRemoveHeadList(&InjpInjectionListHead, &InjpInjectionListLock);
    }
}

NTSTATUS
INJAPI
InjRegisterInjection(
    OUT PHINJECTION Handle,
    IN PVOID SourceImageBase OPTIONAL,
    IN PWCHAR SourceImagePath,
    IN HANDLE TargetProcessId OPTIONAL,
    IN PWCHAR TargetProcessName
)
{
    SIZE_T Length;
    PINJ_MAPPING Mapping;
    
    Mapping = InjpAllocate(sizeof(INJ_MAPPING));
    if (!Mapping) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Mapping->Source.Base = SourceImageBase;
    if (SourceImagePath) {

        Length = wcslen(SourceImagePath) * sizeof(WCHAR);
        RtlCopyMemory(Mapping->Source.Path, SourceImagePath, Length);
    }
    
    Mapping->Target.ProcessId = TargetProcessId;
    if (TargetProcessName) {

        Length = wcslen(TargetProcessName) * sizeof(WCHAR);
        RtlCopyMemory(Mapping->Target.Name, TargetProcessName, Length);
    }

    ExInterlockedInsertTailList(&InjpInjectionListHead, &Mapping->Link, &InjpInjectionListLock);

    Mapping->ModuleInfo = NULL;

    *Handle = (HINJECTION)Mapping;
    return STATUS_SUCCESS;
}

