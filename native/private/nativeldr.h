#pragma once

#include "nativecommon.h"
#include "nativertl.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < Loader Definitions >
///

#define LDR_DLL_NOTIFICATION_REASON_LOADED   1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2


///
/// < Loader Enums >
///

typedef enum _LDR_DLL_LOAD_REASON {
    LoadReasonStaticDependency = 0,
    LoadReasonStaticForwarderDependency = 1,
    LoadReasonDynamicForwarderDependency = 2,
    LoadReasonDelayloadDependency = 3,
    LoadReasonDynamicLoad = 4,
    LoadReasonAsImageLoad = 5,
    LoadReasonAsDataLoad = 6,
    LoadReasonUnknown = -1
} LDR_DLL_LOAD_REASON;


///
/// < Loader Structures >
///

typedef struct _LDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;              // 0x00
    LIST_ENTRY32 InMemoryOrderLinks;            // 0x08
    union {
        LIST_ENTRY32 InInitializationOrderLinks;// 0x10
        LIST_ENTRY32 InProgressLinks;           // 0x10
    };
    ULONG DllBase;                              // 0x18 Ptr PVOID
    ULONG EntryPoint;                           // 0x1C Ptr PVOID
    ULONG SizeOfImage;                          // 0x20
    UNICODE_STRING32 FullDllName;               // 0x24
    UNICODE_STRING32 BaseDllName;               // 0x2C
    union {
        UCHAR FlagGroup[4];                     // 0x34
        ULONG Flags;                            // 0x34
        struct {
            ULONG PackagedBinary : 1;
            ULONG MarkedForRemoval : 1;
            ULONG ImageDll : 1;
            ULONG LoadNotificationsSent : 1;
            ULONG TelemetryEntryProcessed : 1;
            ULONG ProcessStaticImport : 1;
            ULONG InLegacyLists : 1;
            ULONG InIndexes : 1;
            ULONG ShimDll : 1;
            ULONG InExceptionTable : 1;
            ULONG ReservedFlags1 : 2;
            ULONG LoadInProgress : 1;
            ULONG LoadConfigProcessed : 1;
            ULONG EntryProcessed : 1;
            ULONG ProtectDelayLoad : 1;
            ULONG ReservedFlags3 : 2;
            ULONG DontCallForThreads : 1;
            ULONG ProcessAttachCalled : 1;
            ULONG ProcessAttachFailed : 1;
            ULONG CorDeferredValidate : 1;
            ULONG CorImage : 1;
            ULONG DontRelocate : 1;
            ULONG CorILOnly : 1;
            ULONG ReservedFlags5 : 3;
            ULONG Redirected : 1;
            ULONG ReservedFlags6 : 2;
            ULONG CompatDatabaseProcessed : 1;
        };
    };
    USHORT ObsoleteLoadCount;                   // 0x38
    USHORT TlsIndex;                            // 0x3A
    LIST_ENTRY32 HashLinks;                     // 0x3C
    ULONG TimeDateStamp;                        // 0x44
    ULONG EntryPointActivationContext;          // 0x48 Ptr struct _ACTIVATION_CONTEXT *
    ULONG Lock;                                 // 0x4C Ptr PVOID
    ULONG DdagNode;                             // 0x50 Ptr PLDR_DDAG_NODE
    LIST_ENTRY32 NodeModuleLink;                // 0x54
    ULONG LoadContext;                          // 0x5C Ptr struct _LDRP_LOAD_CONTEXT *
    ULONG ParentDllBase;                        // 0x60 Ptr PVOID
    ULONG SwitchBackContext;                    // 0x64 Ptr PVOID
    RTL_BALANCED_NODE32 BaseAddressIndexNode;   // 0x68
    RTL_BALANCED_NODE32 MappingInfoIndexNode;   // 0x74
    ULONG OriginalBase;                         // 0x80 ULONG_PTR
    LARGE_INTEGER LoadTime;                     // 0x84
    ULONG BaseNameHashValue;                    // 0x8C
    LDR_DLL_LOAD_REASON LoadReason;             // 0x90
    ULONG ImplicitPathOptions;                  // 0x94
    ULONG ReferenceCount;                       // 0x98
} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

typedef struct _LDR_DATA_TABLE_ENTRY64 {
    LIST_ENTRY64 InLoadOrderLinks;              // 0x00
    LIST_ENTRY64 InMemoryOrderLinks;            // 0x10
    LIST_ENTRY64 InInitializationOrderLinks;    // 0x20
    ULONG64 DllBase;                            // 0x30 PVOID
    ULONG64 EntryPoint;                         // 0x38 PVOID
    ULONG SizeOfImage;                          // 0x40
    UNICODE_STRING64 FullDllName;               // 0x48
    UNICODE_STRING64 BaseDllName;               // 0x58
    union {
        UCHAR FlagGroup[4];                     // 0x68
        ULONG Flags;                            // 0x68
        struct {
            ULONG PackagedBinary : 1;
            ULONG MarkedForRemoval : 1;
            ULONG ImageDll : 1;
            ULONG LoadNotificationsSent : 1;
            ULONG TelemetryEntryProcessed : 1;
            ULONG ProcessStaticImport : 1;
            ULONG InLegacyLists : 1;
            ULONG InIndexes : 1;
            ULONG ShimDll : 1;
            ULONG InExceptionTable : 1;
            ULONG ReservedFlags1 : 2;
            ULONG LoadInProgress : 1;
            ULONG LoadConfigProcessed : 1;
            ULONG EntryProcessed : 1;
            ULONG ProtectDelayLoad : 1;
            ULONG ReservedFlags3 : 2;
            ULONG DontCallForThreads : 1;
            ULONG ProcessAttachCalled : 1;
            ULONG ProcessAttachFailed : 1;
            ULONG CorDeferredValidate : 1;
            ULONG CorImage : 1;
            ULONG DontRelocate : 1;
            ULONG CorILOnly : 1;
            ULONG ReservedFlags5 : 3;
            ULONG Redirected : 1;
            ULONG ReservedFlags6 : 2;
            ULONG CompatDatabaseProcessed : 1;
        };
    };
    USHORT ObsoleteLoadCount;                   // 0x6C
    USHORT TlsIndex;                            // 0x6E
    LIST_ENTRY64 HashLinks;                     // 0x70
    ULONG TimeDateStamp;                        // 0x80
    ULONG64 EntryPointActivationContext;        // 0x88 _ACTIVATION_CONTEXT*
    PVOID Lock;                                 // 0x90
    ULONG64 DdagNode;                           // 0x98 _LDR_DDAG_NODE*
    LIST_ENTRY64 NodeModuleLink;                // 0xA0
    ULONG64 LoadContext;                        // 0xB0 _LDRP_LOAD_CONTEXT*
    PVOID ParentDllBase;                        // 0xB8
    PVOID SwitchBackContext;                    // 0xC0
    RTL_BALANCED_NODE64 BaseAddressIndexNode;   // 0xC8
    RTL_BALANCED_NODE64 MappingInfoIndexNode;   // 0xE0
    ULONG64 OriginalBase;                       // 0xF8
    LARGE_INTEGER LoadTime;                     // 0x100
    ULONG BaseNameHashValue;                    // 0x108
    LDR_DLL_LOAD_REASON LoadReason;             // 0x10C
    ULONG ImplicitPathOptions;                  // 0x110
    ULONG ReferenceCount;                       // 0x114
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;                // 0x00 0x00
    LIST_ENTRY InMemoryOrderLinks;              // 0x10 0x08
    LIST_ENTRY InInitializationOrderLinks;      // 0x20 0x10
    PVOID DllBase;                              // 0x30 0x18
    PVOID EntryPoint;                           // 0x38 0x1C
    ULONG SizeOfImage;                          // 0x40 0x20
    UNICODE_STRING FullDllName;                 // 0x48 0x24
    UNICODE_STRING BaseDllName;                 // 0x58 0x2C 
    union {
        UCHAR FlagGroup[4];                     // 0x68 0x34
        ULONG Flags;                            // 0x68 0x34
        struct {
            ULONG PackagedBinary : 1;
            ULONG MarkedForRemoval : 1;
            ULONG ImageDll : 1;
            ULONG LoadNotificationsSent : 1;
            ULONG TelemetryEntryProcessed : 1;
            ULONG ProcessStaticImport : 1;
            ULONG InLegacyLists : 1;
            ULONG InIndexes : 1;
            ULONG ShimDll : 1;
            ULONG InExceptionTable : 1;
            ULONG ReservedFlags1 : 2;
            ULONG LoadInProgress : 1;
            ULONG LoadConfigProcessed : 1;
            ULONG EntryProcessed : 1;
            ULONG ProtectDelayLoad : 1;
            ULONG ReservedFlags3 : 2;
            ULONG DontCallForThreads : 1;
            ULONG ProcessAttachCalled : 1;
            ULONG ProcessAttachFailed : 1;
            ULONG CorDeferredValidate : 1;
            ULONG CorImage : 1;
            ULONG DontRelocate : 1;
            ULONG CorILOnly : 1;
            ULONG ReservedFlags5 : 3;
            ULONG Redirected : 1;
            ULONG ReservedFlags6 : 2;
            ULONG CompatDatabaseProcessed : 1;
        };
    };
    USHORT ObsoleteLoadCount;               // 0x6C 0x38
    USHORT TlsIndex;                        // 0x6E 0x3A
    LIST_ENTRY HashLinks;                   // 0x70 0x3C
    ULONG TimeDateStamp;                    // 0x80 0x44
    PVOID EntryPointActivationContext;      // 0x88 0x48
    PVOID Lock;                             // 0x90 0x4C
    PVOID DdagNode;                         // 0x98 0x50
    LIST_ENTRY NodeModuleLink;              // 0xA0 0x54
    PVOID LoadContext;                      // 0xB0 0x5C
    PVOID ParentDllBase;                    // 0xB8 0x60
    PVOID SwitchBackContext;                // 0xC0 0x64
    RTL_BALANCED_NODE BaseAddressIndexNode; // 0xC8 0x68
    RTL_BALANCED_NODE MappingInfoIndexNode; // 0xE0 0x74
    ULONG64 OriginalBase;                   // 0xF8 0x80
    LARGE_INTEGER LoadTime;                 // 0x100 0x84
    ULONG BaseNameHashValue;                // 0x108 0x8C
    LDR_DLL_LOAD_REASON LoadReason;         // 0x10C 0x90
    ULONG ImplicitPathOptions;              // 0x110 0x94
    ULONG ReferenceCount;                   // 0x114 0x98
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
typedef LDR_DATA_TABLE_ENTRY *PCLDR_DATA_TABLE_ENTRY;

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
    struct _LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;
typedef const LDR_DLL_NOTIFICATION_DATA *PCLDR_DLL_NOTIFICATION_DATA;


/// 
/// < Loader Routines >
/// 

typedef
VOID
(NTAPI *PLDR_LOADED_MODULE_ENUMERATION_CALLBACK_FUNCTION)(
    IN PLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN PVOID Context,
    IN OUT BOOLEAN* StopEnumeration
    );

typedef
VOID
(CALLBACK *PLDR_DLL_NOTIFICATION_FUNCTION)(
    IN ULONG NotificationReason,
    IN PLDR_DLL_NOTIFICATION_DATA NotificationData,
    IN PVOID Context OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *tLdrAccessResource)(
    IN PVOID DllHandle,
    IN CONST IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *tLdrAddRefDll)(
    IN ULONG Flags,
    IN PVOID DllHandle
    );

typedef
NTSTATUS
(NTAPI *tLdrEnumerateLoadedModules)(
    IN ULONG Flags OPTIONAL,
    IN PLDR_LOADED_MODULE_ENUMERATION_CALLBACK_FUNCTION CallbackFunction,
    IN PVOID Context OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *tLdrFindResource_U)(
    IN PVOID DllHandle,
    IN CONST ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );

typedef
NTSTATUS
(NTAPI *tLdrFindEntryForAddress)(
    IN PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *TableEntry
    );

typedef
NTSTATUS
(NTAPI *tLdrGetDllHandle)(
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

typedef
NTSTATUS
(NTAPI *tLdrGetProcedureAddress)(
    IN PVOID DllHandle,
    IN CONST ANSI_STRING* ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID* ProcedureAddress
    );

typedef
NTSTATUS
(NTAPI *tLdrLoadDll)(
    IN ULONG DllFlags OPTIONAL, // Has changed since Windows 10
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

typedef
NTSTATUS
(NTAPI *tLdrQueryProcessModuleInformation)(
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *tLdrUnloadDll)(
    IN PVOID DllHandle
    );

typedef
NTSTATUS
(NTAPI *tLdrRegisterDllNotification)(
    IN ULONG Flags,
    IN PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    IN PVOID Context OPTIONAL,
    OUT PVOID* Cookie
    );

typedef
NTSTATUS
(NTAPI *tLdrUnregisterDllNotification)(
    IN PVOID Cookie
    );

typedef
NTSTATUS
(NTAPI *PLDR_DISABLE_THREAD_CALLOUTS_FOR_DLL)(
    IN PVOID BaseAddress
    );
NTSYSAPI
NTSTATUS
NTAPI
LdrDisableThreadCalloutsForDll(
    IN PVOID BaseAddress
    );

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus