#pragma once

#include "nativecommon.h"
#include "nativeteb.h"
#include "nativepeb.h"
#include "nativeio.h"
#include "nativemm.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < Process Macros and Definitions >
///

// Process access flags
#ifndef PROCESS_TERMINATE
#define PROCESS_TERMINATE 0x0001
#endif //!PROCESS_TERMINATE
#ifndef PROCESS_CREATE_THREAD
#define PROCESS_CREATE_THREAD 0x0002
#endif //!PROCESS_CREATE_THREAD
#ifndef PROCESS_SET_SESSIONID
#define PROCESS_SET_SESSIONID 0x0004
#endif //!PROCESS_SET_SESSIONID
#ifndef PROCESS_VM_OPERATION
#define PROCESS_VM_OPERATION 0x0008
#endif //!PROCESS_VM_OPERATION
#ifndef PROCESS_VM_READ
#define PROCESS_VM_READ 0x0010
#endif //!PROCESS_VM_READ
#ifndef PROCESS_VM_WRITE
#define PROCESS_VM_WRITE 0x0020
#endif //!PROCESS_VM_WRITE
#ifndef PROCESS_CREATE_PROCESS
#define PROCESS_CREATE_PROCESS 0x0080
#endif //!PROCESS_CREATE_PROCESS
#ifndef PROCESS_SET_QUOTA
#define PROCESS_SET_QUOTA 0x0100
#endif //!PROCESS_SET_QUOTA
#ifndef PROCESS_SET_INFORMATION
#define PROCESS_SET_INFORMATION 0x0200
#endif //!PROCESS_SET_INFORMATION
#ifndef PROCESS_QUERY_INFORMATION
#define PROCESS_QUERY_INFORMATION 0x0400
#endif //!PROCESS_QUERY_INFORMATION
#ifndef PROCESS_SET_PORT
#define PROCESS_SET_PORT 0x0800
#endif //!PROCESS_SET_PORT
#ifndef PROCESS_SUSPEND_RESUME
#define PROCESS_SUSPEND_RESUME 0x0800
#endif //!PROCESS_SUSPEND_RESUME
#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#endif //!PROCESS_QUERY_LIMITED_INFORMATION
#ifndef PROCESS_ALL_ACCESS
#if (NTDDI_VERSION >= NTDDI_VISTA)
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFFF)
#else
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFF)
#endif
#endif //!PROCESS_ALL_ACCESS

// Process creation flags
#define PROCESS_CREATE_FLAGS_BREAKAWAY 0x00000001
#define PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT 0x00000002
#define PROCESS_CREATE_FLAGS_INHERIT_HANDLES 0x00000004
#define PROCESS_CREATE_FLAGS_OVERRIDE_ADDRESS_SPACE 0x00000008
#define PROCESS_CREATE_FLAGS_LARGE_PAGES 0x00000010
#define PROCESS_CREATE_FLAGS_LARGE_PAGE_SYSTEM_DLL 0x00000020
#define PROCESS_CREATE_FLAGS_PROTECTED_PROCESS 0x00000040
#define PROCESS_CREATE_FLAGS_CREATE_SESSION 0x00000080 // ?
#define PROCESS_CREATE_FLAGS_INHERIT_FROM_PARENT 0x00000100

// Thread access flags
#ifndef THREAD_TERMINATE
#define THREAD_TERMINATE 0x0001
#endif //!THREAD_TERMINATE
#ifndef THREAD_SUSPEND_RESUME
#define THREAD_SUSPEND_RESUME 0x0002
#endif //!THREAD_SUSPEND_RESUME
#ifndef THREAD_ALERT
#define THREAD_ALERT 0x0004
#endif //!THREAD_ALERT
#ifndef THREAD_GET_CONTEXT
#define THREAD_GET_CONTEXT 0x0008
#endif //!THREAD_GET_CONTEXT
#ifndef THREAD_SET_CONTEXT
#define THREAD_SET_CONTEXT 0x0010
#endif //!THREAD_SET_CONTEXT
#ifndef THREAD_QUERY_INFORMATION
#define THREAD_QUERY_INFORMATION 0x0040
#endif //!THREAD_QUERY_INFORMATION
#ifndef THREAD_SET_INFORMATION
#define THREAD_SET_INFORMATION 0x0020
#endif //!THREAD_SET_INFORMATION
#ifndef THREAD_SET_THREAD_TOKEN
#define THREAD_SET_THREAD_TOKEN 0x0080
#endif //!THREAD_SET_THREAD_TOKEN
#ifndef THREAD_IMPERSONATE
#define THREAD_IMPERSONATE 0x0100
#endif //!THREAD_IMPERSONATE
#ifndef THREAD_DIRECT_IMPERSONATION
#define THREAD_DIRECT_IMPERSONATION 0x0200
#endif //!THREAD_DIRECT_IMPERSONATION
#ifndef THREAD_SET_LIMITED_INFORMATION
#define THREAD_SET_LIMITED_INFORMATION 0x0400
#endif //!THREAD_SET_LIMITED_INFORMATION
#ifndef THREAD_QUERY_LIMITED_INFORMATION
#define THREAD_QUERY_LIMITED_INFORMATION 0x0800
#endif //!THREAD_QUERY_LIMITED_INFORMATION
#ifndef THREAD_RESUME
#define THREAD_RESUME 0x1000
#endif //!THREAD_RESUME
#ifndef THREAD_ALL_ACCESS
#if (NTDDI_VERSION >= NTDDI_VISTA)
#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xFFFF)
#else
#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x3FF)
#endif
#endif //!THREAD_ALL_ACCESS

// thread creation flags
#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED 0x00000001
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH 0x00000002 // ?
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004
#define THREAD_CREATE_FLAGS_HAS_SECURITY_DESCRIPTOR 0x00000010 // ?
#define THREAD_CREATE_FLAGS_ACCESS_CHECK_IN_TARGET 0x00000020 // ?
#define THREAD_CREATE_FLAGS_INITIAL_THREAD 0x00000080

// Job flags
#ifndef JOB_OBJECT_ASSIGN_PROCESS
#define JOB_OBJECT_ASSIGN_PROCESS 0x0001
#endif //!JOB_OBJECT_ASSIGN_PROCESS
#ifndef JOB_OBJECT_SET_ATTRIBUTES
#define JOB_OBJECT_SET_ATTRIBUTES 0x0002
#endif //!JOB_OBJECT_SET_ATTRIBUTES
#ifndef JOB_OBJECT_QUERY
#define JOB_OBJECT_QUERY 0x0004
#endif //!JOB_OBJECT_QUERY
#ifndef JOB_OBJECT_TERMINATE
#define JOB_OBJECT_TERMINATE 0x0008
#endif //!JOB_OBJECT_TERMINATE
#ifndef JOB_OBJECT_SET_SECURITY_ATTRIBUTES
#define JOB_OBJECT_SET_SECURITY_ATTRIBUTES 0x0010
#endif //!JOB_OBJECT_SET_SECURITY_ATTRIBUTES
#ifndef JOB_OBJECT_ALL_ACCESS
#define JOB_OBJECT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1F)
#endif //!JOB_OBJECT_ALL_ACCESS

// TLS and FLS
#define FLS_MAXIMUM_AVAILABLE 128
#define TLS_MINIMUM_AVAILABLE 64
#define TLS_EXPANSION_SLOTS 1024

// Process priority
#define PROCESS_PRIORITY_CLASS_UNKNOWN 0
#define PROCESS_PRIORITY_CLASS_IDLE 1
#define PROCESS_PRIORITY_CLASS_NORMAL 2
#define PROCESS_PRIORITY_CLASS_HIGH 3
#define PROCESS_PRIORITY_CLASS_REALTIME 4
#define PROCESS_PRIORITY_CLASS_BELOW_NORMAL 5
#define PROCESS_PRIORITY_CLASS_ABOVE_NORMAL 6

// Process attributes
#define PS_ATTRIBUTE_NUMBER_MASK 0x0000FFFF
#define PS_ATTRIBUTE_THREAD 0x00010000 // can be used with threads
#define PS_ATTRIBUTE_INPUT 0x00020000 // input only
#define PS_ATTRIBUTE_UNKNOWN 0x00040000


///
/// < Process Enums >
///

// Working set operation types
typedef enum _PROCESS_WORKING_SET_OPERATION {
    ProcessWorkingSetSwap,
    ProcessWorkingSetEmpty,
    ProcessWorkingSetOperationMax
} PROCESS_WORKING_SET_OPERATION;

// Process protection types and signers
typedef enum _PS_PROTECTED_TYPE {
    PsProtectedTypeNone,
    PsProtectedTypeProtectedLight,
    PsProtectedTypeProtected,
    PsProtectedTypeMax
} PS_PROTECTED_TYPE;

typedef enum _PS_PROTECTED_SIGNER {
    PsProtectedSignerNone,
    PsProtectedSignerAuthenticode,
    PsProtectedSignerCodeGen,
    PsProtectedSignerAntimalware,
    PsProtectedSignerLsa,
    PsProtectedSignerWindows,
    PsProtectedSignerWinTcb,
    PsProtectedSignerMax
} PS_PROTECTED_SIGNER;

typedef enum _PS_ATTRIBUTE_NUM {
    PsAttributeParentProcess, // in HANDLE
    PsAttributeDebugPort, // in HANDLE
    PsAttributeToken, // in HANDLE
    PsAttributeClientId, // out PCLIENT_ID
    PsAttributeTebAddress, // out PTEB *
    PsAttributeImageName, // in PWSTR
    PsAttributeImageInfo, // out PSECTION_IMAGE_INFORMATION
    PsAttributeMemoryReserve, // in PPS_MEMORY_RESERVE
    PsAttributePriorityClass, // in UCHAR
    PsAttributeErrorMode, // in ULONG
    PsAttributeStdHandleInfo, // 10, in PPS_STD_HANDLE_INFO
    PsAttributeHandleList, // in PHANDLE
    PsAttributeGroupAffinity, // in PGROUP_AFFINITY
    PsAttributePreferredNode, // in PUSHORT
    PsAttributeIdealProcessor, // in PPROCESSOR_NUMBER
    PsAttributeUmsThread, // ? in PUMS_CREATE_THREAD_ATTRIBUTES
    PsAttributeMitigationOptions, // in UCHAR
    PsAttributeProtectionLevel,
    PsAttributeSecureProcess, // since THRESHOLD
    PsAttributeJobList,
    PsAttributeMax
} PS_ATTRIBUTE_NUM;

typedef enum _PS_CREATE_STATE {
    PsCreateInitialState,
    PsCreateFailOnFileOpen,
    PsCreateFailOnSectionCreate,
    PsCreateFailExeFormat,
    PsCreateFailMachineMismatch,
    PsCreateFailExeName, // Debugger specified
    PsCreateSuccess,
    PsCreateMaximumStates
} PS_CREATE_STATE;

