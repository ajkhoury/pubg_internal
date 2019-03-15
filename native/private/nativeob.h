#pragma once

#include "nativecommon.h"
#include "nativepool.h"
#include "nativeex.h"
#include "nativemm.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// < Objects Macros and Definitions >
///

#define OBJECT_TYPE_CREATE              0x0001
#define OBJECT_TYPE_ALL_ACCESS          (STANDARD_RIGHTS_REQUIRED | 0x1)

#define DIRECTORY_QUERY                 0x0001
#define DIRECTORY_TRAVERSE              0x0002
#define DIRECTORY_CREATE_OBJECT         0x0004
#define DIRECTORY_CREATE_SUBDIRECTORY   0x0008
#define DIRECTORY_ALL_ACCESS            (STANDARD_RIGHTS_REQUIRED | 0xF)

#define SYMBOLIC_LINK_QUERY             0x0001
#define SYMBOLIC_LINK_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | 0x1)

#define DUPLICATE_CLOSE_SOURCE          0x00000001
#define DUPLICATE_SAME_ACCESS           0x00000002
#define DUPLICATE_SAME_ATTRIBUTES       0x00000004

// Object attribute defines
#define OBJ_PROTECT_CLOSE               0x00000001L
#define OBJ_INHERIT                     0x00000002L
#define OBJ_AUDIT_OBJECT_CLOSE          0x00000004L
#define OBJ_PERMANENT                   0x00000010L
#define OBJ_EXCLUSIVE                   0x00000020L
#define OBJ_CASE_INSENSITIVE            0x00000040L
#define OBJ_OPENIF                      0x00000080L
#define OBJ_OPENLINK                    0x00000100L
#define OBJ_KERNEL_HANDLE               0x00000200L
#define OBJ_FORCE_ACCESS_CHECK          0x00000400L
#define OBJ_VALID_ATTRIBUTES            0x000007F2L
//#define OBJ_VALID_ATTRIBUTES_KRNL     0x00001FF2L

#define InitializeObjectAttributes(p,n,a,r,s) { \
    (p)->Length = sizeof(struct _OBJECT_ATTRIBUTES); \
    (p)->RootDirectory = r; \
    (p)->Attributes = a; \
    (p)->ObjectName = n; \
    (p)->SecurityDescriptor = s; \
    (p)->SecurityQualityOfService = NULL; \
    }

#define InitializeObjectAttributes64(p,n,a,r,s) { \
	(p)->Length = sizeof(struct _OBJECT_ATTRIBUTES64); \
	(p)->RootDirectory = r; \
	(p)->Attributes = a; \
	(p)->ObjectName = n; \
	(p)->SecurityDescriptor = s; \
	(p)->SecurityQualityOfService = NULL; \
    }

#define InitializeObjectAttributes32(p,n,a,r,s) { \
	(p)->Length = sizeof(struct _OBJECT_ATTRIBUTES32); \
	(p)->RootDirectory = r; \
	(p)->Attributes = a; \
	(p)->ObjectName = n; \
	(p)->SecurityDescriptor = s; \
	(p)->SecurityQualityOfService = NULL; \
    }


///
/// < Objects Enums >
///

enum _KOBJECTS {
    EventNotificationObject = 0x0,
    EventSynchronizationObject = 0x1,
    MutantObject = 0x2,
    ProcessObject = 0x3,
    QueueObject = 0x4,
    SemaphoreObject = 0x5,
    ThreadObject = 0x6,
    GateObject = 0x7,
    TimerNotificationObject = 0x8,
    TimerSynchronizationObject = 0x9,
    Spare2Object = 0xA,
    Spare3Object = 0xB,
    Spare4Object = 0xC,
    Spare5Object = 0xD,
    Spare6Object = 0xE,
    Spare7Object = 0xF,
    Spare8Object = 0x10,
    Spare9Object = 0x11,
    ApcObject = 0x12,
    DpcObject = 0x13,
    DeviceQueueObject = 0x14,
    EventPairObject = 0x15,
    InterruptObject = 0x16,
    ProfileObject = 0x17,
    ThreadedDpcObject = 0x18,
    MaximumKernelObject = 0x19,
};


///
/// < Object Structures & Types >
///


//
// Object Type Structure
//

typedef struct _OBJECT_TYPE_INITIALIZER {
    USHORT Length;                              // 0x00
    union {
        USHORT ObjectTypeFlags;                 // 0x02
        struct {
            USHORT CaseInsensitive : 1;         // 0x02
            USHORT UnnamedObjectsOnly : 1;      // 0x02
            USHORT UseDefaultObject : 1;        // 0x02
            USHORT SecurityRequired : 1;        // 0x02
            USHORT MaintainHandleCount : 1;     // 0x02
            USHORT MaintainTypeList : 1;        // 0x02
            USHORT SupportsObjectCallbacks : 1; // 0x02
            USHORT CacheAligned : 1;            // 0x02
            USHORT UseExtendedParameters : 1;   // 0x02
            USHORT Reserved : 7;                // 0x02
        };
    };
    ULONG ObjectTypeCode;                       // 0x04
    ULONG InvalidAttributes;                    // 0x08
    GENERIC_MAPPING GenericMapping;             // 0x0C
    ULONG ValidAccessMask;                      // 0x1C
    ULONG RetainAccess;                         // 0x20
    POOL_TYPE PoolType;                         // 0x24
    ULONG DefaultPagedPoolCharge;               // 0x28
    ULONG DefaultNonPagedPoolCharge;            // 0x2C
    PVOID DumpProcedure;                        // 0x30
    PVOID OpenProcedure;                        // 0x38
    PVOID CloseProcedure;                       // 0x40
    PVOID DeleteProcedure;                      // 0x48
    union {
        PVOID ParseProcedure;                   // 0x50
        PVOID ParseProcedureEx;                 // 0x50
    };
    PVOID SecurityProcedure;                    // 0x58
    PVOID QueryNameProcedure;                   // 0x60
    PVOID OkayToCloseProcedure;                 // 0x68
    ULONG WaitObjectFlagMask;                   // 0x70
    USHORT WaitObjectFlagOffset;                // 0x74
    USHORT WaitObjectPointerOffset;             // 0x76
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

typedef struct _OBJECT_TYPE_INITIALIZER64 {
    USHORT Length;                              // 0x00
    union {
        USHORT ObjectTypeFlags;                 // 0x02
        struct {
            USHORT CaseInsensitive : 1;         // 0x02
            USHORT UnnamedObjectsOnly : 1;      // 0x02
            USHORT UseDefaultObject : 1;        // 0x02
            USHORT SecurityRequired : 1;        // 0x02
            USHORT MaintainHandleCount : 1;     // 0x02
            USHORT MaintainTypeList : 1;        // 0x02
            USHORT SupportsObjectCallbacks : 1; // 0x02
            USHORT CacheAligned : 1;            // 0x02
            USHORT UseExtendedParameters : 1;   // 0x02
            USHORT Reserved : 7;                // 0x02
        };
    };
    ULONG ObjectTypeCode;                       // 0x04
    ULONG InvalidAttributes;                    // 0x08
    GENERIC_MAPPING GenericMapping;             // 0x0C
    ULONG ValidAccessMask;                      // 0x1C
    ULONG RetainAccess;                         // 0x20
    POOL_TYPE PoolType;                         // 0x24
    ULONG DefaultPagedPoolCharge;               // 0x28
    ULONG DefaultNonPagedPoolCharge;            // 0x2C
    ULONGLONG DumpProcedure;                    // 0x30 PVOID
    ULONGLONG OpenProcedure;                    // 0x38 PVOID
    ULONGLONG CloseProcedure;                   // 0x40 PVOID
    ULONGLONG DeleteProcedure;                  // 0x48 PVOID
    union {
        ULONGLONG ParseProcedure;               // 0x50 PVOID
        ULONGLONG ParseProcedureEx;             // 0x50 PVOID
    };
    ULONGLONG SecurityProcedure;                // 0x58 PVOID
    ULONGLONG QueryNameProcedure;               // 0x60 PVOID
    ULONGLONG OkayToCloseProcedure;             // 0x68 PVOID
    ULONG WaitObjectFlagMask;                   // 0x70
    USHORT WaitObjectFlagOffset;                // 0x74
    USHORT WaitObjectPointerOffset;             // 0x76
} OBJECT_TYPE_INITIALIZER64, *POBJECT_TYPE_INITIALIZER64;

typedef struct _OBJECT_TYPE {
    LIST_ENTRY TypeList;                        // 0x00
    UNICODE_STRING Name;                        // 0x10
    PVOID DefaultObject;                        // 0x20
    UCHAR Index;                                // 0x28
    ULONG TotalNumberOfObjects;                 // 0x2C
    ULONG TotalNumberOfHandles;                 // 0x30
    ULONG HighWaterNumberOfObjects;             // 0x34
    ULONG HighWaterNumberOfHandles;             // 0x38
    OBJECT_TYPE_INITIALIZER TypeInfo;           // 0x40
    EX_PUSH_LOCK TypeLock;                      // 0xB8
    ULONG Key;                                  // 0xC0
    LIST_ENTRY CallbackList;                    // 0xC8
} OBJECT_TYPE, *POBJECT_TYPE;

typedef struct _OBJECT_TYPE64 {
    LIST_ENTRY64 TypeList;                      // 0x00
    UNICODE_STRING64 Name;                      // 0x10
    ULONGLONG DefaultObject;                    // 0x20 PVOID
    UCHAR Index;                                // 0x28
    CHAR Reserved0[3];                          // 0x29
    ULONG TotalNumberOfObjects;                 // 0x2C
    ULONG TotalNumberOfHandles;                 // 0x30
    ULONG HighWaterNumberOfObjects;             // 0x34
    ULONG HighWaterNumberOfHandles;             // 0x38
    OBJECT_TYPE_INITIALIZER TypeInfo;           // 0x40
    EX_PUSH_LOCK64 TypeLock;                    // 0xB8
    ULONG Key;                                  // 0xC0
    ULONG Reserved1;                            // 0xC4
    LIST_ENTRY64 CallbackList;                  // 0xC8
} OBJECT_TYPE64, *POBJECT_TYPE64;

//
// Object Header Structure
//
// The SecurityQuotaCharged is the amount of quota charged to cover
// the GROUP and DISCRETIONARY ACL fields of the security descriptor
// only. The OWNER and SYSTEM ACL fields get charged for at a fixed
// rate that may be less than or greater than the amount actually used.
//
// If the object has no security, then the SecurityQuotaCharged and the
// SecurityQuotaInUse fields are set to zero.
//
// Modification of the OWNER and SYSTEM ACL fields should never fail
// due to quota exceeded problems.  Modifications to the GROUP and
// DISCRETIONARY ACL fields may fail due to quota exceeded problems.
//

typedef struct _OBJECT_CREATE_INFORMATION {
    ULONG Attributes;                                   // 0x00
    PVOID RootDirectory;                                // 0x08
    KPROCESSOR_MODE ProbeMode;                          // 0x10
    ULONG PagedPoolCharge;                              // 0x14
    ULONG NonPagedPoolCharge;                           // 0x18
    ULONG SecurityDescriptorCharge;                     // 0x1C
    PSECURITY_DESCRIPTOR SecurityDescriptor;            // 0x20
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;           // 0x28
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService; // 0x30
} OBJECT_CREATE_INFORMATION, *POBJECT_CREATE_INFORMATION;

typedef struct _OBJECT_HEADER {
    LONG_PTR PointerCount;                              // 0x00
    union {
        LONG_PTR HandleCount;                           // 0x08
        PVOID NextToFree;                               // 0x08
    };
    EX_PUSH_LOCK Lock;                                  // 0x10
    UCHAR TypeIndex;                                    // 0x18
    struct {
        UCHAR TraceFlags;                               // 0x19
        union {
            UCHAR DbgRefTrace : 1;                      // 0x19
            UCHAR DbgTracePermanent : 1;                // 0x19
        };
    };
    UCHAR InfoMask;                                     // 0x1A
    struct {
        UCHAR Flags;                                    // 0x1B
        union {
            UCHAR NewObject : 1;                        // 0x1B
            UCHAR KernelObject : 1;                     // 0x1B
            UCHAR KernelOnlyAccess : 1;                 // 0x1B
            UCHAR ExclusiveObject : 1;                  // 0x1B
            UCHAR PermanentObject : 1;                  // 0x1B
            UCHAR DefaultSecurityQuota : 1;             // 0x1B
            UCHAR SingleHandleEntry : 1;                // 0x1B
            UCHAR DeletedInline : 1;                    // 0x1B
        };
    };
    ULONG Reserved;                                     // 0x1C   
    union {
        POBJECT_CREATE_INFORMATION ObjectCreateInfo;    // 0x20
        PVOID QuotaBlockCharged;                        // 0x20
    };
    PSECURITY_DESCRIPTOR SecurityDescriptor;            // 0x28
    QUAD Body;                                          // 0x30
} OBJECT_HEADER, *POBJECT_HEADER;

typedef struct _OBJECT_HEADER64 {
    LONGLONG PointerCount;                              // 0x00 LONG_PTR
    union {
        LONGLONG HandleCount;                           // 0x08 LONG_PTR
        ULONGLONG NextToFree;                           // 0x08 PVOID;
    };
    EX_PUSH_LOCK64 Lock;                                // 0x10
    UCHAR TypeIndex;                                    // 0x18
    struct {
        UCHAR TraceFlags;                               // 0x19
        union {
            UCHAR DbgRefTrace : 1;                      // 0x19
            UCHAR DbgTracePermanent : 1;                // 0x19
        };
    };
    UCHAR InfoMask;                                     // 0x1A
    struct {
        UCHAR Flags;                                    // 0x1B
        union {
            UCHAR NewObject : 1;                        // 0x1B
            UCHAR KernelObject : 1;                     // 0x1B
            UCHAR KernelOnlyAccess : 1;                 // 0x1B
            UCHAR ExclusiveObject : 1;                  // 0x1B
            UCHAR PermanentObject : 1;                  // 0x1B
            UCHAR DefaultSecurityQuota : 1;             // 0x1B
            UCHAR SingleHandleEntry : 1;                // 0x1B
            UCHAR DeletedInline : 1;                    // 0x1B
        };
    };
    ULONG Reserved;                                     // 0x1C   
    union {
        ULONGLONG ObjectCreateInfo;                     // 0x20 POBJECT_CREATE_INFORMATION
        ULONGLONG QuotaBlockCharged;                    // 0x20 PVOID
    };
    ULONGLONG SecurityDescriptor;                       // 0x28 PSECURITY_DESCRIPTOR
    QUAD Body;                                          // 0x30
} OBJECT_HEADER64, *POBJECT_HEADER64;


///
/// < Objects Routines >
///

typedef
NTSTATUS
(NTAPI *PNT_DUPLICATE_OBJECT)(
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options
    );
NTSYSAPI
NTSTATUS
NTAPI
NtDuplicateObject(
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options
    );

typedef
NTSTATUS
(NTAPI *PNT_MAKE_TEMPORARY_OBJECT)(
    IN HANDLE Handle
    );
NTSYSAPI
NTSTATUS
NTAPI
NtMakeTemporaryObject(
    IN HANDLE Handle
    );

typedef
NTSTATUS
(NTAPI *PNT_MAKE_PERMANENT_OBJECT)(
    IN HANDLE Handle
    );
NTSYSAPI
NTSTATUS
NTAPI
NtMakePermanentObject(
    IN HANDLE Handle
    );

typedef
NTSTATUS
(NTAPI *PNT_SIGNAL_AND_WAIT_FOR_SINGLE_OBJECT)(
    IN HANDLE SignalHandle,
    IN HANDLE WaitHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtSignalAndWaitForSingleObject(
    IN HANDLE SignalHandle,
    IN HANDLE WaitHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_WAIT_FOR_SINGLE_OBJECT)(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_WAIT_FOR_MULTIPLE_OBJECTS)(
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtWaitForMultipleObjects(
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

#if (NTDDI_VERSION >= NTDDI_WS03)
typedef
NTSTATUS
(NTAPI *PNT_WAIT_FOR_MULTIPLE_OBJECTS32)(
    IN ULONG Count,
    IN LONG Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtWaitForMultipleObjects32(
    IN ULONG Count,
    IN LONG Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
#endif

typedef
NTSTATUS
(NTAPI *PNT_SET_SECURITY_OBJECT)(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );
NTSYSAPI
NTSTATUS
NTAPI
NtSetSecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

typedef
NTSTATUS
(NTAPI *PNT_QUERY_SECURITY_OBJECT)(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );
NTSYSAPI
NTSTATUS
NTAPI
NtQuerySecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );

typedef
NTSTATUS
(NTAPI *PNT_CLOSE)(
    IN HANDLE Handle
    );
NTSYSAPI
NTSTATUS
NTAPI
NtClose(
    IN HANDLE Handle
    );

#if (NTDDI_VERSION >= NTDDI_WIN10)
typedef
NTSTATUS
(NTAPI *PNT_COMPARE_OBJECTS)(
    IN HANDLE FirstObjectHandle,
    IN HANDLE SecondObjectHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCompareObjects(
    IN HANDLE FirstObjectHandle,
    IN HANDLE SecondObjectHandle
    );
#endif

typedef
NTSTATUS
(NTAPI *PNT_CREATE_SYMBOLIC_LINK_OBJECT)(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCreateSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    );

typedef
NTSTATUS
(NTAPI *PNT_OPEN_SYMBOLIC_LINK_OBJECT)(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef
NTSTATUS
(NTAPI *PNT_QUERY_SYMBOLIC_LINK_OBJECT)(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtQuerySymbolicLinkObject(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    );

#ifdef __cplusplus
} // extern "C"
#endif