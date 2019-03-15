/**
 * Blackout Driver
 * Copyright (c) 2018 Archetype Entertainment Private Limited. All rights reserved.
 *
 * @file loader.h
 * @author Aidan Khoury (dude719)
 * @date 9/4/2018
 */

#ifndef _BLACKOUT_DRIVER_LOADER_H_
#define _BLACKOUT_DRIVER_LOADER_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable:4214) // nonstandard extension used: bit field types other than int
#endif


// Ldr API calling convention
#define LDRAPI  NTAPI

//
// These are how you currently pass implicit path flags to LdrLoadDll.
//
#define LDR_PATH_FLAGS(x)               ((WCHAR*)(((ULONG_PTR)(ULONG)(x)) | (ULONG_PTR)1))
#define LDR_IS_PATH_FLAGS(x)            (((ULONG)(ULONG_PTR)(x)) & (ULONG)1)
#define LDR_GET_PATH_FLAGS(x)           ((((ULONG)(ULONG_PTR)(x)) & ~(ULONG)1))

//
// Resource Name Mask.
//
#define LDR_RESOURCE_ID_NAME_MASK       ((~(ULONG_PTR)0) << 16) // lower 16bits clear
#define LDR_RESOURCE_ID_NAME_MINVAL     (( (ULONG_PTR)1) << 16) // 17th bit set

//
// These are how you currently pass the flag to FindResource.
//
// VIEW_TO_DATAFILE and DATAFILE_TO_VIEW are idempotent,
// so you can covert a datafile to a datafile with VIEW_TO_DATAFILE.
// Think of better names therefore..
//
#define LDR_VIEW_TO_DATAFILE(x)         ((PVOID)(((ULONG_PTR)(x)) | (ULONG_PTR)1))
#define LDR_IS_DATAFILE(x)              (((ULONG_PTR)(x)) & (ULONG_PTR)1)
#define LDR_IS_VIEW(x)                  (!LDR_IS_DATAFILE(x))
#define LDR_DATAFILE_TO_VIEW(x)         ((PVOID)(((ULONG_PTR)(x)) & ~(ULONG_PTR)1))

//
// These functions work on ULONG, ULONG_PTR, ULONG64, etc.
// They do not work on pointers.
//
#define LDR_VIEW_TO_DATAFILE_INTEGER(x) ((x) | 1)
#define LDR_IS_DATAFILE_INTEGER(x)      (((x) & 1) == 1)
#define LDR_IS_VIEW_INTEGER(x)          (((x) & 1) == 0)
#define LDR_DATAFILE_TO_VIEW_INTEGER(x) ((x) - ((x) & 1))

//
// These are flags to a function that doesn't yet exist:
//    LdrpSearchResourceSectionEx and/or LdrpSearchOutOfProcessResourceSection
//
#define LDRP_FIND_RESOURCE_DATA                     (0x00000000)
#define LDRP_FIND_RESOURCE_DIRECTORY                (0x00000002)

//
// Flags to LdrFindResourceEx/LdrpSearchResourceSection/LdrFindOutOfProcessResource.
//
#define LDR_FIND_RESOURCE_LANGUAGE_CAN_FALLBACK     (0x00000000)
#define LDR_FIND_RESOURCE_LANGUAGE_EXACT            (0x00000004)
#define LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION (0x00000008)


//
// DLL Entry Point Initializer Routine
//
typedef
BOOLEAN
(NTAPI *PLDR_INIT_ROUTINE)(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    );

#define LDR_DLL_PROCESS_ATTACH 1
#define LDR_DLL_THREAD_ATTACH  2
#define LDR_DLL_THREAD_DETACH  3
#define LDR_DLL_PROCESS_DETACH 0

//
// Loader DDAG Definitions
//

typedef enum _LDR_DDAG_STATE {
    LdrModulesMerged = -5,
    LdrModulesInitError = -4,
    LdrModulesSnapError = -3,
    LdrModulesUnloaded = -2,
    LdrModulesUnloading = -1,
    LdrModulesPlaceHolder = 0,
    LdrModulesMapping = 1,
    LdrModulesMapped = 2,
    LdrModulesWaitingForDependencies = 3,
    LdrModulesSnapping = 4,
    LdrModulesSnapped = 5,
    LdrModulesCondensed = 6,
    LdrModulesReadyToInit = 7,
    LdrModulesInitializing = 8,
    LdrModulesReadyToRun = 9
} LDR_DDAG_STATE;

typedef struct _LDR_SERVICE_TAG_RECORD {
    struct _LDR_SERVICE_TAG_RECORD *Next;
    ULONG ServiceTag;
} LDR_SERVICE_TAG_RECORD, *PLDR_SERVICE_TAG_RECORD;

typedef struct _LDRP_CSLIST {
    PSINGLE_LIST_ENTRY Tail;
} LDRP_CSLIST, *PLDRP_CSLIST;

typedef struct _LDRP_CSLIST32 {
    ULONG Tail; // PSINGLE_LIST_ENTRY32
} LDRP_CSLIST32, *PLDRP_CSLIST32;

typedef struct _LDR_DDAG_NODE {
    LIST_ENTRY Modules;                             // 0x00   
    PLDR_SERVICE_TAG_RECORD ServiceTagList;         // 0x10
    ULONG LoadCount;                                // 0x18
    union {
        ULONG ReferenceCount;                       // 0x1C
        ULONG LoadWhileUnloadingCount;              // 0x1C
    } u1;
    union {
        ULONG DependencyCount;                      // 0x20
        ULONG LowestLink;                           // 0x20
    } u2;
    union {
        LDRP_CSLIST Dependencies;                   // 0x28
        SINGLE_LIST_ENTRY RemovalLink;              // 0x28
    } u3;
    LDRP_CSLIST IncomingDependencies;               // 0x30
    LDR_DDAG_STATE State;                           // 0x38
    SINGLE_LIST_ENTRY CondenseLink;                 // 0x40
    ULONG PreorderNumber;                           // 0x48
} LDR_DDAG_NODE, *PLDR_DDAG_NODE;

typedef struct _LDR_DDAG_NODE32 {
    LIST_ENTRY32 Modules;                           // 0x00
    ULONG ServiceTagList;                           // 0x08 PLDR_SERVICE_TAG_RECORD
    ULONG LoadCount;                                // 0x0C
    union {
        ULONG ReferenceCount;                       // 0x10
        ULONG LoadWhileUnloadingCount;              // 0x10
    } u1;
    union {
        ULONG DependencyCount;                      // 0x14
        ULONG LowestLink;                           // 0x14
    } u2;
    union {
        LDRP_CSLIST32 Dependencies;                 // 0x18
        SINGLE_LIST_ENTRY32 RemovalLink;            // 0x18
    } u3;
    LDRP_CSLIST32 IncomingDependencies;             // 0x1C
    LDR_DDAG_STATE State;                           // 0x20
    SINGLE_LIST_ENTRY32 CondenseLink;               // 0x24
    ULONG PreorderNumber;                           // 0x28
} LDR_DDAG_NODE32, *PLDR_DDAG_NODE32;

//
// DLL Load Reasons
//

typedef enum _LDR_DLL_LOAD_REASON {
    LoadReasonStaticDependency,
    LoadReasonStaticForwarderDependency,
    LoadReasonDynamicForwarderDependency,
    LoadReasonDelayloadDependency,
    LoadReasonDynamicLoad,
    LoadReasonAsImageLoad,
    LoadReasonAsDataLoad,
    // winver >= rs3
    LoadReasonEnclavePrimary,
    LoadReasonEnclaveDependency,
    LoadReasonUnknown = -1
} LDR_DLL_LOAD_REASON, *PLDR_DLL_LOAD_REASON;

//
// Private flags for loader data table entries
//

#define LDRP_PACKAGED_BINARY                0x00000001
#define LDRP_STATIC_LINK                    0x00000002
#define LDRP_IMAGE_DLL                      0x00000004
#define LDRP_LOAD_NOTIFICATIONS_SENT        0x00000008
#define LDRP_TELEMETRY_ENTRY_PROCESSED      0x00000010
#define LDRP_PROCESS_STATIC_IMPORT          0x00000020
#define LDRP_IN_LEGACY_LISTS                0x00000040
#define LDRP_IN_INDEXES                     0x00000080
#define LDRP_SHIM_DLL                       0x00000100
#define LDRP_IN_EXCEPTION_TABLE             0x00000200
#define LDRP_LOAD_IN_PROGRESS               0x00001000
#define LDRP_UNLOAD_IN_PROGRESS             0x00002000
#define LDRP_ENTRY_PROCESSED                0x00004000
#define LDRP_PROTECT_DELAY_LOAD             0x00008000
#define LDRP_CURRENT_LOAD_RESERVED          0x00010000
#define LDRP_FAILED_BUILTIN_LOAD_RESERVED   0x00020000
#define LDRP_DONT_CALL_FOR_THREADS          0x00040000
#define LDRP_PROCESS_ATTACH_CALLED          0x00080000
#define LDRP_PROCESS_ATTACH_FAILED          0x00100000
#define LDRP_DEBUG_SYMBOLS_LOADED           0x00100000
#define LDRP_COR_DEFERRED_VALIDATE          0x00200000
#define LDRP_IMAGE_NOT_AT_BASE              0x00200000
#define LDRP_COR_IMAGE                      0x00400000
#define LDRP_DONT_RELOCATE                  0x00800000
#define LDRP_COR_OWNS_UNMAP                 0x00800000
#define LDRP_COR_IL_ONLY                    0x01000000
#define LDRP_SYSTEM_MAPPED                  0x01000000
#define LDRP_IMAGE_VERIFYING_RESERVED       0x02000000
#define LDRP_DRIVER_DEPENDENT_DLL           0x04000000
#define LDRP_ENTRY_NATIVE_RESERVED          0x08000000
#define LDRP_REDIRECTED                     0x10000000
#define LDRP_NON_PAGED_DEBUG_INFO_RESERVED  0x20000000
#define LDRP_MM_LOADED_RESERVED             0x40000000
#define LDRP_COMPAT_DATABASE_PROCESSED      0x80000000

#define LDR_DATA_TABLE_ENTRY_SIZE_WINXP FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, DdagNode)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN7  FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, BaseNameHashValue)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN8  FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, ImplicitPathOptions)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN10 sizeof(LDR_DATA_TABLE_ENTRY)

//
// Loader Data Table. Used to track DLLs loaded into an
// image.
//

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;                    // 0x00 0x00
    LIST_ENTRY InMemoryOrderLinks;                  // 0x10 0x08
    LIST_ENTRY InInitializationOrderLinks;          // 0x20 0x10
    PVOID DllBase;                                  // 0x30 0x18
    PVOID EntryPoint;                               // 0x38 0x1C
    ULONG SizeOfImage;                              // 0x40 0x20
    UNICODE_STRING FullDllName;                     // 0x48 0x24
    UNICODE_STRING BaseDllName;                     // 0x58 0x2C
    ULONG Flags;                                    // 0x68 0x34
    USHORT ObsoleteLoadCount;                       // 0x6C 0x38
    USHORT TlsIndex;                                // 0x6E 0x3A
    union {
        LIST_ENTRY HashLinks;                       // 0x70 0x3C
        struct {
            PVOID SectionPointer;                   // 0x70 0x3C
            ULONG CheckSum;                         // 0x70 0x3C
        };
    };
    union {
        ULONG TimeDateStamp;                        // 0x80 0x44
        PVOID LoadedImports;                        // 0x80 0x44
    };
    PVOID EntryPointActivationContext;              // 0x88 0x48
    PVOID Lock;                                     // 0x90 0x4C
    PLDR_DDAG_NODE DdagNode;                        // 0x98 0x50
    LIST_ENTRY NodeModuleLink;                      // 0xA0 0x54
    PVOID LoadContext;                              // 0xB0 0x5C
    PVOID ParentDllBase;                            // 0xB8 0x60
    PVOID SwitchBackContext;                        // 0xC0 0x64
    RTL_BALANCED_NODE BaseAddressIndexNode;         // 0xC8 0x68
    RTL_BALANCED_NODE MappingInfoIndexNode;         // 0xE0 0x74
    ULONGLONG OriginalBase;                         // 0xF8 0x80
    LARGE_INTEGER LoadTime;                         // 0x100 0x88
    ULONG BaseNameHashValue;                        // 0x108 0x90
    ULONG LoadReason;                               // 0x10C 0x94
    ULONG ImplicitPathOptions;                      // 0x110 0x98
    ULONG ReferenceCount;                           // 0x114 0x9C
    ULONG DependentLoadFlags;                       // 0x118 0xA0
    // winver >= rs2
    UCHAR SigningLevel;                             // 0x11C 0xA4
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
typedef const struct _LDR_DATA_TABLE_ENTRY *PCLDR_DATA_TABLE_ENTRY;

typedef struct _LDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;                  // 0x00
    LIST_ENTRY32 InMemoryOrderLinks;                // 0x08
    LIST_ENTRY32 InInitializationOrderLinks;        // 0x10
    ULONG DllBase;                                  // 0x18 PVOID
    ULONG EntryPoint;                               // 0x1C PVOID
    ULONG SizeOfImage;                              // 0x20
    UNICODE_STRING32 FullDllName;                   // 0x24
    UNICODE_STRING32 BaseDllName;                   // 0x2C
    ULONG Flags;                                    // 0x34
    USHORT LoadCount;                               // 0x38
    USHORT TlsIndex;                                // 0x3A
    union {
        LIST_ENTRY32 HashLinks;                     // 0x3C
        struct {
            ULONG SectionPointer;                   // 0x3C PVOID
            ULONG CheckSum;                         // 0x3C
        };
    };
    union {
        ULONG TimeDateStamp;                        // 0x44
        ULONG LoadedImports;                        // 0x44 PVOID
    };
    ULONG EntryPointActivationContext;              // 0x48 PACTIVATION_CONTEXT
    ULONG Lock;                                     // 0x4C PVOID
    ULONG DdagNode;                                 // 0x50 PLDR_DDAG_NODE
    LIST_ENTRY32 NodeModuleLink;                    // 0x54
    ULONG LoadContext;                              // 0x5C PVOID
    ULONG ParentDllBase;                            // 0x60 PVOID
    ULONG SwitchBackContext;                        // 0x64 PVOID
    RTL_BALANCED_NODE32 BaseAddressIndexNode;       // 0x68
    RTL_BALANCED_NODE32 MappingInfoIndexNode;       // 0x74
    ULONG OriginalBase;                             // 0x80 ULONGLONG
    LARGE_INTEGER LoadTime;                         // 0x88
    ULONG BaseNameHashValue;                        // 0x90
    ULONG LoadReason;                               // 0x94
    ULONG ImplicitPathOptions;                      // 0x98
    ULONG ReferenceCount;                           // 0x9C
    ULONG DependentLoadFlags;                       // 0xA0
    // winver >= rs2
    UCHAR SigningLevel;                             // 0xA4
} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;
typedef const struct _LDR_DATA_TABLE_ENTRY32 *PCLDR_DATA_TABLE_ENTRY32;

typedef struct _KLDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;                    // 0x00
    PVOID ExceptionTable;                           // 0x10
    ULONG ExceptionTableSize;                       // 0x18
    PVOID GpValue;                                  // 0x20
    PVOID NonPagedDebugInfo;                        // 0x28 PNON_PAGED_DEBUG_INFO
    PVOID ImageBase;                                // 0x30
    PVOID EntryPoint;                               // 0x38
    ULONG SizeOfImage;                              // 0x40
    UNICODE_STRING FullImageName;                   // 0x48
    UNICODE_STRING BaseImageName;                   // 0x58
    ULONG Flags;                                    // 0x68
    USHORT LoadCount;                               // 0x6C
    union {
        USHORT SignatureLevel : 4;
        USHORT SignatureType : 3;
        USHORT Unused : 9;
        USHORT EntireField;
    } u1;                                           // 0x6E
    PVOID SectionPointer;                           // 0x70
    ULONG CheckSum;                                 // 0x78
    ULONG CoverageSectionSize;                      // 0x7C
    PVOID CoverageSection;                          // 0x80
    PVOID LoadedImports;                            // 0x88
    PVOID Spare;                                    // 0x90
    ULONG SizeOfImageNotRounded;                    // 0x98
    ULONG TimeDateStamp;                            // 0x9C
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;

typedef struct _KLDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;                  // 0x00
    ULONG ExceptionTable;                           // 0x08
    ULONG ExceptionTableSize;                       // 0x0C
    ULONG GpValue;                                  // 0x10
    ULONG NonPagedDebugInfo;                        // 0x14 PNON_PAGED_DEBUG_INFO
    ULONG ImageBase;                                // 0x18
    ULONG EntryPoint;                               // 0x1C
    ULONG SizeOfImage;                              // 0x20
    UNICODE_STRING32 FullImageName;                 // 0x24
    UNICODE_STRING32 BaseImageName;                 // 0x2C
    ULONG Flags;                                    // 0x34
    USHORT LoadCount;                               // 0x38
    union {
        USHORT SignatureLevel : 4;
        USHORT SignatureType : 3;
        USHORT Unused : 9;
        USHORT EntireField;
    } u1;                                           // 0x3A
    ULONG SectionPointer;                           // 0x3C
    ULONG CheckSum;                                 // 0x40
    ULONG CoverageSectionSize;                      // 0x44
    ULONG CoverageSection;                          // 0x48
    ULONG LoadedImports;                            // 0x4C
    ULONG Spare;                                    // 0x50
    ULONG SizeOfImageNotRounded;                    // 0x54
    ULONG TimeDateStamp;                            // 0x58
} KLDR_DATA_TABLE_ENTRY32, *PKLDR_DATA_TABLE_ENTRY32;


//
// Module Information Definitions
//

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UINT8 FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[ANYSIZE_ARRAY];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef struct _RTL_PROCESS_MODULE_INFORMATION_EX {
    USHORT NextOffset;
    RTL_PROCESS_MODULE_INFORMATION BaseInfo;
    ULONG ImageChecksum;
    ULONG TimeDateStamp;
    PVOID DefaultBase;
} RTL_PROCESS_MODULE_INFORMATION_EX, *PRTL_PROCESS_MODULE_INFORMATION_EX;


//
// Module Resources Definitions
//

typedef struct _IMAGE_RESOURCE_DATA_ENTRY *PIMAGE_RESOURCE_DATA_ENTRY;
typedef struct _IMAGE_RESOURCE_DIRECTORY *PIMAGE_RESOURCE_DIRECTORY;
typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING *PIMAGE_RESOURCE_DIRECTORY_STRING;


//
// Imported Loader API
//

NTSYSAPI
NTSTATUS
NTAPI
LdrAccessResource(
    IN PVOID DllHandle,
    IN PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry,
    OUT PVOID *ResourceBuffer OPTIONAL,
    OUT UINT32 *ResourceLength OPTIONAL
    );

typedef struct _LDR_RESOURCE_INFO {
    UINT_PTR Type;
    UINT_PTR Name;
    UINT_PTR Language;
} LDR_RESOURCE_INFO, *PLDR_RESOURCE_INFO;

#define RESOURCE_TYPE_LEVEL 0
#define RESOURCE_NAME_LEVEL 1
#define RESOURCE_LANGUAGE_LEVEL 2
#define RESOURCE_DATA_LEVEL 3

NTSYSAPI
NTSTATUS
NTAPI
LdrFindResource_U(
    IN PVOID DllHandle,
    IN CONST UINT_PTR* ResourceInfo, // PLDR_RESOURCE_INFO
    IN ULONG Level,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrFindResourceEx_U(
    IN ULONG Flags,
    IN PVOID DllHandle,
    IN CONST UINT_PTR* ResourceIdPath, // PLDR_RESOURCE_INFO
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrFindResourceDirectory_U(
    IN PVOID DllHandle,
    IN CONST UINT_PTR *ResourceIdPath,
    IN ULONG Level,
    OUT PIMAGE_RESOURCE_DIRECTORY *ResourceDirectory
    );

typedef struct _LDR_ENUM_RESOURCE_ENTRY {
    union {
        ULONG_PTR NameOrId;
        PIMAGE_RESOURCE_DIRECTORY_STRING Name;
        struct {
            USHORT Id;
            USHORT NameIsPresent;
        };
    } Path[3];
    PVOID Data;
    ULONG Size;
    ULONG Reserved;
} LDR_ENUM_RESOURCE_ENTRY, *PLDR_ENUM_RESOURCE_ENTRY;

#define NAME_FROM_RESOURCE_ENTRY(RootDirectory, Entry) \
    ((Entry)->NameIsString ? ((ULONG_PTR)(RootDirectory) + (Entry)->NameOffset) : (Entry)->Id)

NTSYSAPI
NTSTATUS
NTAPI
LdrEnumResources(
    IN PVOID DllHandle,
    IN CONST UINT_PTR *ResourceIdPath,
    IN ULONG Level,
    IN OUT PULONG ResourceCount,
    OUT PLDR_ENUM_RESOURCE_ENTRY Resources OPTIONAL
    );


//
// Implemented Loader API in loader.c 
//

typedef struct _LDR_MAPPED_IMAGE_INFO {
    PVOID ImageBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    ULONG Flags;
    UNICODE_STRING ImageName;
    WCHAR ImageNameBuffer[MAXIMUM_FILENAME_LENGTH];
} LDR_MAPPED_IMAGE_INFO, *PLDR_MAPPED_IMAGE_INFO;


NTSTATUS
LDRAPI
LdrResolveImagePath(
    IN PEPROCESS Process,
    IN PUNICODE_STRING Path,
    IN PUNICODE_STRING BaseImage,
    OUT PUNICODE_STRING Resolved
    );

NTSTATUS
LDRAPI
LdrFindExportAddressForProcessAscii(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    IN CONST CHAR *Name,
    IN ULONG Ordinal,
    OUT PVOID *ResultExportAddress
    );

NTSTATUS
LDRAPI
LdrFindExportAddressForProcessUnicode(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    IN PCUNICODE_STRING Name,
    OUT PVOID *ResultExportAddress
    );

NTSTATUS
LDRAPI
LdrFindExportAddressAscii(
    IN PVOID ImageBase,
    IN CONST CHAR *Name,
    IN ULONG Ordinal,
    OUT PVOID *ResultExportAddress
    );

NTSTATUS
LDRAPI
LdrFindExportAddressUnicode(
    IN PVOID ImageBase,
    IN PCUNICODE_STRING Name,
    OUT PVOID *ResultExportAddress
    );

PVOID
LDRAPI
LdrGetSystemRoutineAddress(
    IN PLIST_ENTRY ModuleList,
    IN PCUNICODE_STRING SystemRoutineName
    );

// Now private
//PVOID
//LDRAPI
//LdrGetModuleBaseAddress(
//    IN PLIST_ENTRY ModuleList,
//    IN PCUNICODE_STRING ModuleName
//    );
//
//PVOID
//LDRAPI
//LdrGetModuleBaseAddressWoW64(
//    IN PLIST_ENTRY32 ModuleList,
//    IN PCUNICODE_STRING ModuleName
//    );

NTSTATUS
LDRAPI
LdrGetModuleNameUnicodeString(
    IN PEPROCESS Process,
    IN PVOID ImageBase,
    OUT PUNICODE_STRING ModuleName
    );

PVOID
LDRAPI
LdrGetModuleBaseAddressUnicodeString(
    IN PEPROCESS Process,
    IN PCUNICODE_STRING ModuleName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo OPTIONAL
    );

PVOID
LDRAPI
LdrGetModuleBaseAddressAscii(
    IN PEPROCESS Process,
    IN CONST CHAR* ModuleName,
    OUT PLDR_MAPPED_IMAGE_INFO ImageInfo OPTIONAL
    );

NTSTATUS
LDRAPI
LdrMapImageSections( 
    IN PVOID TargetBase, 
    IN PVOID ImageBase,
    IN SIZE_T ImageSize OPTIONAL
    );

NTSTATUS
LDRAPI
LdrProtectImageSections(
    IN PEPROCESS Process,
    IN PVOID NewBase,
    IN SIZE_T ImageSize OPTIONAL,
    IN BOOLEAN EraseHeaders
    );

NTSTATUS
LDRAPI
LdrResolveImageImports(
    IN PEPROCESS Process,
    IN PVOID ImageBase
    );

NTSTATUS
LDRAPI
LdrRelocateImageWithBias(
    IN PVOID NewBase,
    IN LONGLONG AdditionalBias
    );

NTSTATUS
LDRAPI
LdrCallTlsInitializers(
    IN PEPROCESS Process,
    IN BOOLEAN NewThread,
    IN PVOID ImageBase,
    IN ULONG Reason
    );

typedef
NTSTATUS
(NTAPI *PLDR_FOR_EACH_TLS_INITIALIZER_CALLBACK)(
    IN PEPROCESS Process,
    IN PLDR_INIT_ROUTINE TlsInitializerRoutine,
    IN PLDR_MAPPED_IMAGE_INFO ImageInfo,
    IN PVOID SystemArgument OPTIONAL
    );
NTSTATUS
LDRAPI
LdrForEachTlsInitializer(
    IN PEPROCESS Process,
    IN PLDR_MAPPED_IMAGE_INFO ImageInfo,
    IN PLDR_FOR_EACH_TLS_INITIALIZER_CALLBACK ForEachCallback,
    IN PVOID SystemArgument OPTIONAL
    );

NTSTATUS
LDRAPI
LdrFindOrMapImage(
    IN PEPROCESS Process,
    IN PVOID ImageBase OPTIONAL,
    IN PUNICODE_STRING ImagePath OPTIONAL,
    IN BOOLEAN RunInitializers,
    OUT PLDR_MAPPED_IMAGE_INFO LoadedImageInfo OPTIONAL
    );

NTSTATUS
LDRAPI
LdrFreeImageMemory(
    IN PLDR_MAPPED_IMAGE_INFO ImageInfo
    );

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#endif // _BLACKOUT_DRIVER_LOADER_H_