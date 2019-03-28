#pragma once

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_IX86)
#define _X86_
#if !defined(_CHPE_X86_ARM64_) && defined(_M_HYBRID)
#define _CHPE_X86_ARM64_
#endif
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_AMD64)
#define _AMD64_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_ARM)
#define _ARM_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_ARM64)
#define _ARM64_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_M68K)
#define _68K_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_MPPC)
#define _MPPC_
#endif
#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_IA64)
#if !defined(_IA64_)
#define _IA64_
#endif /* !_IA64_ */
#endif
#ifndef _MAC
#if defined(_68K_) || defined(_MPPC_)
#define _MAC
#endif
#endif

#include <winternl.h>
#include <windef.h>
#include <winerror.h>
#include <winioctl.h>
#include <WinNls.h>
#include <stdarg.h>

#include "nativestatus.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < Common Defines >
///

#ifndef NOTHING
#define NOTHING
#endif

#ifndef FASTCALL
#ifndef _WIN64
#define FASTCALL __fastcall
#else
#define FASTCALL
#endif
#endif


///
/// < Common Datatypes >
///

//
// Cardinal types
//
typedef char CCHAR;
typedef short CSHORT;
typedef ULONG CLONG;
typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;
typedef PCSTR PCSZ;

//
// GUID pointer definition
//
typedef GUID *PGUID;

//
// Interrupt Request Level (IRQL)
//
typedef UCHAR KIRQL;
typedef KIRQL *PKIRQL;

//
// Thread priority
//
typedef LONG KPRIORITY;

//
// Spin Lock
//
typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

//
// Physical address.
//
typedef LARGE_INTEGER PHYSICAL_ADDRESS;
typedef PHYSICAL_ADDRESS *PPHYSICAL_ADDRESS;

//
// Processor modes.
//
typedef CCHAR KPROCESSOR_MODE;

//
// The type QUAD and UQUAD are intended to use when a 8 byte aligned structure
// is required, but it is not a floating point number.
//
typedef struct _QUAD {
    union { long long UseThisFieldToCopy; 
    double DoNotUseThisField; };
} QUAD;
typedef QUAD *PQUAD;
typedef QUAD UQUAD;
typedef UQUAD *PUQUAD;


///
/// < Common Enums >
///

//
// Processor modes.
//
typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;

//
// Event type
//
typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;

//
// Timer type
//
typedef enum _TIMER_TYPE {
    NotificationTimer,
    SynchronizationTimer
} TIMER_TYPE;

//
// Wait type
//
typedef enum _WAIT_TYPE {
    WaitAll,
    WaitAny,
    WaitNotification
} WAIT_TYPE;

//
// Product types
//
typedef enum _NT_PRODUCT_TYPE {
    NtProductWinNt = 1,
    NtProductLanManNt,
    NtProductServer
} NT_PRODUCT_TYPE, *PNT_PRODUCT_TYPE;

//
// The bit mask, SharedUserData->SuiteMask, is a ULONG
// so there can be a maximum of 32 entries in this enum.
//
typedef enum _SUITE_TYPE {
    SmallBusiness,
    Enterprise,
    BackOffice,
    CommunicationServer,
    TerminalServer,
    SmallBusinessRestricted,
    EmbeddedNT,
    DataCenter,
    SingleUserTS,
    Personal,
    Blade,
    EmbeddedRestricted,
    SecurityAppliance,
    StorageServer,
    ComputeServer,
    WHServer,
    PhoneNT,
    MaxSuiteType
} SUITE_TYPE;

//
// Lock operation
//
typedef enum _LOCK_OPERATION {
    IoReadAccess,
    IoWriteAccess,
    IoModifyAccess
} LOCK_OPERATION;

//
// Wait reasons
//
typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrSpare0,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    WrKeyedEvent,
    WrTerminated,
    WrProcessInSwap,
    WrCpuRateControl,
    WrCalloutStack,
    WrKernel,
    WrResource,
    WrPushLock,
    WrMutex,
    WrQuantumEnd,
    WrDispatchInt,
    WrPreempted,
    WrYieldExecution,
    WrFastMutex,
    WrGuardedMutex,
    WrRundown,
    WrAlertByThreadId,
    WrDeferredPreempt,
    MaximumWaitReason
} KWAIT_REASON;


///
/// < Common Structures >
///

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _CLIENT_ID32 {
    ULONG UniqueProcess;
    ULONG UniqueThread;
} CLIENT_ID32, *PCLIENT_ID32;

typedef struct _CLIENT_ID64 {
    ULONGLONG UniqueProcess;
    ULONGLONG UniqueThread;
} CLIENT_ID64, *PCLIENT_ID64;

typedef struct _SINGLE_LIST_ENTRY64 {
    ULONGLONG Next;
} SINGLE_LIST_ENTRY64, *PSINGLE_LIST_ENTRY64;

typedef struct _SINGLE_LIST_ENTRY32 {
    ULONG Next;
} SINGLE_LIST_ENTRY32, *PSINGLE_LIST_ENTRY32;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _ANSI_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PSTR Buffer;
} STRING, ANSI_STRING, OEM_STRING, *PSTRING, *PANSI_STRING, *POEM_STRING;
typedef const STRING *PCSTRING;
typedef const ANSI_STRING *PCANSI_STRING;
typedef const OEM_STRING *PCOEM_STRING;

typedef struct _STRING64 {
    USHORT Length;
    USHORT MaximumLength;
    ULONGLONG Buffer;
} STRING64, *PSTRING64;
typedef STRING64 UNICODE_STRING64, *PUNICODE_STRING64;
typedef STRING64 ANSI_STRING64, *PANSI_STRING64;

typedef struct _STRING32 {
    USHORT Length;
    USHORT MaximumLength;
    ULONG Buffer;
} STRING32;
typedef STRING32 *PSTRING32;
typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32 *PUNICODE_STRING32;

typedef struct _CURDIR {
    UNICODE_STRING DosPath; // 0x00
    PVOID Handle;           // 0x10
} CURDIR, *PCURDIR;

typedef struct _CURDIR64 {
    UNICODE_STRING64 DosPath;
    ULONGLONG Handle;
} CURDIR64, *PCURDIR64;

typedef struct _CURDIR32 {
    UNICODE_STRING32 DosPath;
    ULONG Handle; // HANDLE
} CURDIR32, *PCURDIR32;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor; // PSECURITY_DESCRIPTOR;
    PVOID SecurityQualityOfService; // PSECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef const OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

typedef struct _OBJECT_ATTRIBUTES64 {
    ULONG Length;
    ULONG64 RootDirectory;
    ULONG64 ObjectName;
    ULONG Attributes;
    ULONG64 SecurityDescriptor;
    ULONG64 SecurityQualityOfService;
} OBJECT_ATTRIBUTES64, *POBJECT_ATTRIBUTES64;
typedef const OBJECT_ATTRIBUTES64 *PCOBJECT_ATTRIBUTES64;

typedef struct _OBJECT_ATTRIBUTES32 {
    ULONG Length;
    ULONG RootDirectory;
    ULONG ObjectName;
    ULONG Attributes;
    ULONG SecurityDescriptor;
    ULONG SecurityQualityOfService;
} OBJECT_ATTRIBUTES32, *POBJECT_ATTRIBUTES32;
typedef const OBJECT_ATTRIBUTES32 *PCOBJECT_ATTRIBUTES32;

typedef struct _RTL_BALANCED_NODE32 {
    union {
        ULONG Children[2];  // 0x00 struct _RTL_BALANCED_NODE*
        struct {
            ULONG Left;     // 0x00 struct _RTL_BALANCED_NODE*
            ULONG Right;    // 0x04 struct _RTL_BALANCED_NODE*
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    union {
        UCHAR Red : 1;      // 0x08 UCHAR
        UCHAR Balance : 2;  // 0x08 UCHAR
        ULONG ParentValue;  // 0x08 ULONG_PTR
    } DUMMYUNIONNAME2;
} RTL_BALANCED_NODE32, *PRTL_BALANCED_NODE32;

typedef struct _RTL_BALANCED_NODE64 {
    union {
        ULONG64 Children[2]; // struct _RTL_BALANCED_NODE*
        struct {
            ULONG64 Left; // struct _RTL_BALANCED_NODE*
            ULONG64 Right; // struct _RTL_BALANCED_NODE*
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    union {
        UCHAR Red : 1; // UCHAR
        UCHAR Balance : 2; // UCHAR
        ULONG64 ParentValue; // ULONG_PTR
    } DUMMYUNIONNAME2;
} RTL_BALANCED_NODE64, *PRTL_BALANCED_NODE64;

typedef struct _RTL_BALANCED_NODE {
    union {
        struct _RTL_BALANCED_NODE *Children[2];
        struct {
            struct _RTL_BALANCED_NODE *Left;
            struct _RTL_BALANCED_NODE *Right;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
#define RTL_BALANCED_NODE_RESERVED_PARENT_MASK 0x3
    union {
        UCHAR Red : 1;
        UCHAR Balance : 2;
        ULONG_PTR ParentValue;
    } DUMMYUNIONNAME2;
} RTL_BALANCED_NODE, *PRTL_BALANCED_NODE;

#define RTL_BALANCED_NODE_GET_PARENT_POINTER(Node) \
    ((PRTL_BALANCED_NODE)((Node)->ParentValue & ~RTL_BALANCED_NODE_RESERVED_PARENT_MASK))

// RB trees
typedef struct _RTL_RB_TREE {
    PRTL_BALANCED_NODE Root;
    PRTL_BALANCED_NODE Min;
} RTL_RB_TREE, *PRTL_RB_TREE;

typedef struct _RTL_RB_TREE64 {
    ULONGLONG Root; // PRTL_BALANCED_NODE64
    ULONGLONG Min;  // PRTL_BALANCED_NODE64
} RTL_RB_TREE64, *PRTL_RB_TREE64;

// AVL trees
typedef struct _RTL_AVL_TREE {
    PRTL_BALANCED_NODE Root;
} RTL_AVL_TREE, *PRTL_AVL_TREE;

typedef struct _RTL_AVL_TREE64 {
    ULONGLONG Root; // PRTL_BALANCED_NODE64
} RTL_AVL_TREE64, *PRTL_AVL_TREE64;

//
// Legacy thread affinity.
//
typedef ULONG_PTR KAFFINITY;
typedef KAFFINITY *PKAFFINITY;

//
// New thread affinity.
//
typedef struct _KAFFINITY_EX {
    USHORT Count;           // 0x00
    USHORT Size;            // 0x02
    ULONG Reserved;         // 0x04
    ULONG64 Bitmap[20];     // 0x08
} KAFFINITY_EX, *PKAFFINITY_EX;

//
// DPC routine
//
struct _KDPC;
typedef VOID KDEFERRED_ROUTINE(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext OPTIONAL,
    IN PVOID SystemArgument1 OPTIONAL,
    IN PVOID SystemArgument2 OPTIONAL
    );
typedef KDEFERRED_ROUTINE *PKDEFERRED_ROUTINE;

//
// Define DPC importance.
//
// LowImportance - Queue DPC at end of target DPC queue.
// MediumImportance - Queue DPC at end of target DPC queue.
// MediumHighImportance - Queue DPC at end of target DPC queue.
// HighImportance - Queue DPC at front of target DPC DPC queue.
//
typedef enum _KDPC_IMPORTANCE {
    LowImportance,
    MediumImportance,
    HighImportance,
    MediumHighImportance
} KDPC_IMPORTANCE;

//
// Deferred Procedure Call (DPC) object
//
typedef struct _KDPC {
    union {
        ULONG TargetInfoAsUlong;
        struct {
            UCHAR Type;
            UCHAR Importance;
            volatile USHORT Number;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    SINGLE_LIST_ENTRY DpcListEntry;
    KAFFINITY ProcessorHistory;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    __volatile PVOID DpcData;
} KDPC, *PKDPC, *PRKDPC;

//
// Common dispatcher object header
//
// N.B. The size field contains the number of dwords in the structure.
//
#if defined(_X86_)
#define KENCODED_TIMER_PROCESSOR        1   // Timer processor is encoded in header
#endif
#define TIMER_TOLERABLE_DELAY_BITS      6
#define TIMER_EXPIRED_INDEX_BITS        6
#define TIMER_PROCESSOR_INDEX_BITS      5

typedef struct _DISPATCHER_HEADER {
    union {
        union {
            volatile LONG Lock;
            LONG LockNV;
        } DUMMYUNIONNAME;

        struct {                            // Events, Semaphores, Gates, etc.
            UCHAR Type;                     // All (accessible via KOBJECT_TYPE)
            UCHAR Signalling;
            UCHAR Size;
            UCHAR Reserved1;
        } DUMMYSTRUCTNAME;

        struct {                            // Timer
            UCHAR TimerType;
            union {
                UCHAR TimerControlFlags;
                struct {
                    UCHAR Absolute : 1;
                    UCHAR Wake : 1;
                    UCHAR EncodedTolerableDelay : TIMER_TOLERABLE_DELAY_BITS;
                } DUMMYSTRUCTNAME;
            };

            UCHAR Hand;
            union {
                UCHAR TimerMiscFlags;
                struct {
#if !defined(KENCODED_TIMER_PROCESSOR)
                    UCHAR Index : TIMER_EXPIRED_INDEX_BITS;
#else
                    UCHAR Index : 1;
                    UCHAR Processor : TIMER_PROCESSOR_INDEX_BITS;
#endif
                    UCHAR Inserted : 1;
                    volatile UCHAR Expired : 1;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;
        } DUMMYSTRUCTNAME2;

        struct {                            // Timer2
            UCHAR Timer2Type;
            union {
                UCHAR Timer2Flags;
                struct {
                    UCHAR Timer2Inserted : 1;
                    UCHAR Timer2Expiring : 1;
                    UCHAR Timer2CancelPending : 1;
                    UCHAR Timer2SetPending : 1;
                    UCHAR Timer2Running : 1;
                    UCHAR Timer2Disabled : 1;
                    UCHAR Timer2ReservedFlags : 2;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;

            UCHAR Timer2Reserved1;
            UCHAR Timer2Reserved2;
        } DUMMYSTRUCTNAME3;

        struct {                            // Queue
            UCHAR QueueType;
            union {
                UCHAR QueueControlFlags;
                struct {
                    UCHAR Abandoned : 1;
                    UCHAR DisableIncrement : 1;
                    UCHAR QueueReservedControlFlags : 6;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;

            UCHAR QueueSize;
            UCHAR QueueReserved;
        } DUMMYSTRUCTNAME4;

        struct {                            // Thread
            UCHAR ThreadType;
            UCHAR ThreadReserved;
            union {
                UCHAR ThreadControlFlags;
                struct {
                    UCHAR CycleProfiling : 1;
                    UCHAR CounterProfiling : 1;
                    UCHAR GroupScheduling : 1;
                    UCHAR AffinitySet : 1;
                    UCHAR Tagged : 1;
                    UCHAR EnergyProfiling : 1;
#if !defined(_X86_)
                    UCHAR ThreadReservedControlFlags : 2;
#else
                    UCHAR Instrumented : 1;
                    UCHAR ThreadReservedControlFlags : 1;
#endif

                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;

            union {
                UCHAR DebugActive;
#if !defined(_X86_)
                struct {
                    BOOLEAN ActiveDR7 : 1;
                    BOOLEAN Instrumented : 1;
                    BOOLEAN Minimal : 1;
                    BOOLEAN Reserved4 : 3;
                    BOOLEAN UmsScheduled : 1;
                    BOOLEAN UmsPrimary : 1;
                } DUMMYSTRUCTNAME;
#endif
            } DUMMYUNIONNAME2;
        } DUMMYSTRUCTNAME5;

        struct {                         // Mutant
            UCHAR MutantType;
            UCHAR MutantSize;
            BOOLEAN DpcActive;
            UCHAR MutantReserved;
        } DUMMYSTRUCTNAME6;
    } DUMMYUNIONNAME;

    LONG SignalState;                   // Object lock
    LIST_ENTRY WaitListHead;            // Object lock
} DISPATCHER_HEADER, *PDISPATCHER_HEADER;

typedef struct _DISPATCHER_HEADER64 {
    union {
        union {
            volatile LONG Lock;
            LONG LockNV;
        } DUMMYUNIONNAME;

        struct {                            // Events, Semaphores, Gates, etc.
            UCHAR Type;                     // All (accessible via KOBJECT_TYPE)
            UCHAR Signalling;
            UCHAR Size;
            UCHAR Reserved1;
        } DUMMYSTRUCTNAME;

        struct {                            // Timer
            UCHAR TimerType;
            union {
                UCHAR TimerControlFlags;
                struct {
                    UCHAR Absolute : 1;
                    UCHAR Wake : 1;
                    UCHAR EncodedTolerableDelay : TIMER_TOLERABLE_DELAY_BITS;
                } DUMMYSTRUCTNAME;
            };
            UCHAR Hand;
            union {
                UCHAR TimerMiscFlags;
                struct {
                    UCHAR Index : TIMER_EXPIRED_INDEX_BITS;
                    UCHAR Inserted : 1;
                    volatile UCHAR Expired : 1;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;
        } DUMMYSTRUCTNAME2;

        struct {                            // Timer2
            UCHAR Timer2Type;
            union {
                UCHAR Timer2Flags;
                struct {
                    UCHAR Timer2Inserted : 1;
                    UCHAR Timer2Expiring : 1;
                    UCHAR Timer2CancelPending : 1;
                    UCHAR Timer2SetPending : 1;
                    UCHAR Timer2Running : 1;
                    UCHAR Timer2Disabled : 1;
                    UCHAR Timer2ReservedFlags : 2;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;
            UCHAR Timer2Reserved1;
            UCHAR Timer2Reserved2;
        } DUMMYSTRUCTNAME3;

        struct {                            // Queue
            UCHAR QueueType;
            union {
                UCHAR QueueControlFlags;
                struct {
                    UCHAR Abandoned : 1;
                    UCHAR DisableIncrement : 1;
                    UCHAR QueueReservedControlFlags : 6;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;
            UCHAR QueueSize;
            UCHAR QueueReserved;
        } DUMMYSTRUCTNAME4;

        struct {                            // Thread
            UCHAR ThreadType;
            UCHAR ThreadReserved;
            union {
                UCHAR ThreadControlFlags;
                struct {
                    UCHAR CycleProfiling : 1;
                    UCHAR CounterProfiling : 1;
                    UCHAR GroupScheduling : 1;
                    UCHAR AffinitySet : 1;
                    UCHAR Tagged : 1;
                    UCHAR EnergyProfiling : 1;
                    UCHAR ThreadReservedControlFlags : 2;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME;

            union {
                UCHAR DebugActive;
                struct {
                    BOOLEAN ActiveDR7 : 1;
                    BOOLEAN Instrumented : 1;
                    BOOLEAN Minimal : 1;
                    BOOLEAN Reserved4 : 3;
                    BOOLEAN UmsScheduled : 1;
                    BOOLEAN UmsPrimary : 1;
                } DUMMYSTRUCTNAME;
            } DUMMYUNIONNAME2;
        } DUMMYSTRUCTNAME5;

        struct {                         // Mutant
            UCHAR MutantType;
            UCHAR MutantSize;
            BOOLEAN DpcActive;
            UCHAR MutantReserved;
        } DUMMYSTRUCTNAME6;
    } DUMMYUNIONNAME;

    LONG SignalState;                   // Object lock
    LIST_ENTRY64 WaitListHead;          // Object lock
} DISPATCHER_HEADER64, *PDISPATCHER_HEADER64;

//
// Event object
//

typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *PRKEVENT;

typedef struct _KEVENT64 {
    DISPATCHER_HEADER64 Header;
} KEVENT64, *PKEVENT64, *PRKEVENT64;


//
// Gate object
//
// N.B. Gate object services allow the specification of synchronization
//      events. This allows fast mutex to be transparently replaced with
//      gates.
//
typedef struct _KGATE {
    DISPATCHER_HEADER Header;
} KGATE, *PKGATE;

typedef struct _KGATE64 {
    DISPATCHER_HEADER64 Header;
} KGATE64, *PKGATE64;

//
// Timer object
//
// N.B. The period field must be the last member of this structure.
//
typedef struct _KTIMER {
    DISPATCHER_HEADER Header;
    ULARGE_INTEGER DueTime;
    LIST_ENTRY TimerListEntry;
    struct _KDPC *Dpc;
#if !defined(KENCODED_TIMER_PROCESSOR)
    ULONG Processor;
#endif
    ULONG Period;
} KTIMER, *PKTIMER, *PRKTIMER;
#define KTIMER_ACTUAL_LENGTH (FIELD_OFFSET(KTIMER, Period) + sizeof(LONG))

typedef struct _KTIMER64 {
    DISPATCHER_HEADER64 Header;
    ULARGE_INTEGER DueTime;
    LIST_ENTRY64 TimerListEntry;
    ULONGLONG Dpc; // PKDPC
    ULONG Processor;
    ULONG Period;
} KTIMER64, *PKTIMER64, *PRKTIMER64;
#define KTIMER_ACTUAL_LENGTH64 (FIELD_OFFSET64(KTIMER64, Period) + sizeof(LONG))

typedef struct _FAST_MUTEX {
    LONG Count;
    PVOID Owner;
    ULONG Contention;
    KEVENT Event;
    ULONG OldIrql;
} FAST_MUTEX, *PFAST_MUTEX, KGUARDED_MUTEX, *PKGUARDED_MUTEX;

typedef struct _FAST_MUTEX64 {
    LONG Count;
    ULONGLONG Owner; // PVOID
    ULONG Contention;
    KEVENT64 Event;
    ULONG OldIrql;
} FAST_MUTEX64, *PFAST_MUTEX64, KGUARDED_MUTEX64, *PKGUARDED_MUTEX64;

//
// Define system time structure.
//
typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;


//
// Define Global Descriptor Table (GDT) entry structure and constants.
//
// Define descriptor type codes.
//
#define TYPE_LDT    0x02                // 00010 = system ldt
#define TYPE_CODE   0x1B                // 11011 = code, read only, accessed
#define TYPE_DATA   0x13                // 10011 = data, read and write, accessed
#define TYPE_TSS64  0x09                // 01001 = task state segment

//
// Define descriptor privilege levels for user and system.
//
#define DPL_USER 3
#define DPL_SYSTEM 0

//
// Define limit granularity.
//
#define GRANULARITY_BYTE 0
#define GRANULARITY_PAGE 1

//
// Define processor number packing constants.
//
// The compatibility processor number is encoded in the FS segment descriptor.
//
// Bits 19:14 of the segment limit encode the compatible processor number.
// Bits 13:10 are set to ones to ensure that segment limit is at least 15360.
// Bits 9:0 of the segment limit encode the extended processor number.
//
#define KGDT_LEGACY_LIMIT_SHIFT 14
#define KGDT_LIMIT_ENCODE_MASK (0xf << 10)
#define SELECTOR_TABLE_INDEX 0x04

typedef union _KGDTENTRY64 {
    struct {
        USHORT LimitLow;
        USHORT BaseLow;
        union {
            struct {
                UCHAR BaseMiddle;
                UCHAR Flags1;
                UCHAR Flags2;
                UCHAR BaseHigh;
            } Bytes;
            struct {
                ULONG BaseMiddle : 8;
                ULONG Type : 5;
                ULONG Dpl : 2;
                ULONG Present : 1;
                ULONG LimitHigh : 4;
                ULONG System : 1;
                ULONG LongMode : 1;
                ULONG DefaultBig : 1;
                ULONG Granularity : 1;
                ULONG BaseHigh : 8;
            } Bits;
        };
        ULONG BaseUpper;
        ULONG MustBeZero;
    };
    struct {
        LONG64 DataLow;
        LONG64 DataHigh;
    };
} KGDTENTRY64, *PKGDTENTRY64;

//
// Define Local Descriptor Table (LDT) entry structure and constants.
//
typedef struct _KLDTENTRY {
    USHORT LimitLow;
    USHORT BaseLow;
    union {
        struct {
            UCHAR BaseMid;
            UCHAR Flags1;
            UCHAR Flags2;
            UCHAR BaseHi;
        } Bytes;
        struct {
            ULONG BaseMid : 8;
            ULONG Type : 5;
            ULONG Dpl : 2;
            ULONG Pres : 1;
            ULONG LimitHi : 4;
            ULONG Sys : 1;
            ULONG Reserved_0 : 1;
            ULONG Default_Big : 1;
            ULONG Granularity : 1;
            ULONG BaseHi : 8;
        } Bits;
    } HighWord;
} KLDTENTRY, *PKLDTENTRY;
C_ASSERT( (sizeof( KLDTENTRY ) % sizeof( PVOID )) == 0 );
#define KLDT_MAX_LENGTH (8192)
#define KLDT_MAX_SIZE (sizeof(KLDTENTRY) * KLDT_MAX_LENGTH)

//
// Define Interrupt Descriptor Table (IDT) entry structure and constants.
//
typedef union _KIDTENTRY64 {
    struct {
        USHORT OffsetLow;
        USHORT Selector;
        USHORT IstIndex : 3;
        USHORT Reserved0 : 5;
        USHORT Type : 5;
        USHORT Dpl : 2;
        USHORT Present : 1;
        USHORT OffsetMiddle;
        ULONG OffsetHigh;
        ULONG Reserved1;
    };
    ULONG64 Alignment;
} KIDTENTRY64, *PKIDTENTRY64;

//
// Define two union definitions used for parsing addresses into the
// component fields required by a GDT.
//
typedef union _KGDT_BASE {
    struct {
        USHORT BaseLow;
        UCHAR BaseMiddle;
        UCHAR BaseHigh;
        ULONG BaseUpper;
    };
    ULONG64 Base;
} KGDT_BASE, *PKGDT_BASE;
C_ASSERT( sizeof( KGDT_BASE ) == sizeof( ULONG64 ) );

typedef union _KGDT_LIMIT {
    struct {
        USHORT LimitLow;
        USHORT LimitHigh : 4;
        USHORT MustBeZero : 12;
    };
    ULONG Limit;
} KGDT_LIMIT, *PKGDT_LIMIT;
C_ASSERT( sizeof( KGDT_LIMIT ) == sizeof( ULONG ) );

//
// Define Task State Segment (TSS) structure and constants.
//
// Task switches are not supported by the AMD64, but a task state segment
// must be present to define the kernel stack pointer and I/O map base.
//
// N.B. This structure is misaligned as per the AMD64 specification.
//
// N.B. The size of TSS must be <= 0xDFFF.
//
#if defined(_MSC_VER)
#pragma pack(push, 4)
#endif
typedef struct _KTSS64 {
    ULONG Reserved0;
    ULONG64 Rsp0;
    ULONG64 Rsp1;
    ULONG64 Rsp2;
    //
    // Element 0 of the Ist is reserved.
    //
    ULONG64 Ist[8];
    ULONG64 Reserved1;
    USHORT Reserved2;
    USHORT IoMapBase;
} KTSS64, *PKTSS64;
#if defined(_MSC_VER)
#pragma pack(pop)
#endif
C_ASSERT( (sizeof( KTSS64 ) % sizeof( PVOID )) == 0 );

#define TSS_IST_RESERVED 0
#define TSS_IST_PANIC 1
#define TSS_IST_MCA 2
#define TSS_IST_NMI 3

#define IO_ACCESS_MAP_NONE FALSE

//
// Define pseudo descriptor structures for both 64- and 32-bit mode.
//
typedef struct _KDESCRIPTOR {
    USHORT Pad[3];
    USHORT Limit;
    PVOID Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef struct _KDESCRIPTOR32 {
    USHORT Pad[3];
    USHORT Limit;
    ULONG Base;
} KDESCRIPTOR32, *PKDESCRIPTOR32;


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus