#pragma once

#include "NativeCommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < Native Memory Defines >
///

//
// Define the page size for the AMD64 as 4096 (0x1000).
//
#define PAGE_SIZE                                   0x1000

//
// Define the number of trailing zeroes in a page aligned virtual address.
// This is used as the shift count when shifting virtual addresses to
// virtual page numbers.
//
#define PAGE_SHIFT                                  12L

//
// The number of bits in a virtual address.
//
#define VIRTUAL_ADDRESS_BITS                        48
#define VIRTUAL_ADDRESS_MASK                        ((((ULONG_PTR)1) << VIRTUAL_ADDRESS_BITS) - 1)

//
// The number of bits in a physical address.
//

#define PHYSICAL_ADDRESS_BITS                       40

//
// AMD64 Specific portions of Mm component.
//

#define PXE_BASE                0xFFFFF6FB7DBED000UI64
#define PXE_SELFMAP             0xFFFFF6FB7DBEDF68UI64
#define PPE_BASE                0xFFFFF6FB7DA00000UI64
#define PDE_BASE                0xFFFFF6FB40000000UI64
#define PTE_BASE                0xFFFFF68000000000UI64

#define PXE_TOP                 0xFFFFF6FB7DBEDFFFUI64
#define PPE_TOP                 0xFFFFF6FB7DBFFFFFUI64
#define PDE_TOP                 0xFFFFF6FB7FFFFFFFUI64
#define PTE_TOP                 0xFFFFF6FFFFFFFFFFUI64

#define PTI_SHIFT                                   12
#define PDI_SHIFT                                   21
#define PPI_SHIFT                                   30
#define PXI_SHIFT                                   39

#define PTE_SHIFT                                   3

#define PTE_PER_PAGE                                512
#define PDE_PER_PAGE                                512
#define PPE_PER_PAGE                                512
#define PXE_PER_PAGE                                512

#define PTI_MASK_AMD64                              (PTE_PER_PAGE - 1)
#define PDI_MASK_AMD64                              (PDE_PER_PAGE - 1)
#define PPI_MASK                                    (PPE_PER_PAGE - 1)
#define PXI_MASK                                    (PXE_PER_PAGE - 1)

#define GUARD_PAGE_SIZE (PAGE_SIZE * 2)

#define MMPFNLIST_ZERO                              (0)
#define MMPFNLIST_FREE                              (1)
#define MMPFNLIST_STANDBY                           (2)
#define MMPFNLIST_MODIFIED                          (3)
#define MMPFNLIST_MODIFIEDNOWRITE                   (4)
#define MMPFNLIST_BAD                               (5)
#define MMPFNLIST_ACTIVE                            (6)
#define MMPFNLIST_TRANSITION                        (7)

#define MMPFNUSE_PROCESSPRIVATE                     (0)
#define MMPFNUSE_FILE                               (1)
#define MMPFNUSE_PAGEFILEMAPPED                     (2)
#define MMPFNUSE_PAGETABLE                          (3)
#define MMPFNUSE_PAGEDPOOL                          (4)
#define MMPFNUSE_NONPAGEDPOOL                       (5)
#define MMPFNUSE_SYSTEMPTE                          (6)
#define MMPFNUSE_SESSIONPRIVATE                     (7)
#define MMPFNUSE_METAFILE                           (8)
#define MMPFNUSE_AWEPAGE                            (9)
#define MMPFNUSE_DRIVERLOCKPAGE                     (10)

#define SEC_BASED                                   0x200000
#define SEC_NO_CHANGE                               0x400000
#define SEC_GLOBAL                                  0x20000000

#define MEM_EXECUTE_OPTION_DISABLE                  0x01
#define MEM_EXECUTE_OPTION_ENABLE                   0x02
#define MEM_EXECUTE_OPTION_DISABLE_THUNK_EMULATION  0x04
#define MEM_EXECUTE_OPTION_PERMANENT                0x08
#define MEM_EXECUTE_OPTION_EXECUTE_DISPATCH_ENABLE  0x10
#define MEM_EXECUTE_OPTION_IMAGE_DISPATCH_ENABLE    0x20
#define MEM_EXECUTE_OPTION_VALID_FLAGS              0x3F

#ifndef MEMORY_PARTITION_QUERY_ACCESS
#define MEMORY_PARTITION_QUERY_ACCESS               0x0001
#define MEMORY_PARTITION_MODIFY_ACCESS              0x0002
#define MEMORY_PARTITION_ALL_ACCESS                 (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                                    MEMORY_PARTITION_QUERY_ACCESS | MEMORY_PARTITION_MODIFY_ACCESS)
#endif

//
// Definition of a hardware linear address 
//
typedef struct _LINEAR_ADDRESS {
    union {
        ULONGLONG Address;
        struct {
            ULONGLONG PageOffset : 12;  // 11:00
            ULONGLONG PTEIndex : 9;     // 20:12 Page Table Entry
            ULONGLONG PDEIndex : 9;     // 29:21 Page Directory Entry
            ULONGLONG PPEIndex : 9;     // 38:30 Page Directory Pointer Entry
            ULONGLONG PXEIndex : 9;     // 47:39 Page Map Level 4 Table Entry A.K.A. eXtended Page Directory Entry (PXE)
            ULONGLONG Reserved0 : 16;   // 63:48
        };
    } u;  
} LINEAR_ADDRESS, *PLINEAR_ADDRESS;

//
// AMD64 hardware structures
//
// A Page Table Entry on an AMD64 has the following definition.
//
#define _HARDWARE_PTE_WORKING_SET_BITS  11

typedef struct _HARDWARE_PTE {
    ULONGLONG Valid                 : 1;    // Present (P) Bit.
    ULONGLONG Write                 : 1;    // Read/Write (R/W) Bit.
    ULONGLONG Owner                 : 1;    // User/Supervisor (U/S) Bit.
    ULONGLONG WriteThrough          : 1;    // Page-Level Writethrough (PWT) Bit.
    ULONGLONG CacheDisable          : 1;    // Page-Level Cache Disable (PCD) Bit.
    ULONGLONG Accessed              : 1;    // Accessed (A) Bit.
    ULONGLONG Dirty                 : 1;    // Dirty (D) Bit.
    ULONGLONG LargePage             : 1;    // Page Size (PS) Bit. 
                                            // If the lowest level is a PTE (PDE.PS=0), PAT occupies bit 7.
                                            // If the lowest level is a PDE (PDE.PS=1) or PDPE (PDPE.PS=1), PAT occupies bit 12.
    ULONGLONG Global                : 1;    // Global Page (G) Bit.
    ULONGLONG CopyOnWrite           : 1;    // Available to Software (AVL) Bit 0 software field.
    ULONGLONG Prototype             : 1;    // Available to Software (AVL) Bit 1 software field.
    ULONGLONG reserved0             : 1;    // Available to Software (AVL) Bit 2 software field.
    ULONGLONG PageFrameNumber       : 36;   // Page Frame Number or Physical Page Base Address. 
                                            // This is has an architectural limit of 40 bits. A given implementation may support fewer bits
    ULONGLONG reserved1             : 16 - (_HARDWARE_PTE_WORKING_SET_BITS+1);
    ULONGLONG SoftwareWsIndex       : _HARDWARE_PTE_WORKING_SET_BITS; // Software working set index.
    ULONGLONG NoExecute             : 1;    // No Execute (NX) Bit.
} HARDWARE_PTE, *PHARDWARE_PTE;

//
// The Software PTE refers to a paged out page.
//
// If both the Prototype and Transition flags are unset, the PTE refers 
// to a Software PTE.
//
typedef struct _MMPTE_SOFTWARE {
     ULONGLONG Valid                : 1;
     ULONGLONG PageFileLow          : 4;    // Number of the page file (Windows supports up to 16 page files).
     ULONGLONG Protection           : 5;
     ULONGLONG Prototype            : 1;
     ULONGLONG Transition           : 1;
     ULONGLONG PageFileReserved     : 1;
     ULONGLONG PageFileAllocated    : 1;
     ULONGLONG UsedPageTableEntries : 10;
     ULONGLONG Unused               : 8;    // Unused
     ULONGLONG PageFileHigh         : 32;   // The frame number in the page file. Must be multiplied by a page size to get the file offset.
} MMPTE_SOFTWARE, *PMMPTE_SOFTWARE;
  
typedef struct _MMPTE_TRANSITION {
    ULONGLONG Valid                 : 1;
    ULONGLONG Write                 : 1;
    ULONGLONG Spare                 : 2;
    ULONGLONG IoTracker             : 1;
    ULONGLONG Protection            : 5;
    ULONGLONG Prototype             : 1;
    ULONGLONG Transition            : 1;
    ULONGLONG PageFrameNumber       : 36;
    ULONGLONG Unused                : 16;
} MMPTE_TRANSITION, *PMMPTE_TRANSITION;

typedef struct _MMPTE_PROTOTYPE {
    ULONGLONG Valid                 : 1;
    ULONGLONG DemandFillProto       : 1;
    ULONGLONG HiberVerifyConverted  : 1;
    ULONGLONG Unused1               : 5;
    ULONGLONG ReadOnly              : 1;
    ULONGLONG Combined              : 1;
    ULONGLONG Prototype             : 1;
    ULONGLONG Protection            : 5;
    ULONGLONG ProtoAddress          : 48;
} MMPTE_PROTOTYPE, *PMMPTE_PROTOTYPE;

//
// A Subsection PTE refers to an instance of a SUBSECTION object and is 
// used to denote a File Mapping.
//
// If the PTE has the Prototype bit set, and Valid bit unset (P=1, V=0) it 
// is a Subsection PTE.
//
typedef struct _MMPTE_SUBSECTION {
    ULONGLONG Valid                 : 1;
    ULONGLONG Unused0               : 4;
    ULONGLONG Protection            : 5;
    ULONGLONG Prototype             : 1;
    ULONGLONG Unused1               : 4;
    ULONGLONG ExecutePrivilege      : 1;
    ULONGLONG SubsectionAddress     : 48;
} MMPTE_SUBSECTION, *PMMPTE_SUBSECTION;

typedef struct _MMPTE_TIMESTAMP {
    ULONGLONG MustBeZero            : 1;
    ULONGLONG PageFileLow           : 4;
    ULONGLONG Protection            : 5;
    ULONGLONG Prototype             : 1;
    ULONGLONG Transition            : 1;
    ULONGLONG Reserved              : 20;
    ULONGLONG GlobalTimeStamp       : 32;
} MMPTE_TIMESTAMP, *PMMPTE_TIMESTAMP;

typedef struct _MMPTE_LIST {
    ULONGLONG Valid                 : 1;
    ULONGLONG OneEntry              : 1;
    ULONGLONG filler0               : 3;
    //
    // Note the Prototype bit must not be used for lists like freed nonpaged
    // pool because lookaside pops can legitimately reference bogus addresses
    // (since the pop is unsynchronized) and the fault handler must be able to
    // distinguish lists from protos so a retry status can be returned (vs a
    // fatal bugcheck).
    //
    // The same caveat applies to both the Transition and the Protection
    // fields as they are similarly examined in the fault handler and would
    // be misinterpreted if ever nonzero in the freed nonpaged pool chains.
    //
    ULONGLONG Protection            : 5;
    ULONGLONG Prototype             : 1;  // MUST BE ZERO as per above comment.
    ULONGLONG Transition            : 1;
    ULONGLONG filler1               : 16;
    ULONGLONG NextEntry             : 36;
} MMPTE_LIST, *PMMPTE_LIST;

typedef struct _MMPTE_HIGHLOW {
    ULONG LowPart;
    ULONG HighPart;
} MMPTE_HIGHLOW;

typedef struct _MMPTE_HARDWARE_LARGEPAGE {
    ULONGLONG Valid                 : 1;
    ULONGLONG Write                 : 1;
    ULONGLONG Owner                 : 1;
    ULONGLONG WriteThrough          : 1;
    ULONGLONG CacheDisable          : 1;
    ULONGLONG Accessed              : 1;
    ULONGLONG Dirty                 : 1;
    ULONGLONG LargePage             : 1;
    ULONGLONG Global                : 1;
    ULONGLONG CopyOnWrite           : 1;    // software field
    ULONGLONG Prototype             : 1;    // software field
    ULONGLONG reserved0             : 1;    // software field
    ULONGLONG PAT                   : 1;    
    ULONGLONG reserved1             : 8;    // software field
    ULONGLONG PageFrameNumber       : 27;
    ULONGLONG reserved2             : 16;   // software field
} MMPTE_HARDWARE_LARGEPAGE, *PMMPTE_HARDWARE_LARGEPAGE;

//
// A Page Table Entry on AMD64 has the following definition.
// Note the MP version is to avoid stalls when flushing TBs across processors.
//
typedef struct _MMPTE_HARDWARE {
    ULONGLONG Valid                 : 1;
    ULONGLONG Dirty1                : 1;
    ULONGLONG Owner                 : 1;
    ULONGLONG WriteThrough          : 1;
    ULONGLONG CacheDisable          : 1;
    ULONGLONG Accessed              : 1;
    ULONGLONG Dirty                 : 1;
    ULONGLONG LargePage             : 1;
    ULONGLONG Global                : 1;
    ULONGLONG CopyOnWrite           : 1;
    ULONGLONG Unused                : 1;
    ULONGLONG Write                 : 1;    // software field - MP change
    ULONGLONG PageFrameNumber       : 36;
    ULONGLONG ReservedForHardware   : 4;
    ULONGLONG ReservedForSoftware   : 4;
    ULONGLONG WsleAge               : 4;
    ULONGLONG WsleProtection        : 3;
    ULONGLONG NoExecute             : 1;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

typedef struct _MMPTE {
    union {
        ULONGLONG Long;
        MMPTE_HARDWARE Hard;
        MMPTE_HARDWARE_LARGEPAGE HardLarge;
        HARDWARE_PTE Flush;
        MMPTE_PROTOTYPE Proto;
        MMPTE_SOFTWARE Soft;
        MMPTE_TRANSITION Trans;
        MMPTE_SUBSECTION Subsect;
        MMPTE_LIST List;
    } u;
} MMPTE, *PMMPTE;

#define MiGetPxeOffset(va)  ((ULONG)(((ULONGLONG)(va) >> PXI_SHIFT) & PXI_MASK))
#define MiGetPxeIndex(va)   ((ULONG)((ULONGLONG)(va) >> PXI_SHIFT))
#define MiGetPpeOffset(va)  ((ULONG)(((ULONGLONG)(va) >> PPI_SHIFT) & PPI_MASK))
#define MiGetPpeIndex(va)   ((ULONG)((ULONGLONG)(va) >> PPI_SHIFT))
#define MiGetPdeOffset(va)  ((ULONG)(((ULONGLONG)(va) >> PDI_SHIFT) & (PDE_PER_PAGE - 1)))
#define MiGetPdeIndex(va)   ((ULONG)((ULONGLONG)(va) >> PDI_SHIFT))
#define MiGetPteOffset(va)  ((ULONG)(((ULONGLONG)(va) >> PTI_SHIFT) & (PTE_PER_PAGE - 1)))
#define MiGetPteIndex(va)   ((ULONG)((ULONGLONG)(va) >> PTI_SHIFT))

#define MiGetPxeAddress(va) \
    ((PMMPTE)PXE_BASE + MiGetPxeOffset(va))
#define MiGetPpeAddress(va) \
    ((PMMPTE)(((((ULONGLONG)(va) & VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + PPE_BASE))
#define MiGetPdeAddress(va) \
    ((PMMPTE)(((((ULONGLONG)(va) & VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + PDE_BASE))
#define MiGetPteAddress(va) \
    ((PMMPTE)(((((ULONGLONG)(va) & VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + PTE_BASE))

#define MiGetPpeAddressRs1(va) \
    ((PMMPTE)(((((ULONGLONG)(va) >> PPI_SHIFT) << PTE_SHIFT) & ((VIRTUAL_ADDRESS_MASK >> PPI_SHIFT) << PTE_SHIFT)) + PPE_BASE))
#define MiGetPdeAddressRs1(va) \
    ((PMMPTE)(((((ULONGLONG)(va) >> PDI_SHIFT) << PTE_SHIFT) & ((VIRTUAL_ADDRESS_MASK >> PDI_SHIFT) << PTE_SHIFT)) + PDE_BASE))
#define MiGetPteAddressRs1(va) \
    ((PMMPTE)(((((ULONGLONG)(va) >> PTI_SHIFT) << PTE_SHIFT) & ((VIRTUAL_ADDRESS_MASK >> PTI_SHIFT) << PTE_SHIFT)) + PTE_BASE))


#define MI_PDE_MAPS_LARGE_PAGE(PDE)                 ((PDE)->u.Hard.LargePage == 1)
#define MI_GET_PAGE_FRAME_FROM_PTE(PTE)             ((PTE)->u.Hard.PageFrameNumber)
#define MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(PTE)  ((PTE)->u.Trans.PageFrameNumber)
#define MI_GET_PROTECTION_FROM_SOFT_PTE(PTE)        ((ULONG)(PTE)->u.Soft.Protection)
#define MI_GET_PROTECTION_FROM_TRANSITION_PTE(PTE)  ((ULONG)(PTE)->u.Trans.Protection)

///
/// < Memory Enums >
///

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation,             // MEMORY_BASIC_INFORMATION
    MemoryWorkingSetInformation,        // MEMORY_WORKING_SET_INFORMATION
    MemoryMappedFilenameInformation,    // UNICODE_STRING
    MemoryRegionInformation,            // MEMORY_REGION_INFORMATION
    MemoryWorkingSetExInformation,      // MEMORY_WORKING_SET_EX_INFORMATION
    MemorySharedCommitInformation,      // MEMORY_SHARED_COMMIT_INFORMATION
    MemoryImageInformation,             // MEMORY_IMAGE_INFORMATION
    MemoryRegionInformationEx,          
    MemoryPrivilegedBasicInformation,   
    MemoryEnclaveImageInformation,      // since RS3
    MemoryBasicInformationCapped
} MEMORY_INFORMATION_CLASS;

typedef enum _VIRTUAL_MEMORY_INFORMATION_CLASS {
    VmPrefetchInformation,
    VmPagePriorityInformation,
    VmCfgCallTargetInformation,     // CFG_CALL_TARGET_LIST_INFORMATION since RS2
    VmPageDirtyStateInformation     // Since RS3
} VIRTUAL_MEMORY_INFORMATION_CLASS;

typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation,
    SectionRelocationInformation, // name:wow64:whNtQuerySection_SectionRelocationInformation
    MaxSectionInfoClass
} SECTION_INFORMATION_CLASS;

typedef enum _MEMORY_PARTITION_INFORMATION_CLASS {
    SystemMemoryPartitionInformation,       // q: MEMORY_PARTITION_CONFIGURATION_INFORMATION
    SystemMemoryPartitionMoveMemory,        // s: MEMORY_PARTITION_TRANSFER_INFORMATION
    SystemMemoryPartitionAddPagefile,       // s: MEMORY_PARTITION_PAGEFILE_INFORMATION
    SystemMemoryPartitionCombineMemory,     // q; s: MEMORY_PARTITION_PAGE_COMBINE_INFORMATION
    SystemMemoryPartitionInitialAddMemory,  // q; s: MEMORY_PARTITION_INITIAL_ADD_INFORMATION
    SystemMemoryPartitionGetMemoryEvents    // MEMORY_PARTITION_MEMORY_EVENTS_INFORMATION // since REDSTONE2
} MEMORY_PARTITION_INFORMATION_CLASS;



///
/// < Memory Structures >
///

//
// Types to use to contain PFNs and their counts.
//
typedef ULONG PFN_COUNT;
typedef LONG64 SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG64 PFN_NUMBER, *PPFN_NUMBER;

typedef struct _MDL {
    struct _MDL *Next;
    CSHORT Size;
    CSHORT MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;   // see creators for field size annotations.
    PVOID StartVa;          // see creators for validity; could be address 0.
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

typedef struct _MEMORY_RANGE_ENTRY {
    PVOID VirtualAddress;
    SIZE_T NumberOfBytes;
} MEMORY_RANGE_ENTRY, *PMEMORY_RANGE_ENTRY;

//
// These are defined in win
//
//typedef struct _MEMORY_BASIC_INFORMATION {
//    PVOID BaseAddress;
//    PVOID AllocationBase;
//    ULONG AllocationProtect;
//    SIZE_T RegionSize;
//    ULONG State;
//    ULONG Protect;
//    ULONG Type;
//} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
//
//typedef struct _MEMORY_BASIC_INFORMATION32 {
//    ULONG BaseAddress;
//    ULONG AllocationBase;
//    ULONG AllocationProtect;
//    ULONG RegionSize;
//    ULONG State;
//    ULONG Protect;
//    ULONG Type;
//} MEMORY_BASIC_INFORMATION32, *PMEMORY_BASIC_INFORMATION32;
//
//typedef struct DECLSPEC_ALIGN( 16 ) _MEMORY_BASIC_INFORMATION64 {
//    ULONGLONG BaseAddress;
//    ULONGLONG AllocationBase;
//    ULONG     AllocationProtect;
//    ULONG     __alignment1;
//    ULONGLONG RegionSize;
//    ULONG     State;
//    ULONG     Protect;
//    ULONG     Type;
//    ULONG     __alignment2;
//} MEMORY_BASIC_INFORMATION64, *PMEMORY_BASIC_INFORMATION64;

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && !defined(_X86AMD64_)
#if defined(_WIN64)
C_ASSERT( sizeof( MEMORY_BASIC_INFORMATION ) == sizeof( MEMORY_BASIC_INFORMATION64 ) );
#else
C_ASSERT( sizeof( MEMORY_BASIC_INFORMATION ) == sizeof( MEMORY_BASIC_INFORMATION32 ) );
#endif
#endif

typedef struct _MEMORY_WORKING_SET_BLOCK {
    ULONG_PTR Protection : 5;
    ULONG_PTR ShareCount : 3;
    ULONG_PTR Shared : 1;
    ULONG_PTR Node : 3;
#if defined(_WIN64)
    ULONG_PTR VirtualPage : 52;
#else
    ULONG VirtualPage : 20;
#endif
} MEMORY_WORKING_SET_BLOCK, *PMEMORY_WORKING_SET_BLOCK;

typedef struct _MEMORY_WORKING_SET_EX_BLOCK {
    union {
        struct {
            ULONG_PTR Valid : 1;
            ULONG_PTR ShareCount : 3;
            ULONG_PTR Win32Protection : 11;
            ULONG_PTR Shared : 1;
            ULONG_PTR Node : 6;
            ULONG_PTR Locked : 1;
            ULONG_PTR LargePage : 1;
            ULONG_PTR Priority : 3;
            ULONG_PTR Reserved : 3;
            ULONG_PTR SharedOriginal : 1;
            ULONG_PTR Bad : 1;
#if defined(_WIN64)
            ULONG_PTR ReservedUlong : 32;
#endif
        };
        struct {
            ULONG_PTR Valid : 1;
            ULONG_PTR Reserved0 : 14;
            ULONG_PTR Shared : 1;
            ULONG_PTR Reserved1 : 5;
            ULONG_PTR PageTable : 1;
            ULONG_PTR Location : 2;
            ULONG_PTR Priority : 3;
            ULONG_PTR ModifiedList : 1;
            ULONG_PTR Reserved2 : 2;
            ULONG_PTR SharedOriginal : 1;
            ULONG_PTR Bad : 1;
#if defined(_WIN64)
            ULONG_PTR ReservedUlong : 32;
#endif
        } Invalid;
    };
} MEMORY_WORKING_SET_EX_BLOCK, *PMEMORY_WORKING_SET_EX_BLOCK;

typedef struct _MEMORY_WORKING_SET_EX_INFORMATION {
    PVOID VirtualAddress;
    union {
        MEMORY_WORKING_SET_EX_BLOCK VirtualAttributes;
        ULONG_PTR Long;
    } u1;
} MEMORY_WORKING_SET_EX_INFORMATION, *PMEMORY_WORKING_SET_EX_INFORMATION;

typedef struct _MEMORY_WORKING_SET_INFORMATION {
    ULONG_PTR NumberOfEntries;
    MEMORY_WORKING_SET_BLOCK WorkingSetInfo[1];
} MEMORY_WORKING_SET_INFORMATION, *PMEMORY_WORKING_SET_INFORMATION;

typedef struct _MEMORY_REGION_INFORMATION {
    PVOID AllocationBase;
    ULONG AllocationProtect;
    union {
        ULONG RegionType;
        struct {
            ULONG Private : 1;
            ULONG MappedDataFile : 1;
            ULONG MappedImage : 1;
            ULONG MappedPageFile : 1;
            ULONG MappedPhysical : 1;
            ULONG DirectMapped : 1;
            ULONG SoftwareEnclave : 1; // Since RS3
            ULONG PageSize64K : 1;
            ULONG Reserved : 24;
        };
    };
    SIZE_T RegionSize;
    SIZE_T CommitSize;
} MEMORY_REGION_INFORMATION, *PMEMORY_REGION_INFORMATION;

typedef struct _MEMORY_SHARED_COMMIT_INFORMATION {
    SIZE_T CommitSize;
} MEMORY_SHARED_COMMIT_INFORMATION, *PMEMORY_SHARED_COMMIT_INFORMATION;

typedef struct _MEMORY_IMAGE_INFORMATION {
    PVOID ImageBase;
    SIZE_T SizeOfImage;
    union {
        ULONG ImageFlags;
        struct {
            ULONG ImagePartialMap : 1;
            ULONG ImageNotExecutable : 1;
            ULONG ImageSigningLevel : 1; // Since RS3
            ULONG Reserved : 30;
        };
    };
} MEMORY_IMAGE_INFORMATION, *PMEMORY_IMAGE_INFORMATION;

typedef struct _MEMORY_FRAME_INFORMATION {
    ULONGLONG UseDescription : 4;       // MMPFNUSE_*
    ULONGLONG ListDescription : 3;      // MMPFNLIST_*
    ULONGLONG Reserved0 : 1;            // reserved for future expansion
    ULONGLONG Pinned : 1;               // 1 - pinned, 0 - not pinned
    ULONGLONG DontUse : 48;             // *_INFORMATION overlay
    ULONGLONG Priority : 3;             // rev
    ULONGLONG Reserved : 4;             // reserved for future expansion
} MEMORY_FRAME_INFORMATION;

typedef struct _FILEOFFSET_INFORMATION {
    ULONGLONG DontUse : 9;              // overlaid with MEMORY_FRAME_INFORMATION
    ULONGLONG Offset : 48;              // used for mapped files only.
    ULONGLONG Reserved : 7;             // reserved for future expansion
} FILEOFFSET_INFORMATION;

typedef struct _PAGEDIR_INFORMATION {
    ULONGLONG DontUse : 9;              // overlaid with MEMORY_FRAME_INFORMATION
    ULONGLONG PageDirectoryBase : 48;   // used for private pages only.
    ULONGLONG Reserved : 7;             // Reserved for future expansion
} PAGEDIR_INFORMATION;

typedef struct _UNIQUE_PROCESS_INFORMATION {
    ULONGLONG DontUse : 9;              // overlaid with MEMORY_FRAME_INFORMATION
    ULONGLONG UniqueProcessKey : 48;    // ProcessId
    ULONGLONG Reserved : 7;             // Reserved for future expansion
} UNIQUE_PROCESS_INFORMATION, *PUNIQUE_PROCESS_INFORMATION;

typedef struct _MMPFN_IDENTITY {
    union {
        MEMORY_FRAME_INFORMATION e1;    // used for all cases.
        FILEOFFSET_INFORMATION e2;      // used for mapped files only.
        PAGEDIR_INFORMATION e3;         // used for private pages only.
        UNIQUE_PROCESS_INFORMATION e4;  // used for owning process only.
    } u1;
    ULONG_PTR PageFrameIndex;           // used for all cases.
    union {
        struct {
            ULONG_PTR Image : 1;
            ULONG_PTR Mismatch : 1;
        } e1;
        struct {
            ULONG_PTR CombinedPage;
        } e2;
        PVOID FileObject;               // used for mapped files only.
        ULONG_PTR UniqueFileObjectKey;
        ULONG_PTR ProtoPteAddress;
        PVOID VirtualAddress;           // used for everything but mapped files.
    } u2;
} MMPFN_IDENTITY, *PMMPFN_IDENTITY;

typedef struct _MMPFN_MEMSNAP_INFORMATION {
    ULONG_PTR InitialPageFrameIndex;
    ULONG_PTR Count;
} MMPFN_MEMSNAP_INFORMATION, *PMMPFN_MEMSNAP_INFORMATION;

typedef struct _SECTIONBASICINFO {
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID TransferAddress;
    ULONG ZeroBits;
    SIZE_T MaximumStackSize;
    SIZE_T CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    //ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    union {
        UCHAR ImageFlags;
        struct {
            UCHAR ComPlusNativeReady : 1;
            UCHAR ComPlusILOnly : 1;
            UCHAR ImageDynamicallyRelocated : 1;
            UCHAR ImageMappedFlat : 1;
            UCHAR BaseBelow4gb : 1;
            UCHAR ComPlusPrefer32bit : 1;
            UCHAR Reserved : 2;
        };
    };
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG CheckSum;
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

//
// This structure is used only by Wow64 processes. The offsets
// of structure elements should the same as viewed by a native Win64 application.
//
typedef struct _SECTION_IMAGE_INFORMATION64 {
    ULONGLONG TransferAddress;
    ULONG ZeroBits;
    ULONGLONG MaximumStackSize;
    ULONGLONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    //ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    union {
        UCHAR ImageFlags;
        struct {
            UCHAR ComPlusNativeReady : 1;
            UCHAR ComPlusILOnly : 1;
            UCHAR ImageDynamicallyRelocated : 1;
            UCHAR ImageMappedFlat : 1;
            UCHAR BaseBelow4gb : 1;
            UCHAR ComPlusPrefer32bit : 1;
            UCHAR Reserved : 2;
        };
    };
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG CheckSum;
} SECTION_IMAGE_INFORMATION64, *PSECTION_IMAGE_INFORMATION64;
#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && defined(_WIN64) && !defined(_X86AMD64_)
C_ASSERT( sizeof( SECTION_IMAGE_INFORMATION ) == sizeof( SECTION_IMAGE_INFORMATION64 ) );
#endif

typedef struct _SECTION_INTERNAL_IMAGE_INFORMATION {
    SECTION_IMAGE_INFORMATION SectionInformation;
    union {
        ULONG ExtendedFlags;
        struct {
            ULONG ImageExportSuppressionEnabled : 1;
            ULONG Reserved : 31;
        };
    };
} SECTION_INTERNAL_IMAGE_INFORMATION, *PSECTION_INTERNAL_IMAGE_INFORMATION;


typedef struct _MEMORY_PARTITION_CONFIGURATION_INFORMATION {
    ULONG Flags;
    ULONG NumaNode;
    ULONG Channel;
    ULONG NumberOfNumaNodes;
    ULONG_PTR ResidentAvailablePages;
    ULONG_PTR CommittedPages;
    ULONG_PTR CommitLimit;
    ULONG_PTR PeakCommitment;
    ULONG_PTR TotalNumberOfPages;
    ULONG_PTR AvailablePages;
    ULONG_PTR ZeroPages;
    ULONG_PTR FreePages;
    ULONG_PTR StandbyPages;
    ULONG_PTR StandbyPageCountByPriority[8]; // since RS2
    ULONG_PTR RepurposedPagesByPriority[8];
    ULONG_PTR MaximumCommitLimit;
    ULONG_PTR DonatedPagesToPartitions;
    ULONG PartitionId; // since RS3
} MEMORY_PARTITION_CONFIGURATION_INFORMATION, *PMEMORY_PARTITION_CONFIGURATION_INFORMATION;

typedef struct _MEMORY_PARTITION_TRANSFER_INFORMATION {
    ULONG_PTR NumberOfPages;
    ULONG NumaNode;
    ULONG Flags;
} MEMORY_PARTITION_TRANSFER_INFORMATION, *PMEMORY_PARTITION_TRANSFER_INFORMATION;

typedef struct _MEMORY_PARTITION_PAGEFILE_INFORMATION {
    UNICODE_STRING PageFileName;
    LARGE_INTEGER MinimumSize;
    LARGE_INTEGER MaximumSize;
    ULONG Flags;
} MEMORY_PARTITION_PAGEFILE_INFORMATION, *PMEMORY_PARTITION_PAGEFILE_INFORMATION;

typedef struct _MEMORY_PARTITION_PAGE_COMBINE_INFORMATION {
    HANDLE StopHandle;
    ULONG Flags;
    ULONG_PTR TotalNumberOfPages;
} MEMORY_PARTITION_PAGE_COMBINE_INFORMATION, *PMEMORY_PARTITION_PAGE_COMBINE_INFORMATION;

typedef struct _MEMORY_PARTITION_PAGE_RANGE {
    ULONG_PTR StartPage;
    ULONG_PTR NumberOfPages;
} MEMORY_PARTITION_PAGE_RANGE, *PMEMORY_PARTITION_PAGE_RANGE;

typedef struct _MEMORY_PARTITION_INITIAL_ADD_INFORMATION {
    ULONG Flags;
    ULONG NumberOfRanges;
    ULONG_PTR NumberOfPagesAdded;
    MEMORY_PARTITION_PAGE_RANGE PartitionRanges[1];
} MEMORY_PARTITION_INITIAL_ADD_INFORMATION, *PMEMORY_PARTITION_INITIAL_ADD_INFORMATION;

typedef struct _MEMORY_PARTITION_MEMORY_EVENTS_INFORMATION {
    union {
        struct {
            ULONG CommitEvents : 1;
            ULONG Spare : 31;
        };
        ULONG AllFlags;
    } Flags;
    ULONG HandleAttributes;
    ULONG DesiredAccess;
    HANDLE LowCommitCondition; // \\KernelObjects\\LowCommitCondition
    HANDLE HighCommitCondition; // \\KernelObjects\\HighCommitCondition
    HANDLE MaximumCommitCondition; // \\KernelObjects\\MaximumCommitCondition
} MEMORY_PARTITION_MEMORY_EVENTS_INFORMATION, *PMEMORY_PARTITION_MEMORY_EVENTS_INFORMATION;




///
/// < Memory Routines >
///

typedef NTSTATUS( NTAPI *tNtAllocateVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

typedef NTSTATUS( NTAPI *tNtFreeVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    );

typedef NTSTATUS( NTAPI *tNtReadVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    OUT PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtWriteVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesWritten OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtProtectVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
    );

typedef NTSTATUS( NTAPI *tNtQueryVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN SIZE_T MemoryInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_WINBLUE)
typedef NTSTATUS( NTAPI *tNtSetInformationVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN VIRTUAL_MEMORY_INFORMATION_CLASS VmInformationClass,
    IN ULONG_PTR NumberOfEntries,
    IN PMEMORY_RANGE_ENTRY VirtualAddresses,
    IN PVOID VmInformation,
    IN ULONG VmInformationLength
    );
#endif

typedef NTSTATUS( NTAPI *tNtLockVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG MapType
    );

typedef NTSTATUS( NTAPI *tNtUnlockVirtualMemory )(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG MapType
    );

typedef NTSTATUS( NTAPI *tNtCreateSection )(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtOpenSection )(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef NTSTATUS( NTAPI *tNtMapViewOfSection )(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Win32Protect
    );

typedef NTSTATUS( NTAPI *tNtUnmapViewOfSection )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_WIN8)
typedef NTSTATUS( NTAPI *tNtUnmapViewOfSectionEx )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN ULONG Flags
    );
#endif

typedef NTSTATUS( NTAPI *tNtExtendSection )(
    IN HANDLE SectionHandle,
    IN OUT PLARGE_INTEGER NewSectionSize
    );

typedef NTSTATUS( NTAPI *tNtQuerySection )(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN SIZE_T SectionInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtAreMappedFilesTheSame )(
    IN PVOID File1MappedAsAnImage,
    IN PVOID File2MappedAsFile
    );

#if (NTDDI_VERSION >= NTDDI_WINBLUE)
typedef NTSTATUS( NTAPI *tNtCreatePartition )(
    OUT PHANDLE PartitionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN ULONG PreferredNode
    );

typedef NTSTATUS( NTAPI *tNtOpenPartition )(
    OUT PHANDLE PartitionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef NTSTATUS( NTAPI *tNtManagePartition )(
    IN MEMORY_PARTITION_INFORMATION_CLASS PartitionInformationClass,
    IN PVOID PartitionInformation,
    IN ULONG PartitionInformationLength
    );
#endif

// User physical pages
typedef NTSTATUS( NTAPI *tNtMapUserPhysicalPages )(
    IN PVOID VirtualAddress,
    IN ULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtMapUserPhysicalPagesScatter )(
    IN PVOID *VirtualAddresses,
    IN ULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtAllocateUserPhysicalPages )(
    IN HANDLE ProcessHandle,
    IN OUT PULONG_PTR NumberOfPages,
    OUT PULONG_PTR UserPfnArray
    );

typedef NTSTATUS( NTAPI *tNtFreeUserPhysicalPages )(
    IN HANDLE ProcessHandle,
    IN OUT PULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray
    );

// Sessions
#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef NTSTATUS( NTAPI *tNtOpenSession )(
    OUT PHANDLE SessionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
#endif

// Misc.
typedef NTSTATUS( NTAPI *tNtGetWriteWatch )(
    IN HANDLE ProcessHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T RegionSize,
    OUT PVOID *UserAddressArray,
    IN OUT PULONG_PTR EntriesInUserAddressArray,
    OUT PULONG Granularity
    );

typedef NTSTATUS( NTAPI *tNtResetWriteWatch )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN SIZE_T RegionSize
    );

typedef NTSTATUS( NTAPI *tNtCreatePagingFile )(
    IN PUNICODE_STRING PageFileName,
    IN PLARGE_INTEGER MinimumSize,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG Priority
    );

typedef NTSTATUS( NTAPI *tNtFlushInstructionCache )(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN SIZE_T Length
    );

typedef NTSTATUS( NTAPI *tNtFlushWriteBuffer )(
    VOID
    );

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus