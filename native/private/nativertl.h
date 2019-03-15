#pragma once

#include "nativecommon.h"
#include "nativemm.h"
#include "nativeinfo.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < RTL Macros and Definitions >
///

#define NTRTLAPI NTSYSAPI

// Common
#define RTL_MAX_DRIVE_LETTERS                               32
#define RTL_DRIVE_LETTER_VALID                              ((USHORT)0x0001)

#define RTL_CONSTANT_OBJECT_ATTRIBUTES(n, a) { sizeof(OBJECT_ATTRIBUTES), NULL, n, a, NULL, NULL }
#define RTL_INIT_OBJECT_ATTRIBUTES(n, a) RTL_CONSTANT_OBJECT_ATTRIBUTES(n, a)

// String
#define RTL_CONSTANT_STRING(s) { sizeof(s) - sizeof((s)[0]), sizeof(s), s }

#define RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE         (0x00000001) 
#define RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING   (0x00000002)

#define HASH_STRING_ALGORITHM_DEFAULT                       (0)
#define HASH_STRING_ALGORITHM_X65599                        (1)
#define HASH_STRING_ALGORITHM_INVALID                       (0xFFFFFFFF)

#define RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END        (0x00000001)
#define RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET (0x00000002)
#define RTL_FIND_CHAR_IN_UNICODE_STRING_CASE_INSENSITIVE    (0x00000004)

// Process
#define RTL_USER_PROC_PARAMS_NORMALIZED                     (0x00000001)
#define RTL_USER_PROC_PROFILE_USER                          (0x00000002)
#define RTL_USER_PROC_PROFILE_KERNEL                        (0x00000004)
#define RTL_USER_PROC_PROFILE_SERVER                        (0x00000008)
#define RTL_USER_PROC_RESERVE_1MB                           (0x00000020)
#define RTL_USER_PROC_RESERVE_16MB                          (0x00000040)
#define RTL_USER_PROC_CASE_SENSITIVE                        (0x00000080)
#define RTL_USER_PROC_DISABLE_HEAP_DECOMMIT                 (0x00000100)
#define RTL_USER_PROC_DLL_REDIRECTION_LOCAL                 (0x00001000)
#define RTL_USER_PROC_APP_MANIFEST_PRESENT                  (0x00002000)
#define RTL_USER_PROC_IMAGE_KEY_MISSING                     (0x00004000)
#define RTL_USER_PROC_OPTIN_PROCESS                         (0x00020000)

#define RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED            (0x00000001)
#define RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES             (0x00000002)
#define RTL_CLONE_PROCESS_FLAGS_NO_SYNCHRONIZE              (0x00000004) // don't update synchronization objects

#define RTL_USER_PROCESS_PARAMETERS_NORMALIZED              (0x01)
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_USER            (0x02)
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_KERNEL          (0x04)
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_SERVER          (0x08)
#define RTL_USER_PROCESS_PARAMETERS_UNKNOWN                 (0x10)
#define RTL_USER_PROCESS_PARAMETERS_RESERVE_1MB             (0x20)
#define RTL_USER_PROCESS_PARAMETERS_RESERVE_16MB            (0x40)
#define RTL_USER_PROCESS_PARAMETERS_CASE_SENSITIVE          (0x80)
#define RTL_USER_PROCESS_PARAMETERS_DISABLE_HEAP_CHECKS     (0x100)
#define RTL_USER_PROCESS_PARAMETERS_PROCESS_OR_1            (0x200)
#define RTL_USER_PROCESS_PARAMETERS_PROCESS_OR_2            (0x400)
#define RTL_USER_PROCESS_PARAMETERS_PRIVATE_DLL_PATH        (0x1000)
#define RTL_USER_PROCESS_PARAMETERS_LOCAL_DLL_PATH          (0x2000)
#define RTL_USER_PROCESS_PARAMETERS_IMAGE_KEY_MISSING       (0x4000)
#define RTL_USER_PROCESS_PARAMETERS_NX                      (0x20000)

// Image
#define RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK          (0x00000001)

// Directory and paths
#define RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION (0x00000001)
#define RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH (0x00000002)
#define RTL_DOS_SEARCH_PATH_FLAG_APPLY_DEFAULT_EXTENSION_WHEN_NOT_RELATIVE_PATH_EVEN_IF_FILE_HAS_EXTENSION (0x00000004)

// Heap
#define RTL_HEAP_BUSY                                       ((USHORT)0x0001)
#define RTL_HEAP_SEGMENT                                    ((USHORT)0x0002)
#define RTL_HEAP_SETTABLE_VALUE                             ((USHORT)0x0010)
#define RTL_HEAP_SETTABLE_FLAG1                             ((USHORT)0x0020)
#define RTL_HEAP_SETTABLE_FLAG2                             ((USHORT)0x0040)
#define RTL_HEAP_SETTABLE_FLAG3                             ((USHORT)0x0080)
#define RTL_HEAP_SETTABLE_FLAGS                             ((USHORT)0x00E0)
#define RTL_HEAP_UNCOMMITTED_RANGE                          ((USHORT)0x0100)
#define RTL_HEAP_PROTECTED_ENTRY                            ((USHORT)0x0200)

#define HEAP_SETTABLE_USER_VALUE                            (0x00000100)
#define HEAP_SETTABLE_USER_FLAG1                            (0x00000200)
#define HEAP_SETTABLE_USER_FLAG2                            (0x00000400)
#define HEAP_SETTABLE_USER_FLAG3                            (0x00000800)
#define HEAP_SETTABLE_USER_FLAGS                            (0x00000E00)

#define HEAP_CLASS_0                                        (0x00000000) // Process heap
#define HEAP_CLASS_1                                        (0x00001000) // Private heap
#define HEAP_CLASS_2                                        (0x00002000) // Kernel heap
#define HEAP_CLASS_3                                        (0x00003000) // GDI heap
#define HEAP_CLASS_4                                        (0x00004000) // User heap
#define HEAP_CLASS_5                                        (0x00005000) // Console heap
#define HEAP_CLASS_6                                        (0x00006000) // User desktop heap
#define HEAP_CLASS_7                                        (0x00007000) // CSR shared heap
#define HEAP_CLASS_8                                        (0x00008000) // CSR port heap
#define HEAP_CLASS_MASK                                     (0x0000F000)

#define RTL_HEAP_MAKE_TAG HEAP_MAKE_TAG_FLAGS
#define HEAP_USAGE_ALLOCATED_BLOCKS HEAP_REALLOC_IN_PLACE_ONLY
#define HEAP_USAGE_FREE_BUFFER HEAP_ZERO_MEMORY

#define HeapDebuggingInformation                            (0x80000002)

// Error
#define RTL_ERRORMODE_NOGPFAULTERRORBOX                     (0x0020)
#define RTL_ERRORMODE_NOOPENFILEERRORBOX                    (0x0040)

// Debugging
#define RTL_QUERY_PROCESS_MODULES                           (0x00000001)
#define RTL_QUERY_PROCESS_BACKTRACES                        (0x00000002)
#define RTL_QUERY_PROCESS_HEAP_SUMMARY                      (0x00000004)
#define RTL_QUERY_PROCESS_HEAP_TAGS                         (0x00000008)
#define RTL_QUERY_PROCESS_HEAP_ENTRIES                      (0x00000010)
#define RTL_QUERY_PROCESS_LOCKS                             (0x00000020)
#define RTL_QUERY_PROCESS_MODULES32                         (0x00000040)
#define RTL_QUERY_PROCESS_VERIFIER_OPTIONS                  (0x00000080) 
#define RTL_QUERY_PROCESS_MODULESEX                         (0x00000100) 
#define RTL_QUERY_PROCESS_HEAP_ENTRIES_EX                   (0x00000200) 
#define RTL_QUERY_PROCESS_CS_OWNER                          (0x00000400)
#define RTL_QUERY_PROCESS_NONINVASIVE                       (0x80000000)

#define DBG_STATUS_CONTROL_C                                (1)
#define DBG_STATUS_SYSRQ                                    (2)
#define DBG_STATUS_BUGCHECK_FIRST                           (3)
#define DBG_STATUS_BUGCHECK_SECOND                          (4)
#define DBG_STATUS_FATAL                                    (5)
#define DBG_STATUS_DEBUG_CONTROL                            (6)
#define DBG_STATUS_WORKER                                   (7)

///
/// < RTL Enums >
///

typedef enum _RTL_NORM_FORM {
    NormOther = 0x0,
    NormC = 0x1,
    NormD = 0x2,
    NormKC = 0x5,
    NormKD = 0x6,
    NormIdna = 0xd,
    DisallowUnassigned = 0x100,
    NormCDisallowUnassigned = 0x101,
    NormDDisallowUnassigned = 0x102,
    NormKCDisallowUnassigned = 0x105,
    NormKDDisallowUnassigned = 0x106,
    NormIdnaDisallowUnassigned = 0x10d
} RTL_NORM_FORM;

typedef enum _RTL_PATH_TYPE {
    RtlPathTypeUnknown,
    RtlPathTypeUncAbsolute,
    RtlPathTypeDriveAbsolute,
    RtlPathTypeDriveRelative,
    RtlPathTypeRooted,
    RtlPathTypeRelative,
    RtlPathTypeLocalDevice,
    RtlPathTypeRootLocalDevice
} RTL_PATH_TYPE;


///
/// < RTL Structures >
///

typedef struct _RTL_DRIVE_LETTER_CURDIR {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING sPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_DRIVE_LETTER_CURDIR32 {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING32 DosPath;
} RTL_DRIVE_LETTER_CURDIR32, *PRTL_DRIVE_LETTER_CURDIR32;

typedef struct _RTLP_CURDIR_REF {
    LONG RefCount;
    HANDLE Handle;
} RTLP_CURDIR_REF;
typedef struct _RTLP_CURDIR_REF *PRTLP_CURDIR_REF;

typedef struct _RTL_RELATIVE_NAME_U {
    UNICODE_STRING RelativeName;
    HANDLE ContainingDirectory;
    PRTLP_CURDIR_REF CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

//
// Loader
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
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULE_INFORMATION64 {
    ULONGLONG Section;          // 0x00
    ULONGLONG MappedBase;       // 0x08
    ULONGLONG ImageBase;        // 0x10
    ULONG ImageSize;            // 0x18
    ULONG Flags;                // 0x1C
    USHORT LoadOrderIndex;      // 0x20
    USHORT InitOrderIndex;      // 0x22
    USHORT LoadCount;           // 0x24
    USHORT OffsetToFileName;    // 0x26
    UCHAR FullPathName[256];    // 0x28
} RTL_PROCESS_MODULE_INFORMATION64, *PRTL_PROCESS_MODULE_INFORMATION64;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef struct _RTL_PROCESS_MODULES64 {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION64 Modules[1];
} RTL_PROCESS_MODULES64, *PRTL_PROCESS_MODULES64;

//
// Compression
//
typedef struct _COMPRESSED_DATA_INFO {
    USHORT CompressionFormatAndEngine; // COMPRESSION_FORMAT_* and COMPRESSION_ENGINE
    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved;
    USHORT NumberOfChunks;
    ULONG CompressedChunkSizes[1];
} COMPRESSED_DATA_INFO, *PCOMPRESSED_DATA_INFO;

//
// Process 
//
typedef struct _RTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION {
    HANDLE ReflectionProcessHandle;
    HANDLE ReflectionThreadHandle;
    CLIENT_ID ReflectionClientId;
} RTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION, *PRTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG MaximumLength;            // 0x00
    ULONG Length;                   // 0x04
    ULONG Flags;                    // 0x08
    ULONG DebugFlags;               // 0x0C
    HANDLE ConsoleHandle;           // 0x10
    ULONG ConsoleFlags;             // 0x18
    HANDLE StdInputHandle;          // 0x20
    HANDLE StdOutputHandle;         // 0x28
    HANDLE StdErrorHandle;          // 0x30
    CURDIR CurrentDirectoryPath;    // 0x38
    UNICODE_STRING DllPath;         // 0x50
    UNICODE_STRING ImagePathName;   // 0x60
    UNICODE_STRING CommandLine;     // 0x70
    PWSTR Environment;              // 0x80
    ULONG StartingPositionLeft;     // 0x88
    ULONG StartingPositionTop;      // 0x8C
    ULONG Width;                    // 0x90
    ULONG Height;                   // 0x94
    ULONG CharWidth;                // 0x98
    ULONG CharHeight;               // 0x9C
    ULONG ConsoleTextAttributes;    // 0xA0
    ULONG WindowFlags;              // 0xA4
    ULONG ShowWindowFlags;          // 0xA8
    UNICODE_STRING WindowTitle;     //
    UNICODE_STRING Desktop;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectories[RTL_MAX_DRIVE_LETTERS];
#if (NTDDI_VERSION >= NTDDI_VISTA)
    SIZE_T EnvironmentSize;
#endif
#if (NTDDI_VERSION >= NTDDI_WIN7)
    SIZE_T EnvironmentVersion;
#endif
#if (NTDDI_VERSION >= NTDDI_WIN8)
    PVOID PackageDependencyData;
    ULONG ProcessGroupId;
    ULONG LoaderThreads;
#endif
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _RTL_USER_PROCESS_INFORMATION {
    ULONG Size;
    HANDLE Process;
    HANDLE Thread;
    CLIENT_ID ClientId;
    SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;


///
/// < Exception handling >
///
#ifdef _WIN64
typedef enum _FUNCTION_TABLE_TYPE {
    RF_SORTED,
    RF_UNSORTED,
    RF_CALLBACK,
    RF_KERNEL_DYNAMIC
} FUNCTION_TABLE_TYPE;

typedef struct _DYNAMIC_FUNCTION_TABLE {
    LIST_ENTRY ListEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LARGE_INTEGER TimeStamp;
    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    PGET_RUNTIME_FUNCTION_CALLBACK Callback;
    PVOID Context;
    PWSTR OutOfProcessCallbackDll;
    FUNCTION_TABLE_TYPE Type;
    ULONG EntryCount;
} DYNAMIC_FUNCTION_TABLE, *PDYNAMIC_FUNCTION_TABLE;
#endif

///
/// < Heaps >
///

typedef struct _RTL_HEAP_TAG_INFO {
    ULONG NumberOfAllocations;
    ULONG NumberOfFrees;
    SIZE_T BytesAllocated;
} RTL_HEAP_TAG_INFO, *PRTL_HEAP_TAG_INFO;

typedef struct _RTL_HEAP_ENTRY {
    SIZE_T Size;
    USHORT Flags;
    USHORT AllocatorBackTraceIndex;
    union {
        struct {
            SIZE_T Settable;
            ULONG Tag;
        } DUMMYSTRUCTNAME;
        struct {
            SIZE_T CommittedSize;
            PVOID FirstBlock;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME;
} RTL_HEAP_ENTRY, *PRTL_HEAP_ENTRY;

typedef struct _RTL_HEAP_TAG {
    ULONG NumberOfAllocations;
    ULONG NumberOfFrees;
    SIZE_T BytesAllocated;
    USHORT TagIndex;
    USHORT CreatorBackTraceIndex;
    WCHAR TagName[24];
} RTL_HEAP_TAG, *PRTL_HEAP_TAG;

typedef struct _RTL_HEAP_INFORMATION {
    PVOID BaseAddress;
    ULONG Flags;
    USHORT EntryOverhead;
    USHORT CreatorBackTraceIndex;
    SIZE_T BytesAllocated;
    SIZE_T BytesCommitted;
    ULONG NumberOfTags;
    ULONG NumberOfEntries;
    ULONG NumberOfPseudoTags;
    ULONG PseudoTagGranularity;
    ULONG Reserved[5];
    PRTL_HEAP_TAG Tags;
    PRTL_HEAP_ENTRY Entries;
} RTL_HEAP_INFORMATION, *PRTL_HEAP_INFORMATION;

typedef struct _RTL_PROCESS_HEAPS {
    ULONG NumberOfHeaps;
    RTL_HEAP_INFORMATION Heaps[1];
} RTL_PROCESS_HEAPS, *PRTL_PROCESS_HEAPS;

typedef NTSTATUS(NTAPI *PRTL_HEAP_LEAK_ENUMERATION_ROUTINE)(
    IN LONG Reserved,
    IN PVOID HeapHandle,
    IN PVOID BaseAddress,
    IN SIZE_T BlockSize,
    IN ULONG StackTraceDepth,
    IN PVOID *StackTrace
    );

typedef NTSTATUS(NTAPI *PRTL_HEAP_COMMIT_ROUTINE)(
    IN PVOID Base,
    IN OUT PVOID *CommitAddress,
    IN OUT PSIZE_T CommitSize
    );

typedef struct _RTL_HEAP_PARAMETERS {
    ULONG Length;
    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;
    SIZE_T MaximumAllocationSize;
    SIZE_T VirtualMemoryThreshold;
    SIZE_T InitialCommit;
    SIZE_T InitialReserve;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
    SIZE_T Reserved[2];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

typedef struct _RTL_HEAP_USAGE_ENTRY {
    struct _RTL_HEAP_USAGE_ENTRY *Next;
    PVOID Address;
    SIZE_T Size;
    USHORT AllocatorBackTraceIndex;
    USHORT TagIndex;
} RTL_HEAP_USAGE_ENTRY, *PRTL_HEAP_USAGE_ENTRY;

typedef struct _RTL_HEAP_USAGE {
    ULONG Length;
    SIZE_T BytesAllocated;
    SIZE_T BytesCommitted;
    SIZE_T BytesReserved;
    SIZE_T BytesReservedMaximum;
    PRTL_HEAP_USAGE_ENTRY Entries;
    PRTL_HEAP_USAGE_ENTRY AddedEntries;
    PRTL_HEAP_USAGE_ENTRY RemovedEntries;
    ULONG_PTR Reserved[8];
} RTL_HEAP_USAGE, *PRTL_HEAP_USAGE;

typedef struct _RTL_HEAP_WALK_ENTRY {
    PVOID DataAddress;
    SIZE_T DataSize;
    UCHAR OverheadBytes;
    UCHAR SegmentIndex;
    USHORT Flags;
    union {
        struct {
            SIZE_T Settable;
            USHORT TagIndex;
            USHORT AllocatorBackTraceIndex;
            ULONG Reserved[2];
        } Block;
        struct {
            ULONG CommittedSize;
            ULONG UnCommittedSize;
            PVOID FirstEntry;
            PVOID LastEntry;
        } Segment;
    };
} RTL_HEAP_WALK_ENTRY, *PRTL_HEAP_WALK_ENTRY;

typedef struct _HEAP_DEBUGGING_INFORMATION {
    PVOID InterceptorFunction;
    USHORT InterceptorValue;
    ULONG ExtendedOptions;
    ULONG StackTraceDepth;
    SIZE_T MinTotalBlockSize;
    SIZE_T MaxTotalBlockSize;
    PRTL_HEAP_LEAK_ENUMERATION_ROUTINE HeapLeakEnumerationRoutine;
} HEAP_DEBUGGING_INFORMATION, *PHEAP_DEBUGGING_INFORMATION;


///
/// < Debugging >
///

typedef struct _RTL_PROCESS_VERIFIER_OPTIONS {
    ULONG SizeStruct;
    ULONG Option;
    UCHAR OptionData[1];
} RTL_PROCESS_VERIFIER_OPTIONS, *PRTL_PROCESS_VERIFIER_OPTIONS;

typedef struct _RTL_DEBUG_INFORMATION {
    HANDLE SectionHandleClient;
    PVOID ViewBaseClient;
    PVOID ViewBaseTarget;
    ULONG_PTR ViewBaseDelta;
    HANDLE EventPairClient;
    HANDLE EventPairTarget;
    HANDLE TargetProcessId;
    HANDLE TargetThreadHandle;
    ULONG Flags;
    SIZE_T OffsetFree;
    SIZE_T CommitSize;
    SIZE_T ViewSize;
    union {
        struct _RTL_PROCESS_MODULES *Modules;
        struct _RTL_PROCESS_MODULE_INFORMATION_EX *ModulesEx;
    };
    struct _RTL_PROCESS_BACKTRACES *BackTraces;
    struct _RTL_PROCESS_HEAPS *Heaps;
    struct _RTL_PROCESS_LOCKS *Locks;
    PVOID SpecificHeap;
    HANDLE TargetProcessHandle;
    PRTL_PROCESS_VERIFIER_OPTIONS VerifierOptions;
    PVOID ProcessHeap;
    HANDLE CriticalSectionHandle;
    HANDLE CriticalSectionOwnerThread;
    PVOID Reserved[4];
} RTL_DEBUG_INFORMATION, *PRTL_DEBUG_INFORMATION;



///
/// < Doubly and Singly Linked Lists >
///


FORCEINLINE
VOID
InitializeListHead(
    OUT PLIST_ENTRY ListHead
)
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

_Check_return_
FORCEINLINE
BOOLEAN
IsListEmpty(
    IN PLIST_ENTRY ListHead
)
{
    return (ListHead->Flink == ListHead);
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
    IN PLIST_ENTRY Entry
)
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;

    return Flink == Blink;
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
    IN OUT PLIST_ENTRY ListHead
)
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;

    return Entry;
}

FORCEINLINE
PLIST_ENTRY
RemoveTailList(
    IN OUT PLIST_ENTRY ListHead
)
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;

    return Entry;
}

FORCEINLINE
VOID
InsertTailList(
    IN OUT PLIST_ENTRY ListHead,
    IN OUT PLIST_ENTRY Entry
)
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}

FORCEINLINE
VOID
InsertHeadList(
    IN OUT PLIST_ENTRY ListHead,
    IN OUT PLIST_ENTRY Entry
)
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

FORCEINLINE
VOID
AppendTailList(
    IN OUT PLIST_ENTRY ListHead,
    IN OUT PLIST_ENTRY ListToAppend
)
{
    PLIST_ENTRY ListEnd = ListHead->Blink;

    ListHead->Blink->Flink = ListToAppend;
    ListHead->Blink = ListToAppend->Blink;
    ListToAppend->Blink->Flink = ListHead;
    ListToAppend->Blink = ListEnd;
}

FORCEINLINE
PSINGLE_LIST_ENTRY
PopEntryList(
    IN OUT PSINGLE_LIST_ENTRY ListHead
)
{
    PSINGLE_LIST_ENTRY FirstEntry;

    FirstEntry = ListHead->Next;

    if (FirstEntry)
        ListHead->Next = FirstEntry->Next;

    return FirstEntry;
}

FORCEINLINE
VOID
PushEntryList(
    IN OUT PSINGLE_LIST_ENTRY ListHead,
    IN OUT PSINGLE_LIST_ENTRY Entry
)
{
    Entry->Next = ListHead->Next;
    ListHead->Next = Entry;
}



///
/// < Generic Tables/Trees >
///

//
// This enumerated type is used as the function return value of the function
// that is used to search the tree for a key. FoundNode indicates that the
// function found the key. Insert as left indicates that the key was not found
// and the node should be inserted as the left child of the parent. Insert as
// right indicates that the key was not found and the node should be inserted
//  as the right child of the parent.
//

typedef enum _TABLE_SEARCH_RESULT {
    TableEmptyTree,
    TableFoundNode,
    TableInsertAsLeft,
    TableInsertAsRight
} TABLE_SEARCH_RESULT;

//
// The results of a compare can be less than, equal, or greater than.
//

typedef enum _RTL_GENERIC_COMPARE_RESULTS {
    GenericLessThan,
    GenericGreaterThan,
    GenericEqual
} RTL_GENERIC_COMPARE_RESULTS;


//
// Define the Avl version of the generic table package. Note a generic table
// should really be an opaque type. We provide routines to manipulate the structure.
//
// A generic table is package for inserting, deleting, and looking up elements
// in a table (e.g., in a symbol table). To use this package the user
// defines the structure of the elements stored in the table, provides a
// comparison function, a memory allocation function, and a memory
// deallocation function.
//
// Note: the user compare function must impose a complete ordering among
// all of the elements, and the table does not allow for duplicate entries.
//

//
// Add an empty typedef so that functions can reference the
// a pointer to the generic table struct before it is declared.
//

struct _RTL_AVL_TABLE;

//
// The comparison function takes as input pointers to elements containing
// user defined structures and returns the results of comparing the two
// elements.
//

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI RTL_AVL_COMPARE_ROUTINE)(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    );
typedef RTL_AVL_COMPARE_ROUTINE *PRTL_AVL_COMPARE_ROUTINE;

//
// The allocation function is called by the generic table package whenever
// it needs to allocate memory for the table.
//

typedef
PVOID
(NTAPI RTL_AVL_ALLOCATE_ROUTINE)(
    IN struct _RTL_AVL_TABLE *Table,
    IN CLONG ByteSize
    );
typedef RTL_AVL_ALLOCATE_ROUTINE *PRTL_AVL_ALLOCATE_ROUTINE;

//
// The deallocation function is called by the generic table package whenever
// it needs to deallocate memory from the table that was allocated by calling
// the user supplied allocation function.
//

typedef
VOID
(NTAPI RTL_AVL_FREE_ROUTINE)(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID Buffer
    );
typedef RTL_AVL_FREE_ROUTINE *PRTL_AVL_FREE_ROUTINE;

//
// The match function takes as input the user data to be matched and a pointer
// to some match data, which was passed along with the function pointer.  It
// returns TRUE for a match and FALSE for no match.
//
// RTL_AVL_MATCH_FUNCTION returns
//     STATUS_SUCCESS if the IndexRow matches
//     STATUS_NO_MATCH if the IndexRow does not match, but the enumeration should
//         continue
//     STATUS_NO_MORE_MATCHES if the IndexRow does not match, and the enumeration
//         should terminate
//

typedef
NTSTATUS
(NTAPI RTL_AVL_MATCH_FUNCTION)(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID UserData,
    IN PVOID MatchData
    );
typedef RTL_AVL_MATCH_FUNCTION *PRTL_AVL_MATCH_FUNCTION;

//
// Define the balanced tree links and Balance field. (No Rank field
// defined at this time.)
//
// Callers should treat this structure as opaque!
//
// The root of a balanced binary tree is not a real node in the tree
// but rather points to a real node which is the root.  It is always
// in the table below, and its fields are used as follows:
//
//     Parent      Pointer to self, to allow for detection of the root.
//     LeftChild   NULL
//     RightChild  Pointer to real root
//     Balance     Undefined, however it is set to a convenient value
//                 (depending on the algorithm) prior to rebalancing
//                 in insert and delete routines.
//

typedef struct _RTL_BALANCED_LINKS {
    struct _RTL_BALANCED_LINKS *Parent;
    struct _RTL_BALANCED_LINKS *LeftChild;
    struct _RTL_BALANCED_LINKS *RightChild;
    CHAR Balance;
    UCHAR Reserved[3];
} RTL_BALANCED_LINKS, *PRTL_BALANCED_LINKS;

//
// To use the generic table package the user declares a variable of type
// GENERIC_TABLE and then uses the routines described below to initialize
// the table and to manipulate the table.  Note that the generic table
// should really be an opaque type.
//

typedef struct _RTL_AVL_TABLE {
    RTL_BALANCED_LINKS BalancedRoot;
    PVOID OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    ULONG DepthOfTree;
    PRTL_BALANCED_LINKS RestartKey;
    ULONG DeleteCount;
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine;
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_AVL_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_AVL_TABLE, *PRTL_AVL_TABLE;

//
// The procedure InitializeGenericTable takes as input an uninitialized
// generic table variable and pointers to the three user supplied routines.
// This must be called for every individual generic table variable before
// it can be used.
//

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTableAvl(
    PRTL_AVL_TABLE Table,
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine,
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_AVL_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

//
// The function InsertElementGenericTable will insert a new element
// in a table. It does this by allocating space for the new element
// (this includes AVL links), inserting the element in the table, and
// then returning to the user a pointer to the new element.  If an element
// with the same key already exists in the table the return value is a pointer
// to the old element. The optional output parameter NewElement is used
// to indicate if the element previously existed in the table. Note: the user
// supplied Buffer is only used for searching the table, upon insertion its
// contents are copied to the newly created element. This means that
// pointer to the input buffer will not point to the new element.
//

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableAvl(
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

//
// The function InsertElementGenericTableFull will insert a new element
// in a table.  It does this by allocating space for the new element
// (this includes AVL links), inserting the element in the table, and
// then returning to the user a pointer to the new element.  If an element
// with the same key already exists in the table the return value is a pointer
// to the old element.  The optional output parameter NewElement is used
// to indicate if the element previously existed in the table.  Note: the user
// supplied Buffer is only used for searching the table, upon insertion its
// contents are copied to the newly created element.  This means that
// pointer to the input buffer will not point to the new element.
// This routine is passed the NodeOrParent and SearchResult from a
// previous RtlLookupElementGenericTableFull.
//

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFullAvl(
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

//
// The function DeleteElementGenericTable will find and delete an element
// from a generic table. If the element is located and deleted the return
// value is TRUE, otherwise if the element is not located the return value
// is FALSE. The user supplied input buffer is only used as a key in
// locating the element in the table.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTableAvl(
    PRTL_AVL_TABLE Table,
    PVOID Buffer
    );

//
// The function LookupElementGenericTable will find an element in a generic
// table.  If the element is located the return value is a pointer to
// the user defined structure associated with the element, otherwise if
// the element is not located the return value is NULL.  The user supplied
// input buffer is only used as a key in locating the element in the table.
//

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableAvl(
    PRTL_AVL_TABLE Table,
    PVOID Buffer
    );

//
// The function LookupElementGenericTableFull will find an element in a generic
// table.  If the element is located the return value is a pointer to
// the user defined structure associated with the element.  If the element is not
// located then a pointer to the parent for the insert location is returned.  The
// user must look at the SearchResult value to determine which is being returned.
// The user can use the SearchResult and parent for a subsequent FullInsertElement
// call to optimize the insert.
//

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFullAvl(
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

//
// The function EnumerateGenericTable will return to the caller one-by-one
// the elements of of a table.  The return value is a pointer to the user
// defined structure associated with the element.  The input parameter
// Restart indicates if the enumeration should start from the beginning
// or should return the next element.  If the are no more new elements to
// return the return value is NULL.  As an example of its use, to enumerate
// all of the elements in a table the user would write:
//
//     for (ptr = EnumerateGenericTable(Table, TRUE);
//          ptr != NULL;
//          ptr = EnumerateGenericTable(Table, FALSE)) {
//             :
//     }
//
// NOTE:   This routine does not modify the structure of the tree, but saves
//         the last node returned in the generic table itself, and for this
//         reason requires exclusive access to the table for the duration of
//         the enumeration.
//

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableAvl(
    PRTL_AVL_TABLE Table,
    BOOLEAN Restart
    );

//
// The function EnumerateGenericTableWithoutSplaying will return to the
// caller one-by-one the elements of of a table.  The return value is a
// pointer to the user defined structure associated with the element.
// The input parameter RestartKey indicates if the enumeration should
// start from the beginning or should return the next element.  If the
// are no more new elements to return the return value is NULL.  As an
// example of its use, to enumerate all of the elements in a table the
// user would write:
//
//     RestartKey = NULL;
//     for (ptr = EnumerateGenericTableWithoutSplaying(Table, &RestartKey);
//          ptr != NULL;
//          ptr = EnumerateGenericTableWithoutSplaying(Table, &RestartKey)) {
//             :
//     }
//
// If RestartKey is NULL, the package will start from the least entry in the
// table, otherwise it will start from the last entry returned.
//
// NOTE:   This routine does not modify either the structure of the tree
//         or the generic table itself, but must ensure that no deletes
//         occur for the duration of the enumeration, typically by having
//         at least shared access to the table for the duration.
//

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplayingAvl(
    PRTL_AVL_TABLE Table,
    PVOID *RestartKey
    );

//
// The function EnumerateGenericTableLikeADirectory will return to the
// caller one-by-one the elements of of a table.  The return value is a
// pointer to the user defined structure associated with the element.
// The input parameter RestartKey indicates if the enumeration should
// start from the beginning or should return the next element.  If the
// are no more new elements to return the return value is NULL.  As an
// example of its use, to enumerate all of the elements in a table the
// user would write:
//
//     RestartKey = NULL;
//     for (ptr = EnumerateGenericTableLikeADirectory(Table, &RestartKey, ...);
//          ptr != NULL;
//          ptr = EnumerateGenericTableLikeADirectory(Table, &RestartKey, ...)) {
//             :
//     }
//
// If RestartKey is NULL, the package will start from the least entry in the
// table, otherwise it will start from the last entry returned.
//
// NOTE:   This routine does not modify either the structure of the tree
//         or the generic table itself.  The table must only be acquired
//         shared for the duration of this call, and all synchronization
//         may optionally be dropped between calls.  Enumeration is always
//         correctly resumed in the most efficient manner possible via the
//         IN OUT parameters provided.
//
// ******  Explain NextFlag.  Directory enumeration resumes from a key
//         requires more thought.  Also need the match pattern and IgnoreCase.
//         Should some structure be introduced to carry it all?
//

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableLikeADirectory(
    IN PRTL_AVL_TABLE Table,
    IN PRTL_AVL_MATCH_FUNCTION MatchFunction,
    IN PVOID MatchData,
    IN ULONG NextFlag,
    IN OUT PVOID *RestartKey,
    IN OUT PULONG DeleteCount,
    IN OUT PVOID Buffer
    );

//
// The function GetElementGenericTable will return the i'th element
// inserted in the generic table.  I = 0 implies the first element,
// I = (RtlNumberGenericTableElements(Table)-1) will return the last element
// inserted into the generic table.  The type of I is ULONG.  Values
// of I > than (NumberGenericTableElements(Table)-1) will return NULL.  If
// an arbitrary element is deleted from the generic table it will cause
// all elements inserted after the deleted element to "move up".

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTableAvl(
    PRTL_AVL_TABLE Table,
    ULONG I
    );

//
// The function NumberGenericTableElements returns a ULONG value
// which is the number of generic table elements currently inserted
// in the generic table.

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElementsAvl(
    PRTL_AVL_TABLE Table
    );

//
// The function IsGenericTableEmpty will return to the caller TRUE if
// the input table is empty (i.e., does not contain any elements) and
// FALSE otherwise.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmptyAvl(
    PRTL_AVL_TABLE Table
    );

//
// As an aid to allowing existing generic table users to do (in most
// cases) a single-line edit to switch over to Avl table use, we
// have the following defines and inline routine definitions which
// redirect calls and types.  Note that the type override (performed
// by #define below) will not work in the unexpected event that someone
// has used a pointer or type specifier in their own #define, since
// #define processing is one pass and does not nest.  The __inline
// declarations below do not have this limitation, however.
//
// To switch to using Avl tables, add the following line before your
// includes:
//
// #define RTL_USE_AVL_TABLES 0
//

#ifdef RTL_USE_AVL_TABLES

#undef PRTL_GENERIC_COMPARE_ROUTINE
#undef PRTL_GENERIC_ALLOCATE_ROUTINE
#undef PRTL_GENERIC_FREE_ROUTINE
#undef RTL_GENERIC_TABLE
#undef PRTL_GENERIC_TABLE

#define PRTL_GENERIC_COMPARE_ROUTINE PRTL_AVL_COMPARE_ROUTINE
#define PRTL_GENERIC_ALLOCATE_ROUTINE PRTL_AVL_ALLOCATE_ROUTINE
#define PRTL_GENERIC_FREE_ROUTINE PRTL_AVL_FREE_ROUTINE
#define RTL_GENERIC_TABLE RTL_AVL_TABLE
#define PRTL_GENERIC_TABLE PRTL_AVL_TABLE

#define RtlInitializeGenericTable               RtlInitializeGenericTableAvl
#define RtlInsertElementGenericTable            RtlInsertElementGenericTableAvl
#define RtlInsertElementGenericTableFull        RtlInsertElementGenericTableFullAvl
#define RtlDeleteElementGenericTable            RtlDeleteElementGenericTableAvl
#define RtlLookupElementGenericTable            RtlLookupElementGenericTableAvl
#define RtlLookupElementGenericTableFull        RtlLookupElementGenericTableFullAvl
#define RtlEnumerateGenericTable                RtlEnumerateGenericTableAvl
#define RtlEnumerateGenericTableWithoutSplaying RtlEnumerateGenericTableWithoutSplayingAvl
#define RtlGetElementGenericTable               RtlGetElementGenericTableAvl
#define RtlNumberGenericTableElements           RtlNumberGenericTableElementsAvl
#define RtlIsGenericTableEmpty                  RtlIsGenericTableEmptyAvl

#endif // RTL_USE_AVL_TABLES


//
// Define the splay links and the associated manipulation macros and
// routines.  Note that the SPLAY_LINKS should be an opaque type.
// Routine are provided to traverse and manipulate the structure.
//

typedef struct _RTL_SPLAY_LINKS {
    struct _RTL_SPLAY_LINKS *Parent;
    struct _RTL_SPLAY_LINKS *LeftChild;
    struct _RTL_SPLAY_LINKS *RightChild;
} RTL_SPLAY_LINKS, *PRTL_SPLAY_LINKS;


//
// The macro procedure InitializeSplayLinks takes as input a pointer to
// splay link and initializes its substructure. All splay link nodes must
// be initialized before they are used in the different splay routines and
// macros.
//
// VOID
// RtlInitializeSplayLinks (
//     PRTL_SPLAY_LINKS Links
//     );
//

#define RtlInitializeSplayLinks(Links) { \
    PRTL_SPLAY_LINKS _SplayLinks; \
    _SplayLinks = (PRTL_SPLAY_LINKS)(Links); \
    _SplayLinks->Parent = _SplayLinks; \
    _SplayLinks->LeftChild = NULL; \
    _SplayLinks->RightChild = NULL; \
    }

//
// The macro function Parent takes as input a pointer to a splay link in a
// tree and returns a pointer to the splay link of the parent of the input
// node.  If the input node is the root of the tree the return value is
// equal to the input value.
//
// PRTL_SPLAY_LINKS
// RtlParent (
//     PRTL_SPLAY_LINKS Links
//     );
//

#define RtlParent(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->Parent \
    )

//
// The macro function LeftChild takes as input a pointer to a splay link in
// a tree and returns a pointer to the splay link of the left child of the
// input node.  If the left child does not exist, the return value is NULL.
// 
// PRTL_SPLAY_LINKS
// RtlLeftChild (
//     PRTL_SPLAY_LINKS Links
//     );
//

#define RtlLeftChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->LeftChild \
    )

//
// The macro function RightChild takes as input a pointer to a splay link
// in a tree and returns a pointer to the splay link of the right child of
// the input node.  If the right child does not exist, the return value is
// NULL.
// 
// PRTL_SPLAY_LINKS
// RtlRightChild (
//     PRTL_SPLAY_LINKS Links
//     );
//

#define RtlRightChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->RightChild \
    )

//
//  The macro function IsRoot takes as input a pointer to a splay link
//  in a tree and returns TRUE if the input node is the root of the tree,
//  otherwise it returns FALSE.
//
//  BOOLEAN
//  RtlIsRoot (
//      PRTL_SPLAY_LINKS Links
//      );
//

#define RtlIsRoot(Links) (                          \
    (RtlParent(Links) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
// The macro function IsLeftChild takes as input a pointer to a splay link
// in a tree and returns TRUE if the input node is the left child of its
// parent, otherwise it returns FALSE.
//
// BOOLEAN
// RtlIsLeftChild (
//     PRTL_SPLAY_LINKS Links
//     );
//

#define RtlIsLeftChild(Links) (                                   \
    (RtlLeftChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
// The macro function IsRightChild takes as input a pointer to a splay link
// in a tree and returns TRUE if the input node is the right child of its
// parent, otherwise it returns FALSE.
//
// BOOLEAN
// RtlIsRightChild (
//     PRTL_SPLAY_LINKS Links
//     );
//

#define RtlIsRightChild(Links) (                                   \
    (RtlRightChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

//
// The macro procedure InsertAsLeftChild takes as input a pointer to a splay
// link in a tree and a pointer to a node not in a tree. It inserts the
// second node as the left child of the first node.  The first node must not
// already have a left child, and the second node must not already have a
// parent.
//
// VOID
// RtlInsertAsLeftChild (
//     PRTL_SPLAY_LINKS ParentLinks,
//     PRTL_SPLAY_LINKS ChildLinks
//     );
//

#define RtlInsertAsLeftChild(ParentLinks, ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent; \
    PRTL_SPLAY_LINKS _SplayChild; \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks); \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks); \
    _SplayParent->LeftChild = _SplayChild; \
    _SplayChild->Parent = _SplayParent; \
    }

//
// The macro procedure InsertAsRightChild takes as input a pointer to a splay
// link in a tree and a pointer to a node not in a tree. It inserts the
// second node as the right child of the first node. The first node must not
// already have a right child, and the second node must not already have a
// parent.
//
// VOID
// RtlInsertAsRightChild (
//     PRTL_SPLAY_LINKS ParentLinks,
//     PRTL_SPLAY_LINKS ChildLinks
//     );
//

#define RtlInsertAsRightChild(ParentLinks, ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent; \
    PRTL_SPLAY_LINKS _SplayChild; \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks); \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks); \
    _SplayParent->RightChild = _SplayChild; \
    _SplayChild->Parent = _SplayParent; \
    }

//
// The Splay function takes as input a pointer to a splay link in a tree
// and splays the tree. Its function return value is a pointer to the
// root of the splayed tree.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSplay(
    PRTL_SPLAY_LINKS Links
    );

//
// The Delete function takes as input a pointer to a splay link in a tree
// and deletes that node from the tree. Its function return value is a
// pointer to the root of the tree. If the tree is now empty, the return
// value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlDelete(
    PRTL_SPLAY_LINKS Links
    );

//
// The DeleteNoSplay function takes as input a pointer to a splay link in a tree,
// the caller's pointer to the root of the tree and deletes that node from the
// tree. Upon return the caller's pointer to the root node will correctly point
// at the root of the tree.
//
// It operationally differs from RtlDelete only in that it will not splay the tree.
//

NTSYSAPI
VOID
NTAPI
RtlDeleteNoSplay(
    PRTL_SPLAY_LINKS Links,
    PRTL_SPLAY_LINKS *Root
    );

//
// The SubtreeSuccessor function takes as input a pointer to a splay link
// in a tree and returns a pointer to the successor of the input node of
// the subtree rooted at the input node. If there is not a successor, the
// return value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreeSuccessor(
    PRTL_SPLAY_LINKS Links
    );

//
// The SubtreePredecessor function takes as input a pointer to a splay link
// in a tree and returns a pointer to the predecessor of the input node of
// the subtree rooted at the input node. If there is not a predecessor,
// the return value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreePredecessor(
    PRTL_SPLAY_LINKS Links
    );

//
// The RealSuccessor function takes as input a pointer to a splay link
// in a tree and returns a pointer to the successor of the input node within
// the entire tree. If there is not a successor, the return value is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealSuccessor(
    PRTL_SPLAY_LINKS Links
    );

//
// The RealPredecessor function takes as input a pointer to a splay link
// in a tree and returns a pointer to the predecessor of the input node
// within the entire tree. If there is not a predecessor, the return value
// is NULL.
//

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealPredecessor(
    PRTL_SPLAY_LINKS Links
    );


//
// Define the generic table package. Note a generic table should really
// be an opaque type. We provide routines to manipulate the structure.
//
// A generic table is package for inserting, deleting, and looking up elements
// in a table (e.g., in a symbol table). To use this package the user
// defines the structure of the elements stored in the table, provides a
// comparison function, a memory allocation function, and a memory
// deallocation function.
//
// Note: the user compare function must impose a complete ordering among
// all of the elements, and the table does not allow for duplicate entries.
//

//
//  Do not do the following defines if using Avl
//

#ifndef RTL_USE_AVL_TABLES

//
// Add an empty typedef so that functions can reference the
// a pointer to the generic table struct before it is declared.
//

struct _RTL_GENERIC_TABLE;

//
// The comparison function takes as input pointers to elements containing
// user defined structures and returns the results of comparing the two
// elements.
//

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI RTL_GENERIC_COMPARE_ROUTINE) (
    IN struct _RTL_GENERIC_TABLE *Table,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    );
typedef RTL_GENERIC_COMPARE_ROUTINE *PRTL_GENERIC_COMPARE_ROUTINE;

//
// The allocation function is called by the generic table package whenever
// it needs to allocate memory for the table.
//

typedef
PVOID
(NTAPI RTL_GENERIC_ALLOCATE_ROUTINE) (
    IN struct _RTL_GENERIC_TABLE *Table,
    IN CLONG ByteSize
    );
typedef RTL_GENERIC_ALLOCATE_ROUTINE *PRTL_GENERIC_ALLOCATE_ROUTINE;

//
// The deallocation function is called by the generic table package whenever
// it needs to deallocate memory from the table that was allocated by calling
// the user supplied allocation function.
//

typedef
VOID
(NTAPI RTL_GENERIC_FREE_ROUTINE) (
    IN struct _RTL_GENERIC_TABLE *Table,
    IN PVOID Buffer
    );
typedef RTL_GENERIC_FREE_ROUTINE *PRTL_GENERIC_FREE_ROUTINE;

//
// To use the generic table package the user declares a variable of type
// GENERIC_TABLE and then uses the routines described below to initialize
// the table and to manipulate the table.  Note that the generic table
// should really be an opaque type.
//

typedef struct _RTL_GENERIC_TABLE {
    PRTL_SPLAY_LINKS TableRoot;
    LIST_ENTRY InsertOrderList;
    PLIST_ENTRY OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine;
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_GENERIC_TABLE;
typedef RTL_GENERIC_TABLE *PRTL_GENERIC_TABLE;

//
// The procedure InitializeGenericTable takes as input an uninitialized
// generic table variable and pointers to the three user supplied routines.
// This must be called for every individual generic table variable before
// it can be used.
//

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTable(
    PRTL_GENERIC_TABLE Table,
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine,
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

//
// The function InsertElementGenericTable will insert a new element
// in a table.  It does this by allocating space for the new element
// (this includes splay links), inserting the element in the table, and
// then returning to the user a pointer to the new element.  If an element
// with the same key already exists in the table the return value is a pointer
// to the old element.  The optional output parameter NewElement is used
// to indicate if the element previously existed in the table.  Note: the user
// supplied Buffer is only used for searching the table, upon insertion its
// contents are copied to the newly created element.  This means that
// pointer to the input buffer will not point to the new element.
//

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

//
// The function InsertElementGenericTableFull will insert a new element
// in a table.  It does this by allocating space for the new element
// (this includes splay links), inserting the element in the table, and
// then returning to the user a pointer to the new element.  If an element
// with the same key already exists in the table the return value is a pointer
// to the old element.  The optional output parameter NewElement is used
// to indicate if the element previously existed in the table.  Note: the user
// supplied Buffer is only used for searching the table, upon insertion its
// contents are copied to the newly created element.  This means that
// pointer to the input buffer will not point to the new element.
// This routine is passed the NodeOrParent and SearchResult from a
// previous RtlLookupElementGenericTableFull.
//

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFull(
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

//
// The function DeleteElementGenericTable will find and delete an element
// from a generic table.  If the element is located and deleted the return
// value is TRUE, otherwise if the element is not located the return value
// is FALSE.  The user supplied input buffer is only used as a key in
// locating the element in the table.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

//
// The function LookupElementGenericTable will find an element in a generic
// table.  If the element is located the return value is a pointer to
// the user defined structure associated with the element, otherwise if
// the element is not located the return value is NULL.  The user supplied
// input buffer is only used as a key in locating the element in the table.
//

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

//
// The function LookupElementGenericTableFull will find an element in a generic
// table.  If the element is located the return value is a pointer to
// the user defined structure associated with the element.  If the element is not
// located then a pointer to the parent for the insert location is returned.  The
// user must look at the SearchResult value to determine which is being returned.
// The user can use the SearchResult and parent for a subsequent FullInsertElement
// call to optimize the insert.
//

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFull(
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

//
// The function EnumerateGenericTable will return to the caller one-by-one
// the elements of of a table.  The return value is a pointer to the user
// defined structure associated with the element.  The input parameter
// Restart indicates if the enumeration should start from the beginning
// or should return the next element.  If the are no more new elements to
// return the return value is NULL.  As an example of its use, to enumerate
// all of the elements in a table the user would write:
//
//     for (ptr = EnumerateGenericTable(Table, TRUE);
//          ptr != NULL;
//          ptr = EnumerateGenericTable(Table, FALSE)) {
//             // Process the element pointed to by ptr
//     }
//
//
// PLEASE NOTE:
//
//     If you enumerate a GenericTable using RtlEnumerateGenericTable, you
//     will flatten the table, turning it into a sorted linked list.
//     To enumerate the table without perturbing the splay links, use
//     RtlEnumerateGenericTableWithoutSplaying

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTable(
    PRTL_GENERIC_TABLE Table,
    BOOLEAN Restart
    );

//
// The function EnumerateGenericTableWithoutSplaying will return to the
// caller one-by-one the elements of of a table.  The return value is a
// pointer to the user defined structure associated with the element.
// The input parameter RestartKey indicates if the enumeration should
// start from the beginning or should return the next element.  If the
// are no more new elements to return the return value is NULL.  As an
// example of its use, to enumerate all of the elements in a table the
// user would write:
//
//     RestartKey = NULL;
//     for (ptr = EnumerateGenericTableWithoutSplaying(Table, &RestartKey);
//          ptr != NULL;
//          ptr = EnumerateGenericTableWithoutSplaying(Table, &RestartKey)) {
//             // Process the element pointed to by ptr
//     }
//
// If RestartKey is NULL, the package will start from the least entry in the
// table, otherwise it will start from the last entry returned.
//
// Note that unlike RtlEnumerateGenericTable, this routine will NOT perturb
// the splay order of the tree.
//

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplaying(
    PRTL_GENERIC_TABLE Table,
    PVOID *RestartKey
    );

//
// The function GetElementGenericTable will return the i'th element
// inserted in the generic table.  I = 0 implies the first element,
// I = (RtlNumberGenericTableElements(Table)-1) will return the last element
// inserted into the generic table.  The type of I is ULONG.  Values
// of I > than (NumberGenericTableElements(Table)-1) will return NULL.  If
// an arbitrary element is deleted from the generic table it will cause
// all elements inserted after the deleted element to "move up".

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    ULONG I
    );

//
// The function NumberGenericTableElements returns a ULONG value
// which is the number of generic table elements currently inserted
// in the generic table.

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElements(
    PRTL_GENERIC_TABLE Table
    );

//
// The function IsGenericTableEmpty will return to the caller TRUE if
// the input table is empty (i.e., does not contain any elements) and
// FALSE otherwise.
//

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmpty(
    PRTL_GENERIC_TABLE Table
    );

#endif // RTL_USE_AVL_TABLES



///
/// < Ansi String Routines >
///

FORCEINLINE
VOID
RtlInitStringInline(
    OUT PSTRING DestinationString,
    IN PSTR SourceString OPTIONAL
)
{
    if (SourceString) {
        DestinationString->Length = (USHORT)strlen(SourceString);
        DestinationString->MaximumLength = DestinationString->Length + 1;
    } else {
        DestinationString->MaximumLength = DestinationString->Length = 0;
    }
    DestinationString->Buffer = SourceString;
}

FORCEINLINE
VOID
RtlInitAnsiStringInline(
    OUT PANSI_STRING DestinationString,
    IN PSTR SourceString OPTIONAL
)
{
    RtlInitStringInline(DestinationString, SourceString);
}

typedef VOID( NTAPI *tRtlInitString )(
    OUT PSTRING DestinationString,
    IN PSTR SourceString OPTIONAL
    );

typedef VOID( NTAPI *tRtlInitAnsiString )(
    OUT PANSI_STRING DestinationString,
    IN PSTR SourceString OPTIONAL
    );

typedef VOID( NTAPI *tRtlFreeAnsiString )(
    IN PANSI_STRING AnsiString
    );

typedef VOID( NTAPI *tRtlFreeOemString )(
    IN POEM_STRING OemString
    );

typedef VOID( NTAPI *tRtlCopyString )(
    IN PSTRING DestinationString,
    IN PSTRING SourceString OPTIONAL
    );

typedef CHAR( NTAPI *tRtlUpperChar )(
    IN CHAR Character
    );

typedef LONG( NTAPI *tRtlCompareString )(
    IN PSTRING String1,
    IN PSTRING String2,
    IN BOOLEAN CaseInSensitive
    );

typedef BOOLEAN( NTAPI *tRtlEqualString )(
    IN PSTRING String1,
    IN PSTRING String2,
    IN BOOLEAN CaseInSensitive
    );

typedef BOOLEAN( NTAPI *tRtlPrefixString )(
    IN PSTRING String1,
    IN PSTRING String2,
    IN BOOLEAN CaseInSensitive
    );

typedef NTSTATUS( NTAPI *tRtlAppendStringToString )(
    IN PSTRING Destination,
    IN PSTRING Source
    );

typedef NTSTATUS( NTAPI *tRtlAppendAsciizToString )(
    IN PSTRING Destination,
    IN PSTR Source OPTIONAL
    );

typedef VOID( NTAPI *tRtlUpperString )(
    IN PSTRING DestinationString,
    IN PSTRING SourceString
    );

///
/// < Unicode String Routines >
///

FORCEINLINE
VOID
RtlInitUnicodeStringInline(
    OUT PUNICODE_STRING DestinationString,
    IN PWSTR SourceString OPTIONAL
)
{
    if (SourceString)
        DestinationString->MaximumLength = (DestinationString->Length = (USHORT)(wcslen( SourceString ) * sizeof( WCHAR ))) + sizeof( WCHAR );
    else
        DestinationString->MaximumLength = DestinationString->Length = 0;
    DestinationString->Buffer = SourceString;
}

typedef VOID( NTAPI *tRtlInitUnicodeString )(
    OUT PUNICODE_STRING DestinationString,
    IN PWSTR SourceString OPTIONAL
    );

typedef VOID( NTAPI *tRtlInitUnicodeStringEx )(
    OUT PUNICODE_STRING DestinationString,
    IN PWSTR SourceString OPTIONAL
    );

typedef BOOLEAN( NTAPI *tRtlCreateUnicodeString )(
    OUT PUNICODE_STRING DestinationString,
    IN PWSTR SourceString
    );

typedef BOOLEAN( NTAPI *tRtlCreateUnicodeStringFromAsciiz )(
    OUT PUNICODE_STRING DestinationString,
    IN PSTR SourceString
    );

typedef VOID( NTAPI *tRtlFreeUnicodeString )(
    IN PUNICODE_STRING UnicodeString
    );

typedef NTSTATUS( NTAPI *tRtlDuplicateUnicodeString )(
    IN ULONG Flags,
    IN PUNICODE_STRING StringIn,
    OUT PUNICODE_STRING StringOut
    );

typedef VOID( NTAPI *tRtlCopyUnicodeString )(
    IN PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString
    );

typedef WCHAR( NTAPI *tRtlUpcaseUnicodeChar )(
    IN WCHAR SourceCharacter
    );

typedef WCHAR( NTAPI *tRtlDowncaseUnicodeChar )(
    IN WCHAR SourceCharacter
    );

typedef LONG( NTAPI *tRtlCompareUnicodeString )(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef LONG( NTAPI *tRtlCompareUnicodeStrings )(
    IN PWCH String1,
    IN SIZE_T String1Length,
    IN PWCH String2,
    IN SIZE_T String2Length,
    IN BOOLEAN CaseInSensitive
    );
#endif

typedef BOOLEAN( NTAPI *tRtlEqualUnicodeString )(
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

typedef NTSTATUS( NTAPI *tRtlHashUnicodeString )(
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInSensitive,
    IN ULONG HashAlgorithm,
    OUT PULONG HashValue
    );

typedef NTSTATUS( NTAPI *tRtlValidateUnicodeString )(
    IN ULONG Flags,
    IN PUNICODE_STRING String
    );

typedef BOOLEAN( NTAPI *tRtlPrefixUnicodeString )(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

#if (NTDDI_VERSION >= NTDDI_WINBLUE)
typedef BOOLEAN( NTAPI *tRtlSuffixUnicodeString )(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );
#endif

typedef NTSTATUS( NTAPI *tRtlFindCharInUnicodeString )(
    IN ULONG Flags,
    IN PUNICODE_STRING StringToSearch,
    IN PUNICODE_STRING CharSet,
    OUT PUSHORT NonInclusivePrefixLength
    );

typedef NTSTATUS( NTAPI *tRtlAppendUnicodeStringToString )(
    IN PUNICODE_STRING Destination,
    IN PUNICODE_STRING Source
    );

typedef NTSTATUS( NTAPI *tRtlAppendUnicodeToString )(
    IN PUNICODE_STRING Destination,
    IN PWSTR Source OPTIONAL
    );

typedef NTSTATUS( NTAPI *tRtlUpcaseUnicodeString )(
    IN OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlDowncaseUnicodeString )(
    IN OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef VOID( NTAPI *tRtlEraseUnicodeString )(
    IN OUT PUNICODE_STRING String
    );

typedef NTSTATUS( NTAPI *tRtlAnsiStringToUnicodeString )(
    IN OUT PUNICODE_STRING DestinationString,
    IN PANSI_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlUnicodeStringToAnsiString )(
    IN OUT PANSI_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef WCHAR( NTAPI *tRtlAnsiCharToUnicodeChar )(
    IN OUT PUCHAR *SourceCharacter
    );

typedef NTSTATUS( NTAPI *tRtlUpcaseUnicodeStringToAnsiString )(
    IN OUT PANSI_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlOemStringToUnicodeString )(
    IN OUT PUNICODE_STRING DestinationString,
    IN POEM_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlUnicodeStringToOemString )(
    IN OUT POEM_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlUpcaseUnicodeStringToOemString )(
    IN OUT POEM_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlUnicodeStringToCountedOemString )(
    IN OUT POEM_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef NTSTATUS( NTAPI *tRtlUpcaseUnicodeStringToCountedOemString )(
    IN OUT POEM_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

typedef BOOLEAN( NTAPI *tRtlIsTextUnicode )(
    IN PVOID Buffer,
    IN ULONG Size,
    IN OUT PULONG Result OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef NTSTATUS( NTAPI *tRtlNormalizeString )(
    IN RTL_NORM_FORM NormForm,
    IN PCWSTR SourceString,
    IN LONG SourceStringLength,
    OUT PWSTR DestinationString,
    IN OUT PLONG DestinationStringLength
    );
#endif

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef NTSTATUS( NTAPI *tRtlIsNormalizedString )(
    IN RTL_NORM_FORM NormForm,
    IN PCWSTR SourceString,
    IN LONG SourceStringLength,
    OUT PBOOLEAN Normalized
    );
#endif

typedef NTSTATUS( NTAPI *tRtlStringFromGUID )(
    IN PGUID Guid,
    OUT PUNICODE_STRING GuidString
    );

typedef NTSTATUS( NTAPI *tRtlGUIDFromString )(
    IN PUNICODE_STRING GuidString,
    OUT PGUID Guid
    );

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef LONG( NTAPI *tRtlCompareAltitudes )(
    IN PUNICODE_STRING Altitude1,
    IN PUNICODE_STRING Altitude2
    );
#endif

///
/// < Critical Section Routines >
///

typedef
NTSTATUS
(NTAPI *tRtlInitializeCriticalSection) (
    OUT PRTL_CRITICAL_SECTION CriticalSection
    );
NTRTLAPI
NTSTATUS
NTAPI
RtlInitializeCriticalSection(
    OUT PRTL_CRITICAL_SECTION CriticalSection
    );

typedef
NTSTATUS
(NTAPI *tRtlInitializeCriticalSectionAndSpinCount)(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection,
    IN ULONG SpinCount
    );
NTRTLAPI
NTSTATUS
NTAPI
RtlInitializeCriticalSectionAndSpinCount(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection,
    IN ULONG SpinCount
    );

typedef
NTSTATUS
(NTAPI *tRtlDeleteCriticalSection)(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );
NTRTLAPI
NTSTATUS
NTAPI
RtlDeleteCriticalSection(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );

typedef
NTSTATUS
(NTAPI *tRtlEnterCriticalSection)(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );
NTRTLAPI
NTSTATUS
NTAPI
RtlEnterCriticalSection(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );

typedef
NTSTATUS
(NTAPI *tRtlLeaveCriticalSection)(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );
NTRTLAPI
NTSTATUS
NTAPI
RtlLeaveCriticalSection(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );

typedef
BOOLEAN
(NTAPI *tRtlTryEnterCriticalSection)(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );
NTRTLAPI
BOOLEAN
NTAPI
RtlTryEnterCriticalSection(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );


///
/// < Compression Routines >
///

typedef NTSTATUS(NTAPI *tRtlGetCompressionWorkSpaceSize)(
    IN USHORT CompressionFormatAndEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

typedef NTSTATUS(NTAPI *tRtlCompressBuffer)(
    IN USHORT CompressionFormatAndEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

typedef NTSTATUS(NTAPI *tRtlDecompressBuffer)(
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

#if (NTDDI_VERSION >= NTDDI_WIN8)
typedef NTSTATUS(NTAPI *tRtlDecompressBufferEx)(
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );
#endif

typedef NTSTATUS(NTAPI *tRtlDecompressFragment)(
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

typedef NTSTATUS(NTAPI *tRtlDescribeChunk)(
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

typedef NTSTATUS(NTAPI *tRtlReserveChunk)(
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );

typedef NTSTATUS(NTAPI *tRtlDecompressChunks)(
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN PUCHAR CompressedTail,
    IN ULONG CompressedTailSize,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo
    );

typedef NTSTATUS(NTAPI *tRtlCompressChunks)(
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PVOID WorkSpace
    );


///
/// < Process Routines >
///

typedef NTSTATUS(NTAPI *tRtlCreateProcessParameters)(
    OUT PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef NTSTATUS(NTAPI *tRtlCreateProcessParametersEx)(
    OUT PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL,
    IN ULONG Flags // pass RTL_USER_PROC_PARAMS_NORMALIZED to keep parameters normalized
    );
#endif

typedef NTSTATUS(NTAPI *tRtlDestroyProcessParameters)(
    IN _Post_invalid_ PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef PRTL_USER_PROCESS_PARAMETERS(NTAPI *tRtlNormalizeProcessParams)(
    IN OUT PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef PRTL_USER_PROCESS_PARAMETERS(NTAPI *tRtlDeNormalizeProcessParams)(
    IN OUT PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef NTSTATUS(NTAPI *tRtlCreateUserProcess)(
    IN PUNICODE_STRING NtImagePathName,
    IN ULONG AttributesDeprecated,
    IN PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    IN PSECURITY_DESCRIPTOR ProcessSecurityDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN HANDLE ParentProcess OPTIONAL,
    IN BOOLEAN InheritHandles,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE TokenHandle OPTIONAL, // used to be ExceptionPort in Windows XP
    OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation
    );

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef VOID(NTAPI *tRtlExitUserProcess)(
    IN NTSTATUS ExitStatus
    );
#else
#define RtlExitUserProcess RtlExitUserProcess_R
DECLSPEC_NORETURN
FORCEINLINE
VOID
RtlExitUserProcess_R(
    IN NTSTATUS ExitStatus
)
{
    ExitProcess(ExitStatus);
}
#endif

#if (NTDDI_VERSION >= NTDDI_VISTA)

typedef NTSTATUS(NTAPI *tRtlCloneUserProcess)(
    IN ULONG ProcessFlags,
    IN PSECURITY_DESCRIPTOR ProcessSecurityDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation
    );

typedef VOID(NTAPI *tRtlUpdateClonedCriticalSection)(
    IN OUT PRTL_CRITICAL_SECTION CriticalSection
    );

typedef VOID(NTAPI *tRtlUpdateClonedSRWLock)(IN OUT PRTL_SRWLOCK SRWLock,
                                             IN ULONG Shared // TRUE to set to shared acquire
                                             );

#if (NTDDI_VERSION >= NTDDI_WIN7)
typedef NTSTATUS(NTAPI *tRtlCreateProcessReflection)(
    IN HANDLE ProcessHandle,
    IN ULONG Flags,
    IN PVOID StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN HANDLE EventHandle OPTIONAL,
    OUT PRTLP_PROCESS_REFLECTION_REFLECTION_INFORMATION ReflectionInformation OPTIONAL
    );
#endif

#endif


///
/// < Thread Routines >
///

typedef NTSTATUS(NTAPI *PUSER_THREAD_START_ROUTINE)(
    IN PVOID ThreadParameter
    );

typedef NTSTATUS(NTAPI *tRtlCreateUserThread)(
    IN HANDLE Process,
    IN PSECURITY_DESCRIPTOR ThreadSecurityDescriptor OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN ULONG ZeroBits OPTIONAL,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN PUSER_THREAD_START_ROUTINE StartAddress,
    IN PVOID Parameter OPTIONAL,
    OUT PHANDLE Thread OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_VISTA) // should be NTDDI_WINXP, but is NTDDI_VISTA for consistency with RtlExitUserProcess
typedef VOID(NTAPI *tRtlExitUserThread)(
    IN NTSTATUS ExitStatus
    );
#else
#define RtlExitUserThread RtlExitUserThread_R
DECLSPEC_NORETURN
FORCEINLINE
VOID
RtlExitUserThread_R(
    IN NTSTATUS ExitStatus
)
{
    ExitThread(ExitStatus);
}
#endif

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef NTSTATUS(NTAPI *tRtlCreateUserStack)(
    IN SIZE_T CommittedStackSize OPTIONAL,
    IN SIZE_T MaximumStackSize OPTIONAL,
    IN ULONG_PTR ZeroBits OPTIONAL,
    IN SIZE_T PageSize,
    IN ULONG_PTR ReserveAlignment,
    OUT PVOID InitialTeb // PINITIAL_TEB
    );

typedef NTSTATUS(NTAPI *tRtlFreeUserStack)(
    IN PVOID AllocationBase
    );
#endif

typedef VOID(NTAPI *tRtlInitializeContext)(
    IN HANDLE Process,
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlRemoteCall)(
    IN HANDLE Process,
    IN HANDLE Thread,
    IN PVOID CallSite,
    IN ULONG ArgumentCount,
    IN PULONG_PTR Arguments OPTIONAL,
    IN BOOLEAN PassContext,
    IN BOOLEAN AlreadySuspended
    );

#ifdef _WIN64
typedef NTSTATUS(NTAPI *tRtlWow64GetThreadContext)(
    IN HANDLE ThreadHandle,
    IN OUT PWOW64_CONTEXT ThreadContext
    );

typedef NTSTATUS(NTAPI *tRtlWow64SetThreadContext)(
    IN HANDLE ThreadHandle,
    IN PWOW64_CONTEXT ThreadContext
    );
#endif


///
/// < Runtime Exception Handling Routines >
///

#ifdef _WIN64
typedef
PLIST_ENTRY(NTAPI *tRtlGetFunctionTableListHead)(
    VOID
    );
#endif


/// 
/// < Image Routines >
///

typedef PVOID(NTAPI *tRtlPcToFileHeader)(
    IN PVOID PcValue,
    OUT PVOID *BaseOfImage
    );

typedef PIMAGE_NT_HEADERS(NTAPI *tRtlImageNtHeader)(
    IN PVOID Base
    );

typedef NTSTATUS(NTAPI *tRtlImageNtHeaderEx)(
    IN ULONG Flags,
    IN PVOID Base,
    IN ULONG64 Size,
    OUT PIMAGE_NT_HEADERS *OutHeaders
    );

typedef PVOID(NTAPI *tRtlAddressInSectionTable)(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID BaseOfImage,
    IN ULONG VirtualAddress
    );

typedef PIMAGE_SECTION_HEADER(NTAPI *tRtlSectionTableFromVirtualAddress)(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID BaseOfImage,
    IN ULONG VirtualAddress
    );

typedef PVOID(NTAPI *tRtlImageDirectoryEntryToData)(
    IN PVOID BaseOfImage,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

typedef PIMAGE_SECTION_HEADER(NTAPI *tRtlImageRvaToSection)(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    );

typedef PVOID(NTAPI *tRtlImageRvaToVa)(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    );


///
/// < Memory Routines >
///

typedef SIZE_T(NTAPI *tRtlCompareMemoryUlong)(
    IN PVOID Source,
    IN SIZE_T Length,
    IN ULONG Pattern
    );

typedef VOID(NTAPI *tRtlFillMemoryUlong)(
    OUT PVOID Destination,
    IN SIZE_T Length,
    IN ULONG Pattern
    );

typedef VOID(NTAPI *tRtlFillMemoryUlonglong)(
    OUT PVOID Destination,
    IN SIZE_T Length,
    IN ULONGLONG Pattern
    );


///
/// < Directory and Path Routines >
///

typedef RTL_PATH_TYPE(NTAPI *tRtlDetermineDosPathNameType_U)(
    IN PCWSTR DosFileName
    );

typedef ULONG(NTAPI *tRtlIsDosDeviceName_U)(
    IN PCWSTR DosFileName
    );

typedef ULONG(NTAPI *tRtlGetFullPathName_U)(
    IN PCWSTR FileName,
    IN ULONG BufferLength,
    OUT PWSTR Buffer,
    OUT PWSTR *FilePart OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_WIN7)
typedef NTSTATUS(NTAPI *tRtlGetFullPathName_UEx)(
    IN PWSTR FileName,
    IN ULONG BufferLength,
    OUT PWSTR Buffer,
    OUT PWSTR *FilePart OPTIONAL,
    OUT RTL_PATH_TYPE *InputPathType OPTIONAL
    );
#endif

#if (NTDDI_VERSION >= NTDDI_WS03)
typedef NTSTATUS(NTAPI *tRtlGetFullPathName_UstrEx)(
    IN PUNICODE_STRING FileName,
    IN OUT PUNICODE_STRING StaticString,
    OUT PUNICODE_STRING DynamicString OPTIONAL,
    OUT PUNICODE_STRING *StringUsed OPTIONAL,
    OUT SIZE_T *FilePartPrefixCch OPTIONAL,
    OUT PBOOLEAN NameInvalid OPTIONAL,
    OUT RTL_PATH_TYPE *InputPathType,
    OUT SIZE_T *BytesRequired OPTIONAL
    );
#endif

typedef ULONG(NTAPI *tRtlGetCurrentDirectory_U)(
    IN ULONG BufferLength,
    OUT PWSTR Buffer
    );

typedef NTSTATUS(NTAPI *tRtlSetCurrentDirectory_U)(
    IN PUNICODE_STRING PathName
    );

typedef ULONG(NTAPI *tRtlGetLongestNtPathLength)(
    VOID
    );

typedef BOOLEAN(NTAPI *tRtlDosPathNameToNtPathName_U)(
    IN PCWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_WS03)
typedef NTSTATUS(NTAPI *tRtlDosPathNameToNtPathName_U_WithStatus)(
    IN PCWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );

typedef BOOLEAN(NTAPI *tRtlDosPathNameToRelativeNtPathName_U)(
    IN PWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlDosPathNameToRelativeNtPathName_U_WithStatus)(
    IN PWSTR DosFileName,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );

typedef VOID(NTAPI *tRtlReleaseRelativeName)(
    IN OUT PRTL_RELATIVE_NAME_U RelativeName
    );
#endif

typedef ULONG(NTAPI *tRtlDosSearchPath_U)(
    IN PWSTR Path,
    IN PWSTR FileName,
    IN PWSTR Extension OPTIONAL,
    IN ULONG BufferLength,
    OUT PWSTR Buffer,
    OUT PWSTR *FilePart OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlDosSearchPath_Ustr)(
    IN ULONG Flags,
    IN PUNICODE_STRING Path,
    IN PUNICODE_STRING FileName,
    IN PUNICODE_STRING DefaultExtension OPTIONAL,
    OUT PUNICODE_STRING StaticString OPTIONAL,
    OUT PUNICODE_STRING DynamicString OPTIONAL,
    OUT PCUNICODE_STRING *FullFileNameOut OPTIONAL,
    OUT SIZE_T *FilePartPrefixCch OPTIONAL,
    OUT SIZE_T *BytesRequired OPTIONAL
    );

typedef BOOLEAN(NTAPI *tRtlDoesFileExists_U)(
    IN PWSTR FileName
    );

typedef NTSTATUS(NTAPI *tRtlDosApplyFileIsolationRedirection_Ustr)(
    IN ULONG Flags,
    IN PUNICODE_STRING OriginalName,
    IN PUNICODE_STRING Extension,
    IN OUT PUNICODE_STRING StaticString,
    IN OUT PUNICODE_STRING DynamicString,
    IN OUT PUNICODE_STRING *NewName,
    IN PULONG  NewFlags,
    IN PSIZE_T FileNameSize,
    IN PSIZE_T RequiredLength
    );


///
/// < Heap Routines >
///

typedef PVOID(NTAPI *tRtlCreateHeap)(
    IN ULONG Flags,
    IN PVOID HeapBase OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize OPTIONAL,
    IN PVOID Lock OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

typedef PVOID(NTAPI *tRtlDestroyHeap)(
    IN _Post_invalid_ PVOID HeapHandle
    );

typedef PVOID(NTAPI *tRtlAllocateHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags OPTIONAL,
    IN SIZE_T Size
    );

typedef BOOLEAN(NTAPI *tRtlFreeHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags OPTIONAL,
    IN _Post_invalid_ PVOID BaseAddress
    );

typedef SIZE_T(NTAPI *tRtlSizeHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

typedef NTSTATUS(NTAPI *tRtlZeroHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

typedef VOID(NTAPI *tRtlProtectHeap)(
    IN PVOID HeapHandle,
    IN BOOLEAN MakeReadOnly
    );

typedef BOOLEAN(NTAPI *tRtlLockHeap)(
    IN PVOID HeapHandle
    );

typedef BOOLEAN(NTAPI *tRtlUnlockHeap)(
    IN PVOID HeapHandle
    );

typedef PVOID(NTAPI *tRtlReAllocateHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T Size
    );

typedef BOOLEAN(NTAPI *tRtlGetUserInfoHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    OUT PVOID *UserValue OPTIONAL,
    OUT PULONG UserFlags OPTIONAL
    );

typedef BOOLEAN(NTAPI *tRtlSetUserValueHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN PVOID UserValue
    );

typedef BOOLEAN(NTAPI *tRtlSetUserFlagsHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG UserFlagsReset,
    IN ULONG UserFlagsSet
    );

typedef ULONG(NTAPI *tRtlCreateTagHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PWSTR TagPrefix OPTIONAL,
    IN PWSTR TagNames
    );

typedef PWSTR(NTAPI *tRtlQueryTagHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN USHORT TagIndex,
    IN BOOLEAN ResetCounters,
    OUT PRTL_HEAP_TAG_INFO TagInfo OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlExtendHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID Base,
    IN SIZE_T Size
    );

typedef SIZE_T(NTAPI *tRtlCompactHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

typedef BOOLEAN(NTAPI *tRtlValidateHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

typedef BOOLEAN(NTAPI *tRtlValidateProcessHeaps)(
    VOID
    );

typedef ULONG(NTAPI *tRtlGetProcessHeaps)(
    IN ULONG NumberOfHeaps,
    OUT PVOID *ProcessHeaps
    );

typedef NTSTATUS(NTAPI *PRTL_ENUM_HEAPS_ROUTINE)(
    IN PVOID HeapHandle,
    IN PVOID Parameter
    );

typedef NTSTATUS(NTAPI *tRtlEnumProcessHeaps)(
    IN PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    IN PVOID Parameter
    );

typedef NTSTATUS(NTAPI *tRtlUsageHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN OUT PRTL_HEAP_USAGE Usage
    );

typedef NTSTATUS(NTAPI *tRtlWalkHeap)(
    IN PVOID HeapHandle,
    IN OUT PRTL_HEAP_WALK_ENTRY Entry
    );

typedef NTSTATUS(NTAPI *tRtlQueryHeapInformation)(
    IN PVOID HeapHandle,
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    OUT PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlSetHeapInformation)(
    IN PVOID HeapHandle,
    IN HEAP_INFORMATION_CLASS HeapInformationClass,
    IN PVOID HeapInformation OPTIONAL,
    IN SIZE_T HeapInformationLength OPTIONAL
    );

typedef ULONG(NTAPI *tRtlMultipleAllocateHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Count,
    OUT PVOID *Array
    );

typedef ULONG(NTAPI *tRtlMultipleFreeHeap)(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Count,
    IN PVOID *Array
    );

#if (NTDDI_VERSION >= NTDDI_WIN7)
typedef VOID(NTAPI *tRtlDetectHeapLeaks)(
    VOID
    );
#endif


///
/// < Error Routines >
///

typedef ULONG(NTAPI *tRtlNtStatusToDosError)(
    IN NTSTATUS Status
    );

typedef ULONG(NTAPI *tRtlNtStatusToDosErrorNoTeb)(
    IN NTSTATUS Status
    );

typedef NTSTATUS(NTAPI *tRtlGetLastNtStatus)(
    VOID
    );

typedef LONG(NTAPI *tRtlGetLastWin32Error)(
    VOID
    );

typedef VOID(NTAPI *tRtlSetLastWin32ErrorAndNtStatusFromNtStatus)(
    IN NTSTATUS Status
    );

typedef VOID(NTAPI *tRtlSetLastWin32Error)(
    IN LONG Win32Error
    );

typedef VOID(NTAPI *tRtlRestoreLastWin32Error)(
    IN LONG Win32Error
    );

typedef ULONG(NTAPI *tRtlGetThreadErrorMode)(
    VOID
    );

typedef NTSTATUS(NTAPI *tRtlSetThreadErrorMode)(
    IN ULONG NewMode,
    OUT PULONG OldMode OPTIONAL
    );


///
/// < Debugging Routines >
///

typedef PRTL_DEBUG_INFORMATION(NTAPI *tRtlCreateQueryDebugBuffer)(
    IN ULONG MaximumCommit OPTIONAL,
    IN BOOLEAN UseEventPair
    );

typedef NTSTATUS(NTAPI *tRtlDestroyQueryDebugBuffer)(
    IN PRTL_DEBUG_INFORMATION Buffer
    );

#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef PVOID(NTAPI *tRtlCommitDebugInfo)(
    IN OUT PRTL_DEBUG_INFORMATION Buffer,
    IN SIZE_T Size
    );

typedef VOID(NTAPI *tRtlDeCommitDebugInfo)(
    IN OUT PRTL_DEBUG_INFORMATION Buffer,
    IN PVOID p,
    IN SIZE_T Size
    );
#endif

typedef NTSTATUS(NTAPI *tRtlQueryProcessDebugInformation)(
    IN HANDLE UniqueProcessId,
    IN ULONG Flags,
    IN OUT PRTL_DEBUG_INFORMATION Buffer
    );

typedef VOID(NTAPI *tDbgUserBreakPoint)(
    VOID
    );

typedef VOID(NTAPI *tDbgBreakPoint)(
    VOID
    );

typedef VOID(NTAPI *tDbgBreakPointWithStatus)(
    IN ULONG Status
    );

typedef ULONG(__cdecl *tDbgPrint)(
    IN _Printf_format_string_ PSTR Format,
    ...
    );

typedef ULONG(__cdecl *tDbgPrintEx)(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN _Printf_format_string_ PSTR Format,
    ...
    );

typedef ULONG(NTAPI *tvDbgPrintEx)(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist
    );

typedef ULONG(NTAPI *tvDbgPrintExWithPrefix)(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    IN va_list arglist
    );

typedef NTSTATUS(NTAPI *tDbgQueryDebugFilterState)(
    IN ULONG ComponentId,
    IN ULONG Level
    );

typedef NTSTATUS(NTAPI *tDbgSetDebugFilterState)(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    );

typedef ULONG(NTAPI *tDbgPrompt)(
    IN PCH Prompt,
    OUT PCH Response,
    IN ULONG Length
    );


///
/// < Vectored Exception Handler Routines >
///

typedef PVOID(NTAPI *tRtlAddVectoredExceptionHandler)(
    IN ULONG First,
    IN PVECTORED_EXCEPTION_HANDLER Handler
    );

typedef ULONG(NTAPI *tRtlRemoveVectoredExceptionHandler)(
    IN PVOID Handle
    );

typedef PVOID(NTAPI *tRtlAddVectoredContinueHandler)(
    IN ULONG First,
    IN PVECTORED_EXCEPTION_HANDLER Handler
    );

typedef ULONG(NTAPI *tRtlRemoveVectoredContinueHandler)(
    IN PVOID Handle
    );


///
/// < Misc Security Routines >
///

typedef VOID(NTAPI *tRtlRunEncodeUnicodeString)(
    IN OUT PUCHAR Seed,
    IN PUNICODE_STRING String
    );

typedef VOID(NTAPI *tRtlRunDecodeUnicodeString)(
    IN UCHAR Seed,
    IN PUNICODE_STRING String
    );

typedef NTSTATUS(NTAPI *tRtlImpersonateSelf)(
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

typedef NTSTATUS(NTAPI *tRtlImpersonateSelfEx)(
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN ACCESS_MASK AdditionalAccess OPTIONAL,
    OUT PHANDLE ThreadToken OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlAdjustPrivilege)(
    IN ULONG Privilege,
    IN BOOLEAN Enable,
    IN BOOLEAN Client,
    OUT PBOOLEAN WasEnabled
    );

typedef NTSTATUS(NTAPI *tRtlAcquirePrivilege)(
    IN PULONG Privilege,
    IN ULONG NumPriv,
    IN ULONG Flags,
    OUT PVOID *ReturnedState
    );
#define RTL_ACQUIRE_PRIVILEGE_REVERT    (0x00000001)
#define RTL_ACQUIRE_PRIVILEGE_PROCESS   (0x00000002)

typedef VOID(NTAPI *tRtlReleasePrivilege)(
    IN PVOID StatePointer
    );

typedef NTSTATUS(NTAPI *tRtlRemovePrivileges)(
    IN HANDLE hToken,
    IN PULONG PrivilegesToKeep,
    IN ULONG PrivilegeCount
    );

typedef PVOID(NTAPI *tRtlCreateBoundaryDescriptor)(
    IN PUNICODE_STRING Name,
    IN ULONG Flags
    );

typedef VOID(NTAPI *tRtlDeleteBoundaryDescriptor)(
    IN PVOID BoundaryDescriptor
    );

typedef NTSTATUS(NTAPI *tRtlAddSIDToBoundaryDescriptor)(
    IN OUT PVOID *BoundaryDescriptor,
    IN PSID RequiredSid
    );

typedef NTSTATUS(NTAPI *tRtlAddIntegrityLabelToBoundaryDescriptor)(
    IN OUT PVOID *BoundaryDescriptor,
    IN PSID IntegrityLabel
    );


///
/// < Version Routines >
///

typedef NTSTATUS(NTAPI *tRtlGetVersion)(
    OUT PRTL_OSVERSIONINFOW lpVersionInformation
    );

typedef NTSTATUS(NTAPI *tRtlVerifyVersionInfo)(
    IN PRTL_OSVERSIONINFOEXW VersionInfo,
    IN ULONG TypeMask,
    IN ULONGLONG ConditionMask
    );


///
/// < System Info Routines >
///

typedef ULONG(NTAPI *tRtlGetNtGlobalFlags)(
    VOID
    );

typedef BOOLEAN(NTAPI *tRtlGetNtProductType)(
    OUT PNT_PRODUCT_TYPE NtProductType
    );

typedef VOID(NTAPI *tRtlGetNtVersionNumbers)(
    OUT PULONG NtMajorVersion OPTIONAL,
    OUT PULONG NtMinorVersion OPTIONAL,
    OUT PULONG NtBuildNumber OPTIONAL
    );

typedef NTSTATUS(NTAPI *tRtlGetNativeSystemInformation)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
