/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file loader.h
 * @author Aidan Khoury (ajkhoury)
 * @date 10/15/2018
 */

#include "loader.h"
#include "apiset.h"

#include "image.h"
#include "file.h"
#include "process.h"
#include "util.h"
#include "mm.h"
#include "codegen.h"

#include "driver.h"
#include "log.h"

#include <ntstrsafe.h>

//
// System images names.
//
#define LDRP_KERNEL_NAME     L"ntoskrnl.exe"
#define LDRP_HAL_NAME        L"hal.dll"
#define LDRP_NETIO_NAME      L"netio.sys"

//
// Mark a HIGHADJ entry as needing an increment if reprocessing.
//
#define LDRP_RELOCATION_INCREMENT   0x1

//
// Mark a HIGHADJ entry as not suitable for reprocessing.
//
#define LDRP_RELOCATION_FINAL       0x2

//
// DLL Main Routine
//
typedef
BOOLEAN
(NTAPI *PDLL_INIT_ROUTINE)(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    );

//
// The default '.DLL' extension.
//
CONST UNICODE_STRING LdrpApiDefaultExtension = RTL_CONSTANT_STRING(L".DLL");

CONST UNICODE_STRING LdrpSystemRootName = RTL_CONSTANT_STRING(L"\\SystemRoot");

CONST ULONGLONG LdrpApiSetPrefixAPI_ = (ULONGLONG)0x2D004900500041; // L"api-"
CONST ULONGLONG LdrpApiSetPrefixEXT_ = (ULONGLONG)0x2D005400580045; // L"ext-"

//
// LdrpLoadDll path info structures.
//

typedef struct _LDRP_DLL_PATH {
    WCHAR *DllSearchPath;                   // 0x00
    PVOID Reserved0;                        // 0x08
    PVOID Reserved1;                        // 0x10
    ULONG ImplicitPathFlags;                // 0x18
    WCHAR *DllName;                         // 0x20
    PVOID Reserved2[10];                    // 0x28
    ULONG Reserved3;                        // 0x78
    BOOLEAN IsSearchPathAllocated;          // 0x7C
} LDRP_DLL_PATH, *PLDRP_DLL_PATH;           // sizeof 0x80

typedef struct _LDRP_DLL_PATH32 {
    ULONG DllSearchPath;                    // 0x00 WCHAR*
    ULONG Reserved0;                        // 0x04
    ULONG Reserved1;                        // 0x08
    ULONG ImplicitPathFlags;                // 0x0C
    ULONG DllName;                          // 0x10 WCHAR*
    ULONG Reserved2[15];                    // 0x14
} LDRP_DLL_PATH32, *PLDRP_DLL_PATH32;       // sizeof 0x50

typedef struct _LDRP_DLL_PATH_WIN81 {
    WCHAR *DllSearchPath;                   // 0x00
    PVOID Reserved0;                        // 0x08
    ULONG ImplicitPathFlags;                // 0x10
    WCHAR *DllName;                         // 0x18
    PVOID Reserved1;                        // 0x20
} LDRP_DLL_PATH_WIN81, *PLDRP_DLL_PATH_WIN81; // sizeof 0x28

typedef struct _LDRP_DLL_PATH32_WIN81 {
    ULONG DllSearchPath;                    // 0x00 WCHAR*
    ULONG Reserved0;                        // 0x04 PVOID
    ULONG ImplicitPathFlags;                // 0x08
    ULONG DllName;                          // 0x0C WCHAR*
    ULONG Reserved1;                        // 0x10 PVOID
} LDRP_DLL_PATH32_WIN81, *PLDRP_DLL_PATH32_WIN81; // sizeof 0x14

typedef struct _LDRP_DLL_PATH_WIN8 {
    WCHAR *DllSearchPath;                   // 0x00
    WCHAR *DllName;                         // 0x08
    BOOLEAN ImplicitPath;                   // 0x10
    BOOLEAN ExternalSearchPath;             // 0x11
} LDRP_DLL_PATH_WIN8, *PLDRP_DLL_PATH_WIN8; // sizeof 0x18

typedef struct _LDRP_DLL_PATH32_WIN8 {
    ULONG DllSearchPath;                    // 0x00
    ULONG DllName;                          // 0x04
    BOOLEAN ImplicitPath;                   // 0x08
    BOOLEAN ExternalSearchPath;             // 0x09
} LDRP_DLL_PATH32_WIN8, *PLDRP_DLL_PATH32_WIN8; // sizeof 0x0C


//
// Forward Declarations
//

typedef
NTSTATUS
(NTAPI *PUSER_SHELLCODE_START_ROUTINE)(
    PVOID Parameter1,
    PVOID Parameter2,
    PVOID Parameter3
    );
static
NTSTATUS
LdrpExecuteShellcode(
    IN PEPROCESS Process,
    IN BOOLEAN NewThread,
    IN BOOLEAN Wait,
    IN PUSER_SHELLCODE_START_ROUTINE StartRoutine,
    IN PVOID Parameter1 OPTIONAL,
    IN PVOID Parameter2 OPTIONAL,
    IN PVOID Parameter3 OPTIONAL
    );


//
// Private Implementation
//

#define LDRP_IS_POWER_OF_TWO(x) \
    (((x) != 0) && (((x) & ((x)-1)) == 0))

FORCEINLINE
SIZE_T
LdrpAlignToSectionAlignment(
    IN SIZE_T Value,
    IN UINT32 SectionAlignment,
    IN UINT32 FileAlignment
)
{
    //
    // 512 is the minimum valid FileAlignment according to the documentation
    // although ntoskrnl.exe has an alignment of 0x80 in some Windows versions.
    //
    if (SectionAlignment < PAGE_SIZE) // page size
        SectionAlignment = FileAlignment;
    if (SectionAlignment && (Value % ((SIZE_T)SectionAlignment))) {
        return ((SIZE_T)SectionAlignment) * (Value / (SIZE_T)(SectionAlignment));
    } else {
        return Value;
    }
}

FORCEINLINE
SIZE_T
LdrpAlignToFileAlignment(
    IN SIZE_T Value,
    IN UINT32 FileAlignment
)
{
    //
    // The alignment factor (in bytes) that is used to align the raw data of 
    // sections in the image file.  The value should be a power of 2 between 
    // 512 and 64 K, inclusive.  The default is 512. If the SectionAlignment 
    // is less than the architecture's page size, then FileAlignment must 
    // match SectionAlignment.
    //
    if (FileAlignment > 512) {

        //
        // Make sure it's a power of two!
        //
        BO_ASSERT(LDRP_IS_POWER_OF_TWO(FileAlignment));
    }

    if (!FileAlignment)
        FileAlignment = 512;
    if (FileAlignment >= 512) {
        return (Value / ((SIZE_T)FileAlignment)) * ((SIZE_T)FileAlignment);
    } else {
        return Value;
    }
}

static
BOOLEAN
LdrpIsSectionValid(
    IN PIMAGE_SECTION_HEADER Section,
    IN ULONG SectionAlignment,
    IN ULONG FileAlignment,
    IN SIZE_T ImageSize OPTIONAL
)
{
    SIZE_T PointerToRawDataAligned;
    SIZE_T VirtualAddressAligned;

    //
    // Check for corrupt fields.
    //
    if (!Section->Misc.VirtualSize && !Section->SizeOfRawData) {
        return FALSE;
    }

    //
    // Skip sections with invalid characteristics.
    //
    if (!(Section->Characteristics & (IMAGE_SCN_MEM_READ |
                                      IMAGE_SCN_MEM_WRITE |
                                      IMAGE_SCN_MEM_EXECUTE))) {
        return FALSE;
    }

    //
    // Make sure the size of raw data is a valid value.
    //
    if (Section->SizeOfRawData > ImageSize) {
        return FALSE;
    }

    //
    // Validate the aligned pointer to raw data.
    //
    PointerToRawDataAligned = LdrpAlignToFileAlignment((SIZE_T)Section->PointerToRawData,
                                                       FileAlignment);
    if (PointerToRawDataAligned > ImageSize) {
        return FALSE;
    }

    //
    // Validate the aligned virtual address.
    //
    VirtualAddressAligned = LdrpAlignToSectionAlignment((SIZE_T)Section->VirtualAddress,
                                                        SectionAlignment,
                                                        FileAlignment
                                                        );
    if (VirtualAddressAligned >= 0x10000000) {
        return FALSE;
    }

    //
    // Check that the pointer to raw data is valid.
    //
    if (!Section->PointerToRawData) {
        return FALSE;
    }

    //
    // Looks good!
    //
    return TRUE;
}

static
ULONG
LdrpGetSectionProtectionForCharacteristics(
    IN ULONG Characteristics
)
{
    ULONG Protect = 0;

    if (Characteristics & IMAGE_SCN_MEM_NOT_CACHED) {
        Protect |= PAGE_NOCACHE;
    }
    if (Characteristics & IMAGE_SCN_MEM_EXECUTE) {
        if (Characteristics & IMAGE_SCN_MEM_READ) {
            if (Characteristics & IMAGE_SCN_MEM_WRITE) {
                Protect |= PAGE_EXECUTE_READWRITE;
            } else {
                Protect |= PAGE_EXECUTE_READ;
            }
        } else if (Characteristics & IMAGE_SCN_MEM_WRITE) {
            Protect |= PAGE_EXECUTE_WRITECOPY;
        } else {
            Protect |= PAGE_EXECUTE;
        }
    } else if (Characteristics & IMAGE_SCN_MEM_READ) {
        if (Characteristics & IMAGE_SCN_MEM_WRITE) {
            Protect |= PAGE_READWRITE;
        } else {
            Protect |= PAGE_READONLY;
        }
    } else if (Characteristics & IMAGE_SCN_MEM_WRITE) {
        Protect |= PAGE_WRITECOPY;
    } else {
        Protect |= PAGE_NOACCESS;
    }

    return Protect;
}

static
PIMAGE_BASE_RELOCATION
LdrpProcessRelocationBlockLongLong(
    IN PVOID VA,
    IN ULONG SizeOfBlock,
    IN USHORT* NextOffset,
    IN LONGLONG Diff
)
{
    PUCHAR FixupVA;
    USHORT Offset;
    LONG Temp;

    while (SizeOfBlock--) {

        Offset = *NextOffset & (USHORT)0x0FFF;
        FixupVA = (PUCHAR)VA + Offset;

        //
        // Apply the fixups.
        //

        switch ((*NextOffset) >> 12) {
        case IMAGE_REL_BASED_HIGHLOW:
            //
            // HighLow - (32-bits) relocate the high and low half
            //      of an address.
            //
            *(LONG UNALIGNED *)FixupVA += (LONG)Diff;
            break;

        case IMAGE_REL_BASED_HIGH:
            //
            // High - (16-bits) relocate the high half of an address.
            //
            Temp = *(USHORT*)FixupVA << 16;
            Temp += (ULONG)Diff;
            *(USHORT*)FixupVA = (USHORT)(Temp >> 16);
            break;

        case IMAGE_REL_BASED_HIGHADJ:
            //
            // Adjust high - (16-bits) relocate the high half of an
            //      address and adjust for sign extension of low half.
            //

            //
            // If the address has already been relocated then don't
            // process it again now or information will be lost.
            //
            if (Offset & LDRP_RELOCATION_FINAL) {
                ++NextOffset;
                --SizeOfBlock;
                break;
            }

            Temp = *(USHORT*)FixupVA << 16;
            ++NextOffset;
            --SizeOfBlock;
            Temp += (LONG)(*(SHORT*)NextOffset);
            Temp += (ULONG)Diff;
            Temp += 0x8000;
            *(USHORT*)FixupVA = (USHORT)(Temp >> 16);

            break;

        case IMAGE_REL_BASED_LOW:
            //
            // Low - (16-bit) relocate the low half of an address.
            //
            Temp = *(SHORT*)FixupVA;
            Temp += (ULONG)Diff;
            *(USHORT*)FixupVA = (USHORT)Temp;
            break;

        case IMAGE_REL_BASED_DIR64:
            //
            // Dir64 - relocate a 64-bit address.
            //
            *(ULONGLONG UNALIGNED*)FixupVA += Diff;
            break;

        case IMAGE_REL_BASED_ABSOLUTE:
            //
            // Absolute - no fixup required.
            //
            break;

        case IMAGE_REL_BASED_RESERVED:
            //
            // Used to be a Section Relative reloc. Ignore.
            //
            break;

        default:
            //
            // Illegal - illegal relocation type.
            //
            return (PIMAGE_BASE_RELOCATION)NULL;
        }

        ++NextOffset;
    }

    return (PIMAGE_BASE_RELOCATION)NextOffset;
}



static
NTSTATUS
LdrpResolveApiSet(
    IN PEPROCESS Process,
    IN PCUNICODE_STRING Name,
    IN PCUNICODE_STRING BaseName,
    OUT PUNICODE_STRING ResolvedName
)
{
    NTSTATUS Status;
    PPEB Peb;
    BOOLEAN WasResolved;
    PAPI_SET_NAMESPACE ApiSetMap;

    Peb = PsGetProcessWow64Process(Process);
    if (Peb) {

        ApiSetMap = (PAPI_SET_NAMESPACE)((PPEB32)Peb)->ApiSetMap;

    } else {

        Peb = PsGetProcessPeb(Process);
        if (!Peb) {
            return STATUS_UNSUCCESSFUL;
        }

        ApiSetMap = (PAPI_SET_NAMESPACE)((PPEB64)Peb)->ApiSetMap;
    }

    if (!ApiSetMap) {
        return STATUS_NOT_FOUND;
    }

    //LOG_DEBUG("ApiSet Schema Version %u", ApiSetMap->Version);

    switch (ApiSetMap->Version) {
        //
        // API set schema version 2
        //
    case API_SET_SCHEMA_VERSION_V2:
        Status = ApiSetResolveToHostV2(ApiSetMap, Name, BaseName, &WasResolved, ResolvedName);
        if (NT_SUCCESS(Status) && !WasResolved) {
            Status = STATUS_NOT_FOUND;
        }
        break;

        //
        // API set schema version 3
        //
    case API_SET_SCHEMA_VERSION_V3:
        Status = ApiSetResolveToHostV3(ApiSetMap, Name, BaseName, &WasResolved, ResolvedName);
        if (NT_SUCCESS(Status) && !WasResolved) {
            Status = STATUS_NOT_FOUND;
        }
        break;

        //
        // API set schema version 4
        //
    case API_SET_SCHEMA_VERSION_V4:
        Status = ApiSetResolveToHostV4(ApiSetMap, Name, BaseName, &WasResolved, ResolvedName);
        if (NT_SUCCESS(Status) && !WasResolved) {
            Status = STATUS_NOT_FOUND;
        }
        break;

        //
        // API set schema version 6
        //
    case API_SET_SCHEMA_VERSION_V6:
        Status = ApiSetResolveToHostV6(ApiSetMap, Name, BaseName, &WasResolved, ResolvedName);
        if (NT_SUCCESS(Status) && !WasResolved) {
            Status = STATUS_NOT_FOUND;
        }
        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    return Status;
}

static
PVOID
LdrpGetModuleBaseAddress(
    IN PLIST_ENTRY ModuleList,
    IN PCUNICODE_STRING ModuleName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo OPTIONAL
)
{
    PVOID ModuleBase = NULL;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;

    //
    // Search for the specified module.
    //
    NextEntry = ModuleList->Flink;
    while (NextEntry != ModuleList) {
        DataTableEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (RtlEqualUnicodeString(ModuleName, &DataTableEntry->BaseImageName, TRUE)) {
            ModuleBase = DataTableEntry->ImageBase;
            if (ModuleBase != NULL) {

                if (ARGUMENT_PRESENT(ImageInfo)) {
                    ImageInfo->ImageBase = DataTableEntry->ImageBase;
                    ImageInfo->EntryPoint = DataTableEntry->EntryPoint;
                    ImageInfo->SizeOfImage = DataTableEntry->SizeOfImage; 
                    ImageInfo->Flags = DataTableEntry->Flags;

                    ImageInfo->ImageName.Length = DataTableEntry->BaseImageName.Length;
                    ImageInfo->ImageName.MaximumLength = sizeof(ImageInfo->ImageNameBuffer);
                    ImageInfo->ImageName.Buffer = ImageInfo->ImageNameBuffer;
                    RtlCopyMemory(ImageInfo->ImageName.Buffer,
                                  DataTableEntry->BaseImageName.Buffer,
                                  DataTableEntry->BaseImageName.Length);
                }

                break;
            }
        }

        NextEntry = NextEntry->Flink;
    }

    return ModuleBase;
}

static
PVOID
LdrpGetModuleBaseAddressWoW64(
    IN PLIST_ENTRY32 ModuleList,
    IN PCUNICODE_STRING ModuleName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo OPTIONAL
)
{
    PVOID ModuleBase = NULL;
    UNICODE_STRING BaseImageName;
    PKLDR_DATA_TABLE_ENTRY32 DataTableEntry;
    PLIST_ENTRY32 NextEntry;

    //
    // Search for the specified module.
    //
    NextEntry = (PLIST_ENTRY32)ModuleList->Flink;
    while (NextEntry != ModuleList) {
        DataTableEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

        BaseImageName.Length = DataTableEntry->BaseImageName.Length;
        BaseImageName.MaximumLength = DataTableEntry->BaseImageName.MaximumLength;
        BaseImageName.Buffer = (PWCH)DataTableEntry->BaseImageName.Buffer;
        if (RtlEqualUnicodeString(ModuleName, &BaseImageName, TRUE)) {

            ModuleBase = (PVOID)DataTableEntry->ImageBase;
            if (ModuleBase != NULL) {

                if (ARGUMENT_PRESENT(ImageInfo)) {
                    ImageInfo->ImageBase = (PVOID)DataTableEntry->ImageBase;
                    ImageInfo->EntryPoint = (PVOID)DataTableEntry->EntryPoint;
                    ImageInfo->SizeOfImage = DataTableEntry->SizeOfImage;
                    ImageInfo->Flags = DataTableEntry->Flags;

                    ImageInfo->ImageName.Length = DataTableEntry->BaseImageName.Length;
                    ImageInfo->ImageName.MaximumLength = sizeof(ImageInfo->ImageNameBuffer);
                    ImageInfo->ImageName.Buffer = ImageInfo->ImageNameBuffer;
                    RtlCopyMemory(ImageInfo->ImageName.Buffer,
                                 (PWCHAR)DataTableEntry->BaseImageName.Buffer,
                                  DataTableEntry->BaseImageName.Length);
                }

                break;
            }
        }

        NextEntry = (PLIST_ENTRY32)NextEntry->Flink;
    }

    return ModuleBase;
}

static
NTSTATUS
LdrpGetModuleName(
    IN PLIST_ENTRY ModuleList,
    IN PVOID ImageBase,
    OUT PUNICODE_STRING ModuleName
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;

    //
    // Search for the specified module.
    //
    NextEntry = ModuleList->Flink;
    while (NextEntry != ModuleList) {
        DataTableEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (ImageBase == DataTableEntry->ImageBase) {
            ModuleName->Length = DataTableEntry->BaseImageName.Length;
            ModuleName->MaximumLength = DataTableEntry->BaseImageName.MaximumLength;
            ModuleName->Buffer = DataTableEntry->BaseImageName.Buffer;
            return STATUS_SUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

    return STATUS_NOT_FOUND;
}

static
NTSTATUS
LdrpGetModuleNameWoW64(
    IN PLIST_ENTRY32 ModuleList,
    IN PVOID ImageBase,
    OUT PUNICODE_STRING ModuleName
)
{
    PKLDR_DATA_TABLE_ENTRY32 DataTableEntry;
    PLIST_ENTRY32 NextEntry;

    //
    // Search for the specified module.
    //
    NextEntry = (PLIST_ENTRY32)ModuleList->Flink;
    while (NextEntry != ModuleList) {
        DataTableEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

        if (ImageBase == (PVOID)DataTableEntry->ImageBase) {
            ModuleName->Length = DataTableEntry->BaseImageName.Length;
            ModuleName->MaximumLength = DataTableEntry->BaseImageName.MaximumLength;
            ModuleName->Buffer = (PWCH)DataTableEntry->BaseImageName.Buffer;
            return STATUS_SUCCESS;
        }

        NextEntry = (PLIST_ENTRY32)NextEntry->Flink;
    }

    return STATUS_NOT_FOUND;
}


//
// Public Implementation
//

NTSTATUS
LDRAPI
LdrResolveImagePath(
    IN PEPROCESS Process,
    IN PUNICODE_STRING Path,
    IN PUNICODE_STRING BaseImagePath,
    OUT PUNICODE_STRING Resolved
)
{
    NTSTATUS Status;
    UNICODE_STRING PathLowUstr;
    UNICODE_STRING DirectoryUstr;
    UNICODE_STRING FilenameUstr;
    UNICODE_STRING ResolvedUstr;
    UNICODE_STRING FullResolvedUstr;
    WCHAR ResolvedBuffer[MAXIMUM_FILENAME_LENGTH];
    WCHAR FullResolvedBuffer[MAXIMUM_FILENAME_LENGTH];

    if (!Process || !Path || !Resolved) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = RtlDowncaseUnicodeString(&PathLowUstr, Path, TRUE);
    if (!NT_SUCCESS(Status)) {
        BO_DBG_BREAK();
        return Status;
    }

    //
    // Strip the directory (if it exists), to extract the file name.
    //
    UtlExtractFileName(&PathLowUstr, &FilenameUstr);

    //
    // Resolve via API Schema.
    //
    Status = LdrpResolveApiSet(Process, &FilenameUstr, BaseImagePath, &ResolvedUstr);
    if (NT_SUCCESS(Status)) {

        RtlInitEmptyUnicodeString(&FullResolvedUstr, FullResolvedBuffer, sizeof(FullResolvedBuffer));
        RtlUnicodeStringCopyString(&FullResolvedUstr, PsGetProcessWow64Process(Process) ?
                                   L"\\SystemRoot\\SysWOW64\\" :
                                   L"\\SystemRoot\\System32\\");
        RtlUnicodeStringCat(&FullResolvedUstr, &ResolvedUstr);

        Status = STATUS_SUCCESS;
        goto Exit;
    }

    RtlInitEmptyUnicodeString(&ResolvedUstr, ResolvedBuffer, sizeof(ResolvedBuffer));

    //
    // Resolve via process executable directory.
    //
    Status = PsGetProcessFullImageName(Process,
                                       ResolvedUstr.Buffer,
                                       ResolvedUstr.MaximumLength,
                                       &ResolvedUstr.Length
                                       );
    if (NT_SUCCESS(Status)) {

        UtlExtractDirectory(&ResolvedUstr, &DirectoryUstr);

        RtlInitEmptyUnicodeString(&FullResolvedUstr, FullResolvedBuffer, sizeof(FullResolvedBuffer));
        RtlCopyUnicodeString(&FullResolvedUstr, &DirectoryUstr);
        RtlUnicodeStringCat(&FullResolvedUstr, &FilenameUstr);

        if (FileExists(&FullResolvedUstr)) {
            Status = STATUS_SUCCESS;
            goto Exit;
        }
    }

    //
    // Resolve via system directory.
    //
    RtlInitEmptyUnicodeString(&FullResolvedUstr, FullResolvedBuffer, sizeof(FullResolvedBuffer));
    RtlUnicodeStringCopyString(&FullResolvedUstr, PsGetProcessWow64Process(Process) ?
                               L"\\SystemRoot\\SysWOW64\\" :
                               L"\\SystemRoot\\System32\\");
    RtlUnicodeStringCat(&FullResolvedUstr, &FilenameUstr);
    if (FileExists(&FullResolvedUstr)) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    ////
    //// Resolve via downlevel directory.
    ////
    //RtlInitEmptyUnicodeString(&FullResolvedUstr, FullResolvedBuffer, sizeof(FullResolvedBuffer));
    //RtlUnicodeStringCopyString(&FullResolvedUstr, PsGetProcessWow64Process(Process) ?
    //                                               L"\\SystemRoot\\SysWOW64\\downlevel" :
    //                                               L"\\SystemRoot\\System32\\downlevel");
    //RtlUnicodeStringCat(&FullResolvedUstr, &FilenameUstr);
    //if (FileExists(&FullResolvedUstr)){
    //    BO_DBG_BREAK();
    //
    //    Status = LdrpResolveApiSet(Process, &FilenameUstr, BaseImagePath, &ResolvedUstr);
    //
    //    Status = STATUS_SUCCESS;
    //    goto Exit;
    //}

    FullResolvedUstr.Length = 0;
    RtlZeroMemory(FullResolvedUstr.Buffer, sizeof(FullResolvedUstr.MaximumLength));
    Status = STATUS_NOT_FOUND;

Exit:
    RtlFreeUnicodeString(&PathLowUstr);

    //
    // Make sure our Resolved buffer can hold the fully resolved path.
    //
    BO_ASSERT(Resolved->MaximumLength >= FullResolvedUstr.Length);

    Resolved->Length = FullResolvedUstr.Length;
    RtlCopyMemory(Resolved->Buffer, FullResolvedUstr.Buffer, FullResolvedUstr.Length);
    Resolved->Buffer[Resolved->Length / sizeof(WCHAR)] = L'\0';

    return Status;
}

NTSTATUS
LDRAPI
LdrFindExportAddressForProcessAscii(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    IN CONST CHAR *Name,
    IN ULONG Ordinal,
    OUT PVOID *ResultExportAddress
)
{
    NTSTATUS Status;
    USHORT OrdinalNumber;
    ULONG *NameTableBase;
    USHORT *NameOrdinalTableBase;
    ULONG *FunctionTableBase;
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;
    ULONG ExportSize;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

    CHAR *ForwardImportDescriptor;
    CHAR *ForwardExportName;
    CHAR *ForwardModuleName;
    UNICODE_STRING ForwardModuleUstr;
    WCHAR ForwardModuleNameUnicode[MAXIMUM_FILENAME_LENGTH];
    ULONG ForwardModuleNameLength;
    ULONG BytesInUnicodeString;
    UNICODE_STRING ResolvedUstr;
    UNICODE_STRING ResolvedNameUstr;
    WCHAR ResolvedBuffer[MAXIMUM_FILENAME_LENGTH];
    PVOID ForwardModuleBase;
    ULONG ForwardExportOrdinal;
    UNICODE_STRING BaseModuleName;
    PUNICODE_STRING BaseModuleNamePtr;

    PVOID ExportAddress;

    if (!ImageBase || (!Name && !Ordinal) ||
        !ARGUMENT_PRESENT(ResultExportAddress)) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Get export directory address.
    //
    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                                                    ImageBase,
                                                    TRUE,
                                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                    &ExportSize
                                                    );
    if (!ExportDirectory) {

        //
        // Looks like no export directory is present!
        //
        return STATUS_NOT_FOUND;
    }

    //
    // Initialize default ExportAddress to NULL.
    //
    ExportAddress = NULL;

    //
    // Initialize the pointer to the array of RVA-based function pointers.
    //
    FunctionTableBase = (ULONG*)((ULONG_PTR)ImageBase + ExportDirectory->AddressOfFunctions);

    //
    // Check if it's an ordinal.
    //
    if (!Name) {

        if (Ordinal > 0) {

            //
            // Don't need to worry about odd behavior here because ordinals cannot be 
            // forwarded (as far as I know, and according to Microsoft's documentation).
            //
            ExportAddress = (PVOID)((ULONG_PTR)ImageBase + FunctionTableBase[Ordinal - 1]);
            Status = STATUS_SUCCESS;

        } else {

            //
            // Bad ordinal parameter!
            //
            Status = STATUS_INVALID_PARAMETER;
        }

        goto Exit;
    }

    //
    // Initialize the pointer to the array of RVA-based ansi export strings.
    //
    NameTableBase = (ULONG*)((ULONG_PTR)ImageBase +
                             ExportDirectory->AddressOfNames);

    //
    // Initialize the pointer to the array of USHORT ordinal numbers.
    //
    NameOrdinalTableBase = (USHORT*)((ULONG_PTR)ImageBase +
                                     ExportDirectory->AddressOfNameOrdinals);

    //
    // Lookup the desired name in the name table using a binary search.
    //
    Low = 0;
    Middle = 0;
    High = ExportDirectory->NumberOfNames - 1;

    while (High >= Low) {
        //
        // Compute the next probe index and compare the import name
        // with the export name entry.
        //
        Middle = (Low + High) >> 1;

        Result = strcmp(Name, (CHAR*)ImageBase + NameTableBase[Middle]);

        if (Result < 0) {
            High = Middle - 1;
        } else if (Result > 0) {
            Low = Middle + 1;
        } else {
            break;
        }
    }

    //
    // If the high index is less than the low index, then a matching
    // table entry was not found. Otherwise, get the ordinal number
    // from the ordinal table.
    //
    if (High < Low) {
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto Exit;
    }

    OrdinalNumber = NameOrdinalTableBase[Middle];

    //
    // If the OrdinalNumber is not within the Export Address Table,
    // then this image does not implement the function.  Return not found.
    //
    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto Exit;
    }

    //
    // Index into the array of RVA export addresses by ordinal number.
    //
    ExportAddress = (PVOID)((ULONG_PTR)ImageBase + FunctionTableBase[OrdinalNumber]);

    //
    // A forward export is inside of the IMAGE_DIRECTORY_ENTRY_EXPORT table.
    //
    // NOTE: Forwarders are not used by the kernel and HAL to each other.
    //
    if ((ExportAddress <= (PVOID)ExportDirectory) ||
        (ExportAddress >= (PVOID)((ULONG_PTR)ExportDirectory + ExportSize))) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    //
    // Looks like a forwarded export. Resolve the forward import descriptor.
    //
    ForwardImportDescriptor = (CHAR*)ExportAddress;
    ForwardExportName = strchr(ForwardImportDescriptor, '.');

    //
    // Make sure the forward export name is valid.
    //
    if (!ForwardExportName) {
        BO_DBG_BREAK();
        Status = STATUS_PROCEDURE_NOT_FOUND;
        goto Exit;
    }

    //
    // Get forward module name length.
    //
    ForwardModuleNameLength = (ULONG)(ForwardExportName - ForwardImportDescriptor);

    //
    // Move ahead one character to get export name.
    //
    ++ForwardExportName;

    //
    // Set the forward module name pointer.
    //
    ForwardModuleName = ForwardImportDescriptor;

    //
    // Create unicode string structure for forward module name.
    //
    Status = RtlMultiByteToUnicodeSize(&BytesInUnicodeString,
                                       ForwardModuleName,
                                       ForwardModuleNameLength
                                       );
    if (!NT_SUCCESS(Status)) {
        BO_DBG_BREAK();
        goto Exit;
    }

    ForwardModuleUstr.Length = (USHORT)BytesInUnicodeString;
    ForwardModuleUstr.MaximumLength = sizeof(ForwardModuleNameUnicode);
    ForwardModuleUstr.Buffer = ForwardModuleNameUnicode;
    Status = RtlMultiByteToUnicodeN(ForwardModuleUstr.Buffer,
                                    ForwardModuleUstr.MaximumLength,
                                    NULL,
                                    ForwardModuleName,
                                    ForwardModuleNameLength
                                    );
    if (!NT_SUCCESS(Status)) {
        BO_DBG_BREAK();
        goto Exit;
    }

    //
    // Append the '.DLL' extension to the end of the module name.
    //
    Status = RtlAppendUnicodeStringToString(&ForwardModuleUstr, &LdrpApiDefaultExtension);
    if (!NT_SUCCESS(Status)) {
        BO_DBG_BREAK();
        goto Exit;
    }

    //
    // Resolve the forwarded module
    //
    Status = LdrGetModuleNameUnicodeString(Process, ImageBase, &BaseModuleName);
    if (NT_SUCCESS(Status)) {
        BaseModuleNamePtr = &BaseModuleName;
    } else {
        BaseModuleNamePtr = NULL;
    }

    RtlInitEmptyUnicodeString(&ResolvedUstr, ResolvedBuffer, sizeof(ResolvedBuffer));
    Status = LdrResolveImagePath(Process, &ForwardModuleUstr, BaseModuleNamePtr, &ResolvedUstr);
    if (!NT_SUCCESS(Status)) {
        BO_DBG_BREAK();
        goto Exit;
    }

    //
    // Strip the path from the resolved name.
    //
    UtlExtractFileName(&ResolvedUstr, &ResolvedNameUstr);

    //
    // Get the module base address.
    //
    ForwardModuleBase = LdrGetModuleBaseAddressUnicodeString(Process, &ResolvedNameUstr, NULL);
    if (!ForwardModuleBase) {
        Status = STATUS_NOT_FOUND;
        goto Exit;
    }

    //
    // Determine if we should search for the forwarded export by ordinal index value
    // or by name.
    //
    if (strchr(ForwardExportName, '#')) {

        ForwardExportOrdinal = (ULONG)UtlAsciiStrToInteger(ForwardExportName + 1);
        ForwardExportName = NULL;

    } else {

        ForwardExportOrdinal = 0;
    }

    //
    // Recurse to find the forwarded export address.
    //
    Status = LdrFindExportAddressForProcessAscii(Process,
                                                 ForwardModuleBase,
                                                 ForwardExportName,
                                                 ForwardExportOrdinal,
                                                 &ExportAddress
                                                 );
Exit:
    *ResultExportAddress = ExportAddress;

    return Status;
}

NTSTATUS
LDRAPI
LdrFindExportAddressForProcessUnicode(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    IN PCUNICODE_STRING Name,
    OUT PVOID *ResultExportAddress
)
{
    NTSTATUS Status;
    ANSI_STRING ExportAnsi;
    UINT32 Ordinal;
    CHAR *ExportName;
    CHAR ExportNameBuffer[512]; // DANGER: Insanely lengthy export symbols will be truncated!

    //
    // Check if we're trying to find the export via ordinal.
    //
    if ((ULONG_PTR)Name <= 0xFFFF) {

        Ordinal = (UINT32)((ULONG_PTR)Name & 0xFFFF);
        ExportName = NULL;

    } else {

        RtlZeroMemory(ExportNameBuffer, sizeof(ExportNameBuffer));
        ExportAnsi.Buffer = ExportNameBuffer;
        ExportAnsi.Length = 0;
        ExportAnsi.MaximumLength = sizeof(ExportNameBuffer);

        Status = RtlUnicodeStringToAnsiString(&ExportAnsi, Name, FALSE);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        ExportName = ExportNameBuffer;
        Ordinal = 0;
    }

    return LdrFindExportAddressForProcessAscii(Process, ImageBase, ExportName, Ordinal, ResultExportAddress);
}

NTSTATUS
LDRAPI
LdrFindExportAddressAscii(
    IN PVOID ImageBase,
    IN CONST CHAR *Name,
    IN ULONG Ordinal,
    OUT PVOID *ResultExportAddress
)
{
    NTSTATUS Status;
    USHORT OrdinalNumber;
    ULONG* NameTableBase;
    USHORT* NameOrdinalTableBase;
    ULONG* FunctionTableBase;
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;
    ULONG ExportSize;
    PVOID ExportAddress;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

    if (!ARGUMENT_PRESENT(ResultExportAddress)) {
        return STATUS_INVALID_PARAMETER;
    }

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                                                    ImageBase,
                                                    TRUE,
                                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                    &ExportSize
                                                    );
    if (!ExportDirectory) {

        //
        // Looks like no export directory is present!
        //
        return STATUS_NOT_FOUND;
    }

    //
    // Initialize the pointer to the array of RVA-based ansi export strings.
    //
    NameTableBase = (ULONG*)((ULONG_PTR)ImageBase + ExportDirectory->AddressOfNames);

    //
    // Initialize the pointer to the array of USHORT ordinal numbers.
    //
    NameOrdinalTableBase = (USHORT*)((ULONG_PTR)ImageBase + ExportDirectory->AddressOfNameOrdinals);

    //
    // Initialize the pointer to the array of RVA-based function pointers.
    //
    FunctionTableBase = (ULONG*)((ULONG_PTR)ImageBase + ExportDirectory->AddressOfFunctions);

    //
    // Check if it's an ordinal.
    //
    if (!Name) {

        if (Ordinal > 0) {

            //
            // Don't need to worry about odd behavior here because ordinals cannot be 
            // forwarded (as far as I know, and according to Microsoft's documentation).
            //
            ExportAddress = (PVOID)((ULONG_PTR)ImageBase + FunctionTableBase[Ordinal - 1]);
            Status = STATUS_SUCCESS;
            goto Exit;
        }

        //
        // Bad ordinal parameter!
        //
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Lookup the desired name in the name table using a binary search.
    //
    Low = 0;
    Middle = 0;
    High = ExportDirectory->NumberOfNames - 1;

    while (High >= Low) {

        //
        // Compute the next probe index and compare the import name
        // with the export name entry.
        //
        Middle = (Low + High) >> 1;

        Result = strcmp(Name, (CHAR*)ImageBase + NameTableBase[Middle]);

        if (Result < 0) {
            High = Middle - 1;
        } else if (Result > 0) {
            Low = Middle + 1;
        } else {
            break;
        }
    }

    //
    // If the high index is less than the low index, then a matching
    // table entry was not found. Otherwise, get the ordinal number
    // from the ordinal table.
    //
    if (High < Low) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    OrdinalNumber = NameOrdinalTableBase[Middle];

    //
    // If the OrdinalNumber is not within the Export Address Table,
    // then this image does not implement the function.  Return not found.
    //
    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    //
    // Index into the array of RVA export addresses by ordinal number.
    //
    ExportAddress = (PVOID)((ULONG_PTR)ImageBase + FunctionTableBase[OrdinalNumber]);

    //
    // A forward export is inside of the IMAGE_DIRECTORY_ENTRY_EXPORT table.
    //
    // NOTE: Forwarders are not used by the kernel and HAL to each other.
    //
    if ((ExportAddress >= (PVOID)ExportDirectory) &&
        (ExportAddress <= (PVOID)((ULONG_PTR)ExportDirectory + ExportSize))) {
        return STATUS_NOT_FOUND;
    }

    // TODO: May need to resolve forwarded exports in a different routine!
Exit:
    *ResultExportAddress = ExportAddress;

    return STATUS_SUCCESS;
}

NTSTATUS
LDRAPI
LdrFindExportAddressUnicode(
    IN PVOID ImageBase,
    IN PCUNICODE_STRING Name,
    OUT PVOID *ResultExportAddress
)
{
    NTSTATUS Status;
    ANSI_STRING ExportAnsi;
    UINT32 Ordinal;
    CHAR *ExportName;
    CHAR ExportNameBuffer[512]; // DANGER: Insanely lengthy export symbols will be truncated!

    //
    // Check if we're trying to find the export via ordinal.
    //
    if ((ULONG_PTR)Name <= 0xFFFF) {

        Ordinal = (UINT32)((ULONG_PTR)Name & 0xFFFF);
        ExportName = NULL;

    } else {

        RtlZeroMemory(ExportNameBuffer, sizeof(ExportNameBuffer));
        ExportAnsi.Buffer = ExportNameBuffer;
        ExportAnsi.Length = 0;
        ExportAnsi.MaximumLength = sizeof(ExportNameBuffer);

        Status = RtlUnicodeStringToAnsiString(&ExportAnsi, Name, FALSE);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        ExportName = ExportNameBuffer;
        Ordinal = 0;
    }

    return LdrFindExportAddressAscii(ImageBase, ExportName, Ordinal, ResultExportAddress);
}

PVOID
LDRAPI
LdrGetSystemRoutineAddress(
    IN PLIST_ENTRY ModuleList,
    IN PCUNICODE_STRING SystemRoutineName
)
{
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    ANSI_STRING AnsiString;
    ULONG BytesInMultiByteString;
    PVOID FunctionAddress;
    BOOLEAN Found;
    UINT32 EntriesChecked;
    CHAR AnsiStringBuffer[MAXIMUM_FILENAME_LENGTH];

    CONST UNICODE_STRING KernelString = RTL_CONSTANT_STRING(LDRP_KERNEL_NAME);
    CONST UNICODE_STRING HalString = RTL_CONSTANT_STRING(LDRP_HAL_NAME);
    //CONST UNICODE_STRING NetioString = RTL_CONSTANT_STRING(LDRP_NETIO_NAME);

    EntriesChecked = 0;
    FunctionAddress = NULL;

    AnsiString.Buffer = AnsiStringBuffer;
    AnsiString.Length = (UINT16)RtlxUnicodeStringToAnsiSize(SystemRoutineName);
    AnsiString.MaximumLength = sizeof(AnsiStringBuffer);

    do {
        Status = RtlUnicodeToMultiByteN(AnsiString.Buffer,
                                        AnsiString.MaximumLength,
                                        &BytesInMultiByteString,
                                        SystemRoutineName->Buffer,
                                        SystemRoutineName->Length
                                        );
        if (NT_SUCCESS(Status)) {
            AnsiString.Buffer[BytesInMultiByteString / sizeof(CHAR)] = '\0';
            break;
        }

        YieldProcessor();
    } while (TRUE);

    //
    // Check only the kernel and the HAL for exports.
    //
    NextEntry = ModuleList->Flink;
    while (NextEntry != ModuleList) {
        Found = FALSE;

        DataTableEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (RtlEqualUnicodeString(&KernelString, &DataTableEntry->BaseImageName, TRUE)) {
            Found = TRUE;
            EntriesChecked += 1;
        } else if (RtlEqualUnicodeString(&HalString, &DataTableEntry->BaseImageName, TRUE)) {
            Found = TRUE;
            EntriesChecked += 1;
        }
        //else if (RtlEqualUnicodeString( &NetioString, &DataTableEntry->BaseImageName, TRUE )) {
        //    Found = TRUE;
        //    EntriesChecked += 1;
        //}

        if (Found) {
            Status = LdrFindExportAddressAscii(DataTableEntry->ImageBase,
                                               AnsiString.Buffer,
                                               0,
                                               &FunctionAddress
                                               );
            if (NT_SUCCESS(Status)) {
                break;
            }

            if (EntriesChecked == 2) {
                break;
            }
        }

        NextEntry = NextEntry->Flink;
    }

    return FunctionAddress;
}

NTSTATUS
LDRAPI
LdrGetModuleNameUnicodeString(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    OUT PUNICODE_STRING ModuleName
)
{
    PLIST_ENTRY ModuleList;
    LARGE_INTEGER WaitTime;
    PVOID Peb;
    ULONG Index;

    //
    // Check if this is a WOW64 process.
    //
    Peb = PsGetProcessWow64Process(Process);
    if (Peb) {

        //
        // Wait for the loader if it's not setup yet.
        //
        if (!((PPEB32)Peb)->Ldr) {
            WaitTime.QuadPart = RELATIVE(MILLISECONDS(250)); // 250 msec.

            Index = 0;
            while (++Index <= 10 && !((PPEB32)Peb)->Ldr) {
                KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
            }

            //
            // Still no loader, fail.
            //
            if (Index > 10 && !((PPEB32)Peb)->Ldr) {
                return STATUS_UNSUCCESSFUL;
            }
        }

        ModuleList = (PLIST_ENTRY)&((PPEB_LDR_DATA32)((PPEB32)Peb)->Ldr)->InLoadOrderModuleList;
        return LdrpGetModuleNameWoW64((PLIST_ENTRY32)ModuleList, ImageBase, ModuleName);

    } else {

        //
        // This must be a 64 bit process.
        //
        Peb = (PVOID)PsGetProcessPeb(Process);
        if (!Peb) {
            return STATUS_UNSUCCESSFUL;
        }

        //
        // Wait for the loader if it's not setup yet.
        //
        if (!((PPEB64)Peb)->Ldr) {
            WaitTime.QuadPart = RELATIVE(MILLISECONDS(250)); // 250 msec.

            Index = 0;
            while (++Index <= 10 && !((PPEB64)Peb)->Ldr) {
                KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
            }

            //
            // Still no loader, fail.
            //
            if (Index > 10 && !((PPEB64)Peb)->Ldr) {
                return STATUS_UNSUCCESSFUL;
            }
        }

        ModuleList = (PLIST_ENTRY)&((PPEB_LDR_DATA64)((PPEB64)Peb)->Ldr)->InLoadOrderModuleList;
        return LdrpGetModuleName(ModuleList, ImageBase, ModuleName);
    }
}

PVOID
LDRAPI
LdrGetModuleBaseAddressUnicodeString(
    IN PEPROCESS Process,
    IN PCUNICODE_STRING ModuleName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo OPTIONAL
)
{
    PLIST_ENTRY ModuleList;
    LARGE_INTEGER WaitTime;
    PVOID Peb;
    ULONG Index;

    //
    // Check if this is a WOW64 process.
    //
    Peb = PsGetProcessWow64Process(Process);
    if (Peb) {

        //
        // Wait for the loader if it's not setup yet.
        //
        if (!((PPEB32)Peb)->Ldr) {
            WaitTime.QuadPart = RELATIVE(MILLISECONDS(250)); // 250 msec.

            Index = 0;
            while (++Index <= 10 && !((PPEB32)Peb)->Ldr) {
                KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
            }

            //
            // Still no loader, fail.
            //
            if (Index > 10 && !((PPEB32)Peb)->Ldr) {
                return NULL;
            }
        }

        ModuleList = (PLIST_ENTRY)&((PPEB_LDR_DATA32)((PPEB32)Peb)->Ldr)->InLoadOrderModuleList;

        return LdrpGetModuleBaseAddressWoW64((PLIST_ENTRY32)ModuleList, ModuleName, ImageInfo);

    } else {

        //
        // This must be a 64 bit process.
        //
        Peb = (PVOID)PsGetProcessPeb(Process);
        if (!Peb) {
            return NULL;
        }

        //
        // Wait for the loader if it's not setup yet.
        //
        if (!((PPEB64)Peb)->Ldr) {
            WaitTime.QuadPart = RELATIVE(MILLISECONDS(250)); // 250 msec.

            Index = 0;
            while (++Index <= 10 && !((PPEB64)Peb)->Ldr) {
                KeDelayExecutionThread(KernelMode, FALSE, &WaitTime);
            }

            //
            // Still no loader, fail.
            //
            if (Index > 10 && !((PPEB64)Peb)->Ldr) {
                return NULL;
            }
        }

        ModuleList = (PLIST_ENTRY)&((PPEB_LDR_DATA64)((PPEB64)Peb)->Ldr)->InLoadOrderModuleList;

        return LdrpGetModuleBaseAddress(ModuleList, ModuleName, ImageInfo);
    }
}

PVOID
LDRAPI
LdrGetModuleBaseAddressAscii(
    IN PEPROCESS Process,
    IN CONST CHAR* ModuleName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo OPTIONAL
)
{
    NTSTATUS Status;
    USHORT ModuleNameLength;
    CHAR* ModuleNameEnd;
    UNICODE_STRING UnicodeString;
    ULONG BytesInUnicodeString;
    WCHAR ModuleUnicodeBuffer[MAXIMUM_FILENAME_LENGTH];

    ModuleNameEnd = (CHAR*)ModuleName;
    while (*ModuleNameEnd++) ;
    ModuleNameLength = (USHORT)(ModuleNameEnd - ModuleName - 1);

    UnicodeString.Buffer = ModuleUnicodeBuffer;
    UnicodeString.Length = (USHORT)(ModuleNameLength * sizeof(WCHAR));
    UnicodeString.MaximumLength = sizeof(ModuleUnicodeBuffer);

    do {
        Status = RtlMultiByteToUnicodeN(UnicodeString.Buffer,
                                        UnicodeString.Length,
                                        &BytesInUnicodeString,
                                        ModuleName,
                                        ModuleNameLength
                                        );
        if (NT_SUCCESS(Status)) {
            UnicodeString.Buffer[BytesInUnicodeString / sizeof(WCHAR)] = L'\0';
            break;
        }

        YieldProcessor();
    } while (TRUE);

    return LdrGetModuleBaseAddressUnicodeString(Process, &UnicodeString, ImageInfo);
}

NTSTATUS
LDRAPI
LdrMapImageSections(
    IN PVOID TargetBase,
    IN PVOID ImageBase,
    IN SIZE_T ImageSize OPTIONAL
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER Section;
    USHORT SectionIndex;
    SIZE_T SizeOfHeaders;
    SIZE_T SizeOfImage;
    ULONG SectionAlignment;
    ULONG FileAlignment;
    SIZE_T VirtualAddressAligned;
    SIZE_T SizeOfRawDataAligned;

    //
    // Grab NT headers.
    //
    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, ImageBase, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Determine the section and file alignments of this image.
    //
    switch (NtHeaders->OptionalHeader.Magic) {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        SizeOfHeaders = (SIZE_T)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfHeaders;
        SizeOfImage = (SIZE_T)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfImage;
        SectionAlignment = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SectionAlignment;
        FileAlignment = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.FileAlignment;
        break;

    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        SizeOfHeaders = (SIZE_T)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SizeOfHeaders;
        SizeOfImage = (SIZE_T)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SizeOfImage;
        SectionAlignment = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SectionAlignment;
        FileAlignment = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.FileAlignment;
        break;

    default:
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (!ImageSize) {
        ImageSize = SizeOfImage;
    }

    //
    // Copy header to destination.
    //
    RtlCopyMemory(TargetBase, ImageBase, SizeOfHeaders);

    //
    // Copy each section over to target.
    //
    Section = IMAGE_FIRST_SECTION(NtHeaders);
    SectionIndex = NtHeaders->FileHeader.NumberOfSections;
    while (SectionIndex > 0) {

        //
        // Check for corrupt fields (common in packed binaries).
        //
        if (LdrpIsSectionValid(Section, SectionAlignment, FileAlignment, ImageSize)) {

            //
            // Copy the section data to the target.
            //
            VirtualAddressAligned = LdrpAlignToSectionAlignment((SIZE_T)Section->VirtualAddress,
                                                                SectionAlignment,
                                                                FileAlignment
                                                                );
            if (!Section->SizeOfRawData) {
                SizeOfRawDataAligned = ALIGN_UP_BY(Section->Misc.VirtualSize, SectionAlignment);
            } else {
                SizeOfRawDataAligned = ALIGN_UP_BY(Section->SizeOfRawData, FileAlignment);
            }

            RtlCopyMemory((PUCHAR)TargetBase + VirtualAddressAligned,
                          (PUCHAR)ImageBase + Section->PointerToRawData,
                          SizeOfRawDataAligned);
        }

        //
        // Iterate to the next section.
        //
        SectionIndex -= 1;
        Section += 1;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LDRAPI
LdrProtectImageSections(
    IN PEPROCESS Process,
    IN PVOID NewBase,
    IN SIZE_T ImageSize OPTIONAL,
    IN BOOLEAN EraseHeaders
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    SIZE_T SizeOfHeaders;
    SIZE_T SizeOfImage;
    PIMAGE_SECTION_HEADER Section;
    USHORT SectionIndex;
    ULONG Protect;
    ULONG SectionAlignment;
    ULONG FileAlignment;
    SIZE_T VirtualAddressAligned;
    SIZE_T VirtualSizeAligned;
    PVOID SectionAddress;

    UNREFERENCED_PARAMETER(Process);

    //
    // Grab the PE header.
    //
    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, NewBase, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Determine the section and file alignments of this image.
    //
    switch (NtHeaders->OptionalHeader.Magic) {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        SizeOfHeaders = (SIZE_T)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfHeaders;
        SizeOfImage = (SIZE_T)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfImage;
        SectionAlignment = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SectionAlignment;
        FileAlignment = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.FileAlignment;
        break;

    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        SizeOfHeaders = (SIZE_T)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SizeOfHeaders;
        SizeOfImage = (SIZE_T)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SizeOfImage;
        SectionAlignment = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SectionAlignment;
        FileAlignment = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.FileAlignment;
        break;

    default:
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (!ImageSize) {
        ImageSize = SizeOfImage;
    }

    //
    // Protect or erase headers.
    //
    if (EraseHeaders) {
        RtlZeroMemory(NewBase, SizeOfHeaders);
        Status = PsFreeVirtualMemory(NtCurrentProcess(), &NewBase, &SizeOfHeaders, MEM_DECOMMIT);
    } else {
        Status = PsProtectVirtualMemory(NtCurrentProcess(), &NewBase, &SizeOfHeaders, PAGE_READONLY, NULL);
    }

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Protect the image sections.
    //
    Section = IMAGE_FIRST_SECTION(NtHeaders);
    SectionIndex = NtHeaders->FileHeader.NumberOfSections;
    while (SectionIndex > 0) {

        //
        // Check for any corrupt fields (common in packed binaries).
        //
        if (LdrpIsSectionValid(Section, SectionAlignment, FileAlignment, ImageSize)) {

            //
            // Get the aligned section virtual address.
            //
            VirtualAddressAligned = LdrpAlignToSectionAlignment(
                                        (SIZE_T)Section->VirtualAddress,
                                        SectionAlignment,
                                        FileAlignment
                                        );

            SectionAddress = (PVOID)((ULONG_PTR)NewBase + VirtualAddressAligned);

            //
            // Get the aligned virtual size.
            //
            if (!Section->Misc.VirtualSize) {

                BO_ASSERT(Section->SizeOfRawData != 0);
                VirtualSizeAligned = ALIGN_UP_BY(Section->SizeOfRawData, SectionAlignment);

            } else {

                VirtualSizeAligned = ALIGN_UP_BY(Section->Misc.VirtualSize, SectionAlignment);
            }

            //
            // Resolve the page protection value based on the section characteristics.
            //
            Protect = LdrpGetSectionProtectionForCharacteristics(Section->Characteristics);
            if (Protect == PAGE_NOACCESS) {

                //
                // If the page protection value resolved to no access, erase the section.
                //
                Status = PsFreeVirtualMemory(NtCurrentProcess(),
                                             &SectionAddress,
                                             &VirtualSizeAligned,
                                             MEM_DECOMMIT
                                             );
                if (!NT_SUCCESS(Status)) {

                    LOG_ERROR("Failed to erase section '%s' with status %08x",
                              Section->Name, Status);
                }

            } else {

                //
                // Set the desired page access protection.
                //
                Status = PsProtectVirtualMemory(NtCurrentProcess(),
                                                &SectionAddress,
                                                &VirtualSizeAligned,
                                                Protect,
                                                NULL
                                                );
                if (!NT_SUCCESS(Status)) {

                    LOG_ERROR("Failed to protect section '%s' with status %08x",
                              Section->Name, Status);
                }
            }
        }

        //
        // Iterate to the next section.
        //
        SectionIndex -= 1;
        Section += 1;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
LdrpLoadDll(
    IN PEPROCESS Process,
    IN PWCHAR DllPath,
    IN PCUNICODE_STRING DllName,
    IN ULONG DllCharacteristics,
    OUT PVOID *DllBase
)
{
    NTSTATUS Status;
    PVOID Peb;
    BOOLEAN Wow64;
    BOOLEAN NewWindows7;
    UNICODE_STRING ResolvedDllName;
    WCHAR ResolvedDllNameBuffer[MAXIMUM_FILENAME_LENGTH];
    WCHAR DefaultPath[MAXIMUM_FILENAME_LENGTH];
    USHORT DefaultPathLength;
    PVOID NtdllBase;
    PVOID NtCreateEventPtr;
    PVOID NtSetEventPtr;
    HANDLE *CompletionEventPtr;
    PVOID LdrLoadDllPtr;
    PVOID LdrpLoadDllPtr;
    UCHAR Pattern[16];
    ULONG PatternSize;
    PUCHAR Found;
    USHORT NtKernelBuild;
    WCHAR *DllPathBuffer;
    WCHAR *DllNameBuffer;
    SIZE_T DllPathLength;
    PVOID DllNameUstrPtr; // PUNICODE_STRING, or PUNICODE_STRING32
    PVOID LdrPathPtr; // PLDRP_DLL_PATH, PLDRP_DLL_PATH32,
                            // PLDRP_DLL_PATH_WIN81, PLDRP_DLL_PATH32_WIN81,
                            // PLDRP_DLL_PATH_WIN8, PLDRP_DLL_PATH32_WIN8,
                            // PUNICODE_STRING, or PUNICODE_STRING32
    PVOID *ResultModuleEntryPtr; // PLDR_DATA_TABLE_ENTRY*, or PLDR_DATA_TABLE_ENTRY32*
    PVOID *ResultDdagNodePtr; // PLDR_DDAG_NODE*, or PLDR_DDAG_NODE32*
    PLDR_DATA_TABLE_ENTRY ResultModuleEntry;
    PLDR_DDAG_NODE ResultDdagNode;
    PNTSTATUS FinalStatus;
    KPROCESSOR_MODE PreviousMode;
    PLONG ReferenceCount;
    ULONG ReferenceCountOffset;
    LARGE_INTEGER WaitTime;

    HANDLE CompletionEvent = NULL;
    PKEVENT SharedCompletionEvent = NULL;
    HCODEGEN CodeGenerator = NULL;
    PVOID CodeBase = NULL;
    SIZE_T CodeSize = 0;
    PVOID DataBase = NULL;
    SIZE_T DataSize = 0;

    CONST UNICODE_STRING NtdllUnicode = RTL_CONSTANT_STRING(L"NTDLL.DLL");
    CONST UNICODE_STRING LdrLoadDllUnicode = RTL_CONSTANT_STRING(L"LdrLoadDll");
    CONST UNICODE_STRING NtCreateEventUnicode = RTL_CONSTANT_STRING(L"NtCreateEvent");
    CONST UNICODE_STRING NtSetEventUnicode = RTL_CONSTANT_STRING(L"NtSetEvent");

    //
    // Determine if this is wow64 process or not.
    //
    Peb = PsGetProcessWow64Process(Process);
    if (Peb) {
        Wow64 = TRUE;
    } else {
        Peb = PsGetProcessPeb(Process);
        if (!Peb) {
            return STATUS_UNSUCCESSFUL;
        }
        Wow64 = FALSE;
    }

    //
    // Get the NT kernel (ntoskrnl) build number.
    //
    NtKernelBuild = UtlGetNtKernelBuild(NULL);
    if (!NtKernelBuild) {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Initialize the resolved DLL name.
    //
    RtlInitEmptyUnicodeString(&ResolvedDllName,
                              ResolvedDllNameBuffer,
                              sizeof(ResolvedDllNameBuffer)
                              );

    //
    // Get suitable file name for the usermode API.
    //
    if (DllName->Buffer[0] == L'\\' &&
        UtlWcsStr(DllName->Buffer, LdrpSystemRootName.Buffer, TRUE) == DllName->Buffer) {

        //
        // Get the system root path in "C:\\Windows" form.
        //
        Status = UtlGetSystemRootPath(&ResolvedDllName);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        //
        // Construct resolved dll name with the resolved system root path.
        //
        RtlUnicodeStringCchCatStringN(
                    &ResolvedDllName,
                    DllName->Buffer + LdrpSystemRootName.Length / sizeof(WCHAR),
                    (DllName->Length - LdrpSystemRootName.Length) / sizeof(WCHAR)
                    );
        ResolvedDllName.Buffer[ResolvedDllName.Length / sizeof(WCHAR)] = L'\0';

    } else {

        ResolvedDllName.Length = DllName->Length;
        ResolvedDllName.MaximumLength = DllName->MaximumLength;
        RtlCopyMemory(ResolvedDllName.Buffer, DllName->Buffer, DllName->Length);
        ResolvedDllName.Buffer[ResolvedDllName.Length / sizeof(WCHAR)] = L'\0';
    }

    //
    // Get the NTDLL base address.
    //
    NtdllBase = LdrGetModuleBaseAddressUnicodeString(Process, &NtdllUnicode, NULL);
    if (!NtdllBase) {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Find the NtCreateEvent export address.
    //
    Status = LdrFindExportAddressForProcessUnicode(Process,
                                                   NtdllBase,
                                                   &NtCreateEventUnicode,
                                                   &NtCreateEventPtr
                                                   );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find the NtCreateEvent export with status %08x", Status);
        return Status;
    }

    //
    // Find the NtSetEvent export address.
    //
    Status = LdrFindExportAddressForProcessUnicode(Process,
                                                   NtdllBase,
                                                   &NtSetEventUnicode,
                                                   &NtSetEventPtr
                                                   );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find the NtSetEvent export with status %08x", Status);
        return Status;
    }

    //
    // Find the LdrLoadDll export address.
    //
    Status = LdrFindExportAddressForProcessUnicode(Process,
                                                   NtdllBase,
                                                   &LdrLoadDllUnicode,
                                                   &LdrLoadDllPtr
                                                   );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to find the LdrLoadDll export with status %08x", Status);
        return Status;
    }

    LdrpLoadDllPtr = NULL;
    NewWindows7 = FALSE;

    //
    // Heuristically search for the LdrpLoadDll routine.
    //
    if (NtKernelBuild >= NT_BUILD_7 && NtKernelBuild < NT_BUILD_8) {

        if (!Wow64) {

            // E8 CC CC CC CC           call  LdrpLoadDll
            // 8B F8                    mov   edi, eax
            // 85 C0                    test  eax, eax
            *(ULONGLONG*)Pattern = 0x85F88BCCCCCCCCE8;
            PatternSize = sizeof(ULONGLONG);

        } else {

            // FF 75 10                 push  [ebp+DllName]
            // E8 CC CC CC CC           call  LdrpLoadDll
            *(ULONG*)Pattern = 0xE81075FF;
            PatternSize = sizeof(ULONG);
        }

        Found = (PUCHAR)UtlFindPattern(LdrLoadDllPtr, 256, 0xCC, Pattern, PatternSize);
        if (Found) {
            if (!Wow64) {
                LdrpLoadDllPtr = UtlCallTargetAddress(Found);
            } else {
                LdrpLoadDllPtr = UtlCallTargetAddress(Found + 3);
            }
        }

        //
        // If it wasn't found, try finding the new Windows 7 (build 7601.24291) signature.
        //
        if (!LdrpLoadDllPtr) {

            if (!Wow64) {

                // E8 CC CC CC CC           call  LdrpLoadDll
                // 8B D8                    mov   ebx, eax
                // 85 C0                    test  eax, eax
                *(ULONGLONG*)Pattern = 0x85D88BCCCCCCCCE8;
                PatternSize = sizeof(ULONGLONG);

                NewWindows7 = TRUE;

            } else {

                // E8 CC CC CC CC           call  RtlWow64EnableFsRedirectionEx
                // E9 CC CC CC CC           jmp   LdrpLoadDllChunk
                *(ULONG*)Pattern = 0xCCCCCCE8;
                *(USHORT*)(Pattern + sizeof(ULONG)) = 0xE9CC;
                PatternSize = sizeof(ULONG) + sizeof(USHORT);

                //
                // First try to find the function chunk.
                //
                Found = (PUCHAR)UtlFindPattern(LdrLoadDllPtr, 256, 0xCC, Pattern, PatternSize);
                if (Found) {

                    //
                    // Get the address of the function chunk.
                    //
                    LdrLoadDllPtr = UtlJmpTargetAddress(Found + 1 + sizeof(LONG));
                    if (!LdrLoadDllPtr) {
                        LdrLoadDllPtr = Found; // Well, I guess try to keep searching.
                    } else {
                        NewWindows7 = TRUE;
                    }

                    // FF 75 10             push  [ebp+DllName]
                    // E8 CC CC CC CC       call  LdrpLoadDll
                    *(ULONG*)Pattern = 0xE81075FF;
                    PatternSize = sizeof(ULONG);
                }
            }

            Found = (PUCHAR)UtlFindPattern(LdrLoadDllPtr, 256, 0xCC, Pattern, PatternSize);
            if (Found) {
                if (!Wow64) {
                    LdrpLoadDllPtr = UtlCallTargetAddress(Found);
                } else {
                    LdrpLoadDllPtr = UtlCallTargetAddress(Found + 3);
                }
            }
        }

    } else if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_10_1607) {

        if (!Wow64) {

            if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_10_1507) {

                // 48 89 44 24 20       mov   [rsp+20h], rax ; ResultLdrDataTableEntry
                // E8 71 00 00 00       call  LdrpLoadDll
                *(ULONGLONG*)Pattern = 0xCCCCE82024448948;
                PatternSize = sizeof(ULONGLONG);

            } else { //if (NtKernelBuild >= NT_BUILD_10_1507 && NtKernelBuild < NT_BUILD_10_1607)

                // 48 8B CF             mov   rcx, rdi
                // E8 CC CC CC CC       call  LdrpLoadDll
                *(ULONGLONG*)Pattern = 0xCCCCCCCCE8CF8B48;
                PatternSize = sizeof(ULONGLONG);
            }

            Found = (PUCHAR)UtlFindPattern(LdrLoadDllPtr, 256, 0xCC, Pattern, PatternSize);
            if (Found) {
                if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_10_1507) {
                    LdrpLoadDllPtr = UtlCallTargetAddress(Found + 5);
                } else {
                    LdrpLoadDllPtr = UtlCallTargetAddress(Found + 3);
                }
            }

        } else {

            // E8 CC CC CC CC
            // 80 7D CC 00
            *(ULONGLONG*)Pattern = 0xCC7D80CCCCCCCCE8;
            *(UCHAR*)(Pattern + sizeof(ULONGLONG)) = 0x00;
            PatternSize = sizeof(ULONGLONG) + sizeof(UCHAR);

            Found = (PUCHAR)UtlFindPattern(LdrLoadDllPtr, 256, 0xCC, Pattern, PatternSize);
            if (Found) {
                LdrpLoadDllPtr = UtlCallTargetAddress(Found);
            }
        }

    } else if (NtKernelBuild >= NT_BUILD_10_1607 && NtKernelBuild <= NT_BUILD_10_1809) {

        if (!Wow64) {

            // 48 8B CF             mov     rcx, rdi    ; DllName
            // E8 CC CC CC CC       call    LdrpLoadDll
            *(ULONG*)Pattern = 0xE8CF8B48;
            PatternSize = sizeof(ULONG);

        } else {

            // 8D 54 24 24(or 20)   mov     rcx, rdi
            // E8 CC CC CC CC       call    LdrpLoadDll
            *(ULONG*)Pattern = 0xCC24548D;
            *(UCHAR*)(Pattern + sizeof(ULONG)) = 0xE8;
            PatternSize = sizeof(ULONG) + sizeof(UCHAR);
        }

        Found = (PUCHAR)UtlFindPattern(LdrLoadDllPtr, 320, 0xCC, Pattern, PatternSize);
        if (Found) {
            if (!Wow64) {
                LdrpLoadDllPtr = UtlCallTargetAddress(Found + 3);
            } else {
                LdrpLoadDllPtr = UtlCallTargetAddress(Found + 4);
            }
        }

    } else {
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Make sure the LdrpLoadDll routine was found.
    //
    if (!LdrpLoadDllPtr) {
        LOG_ERROR("Failed to find LdrpLoadDll address!");
        return STATUS_NOT_FOUND;
    }

    //
    // Allocate buffers in process for the LdrpLoadDll shellcode stub.
    //
    CodeBase = NULL;
    CodeSize = PAGE_SIZE;

    Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                     &CodeBase,
                                     0,
                                     &CodeSize,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    DataBase = NULL;
    DataSize = PAGE_SIZE;

    Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                     &DataBase,
                                     0,
                                     &DataSize,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    //
    // Setup the data for the call to LdrpLoadDll.
    //
    DllNameBuffer = (PWCHAR)((ULONG_PTR)DataBase + sizeof(UNICODE_STRING));
    RtlZeroMemory(DllNameBuffer, ResolvedDllName.MaximumLength);
    RtlCopyMemory(DllNameBuffer, ResolvedDllName.Buffer, ResolvedDllName.Length);

    ResultModuleEntryPtr = (PVOID *)((ULONG_PTR)DataBase +
                                        sizeof(UNICODE_STRING) + ResolvedDllName.MaximumLength);
    ResultDdagNodePtr = (PVOID *)(ResultModuleEntryPtr + 1);
    LdrPathPtr = (PVOID)(ResultModuleEntryPtr + 2);
    FinalStatus = (PNTSTATUS)(ResultModuleEntryPtr + 3);
    CompletionEventPtr = (HANDLE *)(ResultModuleEntryPtr + 4);

    if (LDR_IS_PATH_FLAGS(DllPath) || !DllPath) {
        DllPathLength = 0;
        DllPathBuffer = (WCHAR *)LDR_GET_PATH_FLAGS(DllPath);
    } else {
        DllPathBuffer = (WCHAR *)((ULONG_PTR)LdrPathPtr + sizeof(LDRP_DLL_PATH));
        DllPathLength = (SIZE_T)wcslen(DllPath);
        RtlCopyMemory(DllPathBuffer, DllPath, DllPathLength * sizeof(WCHAR));
    }

    if (!Wow64) {

        DllNameUstrPtr = DataBase;
        ((PUNICODE_STRING)DllNameUstrPtr)->Length = ResolvedDllName.Length;
        ((PUNICODE_STRING)DllNameUstrPtr)->MaximumLength = ResolvedDllName.MaximumLength;
        ((PUNICODE_STRING)DllNameUstrPtr)->Buffer = DllNameBuffer;

        //
        // The DllSearchPath parameter uses a structure in Windows 10.
        //
        if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_8_1) {

            RtlZeroMemory(LdrPathPtr, sizeof(LDRP_DLL_PATH_WIN8));

            if (DllPathLength) {
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->ImplicitPath = FALSE;
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->ExternalSearchPath = FALSE;
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->DllName = NULL;
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->DllSearchPath = DllPathBuffer;
            } else {
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->ImplicitPath = TRUE;
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->ExternalSearchPath = FALSE;
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->DllName = DllNameBuffer;
                ((PLDRP_DLL_PATH_WIN8)LdrPathPtr)->DllSearchPath = DllPathBuffer;
            }

        } else if (NtKernelBuild >= NT_BUILD_8_1 && NtKernelBuild < NT_BUILD_10_1507) {

            RtlZeroMemory(LdrPathPtr, sizeof(LDRP_DLL_PATH_WIN81));

            if (DllPathLength) {
                ((PLDRP_DLL_PATH_WIN81)LdrPathPtr)->DllSearchPath = DllPathBuffer;
            } else {
                ((PLDRP_DLL_PATH_WIN81)LdrPathPtr)->DllName = DllNameBuffer;
                ((PLDRP_DLL_PATH_WIN81)LdrPathPtr)->ImplicitPathFlags = (ULONG)(ULONG_PTR)DllPathBuffer;
            }

        } else if (NtKernelBuild >= NT_BUILD_10_1507) {

            RtlZeroMemory(LdrPathPtr, sizeof(LDRP_DLL_PATH));

            if (DllPathLength) {
                ((PLDRP_DLL_PATH)LdrPathPtr)->DllSearchPath = DllPathBuffer;
            } else {
                ((PLDRP_DLL_PATH)LdrPathPtr)->DllName = DllNameBuffer;
                ((PLDRP_DLL_PATH)LdrPathPtr)->ImplicitPathFlags = (ULONG)(ULONG_PTR)DllPathBuffer;
            }

        } else {

            RtlZeroMemory(LdrPathPtr, sizeof(UNICODE_STRING));

            if (!DllPathLength) {

                Status = PsGetProcessFullImageName(Process,
                                                   DefaultPath,
                                                   sizeof(DefaultPath),
                                                   &DefaultPathLength
                                                   );
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }

                DllPathLength = DefaultPathLength;
                DllPathBuffer = (WCHAR*)((ULONG_PTR)LdrPathPtr + sizeof(LDRP_DLL_PATH));
                RtlCopyMemory(DllPathBuffer, DefaultPath, DllPathLength * sizeof(WCHAR));
            }

            ((PUNICODE_STRING)LdrPathPtr)->Length = (USHORT)(DllPathLength * sizeof(WCHAR));
            ((PUNICODE_STRING)LdrPathPtr)->MaximumLength = (USHORT)((DllPathLength + 1) * sizeof(WCHAR));
            ((PUNICODE_STRING)LdrPathPtr)->Buffer = DllPathBuffer;
        }

    } else {

        DllNameUstrPtr = DataBase;
        ((PUNICODE_STRING32)DllNameUstrPtr)->Length = ResolvedDllName.Length;
        ((PUNICODE_STRING32)DllNameUstrPtr)->MaximumLength = ResolvedDllName.MaximumLength;
        ((PUNICODE_STRING32)DllNameUstrPtr)->Buffer = (ULONG)(ULONG_PTR)DllNameBuffer;

        //
        // The DllSearchPath parameter uses a structure in Windows 10.
        //
        if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_8_1) {

            RtlZeroMemory(LdrPathPtr, sizeof(LDRP_DLL_PATH32_WIN8));

            if (DllPathLength) {
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->ImplicitPath = FALSE;
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->ExternalSearchPath = FALSE;
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->DllName = 0;
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->DllSearchPath = (ULONG)(ULONG_PTR)DllPathBuffer;
            } else {
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->ImplicitPath = TRUE;
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->ExternalSearchPath = FALSE;
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->DllName = (ULONG)(ULONG_PTR)DllNameBuffer;
                ((PLDRP_DLL_PATH32_WIN8)LdrPathPtr)->DllSearchPath = (ULONG)(ULONG_PTR)DllPathBuffer;
            }

        } else if (NtKernelBuild >= NT_BUILD_8_1 && NtKernelBuild < NT_BUILD_10_1507) {

            RtlZeroMemory(LdrPathPtr, sizeof(LDRP_DLL_PATH32_WIN81));

            if (DllPathLength) {
                ((PLDRP_DLL_PATH32_WIN81)LdrPathPtr)->DllSearchPath = (ULONG)(ULONG_PTR)DllPathBuffer;
            } else {
                ((PLDRP_DLL_PATH32_WIN81)LdrPathPtr)->DllName = (ULONG)(ULONG_PTR)DllNameBuffer;
                ((PLDRP_DLL_PATH32_WIN81)LdrPathPtr)->ImplicitPathFlags = (ULONG)(ULONG_PTR)DllPathBuffer;
            }

        } else if (NtKernelBuild >= NT_BUILD_10_1507) {

            RtlZeroMemory(LdrPathPtr, sizeof(LDRP_DLL_PATH));

            if (DllPathLength) {
                ((PLDRP_DLL_PATH32)LdrPathPtr)->DllSearchPath = (ULONG)(ULONG_PTR)DllPathBuffer;
            } else {
                ((PLDRP_DLL_PATH32)LdrPathPtr)->DllName = (ULONG)(ULONG_PTR)DllNameBuffer;
                ((PLDRP_DLL_PATH32)LdrPathPtr)->ImplicitPathFlags = (ULONG)(ULONG_PTR)DllPathBuffer;
            }

        } else {

            //BO_DBG_BREAK();
            RtlZeroMemory(LdrPathPtr, sizeof(UNICODE_STRING));

            if (!DllPathLength) {

                Status = PsGetProcessFullImageName(Process,
                                                   DefaultPath,
                                                   sizeof(DefaultPath),
                                                   &DefaultPathLength
                                                   );
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }

                DllPathLength = DefaultPathLength;
                DllPathBuffer = (WCHAR*)((ULONG_PTR)LdrPathPtr + sizeof(LDRP_DLL_PATH));
                RtlCopyMemory(DllPathBuffer, DefaultPath, DllPathLength * sizeof(WCHAR));
            }

            ((PUNICODE_STRING32)LdrPathPtr)->Length = (USHORT)(DllPathLength * sizeof(WCHAR));
            ((PUNICODE_STRING32)LdrPathPtr)->MaximumLength = (USHORT)((DllPathLength + 1) * sizeof(WCHAR));
            ((PUNICODE_STRING32)LdrPathPtr)->Buffer = (ULONG)(ULONG_PTR)DllPathBuffer;
        }
    }

    //
    // Initialize the code generator.
    //
    Status = CgInitializeGenerator(&CodeGenerator, CodeBase, CodeSize, !Wow64);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    //
    // First create the completion event.
    //
    CgBeginCode(CodeGenerator, FALSE);

    //
    // NTSTATUS
    // NTAPI
    // NtCreateEvent(
    //     OUT PHANDLE EventHandle,
    //     IN ACCESS_MASK DesiredAccess,
    //     IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    //     IN EVENT_TYPE EventType,
    //     IN BOOLEAN InitialState
    //     );
    //
    CgPushPointer(CodeGenerator, (PVOID)CompletionEventPtr);    // EventHandle,
    CgPushUInt32(CodeGenerator, EVENT_ALL_ACCESS);              // DesiredAccess,
    CgPushPointer(CodeGenerator, NULL);                         // ObjectAttributes,
    CgPushUInt32(CodeGenerator, (UINT32)NotificationEvent);     // EventType,
    CgPushBoolean(CodeGenerator, FALSE);                        // InitialState,
    CgCall(CodeGenerator, CC_STDCALL, (PVOID)NtCreateEventPtr); // NtCreateEvent(CompletionEventPtr, EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);

    // mov dword [FinalStatus], eax
    if (!Wow64) {
        CgAddUInt16(CodeGenerator, 0xA348);
        CgAddUInt64(CodeGenerator, (UINT64)FinalStatus);
    } else {
        CgAddUInt8(CodeGenerator, 0xA3);
        CgAddUInt32(CodeGenerator, (UINT32)(ULONG_PTR)FinalStatus);
    }

    CgEndCode(CodeGenerator);
    CgReturn(CodeGenerator, 1 * (Wow64 ? sizeof(ULONG) : 0));

    //
    // Execute the NtCreateEvent shellcode.
    //
    Status = LdrpExecuteShellcode(Process,
                                  TRUE,
                                  TRUE,
                                  (PUSER_SHELLCODE_START_ROUTINE)CodeBase,
                                  NULL,
                                  NULL,
                                  NULL
                                  );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to execute NtCreateEvent shellcode with status %08x", Status);
        goto Exit;
    }

    Status = *FinalStatus;
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to create completion event with status %08x", Status);
        goto Exit;
    }

    CompletionEvent = *CompletionEventPtr;

    //
    // Get the event object for the shared completion event.
    //
    Status = ObReferenceObjectByHandle(CompletionEvent,
                                       EVENT_ALL_ACCESS,
                                       *ExEventObjectType,
                                       UserMode,
                                       &SharedCompletionEvent,
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to reference completion event object with status %08x", Status);
        goto Exit;
    }

    //
    // Reset the generator for the LdrpLoadDll shellcode.
    //
    CgResetGenerator(CodeGenerator);

    //
    // The LdrpLoadDll routine prototype is different depending on the windows version.
    //
    if (NtKernelBuild >= NT_BUILD_7 && NtKernelBuild < NT_BUILD_10_1507) {

        //
        // NTSTATUS
        // STDCALL
        // LdrpLoadDll(
        //      IN PCUNICODE_STRING DllName, 
        //      IN PUNICODE_STRING DllSearchPath,
        //      IN ULONG LoadFlags,
        //      IN BOOLEAN RunInitializers,
        //      IN PLDR_DATA_TABLE_ENTRY ParentModuleEntry,
        //      OUT PLDR_DATA_TABLE_ENTRY *ResultLdrDataTableEntry
        //      );
        //  
        //      -= OR =- 
        //
        // NTSTATUS
        // STDCALL
        // LdrpLoadDll(
        //      IN PCUNICODE_STRING DllName, 
        //      IN PUNICODE_STRING DllSearchPath,
        //      IN ULONG_PTR ImplicitPathFlags,
        //      IN ULONG LoadFlags,
        //      IN BOOLEAN RunInitializers,
        //      IN PLDR_DATA_TABLE_ENTRY ParentModuleEntry,
        //      OUT PLDR_DATA_TABLE_ENTRY *ResultLdrDataTableEntry
        //      );
        //
        CgBeginCode(CodeGenerator, FALSE);

        // LdrpLoadDll(DllNameUstr, LdrPath, 0, TRUE, NULL, ResultModuleEntry);
        CgPushPointer(CodeGenerator, DllNameUstrPtr);               // DllName,
        CgPushPointer(CodeGenerator, LdrPathPtr);                   // DllSearchPath,
        if (NewWindows7)                                            // -
            CgPushPointer(CodeGenerator, (PVOID)0);                 // ImplicitPathFlags,
        CgPushUInt32(CodeGenerator, DllCharacteristics);            // LoadFlags,
        CgPushBoolean(CodeGenerator, TRUE);                         // RunInitializers,
        CgPushPointer(CodeGenerator, NULL);                         // ParentModuleEntry,
        CgPushPointer(CodeGenerator, ResultModuleEntryPtr);         // ResultModuleEntry
        CgCall(CodeGenerator, CC_STDCALL, (PVOID)LdrpLoadDllPtr);   // LdrpLoadDll(DllNameUstrPtr, LdrPathPtr, 0, TRUE, NULL, ResultModuleEntry);

        // mov dword [FinalStatus], eax
        if (!Wow64) {
            CgAddUInt16(CodeGenerator, 0xA348);
            CgAddUInt64(CodeGenerator, (UINT64)FinalStatus);
        } else {
            CgAddUInt8(CodeGenerator, 0xA3);
            CgAddUInt32(CodeGenerator, (UINT32)(ULONG_PTR)FinalStatus);
        }

        // NtSetEvent(CompletionEvent, NULL);
        CgPushPointer(CodeGenerator, (PVOID)CompletionEvent);       // CompletionEvent,
        CgPushPointer(CodeGenerator, NULL);                         // PreviousState
        CgCall(CodeGenerator, CC_STDCALL, (PVOID)NtSetEventPtr);    // NtSetEvent(CompletionEvent, NULL);

        CgEndCode(CodeGenerator);
        CgReturn(CodeGenerator, 1 * (Wow64 ? sizeof(ULONG) : 0));

        ReferenceCountOffset = 0;

    } else if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_10_1507) {
        
        //
        // NTSTATUS
        // STDCALL
        // LdrpLoadDll(
        //      IN PCUNICODE_STRING DllName, 
        //      IN PVOID DllSearchPath, // PLDRP_DLL_PATH_WIN8 or PLDRP_DLL_PATH_WIN81
        //      IN ULONG LoadFlags,
        //      IN BOOLEAN RunInitializers,
        //      OUT PLDR_DATA_TABLE_ENTRY *ResultModuleEntry,
        //      OUT PLDR_DDAG_NODE *ResultDdagNode
        //      );
        //
        CgBeginCode(CodeGenerator, FALSE);

        // LdrpLoadDll(DllNameUstr, LdrPath, 0, TRUE, ResultModuleEntry, ResultDdagNode);
        CgPushPointer(CodeGenerator, DllNameUstrPtr);               // DllName,
        CgPushPointer(CodeGenerator, LdrPathPtr);                   // DllSearchPath,
        CgPushUInt32(CodeGenerator, DllCharacteristics);            // LoadFlags,
        CgPushBoolean(CodeGenerator, TRUE);                         // RunInitializers,
        CgPushPointer(CodeGenerator, ResultModuleEntryPtr);         // ResultModuleEntry,
        CgPushPointer(CodeGenerator, ResultDdagNodePtr);            // ResultDdagNode
        CgCall(CodeGenerator, CC_STDCALL, (PVOID)LdrpLoadDllPtr);   // LdrpLoadDll(DllNameUstrPtr, LdrPathPtr, 0, TRUE, ResultModuleEntry, ResultDdagNode);

        // mov dword [FinalStatus], eax
        if (!Wow64) {
            CgAddUInt16(CodeGenerator, 0xA348);
            CgAddUInt64(CodeGenerator, (UINT64)FinalStatus);
        } else {
            CgAddUInt8(CodeGenerator, 0xA3);
            CgAddUInt32(CodeGenerator, (UINT32)(ULONG_PTR)FinalStatus);
        }

        // NtSetEvent(CompletionEvent, NULL);
        CgPushPointer(CodeGenerator, (PVOID)CompletionEvent);       // CompletionEvent,
        CgPushPointer(CodeGenerator, NULL);                         // PreviousState
        CgCall(CodeGenerator, CC_STDCALL, (PVOID)NtSetEventPtr);    // NtSetEvent(CompletionEvent, NULL);

        CgEndCode(CodeGenerator);
        CgReturn(CodeGenerator, 1 * (Wow64 ? sizeof(ULONG) : 0));

        if (!Wow64) {
            ReferenceCountOffset = FIELD_OFFSET(LDR_DDAG_NODE, u1.ReferenceCount);
        } else {
            ReferenceCountOffset = FIELD_OFFSET(LDR_DDAG_NODE32, u1.ReferenceCount);
        }

    } else if (NtKernelBuild >= NT_BUILD_10_1507 && NtKernelBuild < NT_BUILD_10_1809) {

        //
        // NTSTATUS
        // FASTCALL
        // LdrpLoadDll(
        //      IN PCUNICODE_STRING DllName, 
        //      IN PLDRP_DLL_PATH DllSearchPath,
        //      IN ULONG LoadFlags,
        //      IN BOOLEAN RunInitializers,
        //      OUT PLDR_DATA_TABLE_ENTRY *ResultLdrDataTableEntry
        //      );
        //
        CgBeginCode(CodeGenerator, FALSE);

        // LdrpLoadDll(DllNameUstr, LdrPath, 0, TRUE, ResultModuleEntry);
        CgPushPointer(CodeGenerator, DllNameUstrPtr);               // DllName,
        CgPushPointer(CodeGenerator, LdrPathPtr);                   // DllSearchPath,
        CgPushUInt32(CodeGenerator, DllCharacteristics);            // LoadFlags,
        CgPushBoolean(CodeGenerator, TRUE);                         // RunInitializers,
        CgPushPointer(CodeGenerator, ResultModuleEntryPtr);         // ResultModuleEntry
        CgCall(CodeGenerator, CC_FASTCALL, (PVOID)LdrpLoadDllPtr);  // LdrpLoadDll(DllNameUstrPtr, LdrPathPtr, 0, TRUE, ResultModuleEntry);

        // mov dword [FinalStatus], eax
        if (!Wow64) {
            CgAddUInt16(CodeGenerator, 0xA348);
            CgAddUInt64(CodeGenerator, (UINT64)FinalStatus);
        } else {
            CgAddUInt8(CodeGenerator, 0xA3);
            CgAddUInt32(CodeGenerator, (UINT32)(ULONG_PTR)FinalStatus);
        }

        // NtSetEvent(CompletionEvent, NULL);
        CgPushPointer(CodeGenerator, (PVOID)CompletionEvent);       // CompletionEvent,
        CgPushPointer(CodeGenerator, NULL);                         // PreviousState
        CgCall(CodeGenerator, CC_STDCALL, (PVOID)NtSetEventPtr);    // NtSetEvent(CompletionEvent, NULL);

        CgEndCode(CodeGenerator);
        CgReturn(CodeGenerator, 1 * (Wow64 ? sizeof(ULONG) : 0));

        if (!Wow64) {
            ReferenceCountOffset = FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, ReferenceCount);
        } else {
            ReferenceCountOffset = FIELD_OFFSET(LDR_DATA_TABLE_ENTRY32, ReferenceCount);
        }

    } else if (NtKernelBuild >= NT_BUILD_10_1809) {

        //
        // NTSTATUS
        // STDCALL
        // LdrpLoadDll(
        //      IN PCUNICODE_STRING DllName, 
        //      IN PLDRP_DLL_PATH DllSearchPath,
        //      IN ULONG LoadFlags,
        //      OUT PLDR_DATA_TABLE_ENTRY *ResultModuleEntry
        //      );
        //
        CgBeginCode(CodeGenerator, FALSE);

        // LdrpLoadDll(DllNameUstr, LdrPath, 0, ResultModuleEntry);
        CgPushPointer(CodeGenerator, DllNameUstrPtr);               // DllName,
        CgPushPointer(CodeGenerator, LdrPathPtr);                   // DllSearchPath,
        CgPushUInt32(CodeGenerator, DllCharacteristics);            // LoadFlags,
        CgPushPointer(CodeGenerator, ResultModuleEntryPtr);         // ResultModuleEntry
        CgCall(CodeGenerator, CC_FASTCALL, (PVOID)LdrpLoadDllPtr);  // LdrpLoadDll(DllNameUstrPtr, LdrPathPtr, 0, ResultModuleEntry);

        // mov dword ptr[FinalStatus], eax
        if (!Wow64) {
            CgAddUInt16(CodeGenerator, 0xA348);
            CgAddUInt64(CodeGenerator, (UINT64)FinalStatus);
        } else {
            CgAddUInt8(CodeGenerator, 0xA3);
            CgAddUInt32(CodeGenerator, (UINT32)(ULONG_PTR)FinalStatus);
        }

        // NtSetEvent(CompletionEvent, NULL);
        CgPushPointer(CodeGenerator, (PVOID)CompletionEvent);       // CompletionEvent,
        CgPushPointer(CodeGenerator, NULL);                         // PreviousState
        CgCall(CodeGenerator, CC_STDCALL, (PVOID)NtSetEventPtr);    // NtSetEvent(CompletionEvent, NULL);

        CgEndCode(CodeGenerator);
        CgReturn(CodeGenerator, 1 * (Wow64 ? sizeof(ULONG) : 0));

        if (!Wow64) {
            ReferenceCountOffset = FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, ReferenceCount);
        } else {
            ReferenceCountOffset = FIELD_OFFSET(LDR_DATA_TABLE_ENTRY32, ReferenceCount);
        }

    } else {

        Status = STATUS_NOT_SUPPORTED;
        goto Exit;
    }

    //
    // Execute the remote shellcode thread.
    //
    Status = LdrpExecuteShellcode(Process,
                                  TRUE,
                                  TRUE,
                                  (PUSER_SHELLCODE_START_ROUTINE)CodeBase,
                                  NULL,
                                  NULL,
                                  NULL
                                  );
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to execute LdrpLoadDll shellcode with status %08x", Status);
        goto Exit;
    }

    //
    // Wait for the shellcode to complete execution.
    //
    WaitTime.QuadPart = RELATIVE(SECONDS(30));
    Status = KeWaitForSingleObject(SharedCompletionEvent,
                                   UserRequest,
                                   UserMode,
                                   TRUE,
                                   &WaitTime
                                   );
    if (Status == STATUS_TIMEOUT) {
        LOG_ERROR("Timed out waiting for shellcode to execute");
        Status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    Status = *FinalStatus;
    if (NT_SUCCESS(Status)) {

        //
        // Retrieve results, and decrement the the reference count if needed.
        //
        ResultModuleEntry = *ResultModuleEntryPtr;
        if (ResultModuleEntry) {

            if (ReferenceCountOffset) {
                if (NtKernelBuild >= NT_BUILD_8 && NtKernelBuild < NT_BUILD_10_1507) {

                    ResultDdagNode = *ResultDdagNodePtr;
                    ReferenceCount = (PLONG)((ULONG_PTR)ResultDdagNode + ReferenceCountOffset);
                    if (*ReferenceCount != -1) {
                        InterlockedDecrement(ReferenceCount);
                    }

                } else {

                    ReferenceCount = (PLONG)((ULONG_PTR)ResultModuleEntry + ReferenceCountOffset);
                    if (!Wow64) {

                        ResultDdagNode = ResultModuleEntry->DdagNode;
                        if (ResultDdagNode->LoadCount != -1 &&
                            !(CONTAINING_RECORD(ResultDdagNode->Modules.Flink,
                                                LDR_DATA_TABLE_ENTRY, NodeModuleLink)->Flags & 0x20))
                        {
                            InterlockedDecrement(ReferenceCount);
                        }

                    } else {

                        ResultDdagNode =
                            (PLDR_DDAG_NODE)((PLDR_DATA_TABLE_ENTRY32)ResultModuleEntry)->DdagNode;
                        if (((PLDR_DDAG_NODE32)ResultDdagNode)->LoadCount != -1 &&
                            !(CONTAINING_RECORD(((PLDR_DDAG_NODE32)ResultDdagNode)->Modules.Flink,
                                                LDR_DATA_TABLE_ENTRY32, NodeModuleLink)->Flags & 0x20))
                        {
                            InterlockedDecrement(ReferenceCount);
                        }
                    }
                }
            }

            if (!Wow64) {
                *DllBase = ResultModuleEntry->DllBase;
            } else {
                *DllBase = (PVOID)((PLDR_DATA_TABLE_ENTRY32)ResultModuleEntry)->DllBase;
            }
        }
    }

Exit:
    if (!NT_SUCCESS(Status)){
        *DllBase = NULL;
    }

    if (SharedCompletionEvent) {

        //
        // Dereference the shared event object 
        //
        ObDereferenceObject(SharedCompletionEvent);
    }

    if (CompletionEvent) {

        //
        // Close the completion event.
        //
        //ObCloseHandle(CompletionEvent, UserMode);
        PS_PREVIOUS_MODE_USER(&PreviousMode);
        NtClose(CompletionEvent);
        PS_PREVIOUS_MODE_RESTORE(PreviousMode);
    }

    if (CodeGenerator) {

        //
        // Free the code generator.
        //
        CgDestroyGenerator(CodeGenerator);
    }

    if (DataBase) {

        //
        // Free the shellcode data memory.
        //
        DataSize = 0;
        PsFreeVirtualMemory(NtCurrentProcess(), &DataBase, &DataSize, MEM_RELEASE);
    }

    if (CodeBase) {

        //
        // Free the shellcode code memory.
        //
        CodeSize = 0;
        PsFreeVirtualMemory(NtCurrentProcess(), &CodeBase, &CodeSize, MEM_RELEASE);
    }

    return Status;
}

NTSTATUS
LDRAPI
LdrResolveImageImports(
    IN PEPROCESS Process,
    IN PVOID ImageBase
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    BOOLEAN Is64Bit;
    ULONG Size;
    PIMAGE_IMPORT_DESCRIPTOR ImportTable;
    PIMAGE_IMPORT_BY_NAME ImportByName;
    ULONG ImportByOrdinal;
    PIMAGE_THUNK_DATA64 OriginalFirstThunk;
    PIMAGE_THUNK_DATA64 FirstThunk;
    PIMAGE_THUNK_DATA32 OriginalFirstThunk32;
    PIMAGE_THUNK_DATA32 FirstThunk32;
    CHAR* ModuleName;
    ANSI_STRING ModuleNameAnsi;
    UNICODE_STRING ModuleNameUstr;
    UNICODE_STRING ResolvedUstr;
    UNICODE_STRING ResolvedNameUstr;
    WCHAR ModuleNameBuffer[MAXIMUM_FILENAME_LENGTH];
    WCHAR ResolvedBuffer[MAXIMUM_FILENAME_LENGTH];
    PVOID ModuleBase;
    PVOID FunctionAddress;

    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, ImageBase, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        Is64Bit = TRUE;
    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        Is64Bit = FALSE;
    } else {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    //
    // Get the import directory
    //
    ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(
                                                ImageBase,
                                                TRUE,
                                                IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                &Size
                                                );
    if (!ImportTable) {

        //
        // Looks like no imports are present!
        //
        return STATUS_SUCCESS;
    }

    RtlInitEmptyUnicodeString(&ModuleNameUstr, ModuleNameBuffer, sizeof(ModuleNameBuffer));
    RtlInitEmptyUnicodeString(&ResolvedUstr, ResolvedBuffer, sizeof(ResolvedBuffer));

    //
    // Iterate the import descriptor table
    //
    while (ImportTable->Name != 0) {
        ModuleName = (CHAR*)ImageBase + ImportTable->Name;

        #if defined(ENABLE_EXTENDED_LOGGING)
        LOG_DEBUG("Import module \"%s\" => ", ModuleName);
        #endif // ENABLE_EXTENDED_LOGGING

        RtlInitAnsiString(&ModuleNameAnsi, ModuleName);
        RtlAnsiStringToUnicodeString(&ModuleNameUstr, &ModuleNameAnsi, FALSE);

        //
        // Resolve the correct path for the module.
        //
        Status = LdrResolveImagePath(Process, &ModuleNameUstr, NULL, &ResolvedUstr);
        if (!NT_SUCCESS(Status)) {
            BO_DBG_BREAK();
            LOG_ERROR("Failed to resolve import module path with status %08x", Status);
            return Status;
        }

        //
        // Extract the file name from the resolved path.
        //
        UtlExtractFileName(&ResolvedUstr, &ResolvedNameUstr);
        #if defined(ENABLE_EXTENDED_LOGGING)
        LOG_DEBUG("\"%wZ\" =>", &ResolvedNameUstr);
        #endif // ENABLE_EXTENDED_LOGGING

        //
        // Get the module base address.
        //
        ModuleBase = LdrGetModuleBaseAddressUnicodeString(Process, &ResolvedNameUstr, NULL);
        if (!ModuleBase) {

            //
            // If the image wasn't loaded yet, load it now.
            //
            Status = LdrpLoadDll(Process, NULL, &ResolvedUstr, 0, &ModuleBase);
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
        }

        #if defined(ENABLE_EXTENDED_LOGGING)
        LOG_DEBUG("0x%p {", ModuleBase);
        #endif // ENABLE_EXTENDED_LOGGING

        if (Is64Bit) {

            OriginalFirstThunk = (PIMAGE_THUNK_DATA64)((ULONG_PTR)ImageBase +
                                                            ImportTable->OriginalFirstThunk);
            FirstThunk = (PIMAGE_THUNK_DATA64)((ULONG_PTR)ImageBase + ImportTable->FirstThunk);

            while (OriginalFirstThunk->u1.Function) {

                if (OriginalFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {

                    //
                    // Resolve import address by ordinal.
                    //
                    ImportByOrdinal = (ULONG)(OriginalFirstThunk->u1.Ordinal & 0xFFFF);

                    Status = LdrFindExportAddressForProcessAscii(Process,
                                                                 ModuleBase,
                                                                 NULL,
                                                                 ImportByOrdinal,
                                                                 &FunctionAddress
                                                                 );
                    if (!NT_SUCCESS(Status)) {
                        return Status;
                    }

                    #if defined(ENABLE_EXTENDED_LOGGING)
                    LOG_DEBUG("    '%u' => 0x%p", ImportByOrdinal, FunctionAddress);
                    #endif // ENABLE_EXTENDED_LOGGING

                } else {

                    //
                    // Resolve import address by name.
                    //
                    ImportByName = (PIMAGE_IMPORT_BY_NAME)((ULONG_PTR)ImageBase +
                                                            OriginalFirstThunk->u1.AddressOfData);

                    Status = LdrFindExportAddressForProcessAscii(Process,
                                                                 ModuleBase,
                                                                 ImportByName->Name,
                                                                 0,
                                                                 &FunctionAddress
                                                                 );
                    if (!NT_SUCCESS(Status)) {
                        return Status;
                    }

                    #if defined(ENABLE_EXTENDED_LOGGING)
                    LOG_DEBUG("    \"%s\" => 0x%p", ImportByName->Name, FunctionAddress);
                    #endif // ENABLE_EXTENDED_LOGGING
                }

                FirstThunk->u1.Function = (ULONGLONG)FunctionAddress;

                //
                // Move to the next thunk
                //
                OriginalFirstThunk++;
                FirstThunk++;
            }

        } else { // 32 bit

            OriginalFirstThunk32 = (PIMAGE_THUNK_DATA32)((ULONG_PTR)ImageBase +
                                                            ImportTable->OriginalFirstThunk);
            FirstThunk32 = (PIMAGE_THUNK_DATA32)((ULONG_PTR)ImageBase + ImportTable->FirstThunk);

            while (OriginalFirstThunk32->u1.Function) {

                if (OriginalFirstThunk32->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {

                    //
                    // Resolve import address by ordinal.
                    //
                    ImportByOrdinal = (ULONG)(OriginalFirstThunk32->u1.Ordinal & 0xFFFF);

                    Status = LdrFindExportAddressForProcessAscii(Process,
                                                                 ModuleBase,
                                                                 NULL,
                                                                 ImportByOrdinal,
                                                                 &FunctionAddress
                                                                 );
                    if (!NT_SUCCESS(Status)) {
                        return Status;
                    }

                    #if defined(ENABLE_EXTENDED_LOGGING)
                    LOG_DEBUG("    '%u' => 0x%p", ImportByOrdinal, FunctionAddress);
                    #endif // ENABLE_EXTENDED_LOGGING

                } else {

                    //
                    // Resolve import address by name.
                    //
                    ImportByName = (PIMAGE_IMPORT_BY_NAME)((ULONG_PTR)ImageBase +
                                                            OriginalFirstThunk32->u1.AddressOfData);

                    Status = LdrFindExportAddressForProcessAscii(Process,
                                                                 ModuleBase,
                                                                 ImportByName->Name,
                                                                 0,
                                                                 &FunctionAddress
                                                                 );
                    if (!NT_SUCCESS(Status)) {
                        return Status;
                    }

                    #if defined(ENABLE_EXTENDED_LOGGING)
                    LOG_DEBUG("    \"%s\" => 0x%p", ImportByName->Name, FunctionAddress);
                    #endif // ENABLE_EXTENDED_LOGGING
                }

                FirstThunk32->u1.Function = (ULONG)(ULONG_PTR)FunctionAddress;

                //
                // Move to the next thunk
                //
                OriginalFirstThunk32++;
                FirstThunk32++;
            }
        }

        #if defined(ENABLE_EXTENDED_LOGGING)
        LOG_DEBUG("}");
        #endif // ENABLE_EXTENDED_LOGGING

        //
        // Move to the next import module entry
        //
        ImportTable += 1;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LDRAPI
LdrRelocateImageWithBias(
    IN PVOID NewBase,
    IN LONGLONG AdditionalBias
)
{
    NTSTATUS Status;
    LONGLONG Diff;
    ULONG TotalCountBytes = 0;
    PVOID VA;
    ULONGLONG OldBase;
    ULONG SizeOfBlock;
    USHORT *NextOffset;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;

    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, NewBase, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    //
    // Get old base.
    //
    switch (NtHeaders->OptionalHeader.Magic) {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        OldBase = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        OldBase = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;
        break;
    default:
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto Exit;
    }

    //
    // Locate the relocation section.
    //
    NextBlock = (PIMAGE_BASE_RELOCATION)RtlImageDirectoryEntryToData(
                                            NewBase,
                                            TRUE,
                                            IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                            &TotalCountBytes
                                            );

   //
   // It is possible for a file to have no relocations, but the relocations
   // must not have been stripped.
   //
    if (!NextBlock || !TotalCountBytes) {
        if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
            Status = STATUS_CONFLICTING_ADDRESSES;
        } else {
            Status = STATUS_SUCCESS;
        }
        goto Exit;
    }

    //
    // If the image has a relocation table, then apply the specified fixup
    // information to the image.
    //
    Diff = (LONGLONG)((ULONG_PTR)NewBase - OldBase + AdditionalBias);
    while (TotalCountBytes) {

        SizeOfBlock = NextBlock->SizeOfBlock;

        if (!SizeOfBlock) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }

        TotalCountBytes -= SizeOfBlock;
        SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
        SizeOfBlock /= sizeof(USHORT);
        NextOffset = (USHORT*)((ULONG_PTR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

        VA = (PVOID)((ULONG_PTR)NewBase + NextBlock->VirtualAddress);
        NextBlock = LdrpProcessRelocationBlockLongLong(VA, SizeOfBlock, NextOffset, Diff);

        if (!NextBlock) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

static
NTSTATUS
LdrpFindMappedImage(
    IN PEPROCESS Process,
    IN PUNICODE_STRING ImageName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo
)
{
    if (!Process || !ImageName || !ImageInfo) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Try to find the module.
    //
    if (!LdrGetModuleBaseAddressUnicodeString(Process, ImageName, ImageInfo)) {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

typedef struct _LDRP_APC_CONTEXT {
    KAPC Apc;
    PVOID SystemArgument3;
    PKEVENT QueuedEvent;
} LDRP_APC_CONTEXT, *PLDRP_APC_CONTEXT;

static
VOID
LdrpUserApcKernelRoutine(
    IN PRKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
)
{
    PLDRP_APC_CONTEXT ApcContext = (PLDRP_APC_CONTEXT)Apc;

    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    LOG_DEBUG("Executing User-Mode APC...");
    LOG_DEBUG("NormalRoutine = %p", *NormalRoutine);
    LOG_DEBUG("NormalContext = %p", *NormalContext);
    LOG_DEBUG("SystemArgument1 = %p", *SystemArgument1);
    LOG_DEBUG("SystemArgument2 = %p", *SystemArgument2);

    //
    // Determine if this is a wow64 process or not.
    //
    if (PsGetCurrentProcessWow64Process()) {

        //
        // If so, fix Wow64 APC.
        //
        PsWrapApcWow64Thread(NormalContext, (PVOID*)NormalRoutine);
    }

    //
    // Signal the event that indicates the APC was queued.
    //
    if (ApcContext->QueuedEvent != NULL) {
        KeSetEvent(ApcContext->QueuedEvent, EVENT_INCREMENT, FALSE);
    }

    //
    // Free the user mode APC context.
    //
    MmFreeNonPaged(Apc);
}

static
VOID
LdrpKernelApcKernelRoutine(
    IN PRKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
)
{
    LARGE_INTEGER AlertableTimeout;
    PLDRP_APC_CONTEXT ApcContext;
    PVOID SystemArgument3;

    UNREFERENCED_PARAMETER(NormalRoutine);

    ApcContext = (PLDRP_APC_CONTEXT)Apc;
    SystemArgument3 = ApcContext->SystemArgument3;

    LOG_DEBUG("Forcing User-Mode APC execution...");
    LOG_DEBUG("NormalRoutine = %p", *NormalRoutine);
    LOG_DEBUG("NormalContext = %p", *NormalContext);
    LOG_DEBUG("SystemArgument1 = %p", *SystemArgument1);
    LOG_DEBUG("SystemArgument2 = %p", *SystemArgument2);
    LOG_DEBUG("SystemArgument3 = %p", SystemArgument3);

    //
    // Re-use the same APC structure to initialize the user APC.
    //
    KeInitializeApc(&ApcContext->Apc,
                    PsGetCurrentThread(),
                    OriginalApcEnvironment,
                    (PKKERNEL_ROUTINE)LdrpUserApcKernelRoutine,
                    NULL,
                    (PKNORMAL_ROUTINE)*NormalContext,
                    UserMode,
                    *SystemArgument1
                    );

    //
    // Insert the user mode APC.
    //
    if (!KeInsertQueueApc(&ApcContext->Apc, *SystemArgument2, SystemArgument3, IO_NO_INCREMENT)) {
    
        //
        // If a failure occured trying to insert the APC, free the APC context here.
        //
        MmFreeNonPaged(ApcContext);
    }

    //
    // Small wait in usermode to cause the thread to become alertable by user mode APC.
    //
    AlertableTimeout.QuadPart = RELATIVE(MICROSECONDS(1));
    KeDelayExecutionThread(UserMode, TRUE, &AlertableTimeout);
}

static
VOID
LdrpKernelApcNopNormalRoutine(
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

static
NTSTATUS
LdrpForceUserApc(
    IN PETHREAD Thread,
    IN PKEVENT QueuedEvent,
    IN PVOID StartAddress,
    IN PVOID SystemArgument1 OPTIONAL,
    IN PVOID SystemArgument2 OPTIONAL,
    IN PVOID SystemArgument3 OPTIONAL
)
{
    PLDRP_APC_CONTEXT KernelApcContext;

    if (!Thread || !StartAddress) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Dynamically allocate the APC.
    //
    KernelApcContext = (PLDRP_APC_CONTEXT)MmAllocateNonPaged(sizeof(LDRP_APC_CONTEXT));
    if (!KernelApcContext) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Set the additional context parameter and the queued event.
    //
    KernelApcContext->SystemArgument3 = SystemArgument3;
    KernelApcContext->QueuedEvent = QueuedEvent;

    //
    // Initialize the kernel APC.
    //
    KeInitializeApc(&KernelApcContext->Apc,
                    Thread,
                    OriginalApcEnvironment,
                    (PKKERNEL_ROUTINE)LdrpKernelApcKernelRoutine,
                    NULL,
                    (PKNORMAL_ROUTINE)LdrpKernelApcNopNormalRoutine,
                    KernelMode,
                    StartAddress
                    );

    //
    // Attempt to insert this APC into queue.
    //
    if (!KeInsertQueueApc(&KernelApcContext->Apc,
                          SystemArgument1,
                          SystemArgument2,
                          IO_NO_INCREMENT)) {

        MmFreeNonPaged(KernelApcContext);
        return STATUS_NOT_CAPABLE;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
LdrpQueueUserApc(
    IN PETHREAD Thread,
    IN PKEVENT QueuedEvent OPTIONAL,
    IN PVOID StartAddress,
    IN PVOID SystemArgument1 OPTIONAL,
    IN PVOID SystemArgument2 OPTIONAL,
    IN PVOID SystemArgument3 OPTIONAL
)
{
    PLDRP_APC_CONTEXT KernelApcContext;

    if (!Thread || !StartAddress) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Dynamically allocate the APC.
    //
    KernelApcContext = (PLDRP_APC_CONTEXT)MmAllocateNonPaged(sizeof(LDRP_APC_CONTEXT));
    if (!KernelApcContext) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Set the additional context parameter and completion event.
    //
    KernelApcContext->SystemArgument3 = SystemArgument3;
    KernelApcContext->QueuedEvent = QueuedEvent;

    //
    // Initialize the kernel APC.
    //
    KeInitializeApc(&KernelApcContext->Apc,
                    Thread,
                    OriginalApcEnvironment,
                    (PKKERNEL_ROUTINE)LdrpUserApcKernelRoutine,
                    NULL,
                    (PKNORMAL_ROUTINE)StartAddress,
                    UserMode,
                    SystemArgument1
                    );

    //
    // Attempt to insert this APC into queue.
    //
    if (!KeInsertQueueApc(&KernelApcContext->Apc,
                          SystemArgument2,
                          SystemArgument3,
                          IO_NO_INCREMENT)) {
        MmFreeNonPaged(KernelApcContext);
        return STATUS_NOT_CAPABLE;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
LdrpCallInitRoutineInNewThread(
    IN PEPROCESS Process,
    IN PDLL_INIT_ROUTINE InitRoutine,
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
)
{
    NTSTATUS Status;
    PVOID Peb;
    BOOLEAN Wow64;
    PVOID CodeBase;
    SIZE_T CodeSize;
    HCODEGEN CodeGenerator;
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LARGE_INTEGER WaitTime;
    KPROCESSOR_MODE PreviousMode;

    //
    // Determine if this is wow64 process or not.
    //
    Peb = PsGetProcessWow64Process(Process);
    if (Peb) {
        Wow64 = TRUE;
    } else {
        Peb = PsGetProcessPeb(Process);
        if (!Peb) {
            return STATUS_UNSUCCESSFUL;
        }
        Wow64 = FALSE;
    }

    //
    // Set shellcode defaults.
    //
    CodeBase = NULL;
    CodeSize = PAGE_SIZE;

    //
    // Allocate shellcode buffer for executing our stub.
    //
    Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                     &CodeBase,
                                     0,
                                     &CodeSize,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {
        BO_DBG_BREAK();
        return Status;
    }

    //
    // Initialize a code generator object for our shellcode.
    //
    Status = CgInitializeGenerator(&CodeGenerator, CodeBase, CodeSize, !Wow64);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to initialize shellcode generator with status %08x", Status);
        goto Exit;
    }

    //
    // Build the shellcode logic.
    //
    // Make sure to generate according to the thread routine (USER_THREAD_START_ROUTINE)!
    //
    CgBeginCode(CodeGenerator, FALSE);

    CgPushPointer(CodeGenerator, DllHandle);
    CgPushUInt32(CodeGenerator, Reason);
    CgPushPointer(CodeGenerator, Context);
    CgCall(CodeGenerator, CC_STDCALL, (PVOID)InitRoutine);

    CgEndCode(CodeGenerator);
    CgReturn(CodeGenerator, 3 * (Wow64 ? sizeof(ULONG) : 0));

    //
    // Free the generator resources.
    //
    CgDestroyGenerator(CodeGenerator);

    //
    // Create thread in remote process.
    //
    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    Status = PsCreateThreadEx(&ThreadHandle,
                              THREAD_ALL_ACCESS,
                              &ObjectAttributes,
                              NtCurrentProcess(),
                              (PUSER_THREAD_START_ROUTINE)CodeBase,
                              NULL,
                              0,
                              0,
                              PAGE_SIZE,
                              10 * PAGE_SIZE,
                              NULL
                              );

    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to create new thread with status %08x", Status);
        goto Exit;
    }

    //
    // Wait for the thread to complete, with a 10 second timeout.
    //
    WaitTime.QuadPart = RELATIVE(SECONDS(10));

    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
    Status = NtWaitForSingleObject(ThreadHandle, TRUE, &WaitTime);
    PS_PREVIOUS_MODE_RESTORE(PreviousMode);

    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to wait for new thread with status %08x", Status);
    }

    //
    // Close thread handle so there are no open handles to it.
    //
    PsCloseThreadHandle(ThreadHandle);

Exit:
    //
    // Free the shellcode memory. 
    //
    CodeSize = 0;
    PsFreeVirtualMemory(NtCurrentProcess(), &CodeBase, &CodeSize, MEM_RELEASE);

    return Status;
}

static
NTSTATUS
LdrpCallInitRoutineInExistingThread(
    IN PEPROCESS Process,
    IN PDLL_INIT_ROUTINE InitRoutine,
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
)
{
    NTSTATUS Status;

    Status = LdrpExecuteShellcode(Process,
                                  FALSE,
                                  TRUE,
                                  (PUSER_SHELLCODE_START_ROUTINE)InitRoutine,
                                  DllHandle,
                                  (PVOID)Reason,
                                  Context
                                  );
    return Status;
}

static
NTSTATUS
LdrpCallInitRoutine(
    IN PEPROCESS Process,
    IN BOOLEAN NewThread,
    IN PDLL_INIT_ROUTINE InitRoutine,
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
)
{
    NTSTATUS Status;

    if (NewThread) {

        //
        // Create a new thread.
        //
        Status = LdrpCallInitRoutineInNewThread(
                                        Process,
                                        InitRoutine,
                                        DllHandle,
                                        Reason,
                                        Context
                                        );
    } else {

        //
        // Use an existing thread.
        //
        Status = LdrpCallInitRoutineInExistingThread(
                                        Process,
                                        InitRoutine,
                                        DllHandle,
                                        Reason,
                                        Context
                                        );
    }

    return Status;
}

NTSTATUS
LDRAPI
LdrCallTlsInitializers(
    IN PEPROCESS Process,
    IN BOOLEAN NewThread,
    IN PVOID ImageBase,
    IN ULONG Reason
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG TlsSize;
    PIMAGE_TLS_DIRECTORY TlsImage;
    ULONG *CallbackArray32;
    ULONGLONG *CallbackArray64;
    PDLL_INIT_ROUTINE InitRoutine;

    if (!ImageBase) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Grab the PE header.
    //
    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, ImageBase, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Get the import directory
    //
    TlsImage = (PIMAGE_TLS_DIRECTORY)RtlImageDirectoryEntryToData(
                                        ImageBase,
                                        TRUE,
                                        IMAGE_DIRECTORY_ENTRY_TLS,
                                        &TlsSize
                                        );
    //
    // Check if any TLS initializers exist.
    //
    if (!TlsImage) {
        return STATUS_SUCCESS;
    }

    //
    // Determine what architecture we should get the callback array for.
    //
    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

        CallbackArray64 = (ULONGLONG *)((PIMAGE_TLS_DIRECTORY64)TlsImage)->AddressOfCallBacks;

        //
        // Check that callbacks exists.
        //
        if (!CallbackArray64) {
            return STATUS_NOT_FOUND;
        }

        //
        // Call each TLS initializer.
        //
        while (*CallbackArray64) {

            InitRoutine = (PDLL_INIT_ROUTINE)*CallbackArray64++;

            Status = LdrpCallInitRoutine(Process,
                                         NewThread,
                                         InitRoutine,
                                         ImageBase,
                                         Reason,
                                         NULL
                                         );
            if (!NT_SUCCESS(Status)) {
                BO_DBG_BREAK();
                return Status;
            }
        }

    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

        CallbackArray32 = (ULONG *)((PIMAGE_TLS_DIRECTORY32)TlsImage)->AddressOfCallBacks;

        //
        // Check that callbacks exists.
        //
        if (!CallbackArray32) {
            return STATUS_NOT_FOUND;
        }

        //
        // Call each TLS initializer.
        //
        while (*CallbackArray32) {

            InitRoutine = (PDLL_INIT_ROUTINE)*CallbackArray32++;

            Status = LdrpCallInitRoutine(Process,
                                         NewThread,
                                         InitRoutine,
                                         ImageBase,
                                         Reason,
                                         NULL
                                         );
            if (!NT_SUCCESS(Status)) {
                BO_DBG_BREAK();
                return Status;
            }
        }

    } else {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LDRAPI
LdrForEachTlsInitializer(
    IN PEPROCESS Process,
    IN PLDR_MAPPED_IMAGE_INFO ImageInfo,
    IN PLDR_FOR_EACH_TLS_INITIALIZER_CALLBACK ForEachCallback,
    IN PVOID SystemArgument OPTIONAL
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG TlsSize;
    PIMAGE_TLS_DIRECTORY TlsImage;
    ULONG *CallbackArray32;
    ULONGLONG *CallbackArray64;
    PLDR_INIT_ROUTINE InitRoutine;

    if (!ImageInfo || !ForEachCallback) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Grab the PE header.
    //
    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK,
                                ImageInfo->ImageBase,
                                ImageInfo->SizeOfImage,
                                &NtHeaders
                                );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Get the import directory
    //
    TlsImage = (PIMAGE_TLS_DIRECTORY)RtlImageDirectoryEntryToData(
                                        ImageInfo->ImageBase,
                                        TRUE,
                                        IMAGE_DIRECTORY_ENTRY_TLS,
                                        &TlsSize
                                        );
    //
    // Check if any TLS initializers exist.
    //
    if (!TlsImage) {
        return STATUS_SUCCESS;
    }

    //
    // Determine what architecture we should get the callback array for.
    //
    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

        CallbackArray64 = (ULONGLONG *)((PIMAGE_TLS_DIRECTORY64)TlsImage)->AddressOfCallBacks;

        //
        // Check that callbacks exists.
        //
        if (!CallbackArray64) {
            return STATUS_NOT_FOUND;
        }

        //
        // Call each TLS initializer.
        //
        while (*CallbackArray64) {

            InitRoutine = (PLDR_INIT_ROUTINE)*CallbackArray64++;

            Status = ForEachCallback(Process, InitRoutine, ImageInfo, SystemArgument);

            if (!NT_SUCCESS(Status)) {
                BO_DBG_BREAK();
                return Status;
            }
        }

    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

        CallbackArray32 = (ULONG *)((PIMAGE_TLS_DIRECTORY32)TlsImage)->AddressOfCallBacks;

        //
        // Check that callbacks exists.
        //
        if (!CallbackArray32) {
            return STATUS_NOT_FOUND;
        }

        //
        // Call each TLS initializer.
        //
        while (*CallbackArray32) {

            InitRoutine = (PLDR_INIT_ROUTINE)*CallbackArray32++;

            Status = ForEachCallback(Process, InitRoutine, ImageInfo, SystemArgument);

            if (!NT_SUCCESS(Status)) {
                BO_DBG_BREAK();
                return Status;
            }
        }

    } else {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
LdrpExecuteShellcode(
    IN PEPROCESS Process,
    IN BOOLEAN NewThread,
    IN BOOLEAN Wait,
    IN PUSER_SHELLCODE_START_ROUTINE StartRoutine,
    IN PVOID Parameter1 OPTIONAL,
    IN PVOID Parameter2 OPTIONAL,
    IN PVOID Parameter3 OPTIONAL
)
{
    NTSTATUS Status;
    LARGE_INTEGER WaitTime;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PETHREAD Thread;
    HANDLE ThreadHandle;
    ULONG Tries;
    BOOLEAN NeedsDereference;
    KPROCESSOR_MODE PreviousMode;
    KEVENT QueuedEvent;
    PKEVENT QueuedEventPtr;

    if (NewThread) {
    
        InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
        Status = PsCreateThreadEx(&ThreadHandle,
                                  THREAD_ALL_ACCESS,
                                  &ObjectAttributes,
                                  NtCurrentProcess(),
                                  (PUSER_THREAD_START_ROUTINE)StartRoutine,
                                  Parameter1,
                                  0,
                                  0,
                                  PAGE_SIZE,
                                  256 * PAGE_SIZE,
                                  NULL
                                  );

        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to create new thread for shellcode with status %08x", Status);
            return Status;
        }

        //
        // Wait for the thread to complete, with a 30 second timeout.
        //
        if (!Wait) {

            LOG_INFO("Successfully executed shellcode on new thread");

        } else {

            WaitTime.QuadPart = RELATIVE(SECONDS(30));

            PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
            Status = NtWaitForSingleObject(ThreadHandle, TRUE, &WaitTime);
            PS_PREVIOUS_MODE_RESTORE(PreviousMode);

            if (Status == STATUS_SUCCESS) {
                LOG_INFO("Successfully executed shellcode on new thread");
            } else {
                LOG_ERROR("Failed to wait for new thread with status %08x", Status);
            }
        }

        //
        // Close thread handle so there are no open handles to it.
        //
        PsCloseThreadHandle(ThreadHandle);
        
    } else {

        NeedsDereference = FALSE;
        if (Wait) {
            QueuedEventPtr = &QueuedEvent;
            KeInitializeEvent(QueuedEventPtr, NotificationEvent, FALSE);
        } else {
            QueuedEventPtr = NULL;
        }
    
        //
        // Try to find a suitable thread to hijack.
        //
        Tries = 0;
    tryFindThread:
        //
        // Attempt to find a hijackable thread.
        //
        Status = PsFindHijackableThread(Process, FALSE, &ThreadHandle);
        if (!NT_SUCCESS(Status)) {

            //
            // Give it 10 tries before giving up.
            //
            if (++Tries > 10) {
                LOG_ERROR("Failed to find hijackable thread after 10 tries with status %08x",
                          Status);
                return Status;
            }

            //
            // Wait 100 milliseconds before giving it another try.
            //
            WaitTime.QuadPart = RELATIVE(MILLISECONDS(100));
            KeDelayExecutionThread(KernelMode, TRUE, &WaitTime);
            goto tryFindThread;
        }

        //
        // Object a pointer to the thread object for the target thread to hijack. 
        //
        Status = PsLookupThreadByThreadId(ThreadHandle, &Thread);
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to obtain thread object for thread id %d",
                (ULONG)(ULONG_PTR)ThreadHandle);
            return Status;
        }

        NeedsDereference = TRUE;

        //
        // Force an APC on the specified thread.
        //
        Status = LdrpForceUserApc(Thread,
                                  QueuedEventPtr,
                                  (PVOID)StartRoutine,
                                  Parameter1,
                                  Parameter2,
                                  Parameter3
                                  );

        if (!NT_SUCCESS(Status)) {
            LOG_ERROR("Failed to queued user apc for shellcode on thread id %d with status %08x",
                (ULONG)(ULONG_PTR)PsGetThreadId(Thread), Status);
        }

        //
        // Wait for the queue event (if event was supplied).
        //
        if (QueuedEventPtr) {

            WaitTime.QuadPart = RELATIVE(SECONDS(30));
            Status = KeWaitForSingleObject(QueuedEventPtr, Executive, UserMode, TRUE, &WaitTime);
            if (Status == STATUS_SUCCESS) {
                LOG_INFO("Successfully executed shellcode on existing thread");
            } else {
                LOG_ERROR("Failed waiting for the user apc to queue on thread id %d with status %08x",
                    (ULONG)(ULONG_PTR)PsGetThreadId(Thread), Status);
            }
        }

        //
        // Dereference thread object referenced by PsLookupThreadByThreadId.
        //
        if (NeedsDereference) {
            ObDereferenceObject(Thread);
        }
    }
    
    return Status;
}

static
NTSTATUS
LdrpMapImage(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    IN SIZE_T ImageSize OPTIONAL,
    IN BOOLEAN RunInitializers,
    OUT PLDR_MAPPED_IMAGE_INFO LoadedImageInfo
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    SIZE_T SizeOfHeaders;
    SIZE_T SizeOfImage;
    PVOID TargetBase;
    PDLL_INIT_ROUTINE EntryPointRoutine;

    //
    // Validate the PE header.
    //
    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, ImageBase, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // Determine the size and target image base of this image.
    //
    switch (NtHeaders->OptionalHeader.Magic) {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:     
        TargetBase = (PVOID)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        TargetBase = (PVOID)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;
        break;
    default:
        return STATUS_INVALID_IMAGE_FORMAT;
    }
    SizeOfHeaders = (SIZE_T)NtHeaders->OptionalHeader.SizeOfHeaders;
    SizeOfImage = (SIZE_T)NtHeaders->OptionalHeader.SizeOfImage;

    //
    // Allocate memory for the image in the target process.
    //
    Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                     &TargetBase,
                                     0,
                                     &SizeOfImage,
                                     MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {

        //
        // If a failure occurred while attempting to allocate, try again 
        // with an arbitrary target image base.
        //
        TargetBase = NULL;
        Status = PsAllocateVirtualMemory(NtCurrentProcess(),
                                         &TargetBase,
                                         0,
                                         &SizeOfImage,
                                         MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE
                                         );
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    //
    // Map the image sections into the process.
    //
    LOG_DEBUG("Mapping image sections...");

    Status = LdrMapImageSections(TargetBase, ImageBase, ImageSize);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR(" Failed to map image sections with status %08x", Status);
        goto Exit;
    }

    //
    // Resolve the image relocations.
    //
    LOG_DEBUG("Processing image relocations...");

    Status = LdrRelocateImageWithBias(TargetBase, 0);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR(" Failed to relocate image with status %08x", Status);
        goto Exit;
    }

    //
    // Resolve the image imports.
    //
    LOG_DEBUG("Resolving image import references...");

    Status = LdrResolveImageImports(Process, TargetBase);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to resolve image import references with status %08x", Status);
        goto Exit;
    }

    //
    // Set the appropriate section protection values.
    //
    LOG_DEBUG("Protecting image sections...");

    Status = LdrProtectImageSections(Process, TargetBase, ImageSize, FALSE);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR(" Failed to protect image sections with status %08x", Status);
        goto Exit;
    }

    //
    // See if we should run the image initializers.
    //
    if (NtHeaders->OptionalHeader.AddressOfEntryPoint) {
        EntryPointRoutine = (PDLL_INIT_ROUTINE)((ULONG_PTR)TargetBase +
                                                NtHeaders->OptionalHeader.AddressOfEntryPoint);
    } else {
        EntryPointRoutine = NULL;
    }
    
    if (RunInitializers) {

        //
        // Execute the TLS callback entry points.
        //
        LOG_DEBUG("Executing TLS initializers...");
        
        Status = LdrCallTlsInitializers(Process, TRUE, TargetBase, LDR_DLL_PROCESS_ATTACH);
        if (!NT_SUCCESS(Status)) {
            LOG_ERROR(" Failed to execute TLS initializers with status %08x", Status);
            goto Exit;
        }
        
        //
        // If no entry point exists, then skip executing an entry point.
        //
        if (EntryPointRoutine) {
        
            //
            // Execute the image entry point.
            //
            LOG_DEBUG("Executing image entry point...");
            
            Status = LdrpCallInitRoutine(Process,
                                         FALSE,
                                         EntryPointRoutine,
                                         TargetBase,
                                         LDR_DLL_PROCESS_ATTACH,
                                         NULL
                                         );
            if (!NT_SUCCESS(Status)) {
                LOG_ERROR(" Failed to execute image entry point with status %08x", Status);
                goto Exit;
            }
        }       
    }

    //
    // Return some loaded image information.
    //
    LoadedImageInfo->ImageBase = TargetBase;
    LoadedImageInfo->SizeOfImage = (ULONG)SizeOfImage;
    LoadedImageInfo->EntryPoint = (PVOID)EntryPointRoutine;
    LoadedImageInfo->Flags = 0;

Exit:
    //
    // If the image failed to map, free the allocated target image data.
    //
    if (!NT_SUCCESS(Status)) {
        SizeOfImage = 0;
        PsFreeVirtualMemory(NtCurrentProcess(), &TargetBase, &SizeOfImage, MEM_RELEASE);
    }

    return Status;
}

NTSTATUS
LDRAPI
LdrFindOrMapImage(
    IN PEPROCESS Process,
    IN PVOID ImageBase OPTIONAL,
    IN PUNICODE_STRING ImagePath OPTIONAL,
    IN BOOLEAN RunInitializers,
    OUT PLDR_MAPPED_IMAGE_INFO LoadedImageInfo OPTIONAL
)
{
    NTSTATUS Status;
    PUNICODE_STRING Path;
    UNICODE_STRING Name;
    UNICODE_STRING ResolvedPath;
    WCHAR ResolvedPathBuffer[MAXIMUM_FILENAME_LENGTH];
    LARGE_INTEGER FileSize;
    ULONG BytesRead;
    LDR_MAPPED_IMAGE_INFO ImageInfo;
    HANDLE FileHandle = NULL;
    PVOID FileBuffer = NULL;  

    //
    // Using this stack pointer avoids the dumb compiler warning 4701.
    //
    RtlZeroMemory(&ImageInfo, sizeof(LDR_MAPPED_IMAGE_INFO));
    ImageInfo.ImageNameBuffer[0] = L'\0';
    ImageInfo.ImageName.Length = sizeof(WCHAR);
    ImageInfo.ImageName.MaximumLength = sizeof(ImageInfo.ImageNameBuffer);
    ImageInfo.ImageName.Buffer = ImageInfo.ImageNameBuffer;

    //
    // Check if image is already loaded.
    //
    if (!ImageBase) {

        LOG_DEBUG("Attempting to find or map \"%wZ\"", ImagePath);

        //
        // Resolve the given image path.
        //
        RtlInitEmptyUnicodeString(&ResolvedPath, ResolvedPathBuffer, sizeof(ResolvedPathBuffer));
        Status = LdrResolveImagePath(Process, ImagePath, NULL, &ResolvedPath);
        if (NT_SUCCESS(Status) && ResolvedPath.Length > 2) {

            //
            // Use the resolved image path.
            //
            Path = &ResolvedPath;

        } else {

            //
            // Use the given ImagePath parameter.
            //
            Path = ImagePath;
        }

        //
        // Extract the name from the resolved path.
        //
        UtlExtractFileName(Path, &Name);

        //
        // Check if image is already loaded, if a path was given.
        //
        Status = LdrpFindMappedImage(Process, &Name, &ImageInfo);
        if (NT_SUCCESS(Status)) {
            goto Exit;
        }

        //
        // Set the resolved image name in the image info.
        //
        RtlCopyUnicodeString(&ImageInfo.ImageName, &Name);

        //
        // Load image data from disk.
        //
        Status = FileOpen(Path,
                          SYNCHRONIZE | GENERIC_READ | FILE_READ_DATA,
                          FILE_SHARE_READ,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          &FileHandle
                          );

        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

        //
        // Get the size of image on disk.
        //
        Status = FileGetSize(FileHandle, &FileSize);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

        //
        // Allocate space in kernel for image data.
        //
        FileBuffer = MmAllocatePaged((SIZE_T)FileSize.LowPart);
        if (!FileBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }

        //
        // Read image data into kernel file buffer.
        //
        Status = FileRead(FileHandle, 0, FileBuffer, FileSize.LowPart, &BytesRead);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

        //
        // Close the file handle and indicate we are done with it.
        //
        FileClose(FileHandle);
        FileHandle = NULL;

        //
        // Set the kernel image data base.
        //
        ImageBase = FileBuffer;

    } else {

        //
        // Missing image path. The path is mandatory!
        //
        if (!ImagePath) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        LOG_DEBUG("Attempting to map image memory at 0x%p", ImageBase);

        //
        // No image size data.
        //
        FileSize.QuadPart = 0;

        //
        // Use the provided image path.
        //
        Path = ImagePath;
    }

    //
    // Finally, let's map the image!
    //
    Status = LdrpMapImage(Process,
                          ImageBase,
                          (SIZE_T)FileSize.LowPart,
                          RunInitializers,
                          &ImageInfo
                          );

    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to map image with status %08x", Status);
        goto Exit;
    }

    LOG_DEBUG("Image \"%wZ\" was mapped successfully at address 0x%p", Path, ImageInfo.ImageBase);

Exit:
    //
    // Return the resulting loaded image info.
    //
    if (ARGUMENT_PRESENT(LoadedImageInfo)) {

        LoadedImageInfo->ImageBase = ImageInfo.ImageBase;
        LoadedImageInfo->EntryPoint = ImageInfo.EntryPoint;
        LoadedImageInfo->SizeOfImage = ImageInfo.SizeOfImage;
        LoadedImageInfo->Flags = ImageInfo.Flags;

        LoadedImageInfo->ImageName.Length = ImagePath->Length;
        LoadedImageInfo->ImageName.MaximumLength = sizeof(LoadedImageInfo->ImageNameBuffer);
        LoadedImageInfo->ImageName.Buffer = LoadedImageInfo->ImageNameBuffer;
        RtlCopyMemory(LoadedImageInfo->ImageName.Buffer, ImagePath->Buffer, ImagePath->Length);
        LoadedImageInfo->ImageName.Buffer[ImagePath->Length / sizeof(WCHAR)] = L'\0';
    }

    //
    // Close the file handle if its open.
    //
    if (FileHandle != NULL) {
        FileClose(FileHandle);
    }

    //
    // Free the file buffer.
    //
    if (FileBuffer != NULL) {
        MmFreePaged(FileBuffer);
    }

    return Status;
}

NTSTATUS
LDRAPI
LdrFreeImageMemory(
    IN PLDR_MAPPED_IMAGE_INFO ImageInfo
)
{
    NTSTATUS Status;
    PVOID BaseAddress;
    SIZE_T RegionSize;

    if (!ImageInfo->ImageBase) {
        return STATUS_INVALID_PARAMETER;
    }

    BaseAddress = ImageInfo->ImageBase;
    RegionSize = 0;
    Status = PsFreeVirtualMemory(NtCurrentProcess(), &BaseAddress, &RegionSize, MEM_RELEASE);

    ImageInfo->ImageBase = BaseAddress;

    return Status;
}