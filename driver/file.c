/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file file.c
 * @author Aidan Khoury (ajkhoury)
 * @date 9/4/2018
 */

#include "file.h"

#include "process.h"
#include "loader.h"
#include "util.h"

#define KERNEL_DOS_DEVICES_STRING_PREFIX    L"\\??\\"
CONST UNICODE_STRING FileDosDevicesPrefix = RTL_CONSTANT_STRING(KERNEL_DOS_DEVICES_STRING_PREFIX);

typedef
NTSTATUS
(NTAPI *PNT_UNMAP_VIEW_OF_SECTION)(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL
    );
static PNT_UNMAP_VIEW_OF_SECTION NtUnmapViewOfSection = NULL;

typedef
NTSTATUS
(NTAPI *PNT_FLUSH_VIRTUAL_MEMORY)(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    );
static PNT_FLUSH_VIRTUAL_MEMORY NtFlushVirtualMemory = NULL;

//
// Private Implementation
//

static
NTSTATUS
FilepFindNtUnmapViewOfSection(
    IN PVOID KernelBase,
    OUT PVOID *FoundUnmapViewOfSection
)
{
    NTSTATUS Status;
    PKSERVICE_TABLE_DESCRIPTOR SystemServiceDescriptorTable;
    PVOID Found;
    ULONG SsdtIndex;
    CONST UNICODE_STRING UnmapViewOfSectionUstr = RTL_CONSTANT_STRING(L"ZwUnmapViewOfSection");

    SystemServiceDescriptorTable = UtlGetServiceDescriptorTable(SYSTEM_SERVICE_INDEX);
    if (!SystemServiceDescriptorTable) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = LdrFindExportAddressUnicode(KernelBase, &UnmapViewOfSectionUstr, &Found);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    SsdtIndex = UtlGetServiceDescriptorTableEntryIndex(Found);

    if (SsdtIndex >= SystemServiceDescriptorTable->Limit) {
        return STATUS_ARRAY_BOUNDS_EXCEEDED;
    }

    *FoundUnmapViewOfSection = UtlGetServiceDescriptorTableEntry(
                                        SystemServiceDescriptorTable,
                                        SsdtIndex
                                        );
    if (!*FoundUnmapViewOfSection) {
        return STATUS_NOT_FOUND;
    }

    return Status;
}

static
NTSTATUS
FilepFindNtFlushVirtualMemory(
    IN PVOID KernelBase,
    OUT PVOID *FoundFlushVirtualMemory
)
{
    NTSTATUS Status;
    PKSERVICE_TABLE_DESCRIPTOR SystemServiceDescriptorTable;
    PVOID Found;
    ULONG SsdtIndex;
    CONST UNICODE_STRING FlushVirtualMemoryUstr = RTL_CONSTANT_STRING(L"ZwFlushVirtualMemory");

    SystemServiceDescriptorTable = UtlGetServiceDescriptorTable(SYSTEM_SERVICE_INDEX);
    if (!SystemServiceDescriptorTable) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = LdrFindExportAddressUnicode(KernelBase, &FlushVirtualMemoryUstr, &Found);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    SsdtIndex = UtlGetServiceDescriptorTableEntryIndex(Found);

    if (SsdtIndex >= SystemServiceDescriptorTable->Limit) {
        return STATUS_ARRAY_BOUNDS_EXCEEDED;
    }

    *FoundFlushVirtualMemory = UtlGetServiceDescriptorTableEntry(
                                        SystemServiceDescriptorTable,
                                        SsdtIndex
                                        );
    if (!*FoundFlushVirtualMemory) {
        return STATUS_NOT_FOUND;
    }

    return Status;
}


//
// Public Implementation
//

NTSTATUS
FILEAPI
FileInitialize(
    IN PVOID KernelBase
)
{
    NTSTATUS Status;

    //
    // Find the unexported NtUnmapViewOfSection routine.
    //
    Status = FilepFindNtUnmapViewOfSection(KernelBase, (PVOID*)&NtUnmapViewOfSection);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Find the unexported NtFlushVirtualMemory routine.
    //
    Status = FilepFindNtFlushVirtualMemory(KernelBase, (PVOID*)&NtFlushVirtualMemory);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    return Status;
}

BOOLEAN
FILEAPI
FileExists(
    IN PUNICODE_STRING FilePath
)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE FileHandle;
    KPROCESSOR_MODE PreviousMode;

    if (!FilePath) {
        return FALSE;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               FilePath,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtCreateFile(&FileHandle,
                          FILE_READ_DATA | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatus,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                          );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (NT_SUCCESS(Status)) {

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
        NtClose(FileHandle);
        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

        return TRUE;
    }

    return FALSE;
}

NTSTATUS
FILEAPI
FileCreate(
    IN PUNICODE_STRING FilePath,
    IN ULONG DesiredAccess,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    OUT PHANDLE Handle
)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    UNICODE_STRING FilePathUnicode;
    WCHAR FilePathBuffer[MAXIMUM_FILENAME_LENGTH + sizeof(KERNEL_DOS_DEVICES_STRING_PREFIX)];
    HANDLE FileHandle;
    KPROCESSOR_MODE PreviousMode;

    if (!ARGUMENT_PRESENT(Handle)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (FilePath->Buffer[0] != L'\\' && FilePath->Buffer[0] != L'/') {

        RtlInitEmptyUnicodeString(&FilePathUnicode, FilePathBuffer, sizeof(FilePathBuffer));
        RtlUnicodeStringCopy(&FilePathUnicode, &FileDosDevicesPrefix);
        RtlUnicodeStringCat(&FilePathUnicode, FilePath);
        InitializeObjectAttributes(&ObjectAttributes,
                                   &FilePathUnicode,
                                   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );
    } else {

        InitializeObjectAttributes(&ObjectAttributes,
                                   FilePath,
                                   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );
    }

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtCreateFile(&FileHandle,
                          DesiredAccess,
                          &ObjectAttributes,
                          &IoStatus,
                          NULL,
                          FileAttributes,
                          ShareAccess,
                          CreateDisposition,
                          CreateOptions,
                          NULL,
                          0
                          );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        *Handle = NULL;
        return Status;
    }

    *Handle = FileHandle;
    return Status;
}

NTSTATUS
FILEAPI
FileOpen(
    IN PUNICODE_STRING FilePath,
    IN ULONG DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions,
    OUT PHANDLE Handle
)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    UNICODE_STRING FilePathUnicode;
    WCHAR FilePathBuffer[MAXIMUM_FILENAME_LENGTH + sizeof(KERNEL_DOS_DEVICES_STRING_PREFIX)];
    HANDLE FileHandle;
    KPROCESSOR_MODE PreviousMode;

    if (!ARGUMENT_PRESENT(Handle) || !FilePath || FilePath->Buffer[0] == L'\0') {
        return STATUS_INVALID_PARAMETER;
    }

    if (FilePath->Buffer[0] != L'\\' && FilePath->Buffer[0] != L'/') {

        RtlInitEmptyUnicodeString(&FilePathUnicode, FilePathBuffer, sizeof(FilePathBuffer));
        RtlUnicodeStringCopy(&FilePathUnicode, &FileDosDevicesPrefix);
        RtlUnicodeStringCat(&FilePathUnicode, FilePath);
        InitializeObjectAttributes(&ObjectAttributes,
                                   &FilePathUnicode,
                                   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );
    } else {

        InitializeObjectAttributes(&ObjectAttributes,
                                   FilePath,
                                   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );
    }

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtOpenFile(&FileHandle,
                        DesiredAccess,
                        &ObjectAttributes,
                        &IoStatus,
                        ShareAccess,
                        OpenOptions
                        );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        *Handle = NULL;
        return Status;
    }

    *Handle = FileHandle;
    return Status;
}

NTSTATUS
FILEAPI
FileClose(
    IN HANDLE FileHandle
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
    Status = NtClose(FileHandle);
    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    return Status;
}

NTSTATUS
FILEAPI
FileRead(
    IN HANDLE FileHandle,
    IN ULONG Offset,
    OUT PUCHAR Buffer,
    IN ULONG Size,
    OUT PULONG ReturnSize
)
{
    NTSTATUS Status;
    LARGE_INTEGER ByteOffset;
    IO_STATUS_BLOCK IoStatus;
    KPROCESSOR_MODE PreviousMode;

    if (!ARGUMENT_PRESENT(FileHandle) ||
        !ARGUMENT_PRESENT(Buffer) ||
        !ARGUMENT_PRESENT(ReturnSize)) {
        return STATUS_INVALID_PARAMETER;
    }

    ByteOffset.LowPart = Offset;
    ByteOffset.HighPart = 0;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtReadFile(FileHandle,
                        NULL, NULL, NULL,
                        &IoStatus,
                        Buffer,
                        Size,
                        &ByteOffset,
                        NULL
                        );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        *ReturnSize = 0;
        return Status;
    }

    *ReturnSize = (ULONG)IoStatus.Information;

    return IoStatus.Status;
}

NTSTATUS
FILEAPI
FileGetSize(
    IN HANDLE FileHandle,
    OUT PLARGE_INTEGER FileSize
)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileInformation;
    KPROCESSOR_MODE PreviousMode;

    if (!ARGUMENT_PRESENT(FileSize)) {
        return STATUS_INVALID_PARAMETER;
    }

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtQueryInformationFile(FileHandle,
                                    &IoStatus,
                                    &FileInformation,
                                    sizeof(FileInformation),
                                    FileStandardInformation
                                    );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *FileSize = FileInformation.EndOfFile;

    return Status;
}


static
NTSTATUS
BaseGetNamedObjectDirectory(
    OUT PHANDLE RootDirectory
)
{
    if (!ARGUMENT_PRESENT(RootDirectory)) {
        return STATUS_INVALID_PARAMETER;
    }

    *RootDirectory = NULL;
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
FILEAPI
FileCreateMapping(
    IN HANDLE FileHandle OPTIONAL,
    IN PSECURITY_ATTRIBUTES SecurityAttributes OPTIONAL,
    IN ULONG ProtectionFlags,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN const WCHAR *Name OPTIONAL,
    OUT PHANDLE SectionMapping
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    HANDLE SectionHandle;
    OBJECT_ATTRIBUTES LocalAttributes;
    POBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ObjectName;
    PUNICODE_STRING ObjectNamePtr;
    ACCESS_MASK DesiredAccess;
    PLARGE_INTEGER SectionSize = NULL;
    ULONG AllocationAttributes;

    PVOID SecurityDescriptor;
    ULONG Attributes;
    HANDLE RootDirectory;

    //
    // Validate SectionMapping parameter is present.
    //
    if (!ARGUMENT_PRESENT(SectionMapping)) {
        return STATUS_INVALID_PARAMETER;
    } else {
        *SectionMapping = NULL;
    }

    //
    // Set the default desired access.
    //
    DesiredAccess = STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ; // 0xF0005

    //
    // Get the attributes for the actual allocation and cleanup Protection.
    //
    AllocationAttributes = ProtectionFlags & (SEC_LARGE_PAGES |
                                              SEC_WRITECOMBINE |
                                              SEC_NOCACHE |
                                              SEC_FILE |
                                              SEC_IMAGE |
                                              SEC_RESERVE |
                                              SEC_COMMIT);
    ProtectionFlags ^= AllocationAttributes;
    if (AllocationAttributes == 0) {
        AllocationAttributes = SEC_COMMIT;
    }

    //
    // Set the desired access based on the given protection flags.
    //
    switch (ProtectionFlags) {
    case PAGE_READWRITE:
        DesiredAccess |= SECTION_MAP_WRITE; // 0xF0007
        break;
    case PAGE_EXECUTE_READWRITE:
        DesiredAccess |= (SECTION_MAP_WRITE | SECTION_MAP_EXECUTE); // 0xF000F
        break;
    case PAGE_EXECUTE_READ:
    case PAGE_EXECUTE_WRITECOPY:
        DesiredAccess |= SECTION_MAP_EXECUTE; // 0xF000D
        break;
    case PAGE_READONLY:
    case PAGE_WRITECOPY:
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Set up object name if present.
    //
    if (ARGUMENT_PRESENT(Name)) {
        RtlInitUnicodeString(&ObjectName, Name);
        ObjectNamePtr = &ObjectName;
    } else {
        ObjectNamePtr = NULL;
    }

    //
    // Format the object attributes based on the given parameters.
    //
    if (ARGUMENT_PRESENT(SecurityAttributes)) {

        SecurityDescriptor = SecurityAttributes->lpSecurityDescriptor;
        Attributes = SecurityAttributes->bInheritHandle != 0 ? OBJ_INHERIT : 0;

    } else {

        if (!ObjectNamePtr) {
            ObjectAttributes = NULL;
            goto Next;
        }

        Attributes = 0;
        SecurityDescriptor = NULL;
    }

    if (!ObjectNamePtr) {

        RootDirectory = NULL;
        ObjectAttributes = &LocalAttributes;
        InitializeObjectAttributes(ObjectAttributes,
                                   ObjectNamePtr,
                                   Attributes,
                                   RootDirectory,
                                   SecurityDescriptor);
        goto Next;
    }

    Status = BaseGetNamedObjectDirectory(&RootDirectory);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Attributes |= OBJ_OPENIF;
    ObjectAttributes = &LocalAttributes;
    InitializeObjectAttributes(ObjectAttributes,
                               ObjectNamePtr,
                               Attributes,
                               RootDirectory,
                               SecurityDescriptor);

Next:

    //
    // Set the section size if present. 
    //
    if (ARGUMENT_PRESENT(MaximumSize)) {
        SectionSize = MaximumSize;
    }

    //
    // Validate the file handle.
    //
    if (FileHandle == ((HANDLE)(LONG_PTR)-1)) {
        FileHandle = NULL;
        if (!SectionSize) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    //
    // Create the actual section mapping.
    //
    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtCreateSection(&SectionHandle,
                             DesiredAccess,
                             ObjectAttributes,
                             SectionSize,
                             ProtectionFlags,
                             AllocationAttributes,
                             FileHandle
                             );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        return Status;
    } else if (Status == STATUS_OBJECT_NAME_EXISTS) {
        return STATUS_OBJECT_NAME_COLLISION;
    }

    //
    // Return the section.
    //
    *SectionMapping = SectionHandle;

    return Status;
}

NTSTATUS
FILEAPI
FileMapView(
    IN HANDLE FileMappingHandle,
    IN ULONG DesiredAccess,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN SIZE_T NumberOfBytesToMap,
    IN PVOID BaseAddress OPTIONAL,
    OUT PVOID *ViewBaseAddress
)
{
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    PVOID ViewBase;
    ULONG AllocationType;
    ULONG Protect;
    KPROCESSOR_MODE PreviousMode;

    if (!ARGUMENT_PRESENT(ViewBaseAddress)) {
        return STATUS_INVALID_PARAMETER;
    }

    *ViewBaseAddress = NULL;

    if (ARGUMENT_PRESENT(FileOffset)) {
        SectionOffset = *FileOffset;
    } else {
        SectionOffset.QuadPart = 0;
    }

    ViewSize = NumberOfBytesToMap;
    ViewBase = BaseAddress;

    Protect = 0;
    AllocationType = 0;

    if ((DesiredAccess & FILE_MAP_RESERVE) != 0) {
        DesiredAccess &= ~FILE_MAP_RESERVE;
        AllocationType = MEM_RESERVE;
    }

    if (DesiredAccess & FILE_MAP_TARGETS_INVALID) {
        DesiredAccess &= ~FILE_MAP_TARGETS_INVALID;
        Protect = PAGE_TARGETS_INVALID;
    }

    if (DesiredAccess == (FILE_MAP_EXECUTE | FILE_MAP_COPY)) {
        Protect |= PAGE_EXECUTE_WRITECOPY;
    } else if (DesiredAccess == FILE_MAP_COPY) {
        Protect |= PAGE_WRITECOPY;
    } else if (DesiredAccess & FILE_MAP_WRITE) {
        if (DesiredAccess & FILE_MAP_EXECUTE)
            Protect |= PAGE_EXECUTE_READWRITE;
        else
            Protect |= PAGE_READWRITE;
    } else if (DesiredAccess & FILE_MAP_READ) {
        if (DesiredAccess & FILE_MAP_EXECUTE)
            Protect |= PAGE_EXECUTE_READ;
        else
            Protect |= PAGE_READONLY;
    } else {
        Protect |= PAGE_NOACCESS;
    }

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtMapViewOfSection(FileMappingHandle,
                                NtCurrentProcess(),
                                &ViewBase,
                                0,
                                0,
                                &SectionOffset,
                                &ViewSize,
                                ViewShare,
                                AllocationType,
                                Protect
                                );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *ViewBaseAddress = ViewBase;
    return Status;
}

NTSTATUS
FILEAPI
FileFlushView(
    IN PVOID ViewBaseAddress,
    IN SIZE_T ViewSize
)
{
    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T NumberOfBytesToFlush;
    IO_STATUS_BLOCK IoStatus;

    BaseAddress = ViewBaseAddress;
    NumberOfBytesToFlush = ViewSize;

    if (NtFlushVirtualMemory) {

        KPROCESSOR_MODE PreviousMode;

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

        Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                      &BaseAddress,
                                      &NumberOfBytesToFlush,
                                      &IoStatus
                                      );

        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    } else {

        Status = ZwFlushVirtualMemory(NtCurrentProcess(),
                                      &BaseAddress,
                                      &NumberOfBytesToFlush,
                                      &IoStatus
                                      );
    }

    if (Status == STATUS_NOT_MAPPED_DATA) {
        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
FILEAPI
FileUnmapView(
    IN PVOID ViewBaseAddress
)
{
    NTSTATUS Status;

    if (NtUnmapViewOfSection) {

        KPROCESSOR_MODE PreviousMode;

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

        Status = NtUnmapViewOfSection(NtCurrentProcess(), ViewBaseAddress);

        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    } else {

        Status = ZwUnmapViewOfSection(NtCurrentProcess(), ViewBaseAddress);
    }

    return Status;
}

NTSTATUS
FILEAPI
FileMap(
    IN HANDLE FileHandle,
    OUT PMAPPED_IMAGE MappedImage,
    IN BOOLEAN ReadOnly
)
{
    static ULONG SystemAllocationGranularity = 0;

    NTSTATUS Status;
    PLOADED_IMAGE LoadedImage;
    SYSTEM_BASIC_INFORMATION BasicInformation;
    ULONG NumberOfBytesToMap;
    HANDLE FileMapping;
    PVOID MappedAddress;
    LARGE_INTEGER FileSize;

    //
    // Validate parameters
    //
    if (!FileHandle || !ARGUMENT_PRESENT(MappedImage)) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Initialize the MappedImage and LoadedImage pointers.
    //
    LoadedImage = MappedImage->LoadedImage;
    RtlZeroMemory(LoadedImage, sizeof(LOADED_IMAGE));
    RtlZeroMemory(MappedImage, sizeof(MAPPED_IMAGE));
    MappedImage->LoadedImage = LoadedImage;
    LoadedImage->Version = 1;
    LoadedImage->fReadOnly = ReadOnly;

    //
    // Get the allocation granularity.
    //
    if (!SystemAllocationGranularity) {

        KPROCESSOR_MODE PreviousMode;

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

        Status = NtQuerySystemInformation(SystemBasicInformation,
                                          &BasicInformation,
                                          sizeof(SYSTEM_BASIC_INFORMATION),
                                          NULL
                                          );

        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        SystemAllocationGranularity = BasicInformation.AllocationGranularity;
    }

    MappedImage->AllocationGranularity = SystemAllocationGranularity;

    //
    // Get the given file's size.
    //
    Status = FileGetSize(FileHandle, &FileSize);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    LoadedImage->SizeOfImage = FileSize.LowPart;

    NumberOfBytesToMap = 0xA00000;
    if (FileSize.LowPart < 0xA00000)
        NumberOfBytesToMap = FileSize.LowPart;

    MappedImage->NumberOfBytesMapped = NumberOfBytesToMap;

    //
    // Create a file mapping object.
    //
    Status = FileCreateMapping(FileHandle,
                               NULL,
                               ReadOnly ? PAGE_READONLY : PAGE_READWRITE,
                               NULL,
                               NULL,
                               &FileMapping
                               );

    MappedImage->FileMapping = FileMapping;

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Map view of the file with the specified number of bytes.
    //
    Status = FileMapView(MappedImage->FileMapping,
                         ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE,
                         NULL,
                         MappedImage->NumberOfBytesMapped,
                         NULL,
                         &MappedAddress
                         );

    LoadedImage->MappedAddress = (PUCHAR)MappedAddress;

    if (!NT_SUCCESS(Status)) {
        FileClose(MappedImage->FileMapping);
        return Status;
    }

    //
    // Everything is mapped. Now check the image and find nt image headers
    //
    Status = FileCalculateImagePtrs(MappedImage, MappedImage->NumberOfBytesMapped);
    if (!NT_SUCCESS(Status)) {
        FileClose(FileMapping);
        return Status;
    }

    LoadedImage->hFile = FileHandle;

    return STATUS_SUCCESS;
}

NTSTATUS
FILEAPI
FileCalculateImagePtrs(
    IN PMAPPED_IMAGE MappedImage,
    IN ULONG NumberOfBytesMapped
)
{
    NTSTATUS Status;
    PLOADED_IMAGE LoadedImage;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_FILE_HEADER FileHeader;
    ULONG NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;
    PUCHAR SectionsEnd, End;

    Status = STATUS_SUCCESS;  // Assume the best

    LoadedImage = MappedImage->LoadedImage;

    if (!NumberOfBytesMapped)
        NumberOfBytesMapped = LoadedImage->SizeOfImage;

    DosHeader = (PIMAGE_DOS_HEADER)LoadedImage->MappedAddress;

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE &&
        DosHeader->e_magic != IMAGE_NT_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_NOT_MZ;
        goto tryout;
    }

    if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {

        if (DosHeader->e_lfanew == 0) {
            LoadedImage->fDOSImage = TRUE;
            Status = STATUS_INVALID_IMAGE_WIN_16;
            goto tryout;
        }

        if ((ULONG)DosHeader->e_lfanew > NumberOfBytesMapped) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto tryout;
        }

        if (NumberOfBytesMapped - DosHeader->e_lfanew < 0x108) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto tryout;
        }

        LoadedImage->FileHeader = (PVOID)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);

        if (
            // If IMAGE_NT_HEADERS would extend past the end of file...
            (PUCHAR)LoadedImage->FileHeader + sizeof(IMAGE_NT_HEADERS) >
            (PUCHAR)LoadedImage->MappedAddress + NumberOfBytesMapped ||

            // ..or if it would begin in, or before the IMAGE_DOS_HEADER...
            (PUCHAR)LoadedImage->FileHeader <
            (PUCHAR)LoadedImage->MappedAddress + sizeof(IMAGE_DOS_HEADER)) {
            // ...then e_lfanew is not as expected.
            // (Several Win95 files are in this category.)
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto tryout;
        }

    } else {

        // No DOS header indicates an image built w/o a dos stub
        LoadedImage->FileHeader = (PVOID)((ULONG_PTR)DosHeader);
    }

    NtHeaders = LoadedImage->FileHeader;

    if ((PUCHAR)NtHeaders - (PUCHAR)DosHeader + 0x108 > NumberOfBytesMapped) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto tryout;
    }

    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        if ((USHORT)NtHeaders->Signature == (USHORT)IMAGE_OS2_SIGNATURE ||
            (USHORT)NtHeaders->Signature == (USHORT)IMAGE_OS2_SIGNATURE_LE) {
            LoadedImage->fDOSImage = TRUE;
        }

        Status = STATUS_INVALID_IMAGE_LE_FORMAT;
        goto tryout;
    } else {
        LoadedImage->fDOSImage = FALSE;
    }

    FileHeader = &NtHeaders->FileHeader;

    //
    // No optional header indicates an object.
    //
    if (FileHeader->SizeOfOptionalHeader == 0) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto tryout;
    }

    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

        //
        // 32-bit image. Do some tests.
        //
        if (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase >= 0x80000000) {
            LoadedImage->fSystemImage = TRUE;
        } else {
            LoadedImage->fSystemImage = FALSE;
        }

        if (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.MajorLinkerVersion < 3 &&
            ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.MinorLinkerVersion < 5) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto tryout;
        }

    } else {
        LoadedImage->fSystemImage = FALSE;
    }

    Sections = IMAGE_FIRST_SECTION(NtHeaders);
    NumberOfSections = FileHeader->NumberOfSections;

    LoadedImage->Sections = Sections;
    InitializeListHead(&LoadedImage->Links);
    LoadedImage->Characteristics = FileHeader->Characteristics;
    LoadedImage->NumberOfSections = NumberOfSections;
    LoadedImage->LastRvaSection = LoadedImage->Sections;

    //
    // Make sure the sections are valid.
    //
    SectionsEnd = (PUCHAR)&LoadedImage->Sections[LoadedImage->NumberOfSections];
    End = (PUCHAR)-1;
    if (SectionsEnd >= (PUCHAR)LoadedImage->Sections)
        End = (PUCHAR)SectionsEnd;
    if (SectionsEnd < (PUCHAR)LoadedImage->Sections || End >(PUCHAR)DosHeader + NumberOfBytesMapped) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        //goto tryout;
    }

tryout:
    if (!NT_SUCCESS(Status)) {
        FileUnmapView(LoadedImage->MappedAddress);
        return Status;
    }

    return Status;
}

NTSTATUS
FILEAPI
FileUnmap(
    IN PMAPPED_IMAGE MappedImage
)
{
    PLOADED_IMAGE LoadedImage;

    if (!MappedImage->FileMapping) {
        return STATUS_INVALID_PARAMETER;
    }

    LoadedImage = MappedImage->LoadedImage;

    if (MappedImage->ViewBase) {
        FileUnmapView(MappedImage->ViewBase);
    }

    if (LoadedImage->MappedAddress) {
        FileUnmapView(LoadedImage->MappedAddress);
    }

    FileClose(MappedImage->FileMapping);

    return STATUS_SUCCESS;
}

NTSTATUS
FILEAPI
FileSeek(
    IN HANDLE FileHandle,
    IN PLARGE_INTEGER DistanceToSeek,
    IN ULONG SeekMethod
)
{
    NTSTATUS Status;
    FILE_STANDARD_INFORMATION StandardInfo;
    LARGE_INTEGER PositionInfo;
    LARGE_INTEGER DistanceToMove;
    LARGE_INTEGER NewPosition;
    IO_STATUS_BLOCK IoStatusBlock;
    KPROCESSOR_MODE PreviousMode;

    if (ARGUMENT_PRESENT(DistanceToSeek)) {
        PositionInfo = *DistanceToSeek;
        DistanceToMove = PositionInfo;
    } else {
        PositionInfo.QuadPart = 0;
        DistanceToMove.QuadPart = 0;
    }

    if (SeekMethod == FILE_BEGIN) {

        NewPosition = DistanceToMove;
        PositionInfo = DistanceToMove;

    } else if (SeekMethod == FILE_CURRENT) {

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

        Status = NtQueryInformationFile(FileHandle,
                                        &IoStatusBlock,
                                        &PositionInfo,
                                        sizeof(LARGE_INTEGER),
                                        FilePositionInformation
                                        );

        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        NewPosition.QuadPart = DistanceToMove.QuadPart + PositionInfo.QuadPart;
        PositionInfo.QuadPart += DistanceToMove.QuadPart;

    } else if (SeekMethod == FILE_END) {

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

        Status = NtQueryInformationFile(FileHandle,
                                        &IoStatusBlock,
                                        &StandardInfo,
                                        sizeof(FILE_STANDARD_INFORMATION),
                                        FileStandardInformation
                                        );

        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        NewPosition.QuadPart = DistanceToMove.QuadPart + StandardInfo.EndOfFile.QuadPart;
        PositionInfo.QuadPart = DistanceToMove.QuadPart + StandardInfo.EndOfFile.QuadPart;

    } else {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Cannot seek to a negative position.
    //
    if (NewPosition.QuadPart < 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ARGUMENT_PRESENT(DistanceToSeek) &&
        DistanceToSeek->HighPart == 0 &&
        PositionInfo.HighPart & 0x7FFFFFFF) {
        return STATUS_INVALID_PARAMETER;
    }

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtSetInformationFile(FileHandle,
                                  &IoStatusBlock,
                                  &PositionInfo,
                                  sizeof(LARGE_INTEGER),
                                  FilePositionInformation
                                  );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        if (ARGUMENT_PRESENT(DistanceToSeek)) {
            DistanceToSeek->QuadPart = (LONGLONG)-1;
        }
        return Status;
    }

    if (ARGUMENT_PRESENT(DistanceToSeek)) {
        *DistanceToSeek = PositionInfo;
    }

    return Status;
}

NTSTATUS
FILEAPI
FileSetEnd(
    IN HANDLE FileHandle
)
{
    NTSTATUS Status;
    LARGE_INTEGER FileInformation;
    IO_STATUS_BLOCK IoStatusBlock;
    KPROCESSOR_MODE PreviousMode;

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

    Status = NtQueryInformationFile(FileHandle,
                                    &IoStatusBlock,
                                    &FileInformation,
                                    sizeof(LARGE_INTEGER),
                                    FilePositionInformation
                                    );

    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (NT_SUCCESS(Status)) {

        PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

        Status = NtSetInformationFile(FileHandle,
                                      &IoStatusBlock,
                                      &FileInformation,
                                      sizeof(LARGE_INTEGER),
                                      FileEndOfFileInformation
                                      );

        PS_PREVIOUS_MODE_RESTORE(PreviousMode);

        if (NT_SUCCESS(Status)) {

            PS_PREVIOUS_MODE_KERNEL(&PreviousMode);

            Status = NtSetInformationFile(FileHandle,
                                          &IoStatusBlock,
                                          &FileInformation,
                                          sizeof(LARGE_INTEGER),
                                          FileAllocationInformation
                                          );

            PS_PREVIOUS_MODE_RESTORE(PreviousMode);

        }
    }

    return Status;
}