typedef enum _KTHREAD_STATE {
    Initialized,
    Ready,
    Running,
    Standby,
    Terminated,
    Waiting,
    Transition,
    DeferredReady,
    GateWaitObsolete,
    WaitingForProcessInSwap,
    MaximumThreadState
} KTHREAD_STATE, *PKTHREAD_STATE;



///
/// < Process Structures >
///

typedef struct _PROCESS_DISK_COUNTERS {
    ULONGLONG BytesRead;
    ULONGLONG BytesWritten;
    ULONGLONG ReadOperationCount;
    ULONGLONG WriteOperationCount;
    ULONGLONG FlushOperationCount;
} PROCESS_DISK_COUNTERS, *PPROCESS_DISK_COUNTERS;

typedef struct _PROCESS_ENERGY_VALUES {
    ULONGLONG Cycles[2][4];
    ULONGLONG DiskEnergy;
    ULONGLONG NetworkTailEnergy;
    ULONGLONG MBBTailEnergy;
    ULONGLONG NetworkTxRxBytes;
    ULONGLONG MBBTxRxBytes;
    union {
        struct {
            ULONG Foreground : 1;
        };
        ULONG WindowInformation;
    };
    ULONG PixelArea;
    LONGLONG PixelReportTimestamp;
    ULONGLONG PixelTime;
    LONGLONG ForegroundReportTimestamp;
    ULONGLONG ForegroundTime;
} PROCESS_ENERGY_VALUES, *PPROCESS_ENERGY_VALUES;

typedef struct _VM_COUNTERS {
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} VM_COUNTERS, *PVM_COUNTERS;

typedef struct _VM_COUNTERS64 {
    ULONGLONG PeakVirtualSize;
    ULONGLONG VirtualSize;
    ULONG PageFaultCount;
    ULONGLONG PeakWorkingSetSize;
    ULONGLONG WorkingSetSize;
    ULONGLONG QuotaPeakPagedPoolUsage;
    ULONGLONG QuotaPagedPoolUsage;
    ULONGLONG QuotaPeakNonPagedPoolUsage;
    ULONGLONG QuotaNonPagedPoolUsage;
    ULONGLONG PagefileUsage;
    ULONGLONG PeakPagefileUsage;
} VM_COUNTERS64, *PVM_COUNTERS64;

typedef struct _VM_COUNTERS_EX {
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivateUsage;
} VM_COUNTERS_EX, *PVM_COUNTERS_EX;

typedef struct _VM_COUNTERS_EX64 {
    ULONGLONG PeakVirtualSize;
    ULONGLONG VirtualSize;
    ULONG PageFaultCount;
    ULONGLONG PeakWorkingSetSize;
    ULONGLONG WorkingSetSize;
    ULONGLONG QuotaPeakPagedPoolUsage;
    ULONGLONG QuotaPagedPoolUsage;
    ULONGLONG QuotaPeakNonPagedPoolUsage;
    ULONGLONG QuotaNonPagedPoolUsage;
    ULONGLONG PagefileUsage;
    ULONGLONG PeakPagefileUsage;
    ULONGLONG PrivateUsage;
} VM_COUNTERS_EX64, *PVM_COUNTERS_EX64;

typedef struct _VM_COUNTERS_EX2 {
    VM_COUNTERS_EX CountersEx;
    SIZE_T PrivateWorkingSetSize;
    SIZE_T SharedCommitUsage;
} VM_COUNTERS_EX2, *PVM_COUNTERS_EX2;

typedef struct _VM_COUNTERS_EX2_64 {
    VM_COUNTERS_EX64 CountersEx;
    ULONGLONG PrivateWorkingSetSize;
    ULONGLONG SharedCommitUsage;
} VM_COUNTERS_EX2_64, *PVM_COUNTERS_EX2_64;

typedef struct _PS_ATTRIBUTE {
    ULONG Attribute;
    SIZE_T Size;
    union {
        ULONG Value;
        PVOID ValuePtr;
    };
    PSIZE_T ReturnLength;
} PS_ATTRIBUTE, *PPS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE64 {
    ULONG Attribute;                // 0x00
    ULONGLONG Size;                 // 0x08 SIZE_T
    union {
        ULONG Value;
        ULONGLONG ValuePtr;         // 0x10 PVOID
    };
    ULONGLONG ReturnLength;         // 0x18 PSIZE_T
} PS_ATTRIBUTE64, *PPS_ATTRIBUTE64;

typedef struct _PS_ATTRIBUTE_LIST {
    SIZE_T TotalLength;
    PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

typedef struct _PS_ATTRIBUTE_LIST64 {
    ULONGLONG TotalLength;          // SIZE_T
    PS_ATTRIBUTE64 Attributes[1];
} PS_ATTRIBUTE_LIST64, *PPS_ATTRIBUTE_LIST64;

#define PsAttributeValue(Number, Thread, Input, Unknown) \
    (((Number) & PS_ATTRIBUTE_NUMBER_MASK) | \
    ((Thread) ? PS_ATTRIBUTE_THREAD : 0) | \
    ((Input) ? PS_ATTRIBUTE_INPUT : 0) | \
    ((Unknown) ? PS_ATTRIBUTE_UNKNOWN : 0))

#define PS_ATTRIBUTE_PARENT_PROCESS \
    PsAttributeValue(PsAttributeParentProcess, FALSE, TRUE, TRUE)
#define PS_ATTRIBUTE_DEBUG_PORT \
    PsAttributeValue(PsAttributeDebugPort, FALSE, TRUE, TRUE)
#define PS_ATTRIBUTE_TOKEN \
    PsAttributeValue(PsAttributeToken, FALSE, TRUE, TRUE)
#define PS_ATTRIBUTE_CLIENT_ID \
    PsAttributeValue(PsAttributeClientId, TRUE, FALSE, FALSE)
#define PS_ATTRIBUTE_TEB_ADDRESS \
    PsAttributeValue(PsAttributeTebAddress, TRUE, FALSE, FALSE)
#define PS_ATTRIBUTE_IMAGE_NAME \
    PsAttributeValue(PsAttributeImageName, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_IMAGE_INFO \
    PsAttributeValue(PsAttributeImageInfo, FALSE, FALSE, FALSE)
#define PS_ATTRIBUTE_MEMORY_RESERVE \
    PsAttributeValue(PsAttributeMemoryReserve, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_PRIORITY_CLASS \
    PsAttributeValue(PsAttributePriorityClass, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_ERROR_MODE \
    PsAttributeValue(PsAttributeErrorMode, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_STD_HANDLE_INFO \
    PsAttributeValue(PsAttributeStdHandleInfo, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_HANDLE_LIST \
    PsAttributeValue(PsAttributeHandleList, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_GROUP_AFFINITY \
    PsAttributeValue(PsAttributeGroupAffinity, TRUE, TRUE, FALSE)
#define PS_ATTRIBUTE_PREFERRED_NODE \
    PsAttributeValue(PsAttributePreferredNode, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_IDEAL_PROCESSOR \
    PsAttributeValue(PsAttributeIdealProcessor, TRUE, TRUE, FALSE)
#define PS_ATTRIBUTE_MITIGATION_OPTIONS \
    PsAttributeValue(PsAttributeMitigationOptions, FALSE, TRUE, TRUE)

typedef struct _PS_CREATE_INFO {
    SIZE_T Size;
    PS_CREATE_STATE State;
    union {
        // PsCreateInitialState
        struct {
            union {
                ULONG InitFlags;
                struct {
                    UCHAR WriteOutputOnExit : 1;
                    UCHAR DetectManifest : 1;
                    UCHAR IFEOSkipDebugger : 1;
                    UCHAR IFEODoNotPropagateKeyState : 1;
                    UCHAR SpareBits1 : 4;
                    UCHAR SpareBits2 : 8;
                    USHORT ProhibitedImageCharacteristics : 16;
                };
            };
            ACCESS_MASK AdditionalFileAccess;
        } InitState;

        // PsCreateFailOnSectionCreate
        struct {
            HANDLE FileHandle;
        } FailSection;

        // PsCreateFailExeFormat
        struct {
            USHORT DllCharacteristics;
        } ExeFormat;

        // PsCreateFailExeName
        struct {
            HANDLE IFEOKey;
        } ExeName;

        // PsCreateSuccess
        struct {
            union {
                ULONG OutputFlags;
                struct {
                    UCHAR ProtectedProcess : 1;
                    UCHAR AddressSpaceOverride : 1;
                    UCHAR DevOverrideEnabled : 1; // from Image File Execution Options
                    UCHAR ManifestDetected : 1;
                    UCHAR ProtectedProcessLight : 1;
                    UCHAR SpareBits1 : 3;
                    UCHAR SpareBits2 : 8;
                    USHORT SpareBits3 : 16;
                };
            };
            HANDLE FileHandle;
            HANDLE SectionHandle;
            ULONGLONG UserProcessParametersNative;
            ULONG UserProcessParametersWow64;
            ULONG CurrentParameterFlags;
            ULONGLONG PebAddressNative;
            ULONG PebAddressWow64;
            ULONGLONG ManifestAddress;
            ULONG ManifestSize;
        } SuccessState;
    };
} PS_CREATE_INFO, *PPS_CREATE_INFO;

typedef struct _PS_CREATE_INFO64 {
    ULONGLONG Size;                         // SIZE_T
    PS_CREATE_STATE State;
    union {
        // PsCreateInitialState
        struct {
            union {
                ULONG InitFlags;
                struct {
                    UCHAR WriteOutputOnExit : 1;
                    UCHAR DetectManifest : 1;
                    UCHAR IFEOSkipDebugger : 1;
                    UCHAR IFEODoNotPropagateKeyState : 1;
                    UCHAR SpareBits1 : 4;
                    UCHAR SpareBits2 : 8;
                    USHORT ProhibitedImageCharacteristics : 16;
                };
            };
            ACCESS_MASK AdditionalFileAccess;
        } InitState;

        // PsCreateFailOnSectionCreate
        struct {
            ULONGLONG FileHandle;           // HANDLE
        } FailSection;

        // PsCreateFailExeFormat
        struct {
            USHORT DllCharacteristics;
        } ExeFormat;

        // PsCreateFailExeName
        struct {
            ULONGLONG IFEOKey;              // HANDLE
        } ExeName;

        // PsCreateSuccess
        struct {
            union {
                ULONG OutputFlags;
                struct {
                    UCHAR ProtectedProcess : 1;
                    UCHAR AddressSpaceOverride : 1;
                    UCHAR DevOverrideEnabled : 1; // from Image File Execution Options
                    UCHAR ManifestDetected : 1;
                    UCHAR ProtectedProcessLight : 1;
                    UCHAR SpareBits1 : 3;
                    UCHAR SpareBits2 : 8;
                    USHORT SpareBits3 : 16;
                };
            };
            ULONGLONG FileHandle;                   // HANDLE
            ULONGLONG SectionHandle;                // HANDLE
            ULONGLONG UserProcessParametersNative;
            ULONG UserProcessParametersWow64;
            ULONG CurrentParameterFlags;
            ULONGLONG PebAddressNative;
            ULONG PebAddressWow64;
            ULONGLONG ManifestAddress;
            ULONG ManifestSize;
        } SuccessState;
    };
} PS_CREATE_INFO64, *PPS_CREATE_INFO64;

typedef union _PS_PROTECTION {
    UCHAR Level;
    struct {
        UCHAR Type : 3;
        UCHAR Audit : 1;
        UCHAR Signer : 4;
    };
} PS_PROTECTION, *PPS_PROTECTION;

typedef struct _ALPC_PROCESS_CONTEXT {
    EX_PUSH_LOCK Lock;                  // 0x00
    LIST_ENTRY ViewListHead;            // 0x08
    ULONGLONG PagedPoolQuotaCache;      // 0x18
} ALPC_PROCESS_CONTEXT, *PALPC_PROCESS_CONTEXT;

typedef struct _ALPC_PROCESS_CONTEXT64 {
    EX_PUSH_LOCK64 Lock;                // 0x00
    LIST_ENTRY64 ViewListHead;          // 0x08
    ULONGLONG PagedPoolQuotaCache;      // 0x18
} ALPC_PROCESS_CONTEXT64, *PALPC_PROCESS_CONTEXT46;

typedef struct _MMSUPPORT_FLAGS {
    union {
        struct {
            struct {
                UCHAR WorkingSetType : 3;           // 0x00
                UCHAR Reserved0 : 3;                // 0x00
                UCHAR MaximumWorkingSetHard : 1;    // 0x00
                UCHAR MinimumWorkingSetHard : 1;    // 0x00
            };
            struct {
                UCHAR SessionMaster : 1;            // 0x01
                UCHAR TrimmerState : 2;             // 0x01
                UCHAR Reserved : 1;                 // 0x01
                UCHAR PageStealers : 4;             // 0x01
            };
        };
    } u1;
    union {
        struct {
            UCHAR MemoryPriority;                   // 0x02
            struct {
                UCHAR WsleDeleted : 1;              // 0x03
                UCHAR VmExiting : 1;                // 0x03
                UCHAR ExpansionFailed : 1;          // 0x03
                UCHAR SvmEnabled : 1;               // 0x03
                UCHAR ForceAge : 1;                 // 0x03
                UCHAR NewMaximum : 1;               // 0x03
                UCHAR CommitReleaseState : 2;       // 0x03
            };
        };
    } u2;
} MMSUPPORT_FLAGS, *PMMSUPPORT_FLAGS;

typedef struct _MMWSL_INSTANCE {
    PMMPTE NextPteToTrim;                       // 0x00
    PMMPTE NextPteToAge;                        // 0x08
    PMMPTE NextPteToAccessClear;                // 0x10
    ULONG LastAccessClearingRemainder;          // 0x18
    ULONG LastAgingRemainder;                   // 0x1C
    ULONGLONG LockedEntries;                    // 0x20
} MMWSL_INSTANCE, *PMMWSL_INSTANCE;

typedef struct _MMWSL_INSTANCE64 {
    ULONGLONG NextPteToTrim;                    // 0x00 PMMPTE
    ULONGLONG NextPteToAge;                     // 0x08 PMMPTE
    ULONGLONG NextPteToAccessClear;             // 0x10 PMMPTE
    ULONG LastAccessClearingRemainder;          // 0x18
    ULONG LastAgingRemainder;                   // 0x1C
    ULONGLONG LockedEntries;                    // 0x20
} MMWSL_INSTANCE64, *PMMWSL_INSTANCE64;

typedef struct _MMSUPPORT_INSTANCE {
    USHORT NextPageColor;                        // 0x00
    USHORT LastTrimStamp;                        // 0x02
    ULONG PageFaultCount;                        // 0x04
    ULONGLONG TrimmedPageCount;                  // 0x08
    PMMWSL_INSTANCE VmWorkingSetList;            // 0x10
    LIST_ENTRY WorkingSetExpansionLinks;         // 0x18
    ULONGLONG AgeDistribution[8];                // 0x28
    PKGATE ExitOutswapGate;                      // 0x68
    ULONGLONG MinimumWorkingSetSize;             // 0x70
    ULONGLONG WorkingSetLeafSize;                // 0x78
    ULONGLONG WorkingSetLeafPrivateSize;         // 0x80
    ULONGLONG WorkingSetSize;                    // 0x88
    ULONGLONG WorkingSetPrivateSize;             // 0x90
    ULONGLONG MaximumWorkingSetSize;             // 0x98
    ULONGLONG PeakWorkingSetSize;                // 0xA0
    ULONG HardFaultCount;                        // 0xA8
    USHORT PartitionId;                          // 0xAC
    USHORT Pad0;                                 // 0xAE
    union {
        PVOID InstancedWorkingSet;               // 0xB0
    } u1;
    ULONGLONG Reserved0;                         // 0xB8
    MMSUPPORT_FLAGS Flags;                       // 0xC0
} MMSUPPORT_INSTANCE, *PMMSUPPORT_INSTANCE;

typedef struct _MMSUPPORT_INSTANCE64 {
    USHORT NextPageColor;                       // 0x00
    USHORT LastTrimStamp;                       // 0x02
    ULONG PageFaultCount;                       // 0x04
    ULONGLONG TrimmedPageCount;                 // 0x08
    ULONGLONG VmWorkingSetList;                 // 0x10 PMMWSL_INSTANCE64
    LIST_ENTRY64 WorkingSetExpansionLinks;      // 0x18
    ULONGLONG AgeDistribution[8];               // 0x28
    ULONGLONG ExitOutswapGate;                  // 0x68 PKGATE
    ULONGLONG MinimumWorkingSetSize;            // 0x70
    ULONGLONG WorkingSetLeafSize;               // 0x78
    ULONGLONG WorkingSetLeafPrivateSize;        // 0x80
    ULONGLONG WorkingSetSize;                   // 0x88
    ULONGLONG WorkingSetPrivateSize;            // 0x90
    ULONGLONG MaximumWorkingSetSize;            // 0x98
    ULONGLONG PeakWorkingSetSize;               // 0xA0
    ULONG HardFaultCount;                       // 0xA8
    USHORT PartitionId;                         // 0xAC
    USHORT Pad0;                                // 0xAE
    union {
        ULONGLONG InstancedWorkingSet;          // 0xB0 PVOID
    } u1;
    ULONGLONG Reserved0;                        // 0xB8
    MMSUPPORT_FLAGS Flags;                      // 0xC0
} MMSUPPORT_INSTANCE64, *PMMSUPPORT_INSTANCE64;

typedef struct _MMSUPPORT_SHARED {
    volatile LONG WorkingSetLock;               // 0x00
    LONG GoodCitizenWaiting;                    // 0x04
    ULONGLONG ReleasedCommitDebt;               // 0x08
    ULONGLONG ResetPagesRepurposedCount;        // 0x10
    PVOID WsSwapSupport;                        // 0x18
    PVOID CommitReleaseContext;                 // 0x20
    PVOID AccessLog;                            // 0x28
    volatile ULONGLONG ChargedWslePages;        // 0x30
    ULONGLONG ActualWslePages;                  // 0x38
    PVOID ShadowMapping;                        // 0x40
} MMSUPPORT_SHARED, *PMMSUPPORT_SHARED;

typedef struct _MMSUPPORT_SHARED64 {
    volatile LONG WorkingSetLock;               // 0x00
    LONG GoodCitizenWaiting;                    // 0x04
    ULONGLONG ReleasedCommitDebt;               // 0x08
    ULONGLONG ResetPagesRepurposedCount;        // 0x10
    ULONGLONG WsSwapSupport;                    // 0x18 PVOID
    ULONGLONG CommitReleaseContext;             // 0x20 PVOID
    ULONGLONG AccessLog;                        // 0x28 PVOID
    volatile ULONGLONG ChargedWslePages;        // 0x30
    ULONGLONG ActualWslePages;                  // 0x38
    ULONGLONG ShadowMapping;                    // 0x40 PVOID
} MMSUPPORT_SHARED64, *PMMSUPPORT_SHARED64;

typedef struct _MMSUPPORT_FULL {
    MMSUPPORT_INSTANCE Instance;                // 0x00
    MMSUPPORT_SHARED Shared;                    // 0xC8
} MMSUPPORT_FULL, *PMMSUPPORT_FULL;

typedef struct _MMSUPPORT_FULL64 {
    MMSUPPORT_INSTANCE Instance;                // 0x00
    MMSUPPORT_SHARED64 Shared;                  // 0xC8
} MMSUPPORT_FULL64, *PMMSUPPORT_FULL64;

typedef struct _MMSUPPORT {
    LONG WorkingSetLock;                        // 0x00
    PKGATE ExitOutswapGate;                     // 0x08
    PVOID AccessLog;                            // 0x10
    LIST_ENTRY WorkingSetExpansionLinks;        // 0x18
    ULONGLONG AgeDistribution[7];               // 0x28
    ULONG64 MinimumWorkingSetSize;              // 0x60
    ULONG64 WorkingSetLeafSize;                 // 0x68
    ULONG64 WorkingSetLeafPrivateSize;          // 0x70
    ULONG64 WorkingSetSize;                     // 0x78
    ULONG64 WorkingSetPrivateSize;              // 0x80
    ULONG64 MaximumWorkingSetSize;              // 0x88
    ULONG64 ChargedWslePages;                   // 0x90
    ULONG64 ActualWslePages;                    // 0x98
    ULONG64 WorkingSetSizeOverhead;             // 0xA0
    ULONG64 PeakWorkingSetSize;                 // 0xA8
    ULONG HardFaultCount;                       // 0xB0
    USHORT PartitionId;                         // 0xB4
    USHORT Pad0;                                // 0xB6
    PVOID VmWorkingSetList;                     // 0xB8
    USHORT NextPageColor;                       // 0xC0
    USHORT LastTrimStamp;                       // 0xC2
    ULONG PageFaultCount;                       // 0xC4
    ULONG64 TrimmedPageCount;                   // 0xC8
    ULONG64 Reserved0;                          // 0xD0
    MMSUPPORT_FLAGS Flags;                      // 0xD8
    ULONG64 ReleasedCommitDebt;                 // 0xE0
    PVOID WsSwapSupport;                        // 0xE8
    PVOID CommitReAcquireFailSupport;           // 0xF0
} MMSUPPORT, *PMMSUPPORT;

typedef struct _MMSUPPORT64 {
    LONG WorkingSetLock;                        // 0x00
    ULONGLONG ExitOutswapGate;                  // 0x08 PKGATE
    ULONGLONG AccessLog;                        // 0x10 PVOID
    LIST_ENTRY64 WorkingSetExpansionLinks;      // 0x18
    ULONGLONG AgeDistribution[7];               // 0x28
    ULONG64 MinimumWorkingSetSize;              // 0x60
    ULONG64 WorkingSetLeafSize;                 // 0x68
    ULONG64 WorkingSetLeafPrivateSize;          // 0x70
    ULONG64 WorkingSetSize;                     // 0x78
    ULONG64 WorkingSetPrivateSize;              // 0x80
    ULONG64 MaximumWorkingSetSize;              // 0x88
    ULONG64 ChargedWslePages;                   // 0x90
    ULONG64 ActualWslePages;                    // 0x98
    ULONG64 WorkingSetSizeOverhead;             // 0xA0
    ULONG64 PeakWorkingSetSize;                 // 0xA8
    ULONG HardFaultCount;                       // 0xB0
    USHORT PartitionId;                         // 0xB4
    USHORT Pad0;                                // 0xB6
    ULONGLONG VmWorkingSetList;                 // 0xB8 PVOID
    USHORT NextPageColor;                       // 0xC0
    USHORT LastTrimStamp;                       // 0xC2
    ULONG PageFaultCount;                       // 0xC4
    ULONG64 TrimmedPageCount;                   // 0xC8
    ULONG64 Reserved0;                          // 0xD0
    MMSUPPORT_FLAGS Flags;                      // 0xD8
    ULONG64 ReleasedCommitDebt;                 // 0xE0
    ULONGLONG WsSwapSupport;                    // 0xE8 PVOID
    ULONGLONG CommitReAcquireFailSupport;       // 0xF0 PVOID
} MMSUPPORT64, *PMMSUPPORT64;


//
// Audit Information structure: this is a member of the EPROCESS structure
// and currently contains only the name of the exec'ed image file.
//
typedef struct _SE_AUDIT_PROCESS_CREATION_INFO {
    struct _OBJECT_NAME_INFORMATION *ImageFileName;
} SE_AUDIT_PROCESS_CREATION_INFO, *PSE_AUDIT_PROCESS_CREATION_INFO;

typedef struct _SE_AUDIT_PROCESS_CREATION_INFO64 {
    ULONGLONG ImageFileName; // POBJECT_NAME_INFORMATION
} SE_AUDIT_PROCESS_CREATION_INFO64, *PSE_AUDIT_PROCESS_CREATION_INFO64;

//
// Process execute options.
//
// Does not seem to have a purpose outside the KPROCESS. 
//
typedef union _KEXECUTE_OPTIONS {       // 6.1 and higher
    struct {
        UCHAR ExecuteDisable : 1;
        UCHAR ExecuteEnable : 1;
        UCHAR DisableThunkEmulation : 1;
        UCHAR Permanent : 1;
        UCHAR ExecuteDispatchEnable : 1;
        UCHAR ImageDispatchEnable : 1;
        UCHAR DisableExceptionChainValidation : 1;
        UCHAR Spare : 1;
    };
    UCHAR volatile ExecuteOptions;
    UCHAR ExceptionOptionsNV;           // 6.2 and higher
} KEXECUTE_OPTIONS, *PKEXECUTE_OPTIONS;

//
// Process stack count.
//
// Seems to have no purpose outside the KPROCESS.
//
typedef union _KSTACK_COUNT {           // 6.2 and higher
    LONG Value;
    struct {
        ULONG State : 3;
        ULONG StackCount : 29;
    };
} KSTACK_COUNT, *PKSTACK_COUNT;

//
// Kernel process station control block
//
// Seems to have no purpose outside the KPROCESS.
//
typedef struct _KSCB {
    ULONGLONG GenerationCycles;     // 0x00
    ULONGLONG MinQuotaCycleTarget;  // 0x08
    ULONGLONG MaxQuotaCycleTarget;  // 0x10
    ULONGLONG RankCycleTarget;      // 0x18
    ULONGLONG LongTermCycles;       // 0x20
    ULONGLONG LastReportedCycles;   // 0x28
    ULONGLONG OverQuotaHistory;     // 0x30
    ULONGLONG ReadyTime;            // 0x38
    ULONGLONG InsertTime;           // 0x40
    LIST_ENTRY PerProcessorList;    // 0x48
    RTL_BALANCED_NODE QueueNode;    // 0x58
    struct {
        UCHAR Inserted : 1;         // 0x70
        UCHAR MaxOverQuota : 1;     // 0x70
        UCHAR MinOverQuota : 1;     // 0x70
        UCHAR RankBias : 1;         // 0x70
        UCHAR SoftCap : 1;          // 0x70
        UCHAR ShareRankOwner : 1;   // 0x70
        UCHAR Spare1 : 2;           // 0x70
    };
    UCHAR Depth;                    // 0x71
    USHORT ReadySummary;            // 0x72
    ULONG Rank;                     // 0x74
    PULONG ShareRank;               // 0x78
    ULONG OwnerShareRank;           // 0x80
    LIST_ENTRY ReadyListHead[16];   // 0x88
    RTL_RB_TREE ChildScbQueue;      // 0x188
    struct _KSCB *Parent;           // 0x198
    struct _KSCB *Root;             // 0x1A0
} KSCB, *PKSCB;

//
// Kernel scheduling group definitions.
//
typedef struct _KSCHEDULING_GROUP_POLICY {
    union {
        ULONG Value;                // 0x00
        struct {
            union {
                USHORT Weight;      // 0x00
                USHORT MinRate;     // 0x00
            };
            USHORT MaxRate;         // 0x02
        };
    };
    union {
        struct {
            ULONG Type : 1;
            ULONG Disabled : 1;
            ULONG RankBias : 1;
            ULONG Spare1 : 29;
        };
        ULONG AllFlags;             // 0x04
    };
} KSCHEDULING_GROUP_POLICY, *PKSCHEDULING_GROUP_POLICY;

typedef struct _KSCHEDULING_GROUP {
    KSCHEDULING_GROUP_POLICY Policy;
    ULONG RelativeWeight;
    ULONG ChildMinRate;
    ULONG ChildMinWeight;
    ULONG ChildTotalWeight;
    ULONGLONG QueryHistoryTimeStamp;
    LONGLONG NotificationCycles;
    LONGLONG MaxQuotaLimitCycles;
    LONGLONG MaxQuotaCyclesRemaining;
    union {
        LIST_ENTRY SchedulingGroupList;
        LIST_ENTRY Sibling;
    };
    PKDPC NotificationDpc;
    LIST_ENTRY ChildList;
    struct _KSCHEDULING_GROUP *Parent;
    KSCB PerProcessor[1];
} KSCHEDULING_GROUP, *PKSCHEDULING_GROUP;

//
// The KPROCESS structure is the Kernel Core's portion of the EPROCESS structure.
//
typedef struct _KPROCESS {
    DISPATCHER_HEADER Header;               // 0x00
    LIST_ENTRY ProfileListHead;             // 0x18
    PHYSICAL_ADDRESS DirectoryTableBase;    // 0x28
    LIST_ENTRY ThreadListHead;              // 0x30
    ULONG ProcessLock;                      // 0x40
    ULONG Spare0;                           // 0x44
    ULONG64 DeepFreezeStartTime;            // 0x48
    KAFFINITY_EX Affinity;                  // 0x50
    LIST_ENTRY ReadyListHead;               // 0xF8
    SINGLE_LIST_ENTRY SwapListEntry;        // 0x108
    KAFFINITY_EX ActiveProcessors;          // 0x110
    union {
        struct {
            LONG AutoAlignment : 1;         // 0x1B8
            LONG DisableBoost : 1;          // 0x1B8
            LONG DisableQuantum : 1;        // 0x1B8
            LONG DeepFreeze : 1;            // 0x1B8
            LONG TimerVirtualization : 1;   // 0x1B8
            LONG CheckStackExtents : 1;     // 0x1B8
            LONG PpmPolicy : 3;             // 0x1B8
            LONG ActiveGroupsMask : 20;     // 0x1B8
            LONG ReservedFlags : 3;         // 0x1B8
        };
        LONG ProcessFlags;                  // 0x1B8
    };
    CHAR BasePriority;                      // 0x1BC
    CHAR QuantumReset;                      // 0x1BD
    UCHAR Visited;                          // 0x1BE
    KEXECUTE_OPTIONS Flags;                 // 0x1BF
    ULONG ThreadSeed[20];                   // 0x1C0
    USHORT IdealNode[20];                   // 0x210
    USHORT IdealGlobalNode;                 // 0x238
    USHORT Spare1;                          // 0x23A
    KSTACK_COUNT StackCount;                // 0x23C
    LIST_ENTRY ProcessListEntry;            // 0x240
    ULONG64 CycleTime;                      // 0x250
    ULONG64 ContextSwitches;                // 0x258
    PKSCHEDULING_GROUP SchedulingGroup;     // 0x260
    ULONG FreezeCount;                      // 0x268
    ULONG KernelTime;                       // 0x26C
    ULONG UserTime;                         // 0x270
    ULONG ReadyTime;                        // 0x274
    ULONGLONG UserDirectoryTableBase;       // 0x278
    UCHAR AddressPolicy;                    // 0x280
    UCHAR Spare2[71];                       // 0x281
    PVOID InstrumentationCallback;          // 0x2C8
    ULONG64 SecurePid;                      // 0x2D0
} KPROCESS, *PKPROCESS;

typedef struct _KPROCESS64 {
    DISPATCHER_HEADER64 Header;             // 0x00
    LIST_ENTRY64 ProfileListHead;           // 0x18
    PHYSICAL_ADDRESS DirectoryTableBase;    // 0x28
    LIST_ENTRY64 ThreadListHead;            // 0x30
    ULONG ProcessLock;                      // 0x40
    ULONG Spare0;                           // 0x44
    ULONG64 DeepFreezeStartTime;            // 0x48
    KAFFINITY_EX Affinity;                  // 0x50
    LIST_ENTRY64 ReadyListHead;             // 0xF8
    SINGLE_LIST_ENTRY64 SwapListEntry;      // 0x108
    KAFFINITY_EX ActiveProcessors;          // 0x110
    union {
        struct {
            LONG AutoAlignment : 1;         // 0x1B8
            LONG DisableBoost : 1;          // 0x1B8
            LONG DisableQuantum : 1;        // 0x1B8
            LONG DeepFreeze : 1;            // 0x1B8
            LONG TimerVirtualization : 1;   // 0x1B8
            LONG CheckStackExtents : 1;     // 0x1B8
            LONG PpmPolicy : 3;             // 0x1B8
            LONG ActiveGroupsMask : 20;     // 0x1B8
            LONG ReservedFlags : 3;         // 0x1B8
        };
        LONG ProcessFlags;                  // 0x1B8
    };
    CHAR BasePriority;                      // 0x1BC
    CHAR QuantumReset;                      // 0x1BD
    UCHAR Visited;                          // 0x1BE
    KEXECUTE_OPTIONS Flags;                 // 0x1BF
    ULONG ThreadSeed[20];                   // 0x1C0
    USHORT IdealNode[20];                   // 0x210
    USHORT IdealGlobalNode;                 // 0x238
    USHORT Spare1;                          // 0x23A
    KSTACK_COUNT StackCount;                // 0x23C
    LIST_ENTRY64 ProcessListEntry;          // 0x240
    ULONG64 CycleTime;                      // 0x250
    ULONG64 ContextSwitches;                // 0x258
    PKSCHEDULING_GROUP SchedulingGroup;     // 0x260
    ULONG FreezeCount;                      // 0x268
    ULONG KernelTime;                       // 0x26C
    ULONG UserTime;                         // 0x270
    ULONG ReadyTime;                        // 0x274
    ULONGLONG UserDirectoryTableBase;       // 0x278
    UCHAR AddressPolicy;                    // 0x280
    UCHAR Spare2[71];                       // 0x281
    ULONGLONG InstrumentationCallback;      // 0x2C8 PVOID
    ULONG64 SecurePid;                      // 0x2D0
} KPROCESS64, *PKPROCESS64;

//
// EPROCESS sturcture flags.
//
typedef union EPROCESS_FLAGS {
    ULONG Flags;
    struct {
        ULONG CreateReported : 1;                   // 0x304
        ULONG NoDebugInherit : 1;                   // 0x304
        ULONG ProcessExiting : 1;                   // 0x304
        ULONG ProcessDelete : 1;                    // 0x304
        ULONG ControlFlowGuardEnabled : 1;          // 0x304
        ULONG VmDeleted : 1;                        // 0x304
        ULONG OutswapEnabled : 1;                   // 0x304
        ULONG Outswapped : 1;                       // 0x304
        ULONG FailFastOnCommitFail : 1;             // 0x304
        ULONG Wow64VaSpace4Gb : 1;                  // 0x304
        ULONG AddressSpaceInitialized : 2;          // 0x304
        ULONG SetTimerResolution : 1;               // 0x304
        ULONG BreakOnTermination : 1;               // 0x304
        ULONG DeprioritizeViews : 1;                // 0x304
        ULONG WriteWatch : 1;                       // 0x304
        ULONG ProcessInSession : 1;                 // 0x304
        ULONG OverrideAddressSpace : 1;             // 0x304
        ULONG HasAddressSpace : 1;                  // 0x304
        ULONG LaunchPrefetched : 1;                 // 0x304
        ULONG Background : 1;                       // 0x304
        ULONG VmTopDown : 1;                        // 0x304
        ULONG ImageNotifyDone : 1;                  // 0x304
        ULONG PdeUpdateNeeded : 1;                  // 0x304
        ULONG VdmAllowed : 1;                       // 0x304
        ULONG ProcessRundown : 1;                   // 0x304
        ULONG ProcessInserted : 1;                  // 0x304
        ULONG DefaultIoPriority : 3;                // 0x304
        ULONG ProcessSelfDelete : 1;                // 0x304
        ULONG SetTimerResolutionLink : 1;           // 0x304
    };
} EPROCESS_FLAGS;

typedef union _EPROCESS_FLAGS2 {
    ULONG Flags;
    struct {
        ULONG JobNotReallyActive : 1;               // 0x300
        ULONG AccountingFolded : 1;                 // 0x300
        ULONG NewProcessReported : 1;               // 0x300
        ULONG ExitProcessReported : 1;              // 0x300
        ULONG ReportCommitChanges : 1;              // 0x300
        ULONG LastReportMemory : 1;                 // 0x300
        ULONG ForceWakeCharge : 1;                  // 0x300
        ULONG CrossSessionCreate : 1;               // 0x300
        ULONG NeedsHandleRundown : 1;               // 0x300
        ULONG RefTraceEnabled : 1;                  // 0x300
        ULONG DisableDynamicCode : 1;               // 0x300
        ULONG EmptyJobEvaluated : 1;                // 0x300
        ULONG DefaultPagePriority : 3;              // 0x300
        ULONG PrimaryTokenFrozen : 1;               // 0x300
        ULONG ProcessVerifierTarget : 1;            // 0x300
        ULONG StackRandomizationDisabled : 1;       // 0x300
        ULONG AffinityPermanent : 1;                // 0x300
        ULONG AffinityUpdateEnable : 1;             // 0x300
        ULONG PropagateNode : 1;                    // 0x300
        ULONG ExplicitAffinity : 1;                 // 0x300
        ULONG ProcessExecutionState : 2;            // 0x300
        ULONG DisallowStrippedImages : 1;           // 0x300
        ULONG HighEntropyASLREnabled : 1;           // 0x300
        ULONG ExtensionPointDisable : 1;            // 0x300
        ULONG ForceRelocateImages : 1;              // 0x300
        ULONG ProcessStateChangeRequest : 2;        // 0x300
        ULONG ProcessStateChangeInProgress : 1;     // 0x300
        ULONG DisallowWin32kSystemCalls : 1;        // 0x300
    };
} EPROCESS_FLAGS2;

typedef union _EPROCESS_FLAGS3 {
    ULONG Flags;
    union {
        ULONG Minimal : 1;
        ULONG ReplacingPageRoot : 1;
        ULONG DisableNonSystemFonts : 1;
        ULONG AuditNonSystemFontLoading : 1;
        ULONG Crashed : 1;
        ULONG JobVadsAreTracked : 1;
        ULONG VadTrackingDisabled : 1;
        ULONG AuxiliaryProcess : 1;
        ULONG SubsystemProcess : 1;
        ULONG IndirectCpuSets : 1;
        ULONG InPrivate : 1;
        ULONG ProhibitRemoteImageMap : 1;
        ULONG ProhibitLowILImageMap : 1;
        ULONG SignatureMitigationOptIn : 1;
    };
} EPROCESS_FLAGS3;

//
// The kernel EPROCESS structure.
//
typedef struct _EPROCESS {
    union {
        struct {
            KPROCESS Pcb;                               // 0x00
            EX_PUSH_LOCK ProcessLock;                   // 0x2D8
            EX_RUNDOWN_REF RundownProtect;              // 0x2E0
            HANDLE UniqueProcessId;                     // 0x2E8
            LIST_ENTRY ActiveProcessLinks;              // 0x2F0
            EPROCESS_FLAGS2 Flags2;                     // 0x300
            EPROCESS_FLAGS Flags;                       // 0x304
            LARGE_INTEGER CreateTime;                   // 0x308
            ULONGLONG ProcessQuotaUsage[2];             // 0x310
            ULONGLONG ProcessQuotaPeak[2];              // 0x320
            ULONGLONG PeakVirtualSize;                  // 0x330
            ULONGLONG VirtualSize;                      // 0x338
            LIST_ENTRY SessionProcessLinks;             // 0x340
            union {
                PVOID ExceptionPortData;                // 0x350
                ULONGLONG ExceptionPortValue;           // 0x350
                ULONGLONG ExceptionPortState : 3;       // 0x350
            };
            EX_FAST_REF Token;                          // 0x358
            ULONGLONG WorkingSetPage;                   // 0x360
            EX_PUSH_LOCK AddressCreationLock;           // 0x368
            EX_PUSH_LOCK PageTableCommitmentLock;       // 0x370
            struct _ETHREAD* RotateInProgress;          // 0x378
            struct _ETHREAD* ForkInProgress;            // 0x380
            struct _EJOB* CommitChargeJob;              // 0x388
            RTL_AVL_TREE CloneRoot;                     // 0x390
            ULONGLONG NumberOfPrivatePages;             // 0x398
            ULONGLONG NumberOfLockedPages;              // 0x3A0
            PVOID Win32Process;                         // 0x3A8
            struct _EJOB* Job;                          // 0x3B0
            PVOID SectionObject;                        // 0x3B8
            PVOID SectionBaseAddress;                   // 0x3C0
            ULONG Cookie;                               // 0x3C8
            PVOID WorkingSetWatch;                      // 0x3D0
            PVOID Win32WindowStation;                   // 0x3D8
            PVOID InheritedFromUniqueProcessId;         // 0x3E0
            PVOID LdtInformation;                       // 0x3E8
            ULONGLONG OwnerProcessId;                   // 0x3F0
            PPEB Peb;                                   // 0x3F8
            PVOID Session;                              // 0x400
            PVOID AweInfo;                              // 0x408
            PVOID QuotaBlock;                           // 0x410
            PHANDLE_TABLE ObjectTable;                  // 0x418
            PVOID DebugPort;                            // 0x420
            struct _EWOW64PROCESS* WoW64Process;        // 0x428
            PVOID DeviceMap;                            // 0x430
            PVOID EtwDataSource;                        // 0x438
            ULONGLONG PageDirectoryPte;                 // 0x440
            PFILE_OBJECT ImageFilePointer;              // 0x448
            UCHAR ImageFileName[15];                    // 0x450
            UCHAR PriorityClass;                        // 0x45F
            PVOID SecurityPort;                         // 0x460
            SE_AUDIT_PROCESS_CREATION_INFO SeAuditProcessCreationInfo; // 0x468
            LIST_ENTRY JobLinks;                        // 0x470
            PVOID HighestUserAddress;                   // 0x480
            LIST_ENTRY ThreadListHead;                  // 0x488
            ULONG ActiveThreads;                        // 0x498
            ULONG ImagePathHash;                        // 0x49C
            ULONG DefaultHardErrorProcessing;           // 0x4A0
            LONG LastThreadExitStatus;                  // 0x4A4
            EX_FAST_REF PrefetchTrace;                  // 0x4A8
            PVOID LockedPagesList;                      // 0x4B0
            LARGE_INTEGER ReadOperationCount;           // 0x4B8
            LARGE_INTEGER WriteOperationCount;          // 0x4C0
            LARGE_INTEGER OtherOperationCount;          // 0x4C8
            LARGE_INTEGER ReadTransferCount;            // 0x4D0
            LARGE_INTEGER WriteTransferCount;           // 0x4D8
            LARGE_INTEGER OtherTransferCount;           // 0x4E0
            ULONGLONG CommitChargeLimit;                // 0x4E8
            ULONGLONG CommitCharge;                     // 0x4F0
            ULONGLONG CommitChargePeak;                 // 0x4F8
            MMSUPPORT Vm;                               // 0x500
            LIST_ENTRY MmProcessLinks;                  // 0x5F8
            ULONG ModifiedPageCount;                    // 0x608
            LONG ExitStatus;                            // 0x60C
            RTL_AVL_TREE VadRoot;                       // 0x610
            PVOID VadHint;                              // 0x618
            ULONGLONG VadCount;                         // 0x620
            ULONGLONG VadPhysicalPages;                 // 0x628
            ULONGLONG VadPhysicalPagesLimit;            // 0x630
            ALPC_PROCESS_CONTEXT AlpcContext;           // 0x638
            LIST_ENTRY TimerResolutionLink;             // 0x658
            PVOID TimerResolutionStackRecord;           // 0x668
            ULONG RequestedTimerResolution;             // 0x670
            ULONG SmallestTimerResolution;              // 0x674
            LARGE_INTEGER ExitTime;                     // 0x678
            struct _INVERTED_FUNCTION_TABLE* InvertedFunctionTable; // 0x680
            EX_PUSH_LOCK InvertedFunctionTableLock;     // 0x688
            ULONG ActiveThreadsHighWatermark;           // 0x690
            ULONG LargePrivateVadCount;                 // 0x694
            EX_PUSH_LOCK ThreadListLock;                // 0x698
            PVOID WnfContext;                           // 0x6A0
            ULONGLONG Spare0;                           // 0x6A8
            UCHAR SignatureLevel;                       // 0x6B0
            UCHAR SectionSignatureLevel;                // 0x6B1
            PS_PROTECTION Protection;                   // 0x6B2
            UCHAR HangCount;                            // 0x6B3
            EPROCESS_FLAGS3 Flags3;                     // 0x6B4
            LONG DeviceAsid;                            // 0x6B8
            PVOID SvmData;                              // 0x6C0
            EX_PUSH_LOCK SvmProcessLock;                // 0x6C8
            ULONGLONG SvmLock;                          // 0x6D0
            LIST_ENTRY SvmProcessDeviceListHead;        // 0x6D8
            ULONGLONG LastFreezeInterruptTime;          // 0x6E8
            PPROCESS_DISK_COUNTERS DiskCounters;        // 0x6F0
            PVOID PicoContext;                          // 0x6F8
            ULONGLONG TrustletIdentity;                 // 0x700
            ULONG KeepAliveCounter;                     // 0x708
            ULONG NoWakeKeepAliveCounter;               // 0x70C
            ULONG HighPriorityFaultsAllowed;            // 0x710
            PPROCESS_ENERGY_VALUES EnergyValues;        // 0x718
            PVOID VmContext;                            // 0x720
            ULONGLONG SequenceNumber;                   // 0x728
            ULONGLONG CreateInterruptTime;              // 0x730
            ULONGLONG CreateUnbiasedInterruptTime;      // 0x738
            ULONGLONG TotalUnbiasedFrozenTime;          // 0x740
            ULONGLONG LastAppStateUpdateTime;           // 0x748
            union {
                ULONGLONG LastAppStateUptime : 61;      // 0x750
                ULONGLONG LastAppState : 3;             // 0x750
            };
        } Win10;

        struct {
            KPROCESS Pcb;                               // 0x00
            EX_PUSH_LOCK ProcessLock;                   // 0x2D8
            HANDLE UniqueProcessId;                     // 0x2E0
            LIST_ENTRY ActiveProcessLinks;              // 0x2E8
            EX_RUNDOWN_REF RundownProtect;              // 0x2F8
            EPROCESS_FLAGS2 Flags2;                     // 0x300
            EPROCESS_FLAGS Flags;                       // 0x304
            LARGE_INTEGER CreateTime;                   // 0x308
            ULONGLONG ProcessQuotaUsage[2];             // 0x310
            ULONGLONG ProcessQuotaPeak[2];              // 0x320
            ULONGLONG PeakVirtualSize;                  // 0x330
            ULONGLONG VirtualSize;                      // 0x338
            LIST_ENTRY SessionProcessLinks;             // 0x340
            union {
                PVOID ExceptionPortData;                // 0x350
                ULONGLONG ExceptionPortValue;           // 0x350
                ULONGLONG ExceptionPortState : 3;       // 0x350
            };
            EX_FAST_REF Token;                          // 0x358
            ULONG64 WorkingSetPage;                     // 0x360
            EX_PUSH_LOCK AddressCreationLock;           // 0x368
            EX_PUSH_LOCK PageTableCommitmentLock;       // 0x370
            struct _ETHREAD* RotateInProgress;          // 0x378
            struct _ETHREAD* ForkInProgress;            // 0x380
            struct _EJOB* CommitChargeJob;              // 0x388
            RTL_AVL_TREE CloneRoot;                     // 0x390
            ULONG64 NumberOfPrivatePages;               // 0x398
            ULONG64 NumberOfLockedPages;                // 0x3A0
            PVOID Win32Process;                         // 0x3A8
            struct _EJOB* Job;                          // 0x3B0
            PVOID SectionObject;                        // 0x3B8
            PVOID SectionBaseAddress;                   // 0x3C0
            ULONG Cookie;                               // 0x3C8
            PVOID WorkingSetWatch;                      // 0x3D0
            PVOID Win32WindowStation;                   // 0x3D8
            PVOID InheritedFromUniqueProcessId;         // 0x3E0
            PVOID LdtInformation;                       // 0x3E8
            ULONGLONG OwnerProcessId;                   // 0x3F0
            PPEB Peb;                                   // 0x3F8
            PVOID Session;                              // 0x400
            PVOID AweInfo;                              // 0x408
            PVOID QuotaBlock;                           // 0x410
            struct _HANDLE_TABLE* ObjectTable;          // 0x418
            PVOID DebugPort;                            // 0x420
            struct _EWOW64PROCESS* WoW64Process;        // 0x428
            PVOID DeviceMap;                            // 0x430
            PVOID EtwDataSource;                        // 0x438
            ULONGLONG PageDirectoryPte;                 // 0x440
            PFILE_OBJECT ImageFilePointer;              // 0x448
            UCHAR ImageFileName[15];                    // 0x450
            UCHAR PriorityClass;                        // 0x45F
            PVOID SecurityPort;                         // 0x460
            SE_AUDIT_PROCESS_CREATION_INFO SeAuditProcessCreationInfo; // 0x468
            LIST_ENTRY JobLinks;                        // 0x470
            PVOID HighestUserAddress;                   // 0x480
            LIST_ENTRY ThreadListHead;                  // 0x488
            ULONG ActiveThreads;                        // 0x498
            ULONG ImagePathHash;                        // 0x49C
            ULONG DefaultHardErrorProcessing;           // 0x4A0
            LONG LastThreadExitStatus;                  // 0x4A4
            EX_FAST_REF PrefetchTrace;                  // 0x4A8
            PVOID LockedPagesList;                      // 0x4B0
            LARGE_INTEGER ReadOperationCount;           // 0x4B8
            LARGE_INTEGER WriteOperationCount;          // 0x4C0
            LARGE_INTEGER OtherOperationCount;          // 0x4C8
            LARGE_INTEGER ReadTransferCount;            // 0x4D0
            LARGE_INTEGER WriteTransferCount;           // 0x4D8
            LARGE_INTEGER OtherTransferCount;           // 0x4E0
            ULONGLONG CommitChargeLimit;                // 0x4E8
            ULONGLONG CommitCharge;                     // 0x4F0
            ULONGLONG CommitChargePeak;                 // 0x4F8
            MMSUPPORT_FULL Vm;                          // 0x500
            LIST_ENTRY MmProcessLinks;                  // 0x610
            ULONG ModifiedPageCount;                    // 0x620
            LONG ExitStatus;                            // 0x624
            RTL_AVL_TREE VadRoot;                       // 0x628
            PVOID VadHint;                              // 0x630
            ULONGLONG VadCount;                         // 0x638
            ULONGLONG VadPhysicalPages;                 // 0x640
            ULONGLONG VadPhysicalPagesLimit;            // 0x648
            ALPC_PROCESS_CONTEXT AlpcContext;           // 0x650
            LIST_ENTRY TimerResolutionLink;             // 0x670
            PVOID TimerResolutionStackRecord;           // 0x680
            ULONG RequestedTimerResolution;             // 0x688
            ULONG SmallestTimerResolution;              // 0x68C
            LARGE_INTEGER ExitTime;                     // 0x690
            struct _INVERTED_FUNCTION_TABLE* InvertedFunctionTable; // 0x698
            EX_PUSH_LOCK InvertedFunctionTableLock;     // 0x6A0
            ULONG ActiveThreadsHighWatermark;           // 0x6A8
            ULONG LargePrivateVadCount;                 // 0x6AC
            EX_PUSH_LOCK ThreadListLock;                // 0x6B0
            PVOID WnfContext;                           // 0x6B8
            struct _EJOB *ServerSilo;                   // 0x6C0
            UCHAR SignatureLevel;                       // 0x6C8
            UCHAR SectionSignatureLevel;                // 0x6C9
            PS_PROTECTION Protection;                   // 0x6CA
            union {
                UCHAR HangCount : 4;                    // 0x6CB
                UCHAR GhostCount : 4;                   // 0x6CB
            };
            EPROCESS_FLAGS3 Flags3;                     // 0x6CC
            LONG DeviceAsid;                            // 0x6D0
            PVOID SvmData;                              // 0x6D8
            EX_PUSH_LOCK SvmProcessLock;                // 0x6E0
            ULONGLONG SvmLock;                          // 0x6E8
            LIST_ENTRY SvmProcessDeviceListHead;        // 0x6F0
            ULONGLONG LastFreezeInterruptTime;          // 0x700
            PPROCESS_DISK_COUNTERS DiskCounters;        // 0x708
            PVOID PicoContext;                          // 0x710
            ULONGLONG TrustletIdentity;                 // 0x718
            PVOID EnclaveTable;                         // 0x720
            ULONGLONG EnclaveNumber;                    // 0x728
            EX_PUSH_LOCK EnclaveLock;                   // 0x730
            ULONG HighPriorityFaultsAllowed;            // 0x738
            PVOID EnergyContext;                        // 0x740
            PVOID VmContext;                            // 0x748
            ULONGLONG SequenceNumber;                   // 0x750
            ULONGLONG CreateInterruptTime;              // 0x758
            ULONGLONG CreateUnbiasedInterruptTime;      // 0x760
            ULONGLONG TotalUnbiasedFrozenTime;          // 0x768
            ULONGLONG LastAppStateUpdateTime;           // 0x770
            union {
                ULONGLONG LastAppStateUptime : 61;      // 0x778
                ULONGLONG LastAppState : 3;             // 0x778
            };
        } Win10RS3;
    };
} EPROCESS, *PEPROCESS;

typedef struct _EPROCESS64 {
    union {
        struct {
            KPROCESS64 Pcb;                             // 0x00
            EX_PUSH_LOCK64 ProcessLock;                 // 0x2D8
            EX_RUNDOWN_REF64 RundownProtect;            // 0x2E0
            ULONGLONG UniqueProcessId;                  // 0x2E8 HANDLE
            LIST_ENTRY64 ActiveProcessLinks;            // 0x2F0
            EPROCESS_FLAGS2 Flags2;                     // 0x300
            EPROCESS_FLAGS Flags;                       // 0x304
            LARGE_INTEGER CreateTime;                   // 0x308
            ULONGLONG ProcessQuotaUsage[2];             // 0x310
            ULONGLONG ProcessQuotaPeak[2];              // 0x320
            ULONGLONG PeakVirtualSize;                  // 0x330
            ULONGLONG VirtualSize;                      // 0x338
            LIST_ENTRY64 SessionProcessLinks;           // 0x340
            union {
                PVOID ExceptionPortData;                // 0x350
                ULONGLONG ExceptionPortValue;           // 0x350
                ULONGLONG ExceptionPortState : 3;       // 0x350
            };
            EX_FAST_REF64 Token;                        // 0x358
            ULONGLONG WorkingSetPage;                   // 0x360
            EX_PUSH_LOCK64 AddressCreationLock;         // 0x368
            EX_PUSH_LOCK64 PageTableCommitmentLock;     // 0x370
            ULONGLONG RotateInProgress;                 // 0x378 PETHREAD
            ULONGLONG ForkInProgress;                   // 0x380 PETHREAD
            ULONGLONG CommitChargeJob;                  // 0x388 PEJOB
            RTL_AVL_TREE64 CloneRoot;                   // 0x390
            ULONGLONG NumberOfPrivatePages;             // 0x398
            ULONGLONG NumberOfLockedPages;              // 0x3A0
            ULONGLONG Win32Process;                     // 0x3A8 PVOID
            ULONGLONG Job;                              // 0x3B0 PEJOB
            ULONGLONG SectionObject;                    // 0x3B8 PVOID
            ULONGLONG SectionBaseAddress;               // 0x3C0 PVOID
            ULONG Cookie;                               // 0x3C8
            ULONGLONG WorkingSetWatch;                  // 0x3D0 PVOID
            ULONGLONG Win32WindowStation;               // 0x3D8 PVOID
            ULONGLONG InheritedFromUniqueProcessId;     // 0x3E0 PVOID
            ULONGLONG LdtInformation;                   // 0x3E8 PVOID
            ULONGLONG OwnerProcessId;                   // 0x3F0
            ULONGLONG Peb;                              // 0x3F8 PPEB
            ULONGLONG Session;                          // 0x400 PVOID
            ULONGLONG AweInfo;                          // 0x408 PVOID
            ULONGLONG QuotaBlock;                       // 0x410 PVOID
            ULONGLONG ObjectTable;                      // 0x418 PHANDLE_TABLE
            ULONGLONG DebugPort;                        // 0x420 PVOID
            ULONGLONG WoW64Process;                     // 0x428 PEWOW64PROCESS64
            ULONGLONG DeviceMap;                        // 0x430 PVOID
            ULONGLONG EtwDataSource;                    // 0x438 PVOID
            ULONGLONG PageDirectoryPte;                 // 0x440
            ULONGLONG ImageFilePointer;                 // 0x448 PFILE_OBJECT
            UCHAR ImageFileName[15];                    // 0x450
            UCHAR PriorityClass;                        // 0x45F
            ULONGLONG SecurityPort;                     // 0x460 PVOID
            SE_AUDIT_PROCESS_CREATION_INFO64 SeAuditProcessCreationInfo; // 0x468
            LIST_ENTRY64 JobLinks;                      // 0x470
            ULONGLONG HighestUserAddress;               // 0x480 PVOID
            LIST_ENTRY64 ThreadListHead;                // 0x488
            ULONG ActiveThreads;                        // 0x498
            ULONG ImagePathHash;                        // 0x49C
            ULONG DefaultHardErrorProcessing;           // 0x4A0
            LONG LastThreadExitStatus;                  // 0x4A4
            EX_FAST_REF64 PrefetchTrace;                // 0x4A8
            ULONGLONG LockedPagesList;                  // 0x4B0 PVOID
            LARGE_INTEGER ReadOperationCount;           // 0x4B8
            LARGE_INTEGER WriteOperationCount;          // 0x4C0
            LARGE_INTEGER OtherOperationCount;          // 0x4C8
            LARGE_INTEGER ReadTransferCount;            // 0x4D0
            LARGE_INTEGER WriteTransferCount;           // 0x4D8
            LARGE_INTEGER OtherTransferCount;           // 0x4E0
            ULONGLONG CommitChargeLimit;                // 0x4E8
            ULONGLONG CommitCharge;                     // 0x4F0
            ULONGLONG CommitChargePeak;                 // 0x4F8
            MMSUPPORT64 Vm;                             // 0x500
            LIST_ENTRY64 MmProcessLinks;                // 0x5F8
            ULONG ModifiedPageCount;                    // 0x608
            LONG ExitStatus;                            // 0x60C
            RTL_AVL_TREE64 VadRoot;                     // 0x610
            ULONGLONG VadHint;                          // 0x618 PVOID
            ULONGLONG VadCount;                         // 0x620
            ULONGLONG VadPhysicalPages;                 // 0x628
            ULONGLONG VadPhysicalPagesLimit;            // 0x630
            ALPC_PROCESS_CONTEXT64 AlpcContext;         // 0x638
            LIST_ENTRY64 TimerResolutionLink;           // 0x658
            ULONGLONG TimerResolutionStackRecord;       // 0x668 PVOID
            ULONG RequestedTimerResolution;             // 0x670
            ULONG SmallestTimerResolution;              // 0x674
            LARGE_INTEGER ExitTime;                     // 0x678
            ULONGLONG InvertedFunctionTable;            // 0x680 PINVERTED_FUNCTION_TABLE
            EX_PUSH_LOCK64 InvertedFunctionTableLock;   // 0x688
            ULONG ActiveThreadsHighWatermark;           // 0x690
            ULONG LargePrivateVadCount;                 // 0x694
            EX_PUSH_LOCK64 ThreadListLock;              // 0x698
            ULONGLONG WnfContext;                       // 0x6A0 PVOID
            ULONGLONG Spare0;                           // 0x6A8
            UCHAR SignatureLevel;                       // 0x6B0
            UCHAR SectionSignatureLevel;                // 0x6B1
            PS_PROTECTION Protection;                   // 0x6B2
            UCHAR HangCount;                            // 0x6B3
            EPROCESS_FLAGS3 Flags3;                     // 0x6B4
            LONG DeviceAsid;                            // 0x6B8
            ULONGLONG SvmData;                          // 0x6C0 PVOID
            EX_PUSH_LOCK64 SvmProcessLock;              // 0x6C8
            ULONGLONG SvmLock;                          // 0x6D0
            LIST_ENTRY64 SvmProcessDeviceListHead;      // 0x6D8
            ULONGLONG LastFreezeInterruptTime;          // 0x6E8
            ULONGLONG DiskCounters;                     // 0x6F0 PPROCESS_DISK_COUNTERS
            ULONGLONG PicoContext;                      // 0x6F8 PVOID
            ULONGLONG TrustletIdentity;                 // 0x700
            ULONG KeepAliveCounter;                     // 0x708
            ULONG NoWakeKeepAliveCounter;               // 0x70C
            ULONG HighPriorityFaultsAllowed;            // 0x710
            ULONGLONG EnergyValues;                     // 0x718 PPROCESS_ENERGY_VALUES
            ULONGLONG VmContext;                        // 0x720 PVOID
            ULONGLONG SequenceNumber;                   // 0x728
            ULONGLONG CreateInterruptTime;              // 0x730
            ULONGLONG CreateUnbiasedInterruptTime;      // 0x738
            ULONGLONG TotalUnbiasedFrozenTime;          // 0x740
            ULONGLONG LastAppStateUpdateTime;           // 0x748
            union {
                ULONGLONG LastAppStateUptime : 61;      // 0x750
                ULONGLONG LastAppState : 3;             // 0x750
            };
        } Win10;

        struct {
            KPROCESS64 Pcb;                             // 0x00
            EX_PUSH_LOCK64 ProcessLock;                 // 0x2D8
            ULONGLONG UniqueProcessId;                  // 0x2E0 HANDLE
            LIST_ENTRY64 ActiveProcessLinks;            // 0x2E8
            EX_RUNDOWN_REF64 RundownProtect;            // 0x2F8
            EPROCESS_FLAGS2 Flags2;                     // 0x300
            EPROCESS_FLAGS Flags;                       // 0x304
            LARGE_INTEGER CreateTime;                   // 0x308
            ULONGLONG ProcessQuotaUsage[2];             // 0x310
            ULONGLONG ProcessQuotaPeak[2];              // 0x320
            ULONGLONG PeakVirtualSize;                  // 0x330
            ULONGLONG VirtualSize;                      // 0x338
            LIST_ENTRY64 SessionProcessLinks;           // 0x340
            union {
                ULONGLONG ExceptionPortData;            // 0x350 PVOID
                ULONGLONG ExceptionPortValue;           // 0x350
                ULONGLONG ExceptionPortState : 3;       // 0x350
            };
            EX_FAST_REF64 Token;                        // 0x358
            ULONG64 WorkingSetPage;                     // 0x360
            EX_PUSH_LOCK64 AddressCreationLock;         // 0x368
            EX_PUSH_LOCK64 PageTableCommitmentLock;     // 0x370
            ULONGLONG RotateInProgress;                 // 0x378 PETHREAD
            ULONGLONG ForkInProgress;                   // 0x380 PETHREAD
            ULONGLONG CommitChargeJob;                  // 0x388 PEJOB
            RTL_AVL_TREE64 CloneRoot;                   // 0x390
            ULONG64 NumberOfPrivatePages;               // 0x398
            ULONG64 NumberOfLockedPages;                // 0x3A0
            ULONGLONG Win32Process;                     // 0x3A8 PVOID
            ULONGLONG Job;                              // 0x3B0 PEJOB
            ULONGLONG SectionObject;                    // 0x3B8 PVOID
            ULONGLONG SectionBaseAddress;               // 0x3C0 PVOID
            ULONG Cookie;                               // 0x3C8
            ULONGLONG WorkingSetWatch;                  // 0x3D0 PVOID
            ULONGLONG Win32WindowStation;               // 0x3D8 PVOID
            ULONGLONG InheritedFromUniqueProcessId;     // 0x3E0 PVOID
            ULONGLONG LdtInformation;                   // 0x3E8 PVOID
            ULONGLONG OwnerProcessId;                   // 0x3F0
            ULONGLONG Peb;                              // 0x3F8 PPEB
            ULONGLONG Session;                          // 0x400 PVOID
            ULONGLONG AweInfo;                          // 0x408 PVOID
            ULONGLONG QuotaBlock;                       // 0x410 PVOID
            ULONGLONG ObjectTable;                      // 0x418 PHANDLE_TABLE
            ULONGLONG DebugPort;                        // 0x420 PVOID
            ULONGLONG WoW64Process;                     // 0x428 PEWOW64PROCESS64
            ULONGLONG DeviceMap;                        // 0x430 PVOID
            ULONGLONG EtwDataSource;                    // 0x438 PVOID
            ULONGLONG PageDirectoryPte;                 // 0x440
            ULONGLONG ImageFilePointer;                 // 0x448 PFILE_OBJECT
            UCHAR ImageFileName[15];                    // 0x450
            UCHAR PriorityClass;                        // 0x45F
            ULONGLONG SecurityPort;                     // 0x460 PVOID
            SE_AUDIT_PROCESS_CREATION_INFO64 SeAuditProcessCreationInfo; // 0x468
            LIST_ENTRY64 JobLinks;                      // 0x470
            ULONGLONG HighestUserAddress;               // 0x480 PVOID
            LIST_ENTRY64 ThreadListHead;                // 0x488
            ULONG ActiveThreads;                        // 0x498
            ULONG ImagePathHash;                        // 0x49C
            ULONG DefaultHardErrorProcessing;           // 0x4A0
            LONG LastThreadExitStatus;                  // 0x4A4
            EX_FAST_REF64 PrefetchTrace;                // 0x4A8
            ULONGLONG LockedPagesList;                  // 0x4B0 PVOID
            LARGE_INTEGER ReadOperationCount;           // 0x4B8
            LARGE_INTEGER WriteOperationCount;          // 0x4C0
            LARGE_INTEGER OtherOperationCount;          // 0x4C8
            LARGE_INTEGER ReadTransferCount;            // 0x4D0
            LARGE_INTEGER WriteTransferCount;           // 0x4D8
            LARGE_INTEGER OtherTransferCount;           // 0x4E0
            ULONGLONG CommitChargeLimit;                // 0x4E8
            ULONGLONG CommitCharge;                     // 0x4F0
            ULONGLONG CommitChargePeak;                 // 0x4F8
            MMSUPPORT_FULL64 Vm;                        // 0x500
            LIST_ENTRY64 MmProcessLinks;                // 0x610
            ULONG ModifiedPageCount;                    // 0x620
            LONG ExitStatus;                            // 0x624
            RTL_AVL_TREE64 VadRoot;                     // 0x628
            ULONGLONG VadHint;                          // 0x630 PVOID
            ULONGLONG VadCount;                         // 0x638
            ULONGLONG VadPhysicalPages;                 // 0x640
            ULONGLONG VadPhysicalPagesLimit;            // 0x648
            ALPC_PROCESS_CONTEXT64 AlpcContext;         // 0x650
            LIST_ENTRY64 TimerResolutionLink;           // 0x670
            ULONGLONG TimerResolutionStackRecord;       // 0x680 PVOID
            ULONG RequestedTimerResolution;             // 0x688
            ULONG SmallestTimerResolution;              // 0x68C
            LARGE_INTEGER ExitTime;                     // 0x690
            ULONGLONG InvertedFunctionTable;            // 0x698 PINVERTED_FUNCTION_TABLE
            EX_PUSH_LOCK64 InvertedFunctionTableLock;   // 0x6A0
            ULONG ActiveThreadsHighWatermark;           // 0x6A8
            ULONG LargePrivateVadCount;                 // 0x6AC
            EX_PUSH_LOCK64 ThreadListLock;              // 0x6B0
            ULONGLONG WnfContext;                       // 0x6B8 PVOID
            ULONGLONG ServerSilo;                       // 0x6C0 PEJOB
            UCHAR SignatureLevel;                       // 0x6C8
            UCHAR SectionSignatureLevel;                // 0x6C9
            PS_PROTECTION Protection;                   // 0x6CA
            union {
                UCHAR HangCount : 4;                    // 0x6CB
                UCHAR GhostCount : 4;                   // 0x6CB
            };
            EPROCESS_FLAGS3 Flags3;                     // 0x6CC
            LONG DeviceAsid;                            // 0x6D0
            ULONGLONG SvmData;                          // 0x6D8 PVOID
            EX_PUSH_LOCK64 SvmProcessLock;              // 0x6E0
            ULONGLONG SvmLock;                          // 0x6E8
            LIST_ENTRY64 SvmProcessDeviceListHead;      // 0x6F0
            ULONGLONG LastFreezeInterruptTime;          // 0x700
            ULONGLONG DiskCounters;                     // 0x708 PPROCESS_DISK_COUNTERS
            ULONGLONG PicoContext;                      // 0x710 PVOID
            ULONGLONG TrustletIdentity;                 // 0x718
            ULONGLONG EnclaveTable;                     // 0x720 PVOID
            ULONGLONG EnclaveNumber;                    // 0x728
            EX_PUSH_LOCK64 EnclaveLock;                 // 0x730
            ULONG HighPriorityFaultsAllowed;            // 0x738
            ULONGLONG EnergyContext;                    // 0x740 PVOID
            ULONGLONG VmContext;                        // 0x748 PVOID
            ULONGLONG SequenceNumber;                   // 0x750
            ULONGLONG CreateInterruptTime;              // 0x758
            ULONGLONG CreateUnbiasedInterruptTime;      // 0x760
            ULONGLONG TotalUnbiasedFrozenTime;          // 0x768
            ULONGLONG LastAppStateUpdateTime;           // 0x770
            union {
                ULONGLONG LastAppStateUptime : 61;      // 0x778
                ULONGLONG LastAppState : 3;             // 0x778
            };
        } Win10Rs3;
    };
} EPROCESS64, *PEPROCESS64;

typedef struct _EWOW64PROCESS {
    PVOID Peb;                          // 0x00
    USHORT Machine;                     // 0x08
    USHORT Reserved0;                   // 0x0A
    ULONG NtdllType;                    // 0x0C
} EWOW64PROCESS, *PEWOW64PROCESS;

typedef struct _EWOW64PROCESS64 {
    ULONGLONG Peb;                      // 0x00
    USHORT Machine;                     // 0x08
    USHORT Reserved0;                   // 0x0A
    ULONG NtdllType;                    // 0x0C
} EWOW64PROCESS64, *PEWOW64PROCESS64;




///
/// < Process Routines >
///

#define NtCurrentProcess()      ((HANDLE)(LONG_PTR)-1)
#define ZwCurrentProcess()      NtCurrentProcess()
#define NtCurrentThread()       ((HANDLE)(LONG_PTR)-2)
#define ZwCurrentThread()       NtCurrentThread()
#define NtCurrentSession()      ((HANDLE)(LONG_PTR)-3)
#define ZwCurrentSession()      NtCurrentSession()
#define NtCurrentPeb()          (NtCurrentTeb()->ProcessEnvironmentBlock)
#define NtCurrentProcessId()    (NtCurrentTeb()->ClientId.UniqueProcess)
#define NtCurrentThreadId()     (NtCurrentTeb()->ClientId.UniqueThread)

typedef
NTSTATUS
(NTAPI *PUSER_THREAD_START_ROUTINE)(
    IN PVOID Parameter
    );

typedef
NTSTATUS
(NTAPI *PNT_CREATE_THREAD_EX)(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    IN PVOID StartRoutine, // PUSER_THREAD_START_ROUTINE
    IN PVOID Argument OPTIONAL,
    IN ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
    IN SIZE_T ZeroBits,
    IN SIZE_T StackSize,
    IN SIZE_T MaximumStackSize,
    IN PPS_ATTRIBUTE_LIST AttributeList OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCreateThreadEx(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    IN PVOID StartRoutine, // PUSER_THREAD_START_ROUTINE
    IN PVOID Argument OPTIONAL,
    IN ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
    IN SIZE_T ZeroBits,
    IN SIZE_T StackSize,
    IN SIZE_T MaximumStackSize,
    IN PPS_ATTRIBUTE_LIST AttributeList OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_CREATE_THREAD)(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN PCONTEXT ThreadContext,
    IN PINITIAL_TEB InitialTeb,
    IN BOOLEAN CreateSuspended
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN PCONTEXT ThreadContext,
    IN PINITIAL_TEB InitialTeb,
    IN BOOLEAN CreateSuspended
    );

typedef
NTSTATUS
(NTAPI *PNT_OPEN_THREAD)(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN OUT PCLIENT_ID ClientId OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtOpenThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN OUT PCLIENT_ID ClientId OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_TERMINATE_THREAD)(
    IN HANDLE ThreadHandle,
    IN UINT ExitCode
    );
NTSYSAPI
NTSTATUS
NTAPI
NtTerminateThread(
    IN HANDLE ThreadHandle,
    IN UINT ExitCode
    );

typedef
NTSTATUS
(NTAPI *PNT_SUSPEND_THREAD)(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtSuspendThread(
    IN HANDLE ThreadHandle,
    OUT PULONG PreviousSuspendCount OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_RESUME_THREAD)(
    IN HANDLE ThreadHandle,
    OUT PULONG SuspendCount OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtResumeThread(
    IN HANDLE ThreadHandle,
    OUT PULONG SuspendCount OPTIONAL
    );

typedef
VOID
(*PPS_APC_ROUTINE)(
    IN PVOID ApcArgument1 OPTIONAL,
    IN PVOID ApcArgument2 OPTIONAL,
    IN PVOID ApcArgument3 OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_QUEUE_APC_THREAD)(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1 OPTIONAL,
    IN PVOID ApcArgument2 OPTIONAL,
    IN PVOID ApcArgument3 OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtQueueApcThread(
    IN HANDLE ThreadHandle,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1 OPTIONAL,
    IN PVOID ApcArgument2 OPTIONAL,
    IN PVOID ApcArgument3 OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_WIN7)
typedef
NTSTATUS
(NTAPI *PNT_QUEUE_APC_THREAD_EX)(
    IN HANDLE ThreadHandle,
    IN HANDLE UserApcReserveHandle OPTIONAL,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1 OPTIONAL,
    IN PVOID ApcArgument2 OPTIONAL,
    IN PVOID ApcArgument3 OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtQueueApcThreadEx(
    IN HANDLE ThreadHandle,
    IN HANDLE UserApcReserveHandle OPTIONAL,
    IN PPS_APC_ROUTINE ApcRoutine,
    IN PVOID ApcArgument1 OPTIONAL,
    IN PVOID ApcArgument2 OPTIONAL,
    IN PVOID ApcArgument3 OPTIONAL
    );
#endif

typedef
NTSTATUS
(NTAPI *PNT_CREATE_PROCESS)(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_CREATE_PROCESS_EX)(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCreateProcessEx(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel
    );

typedef
NTSTATUS
(NTAPI *PNT_OPEN_PROCESS)(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK AccessMask,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId
    );
NTSYSAPI
NTSTATUS
NTAPI
NtOpenProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK AccessMask,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId
    );

typedef
NTSTATUS
(NTAPI *PNT_TERMINATE_PROCESS)(
    IN HANDLE ProcessHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
NtTerminateProcess(
    IN HANDLE ProcessHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    );

typedef
NTSTATUS
(NTAPI *PNT_SUSPEND_PROCESS)(
    IN HANDLE ProcessHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
NtSuspendProcess(
    IN HANDLE ProcessHandle
    );

typedef
NTSTATUS
(NTAPI *PNT_RESUME_PROCESS)(
    IN HANDLE ProcessHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
NtResumeProcess(
    IN HANDLE ProcessHandle
    );

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus