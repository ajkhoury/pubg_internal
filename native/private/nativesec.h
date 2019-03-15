#pragma once

#include "nativecommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < Security Defines >
///

// Privileges
#define SE_MIN_WELL_KNOWN_PRIVILEGE                 (2L)
#define SE_CREATE_TOKEN_PRIVILEGE                   (2L)
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE             (3L)
#define SE_LOCK_MEMORY_PRIVILEGE                    (4L)
#define SE_INCREASE_QUOTA_PRIVILEGE                 (5L)

#define SE_MACHINE_ACCOUNT_PRIVILEGE                (6L)
#define SE_TCB_PRIVILEGE                            (7L)
#define SE_SECURITY_PRIVILEGE                       (8L)
#define SE_TAKE_OWNERSHIP_PRIVILEGE                 (9L)
#define SE_LOAD_DRIVER_PRIVILEGE                    (10L)
#define SE_SYSTEM_PROFILE_PRIVILEGE                 (11L)
#define SE_SYSTEMTIME_PRIVILEGE                     (12L)
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE            (13L)
#define SE_INC_BASE_PRIORITY_PRIVILEGE              (14L)
#define SE_CREATE_PAGEFILE_PRIVILEGE                (15L)
#define SE_CREATE_PERMANENT_PRIVILEGE               (16L)
#define SE_BACKUP_PRIVILEGE                         (17L)
#define SE_RESTORE_PRIVILEGE                        (18L)
#define SE_SHUTDOWN_PRIVILEGE                       (19L)
#define SE_DEBUG_PRIVILEGE                          (20L)
#define SE_AUDIT_PRIVILEGE                          (21L)
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE             (22L)
#define SE_CHANGE_NOTIFY_PRIVILEGE                  (23L)
#define SE_REMOTE_SHUTDOWN_PRIVILEGE                (24L)
#define SE_UNDOCK_PRIVILEGE                         (25L)
#define SE_SYNC_AGENT_PRIVILEGE                     (26L)
#define SE_ENABLE_DELEGATION_PRIVILEGE              (27L)
#define SE_MANAGE_VOLUME_PRIVILEGE                  (28L)
#define SE_IMPERSONATE_PRIVILEGE                    (29L)
#define SE_CREATE_GLOBAL_PRIVILEGE                  (30L)
#define SE_TRUSTED_CREDMAN_ACCESS_PRIVILEGE         (31L)
#define SE_RELABEL_PRIVILEGE                        (32L)
#define SE_INC_WORKING_SET_PRIVILEGE                (33L)
#define SE_TIME_ZONE_PRIVILEGE                      (34L)
#define SE_CREATE_SYMBOLIC_LINK_PRIVILEGE           (35L)
#define SE_MAX_WELL_KNOWN_PRIVILEGE SE_CREATE_SYMBOLIC_LINK_PRIVILEGE

// Types
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_INVALID       0x00
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_INT64         0x01
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_UINT64        0x02
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_STRING        0x03
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_FQBN          0x04
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_SID           0x05
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_BOOLEAN       0x06
#define TOKEN_SECURITY_ATTRIBUTE_TYPE_OCTET_STRING  0x10

// Flags
#define TOKEN_SECURITY_ATTRIBUTE_NON_INHERITABLE    0x0001
#define TOKEN_SECURITY_ATTRIBUTE_VALUE_CASE_SENSITIVE 0x0002
#define TOKEN_SECURITY_ATTRIBUTE_USE_FOR_DENY_ONLY  0x0004
#define TOKEN_SECURITY_ATTRIBUTE_DISABLED_BY_DEFAULT 0x0008
#define TOKEN_SECURITY_ATTRIBUTE_DISABLED           0x0010
#define TOKEN_SECURITY_ATTRIBUTE_MANDATORY          0x0020
#define TOKEN_SECURITY_ATTRIBUTE_VALID_FLAGS (TOKEN_SECURITY_ATTRIBUTE_NON_INHERITABLE | \
                                              TOKEN_SECURITY_ATTRIBUTE_VALUE_CASE_SENSITIVE | \
                                              TOKEN_SECURITY_ATTRIBUTE_USE_FOR_DENY_ONLY | \
                                              TOKEN_SECURITY_ATTRIBUTE_DISABLED_BY_DEFAULT | \
                                              TOKEN_SECURITY_ATTRIBUTE_DISABLED | \
                                              TOKEN_SECURITY_ATTRIBUTE_MANDATORY)
#define TOKEN_SECURITY_ATTRIBUTE_CUSTOM_FLAGS       0xFFFF0000

// Misc.
typedef enum _FILTER_BOOT_OPTION_OPERATION {
    FilterBootOptionOperationOpenSystemStore,
    FilterBootOptionOperationSetElement,
    FilterBootOptionOperationDeleteElement,
    FilterBootOptionOperationMax
} FILTER_BOOT_OPTION_OPERATION;

///
/// < Security Structures & Typedefs >
///

#if NTDDI_VERSION < 0x0A000000
typedef ULONG SE_SIGNING_LEVEL, *PSE_SIGNING_LEVEL;
#endif 

typedef struct _TOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE {
    ULONG64 Version;
    UNICODE_STRING Name;
} TOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE, *PTOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE;

typedef struct _TOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE {
    PVOID pValue;
    ULONG ValueLength;
} TOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE, *PTOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE;

typedef struct _TOKEN_SECURITY_ATTRIBUTE_V1 {
    UNICODE_STRING Name;
    USHORT ValueType;
    USHORT Reserved;
    ULONG Flags;
    ULONG ValueCount;
    union {
        PLONG64 pInt64;
        PULONG64 pUint64;
        PUNICODE_STRING pString;
        PTOKEN_SECURITY_ATTRIBUTE_FQBN_VALUE pFqbn;
        PTOKEN_SECURITY_ATTRIBUTE_OCTET_STRING_VALUE pOctetString;
    } Values;
} TOKEN_SECURITY_ATTRIBUTE_V1, *PTOKEN_SECURITY_ATTRIBUTE_V1;
#define TOKEN_SECURITY_ATTRIBUTES_INFORMATION_VERSION_V1 1
#define TOKEN_SECURITY_ATTRIBUTES_INFORMATION_VERSION TOKEN_SECURITY_ATTRIBUTES_INFORMATION_VERSION_V1

typedef struct _TOKEN_SECURITY_ATTRIBUTES_INFORMATION {
    USHORT Version;
    USHORT Reserved;
    ULONG AttributeCount;
    union {
        PTOKEN_SECURITY_ATTRIBUTE_V1 pAttributeV1;
    } Attribute;
} TOKEN_SECURITY_ATTRIBUTES_INFORMATION, *PTOKEN_SECURITY_ATTRIBUTES_INFORMATION;


///
/// < Security Routines >
///

typedef NTSTATUS(NTAPI *tNtCreateToken)(
    OUT PHANDLE TokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TOKEN_TYPE TokenType,
    IN PLUID AuthenticationId,
    IN PLARGE_INTEGER ExpirationTime,
    IN PTOKEN_USER User,
    IN PTOKEN_GROUPS Groups,
    IN PTOKEN_PRIVILEGES Privileges,
    IN PTOKEN_OWNER Owner OPTIONAL,
    IN PTOKEN_PRIMARY_GROUP PrimaryGroup,
    IN PTOKEN_DEFAULT_DACL DefaultDacl OPTIONAL,
    IN PTOKEN_SOURCE TokenSource
    );

typedef NTSTATUS(NTAPI *tNtOpenProcessToken)(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    );

typedef NTSTATUS(NTAPI *tNtOpenProcessTokenEx)(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );

typedef NTSTATUS(NTAPI *tNtOpenThreadToken)(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle
    );

typedef NTSTATUS(NTAPI *tNtOpenThreadTokenEx)(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );

typedef NTSTATUS(NTAPI *tNtDuplicateToken)(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle
    );

typedef NTSTATUS(NTAPI *tNtQueryInformationToken)(
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    );

typedef NTSTATUS(NTAPI *tNtSetInformationToken)(
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN PVOID TokenInformation,
    IN ULONG TokenInformationLength
    );

typedef NTSTATUS(NTAPI *tNtAdjustPrivilegesToken)(
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState,
    IN ULONG BufferLength,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef NTSTATUS(NTAPI *tNtAdjustGroupsToken)(
    IN HANDLE TokenHandle,
    IN BOOLEAN ResetToDefault,
    IN PTOKEN_GROUPS NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_GROUPS PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );

typedef NTSTATUS(NTAPI *tNtFilterToken)(
    IN HANDLE ExistingTokenHandle,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PHANDLE NewTokenHandle
    );

typedef NTSTATUS(NTAPI *tNtCompareTokens)(
    IN HANDLE FirstTokenHandle,
    IN HANDLE SecondTokenHandle,
    OUT PBOOLEAN Equal
    );

typedef NTSTATUS(NTAPI *tNtPrivilegeCheck)(
    IN HANDLE ClientToken,
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    OUT PBOOLEAN Result
    );

typedef NTSTATUS(NTAPI *tNtImpersonateAnonymousToken)(
    IN HANDLE ThreadHandle
    );

typedef NTSTATUS(NTAPI *tNtAccessCheck)(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );

typedef NTSTATUS(NTAPI *tNtAccessCheckByType)(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );

typedef NTSTATUS(NTAPI *tNtAccessCheckByTypeResultList)(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );

typedef NTSTATUS(NTAPI *tNtSetCachedSigningLevel)(
    IN ULONG Flags,
    IN SE_SIGNING_LEVEL InputSigningLevel,
    IN PHANDLE SourceFiles,
    IN ULONG SourceFileCount,
    IN HANDLE TargetFile OPTIONAL
    );

typedef NTSTATUS(NTAPI *tNtGetCachedSigningLevel)(
    IN HANDLE File,
    OUT PULONG Flags,
    OUT PSE_SIGNING_LEVEL SigningLevel,
    OUT PUCHAR Thumbprint OPTIONAL,
    IN OUT PULONG ThumbprintSize OPTIONAL,
    OUT PULONG ThumbprintAlgorithm OPTIONAL
    );

typedef NTSTATUS(NTAPI *tNtAccessCheckAndAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtAccessCheckByTypeAndAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtAccessCheckByTypeResultListAndAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtAccessCheckByTypeResultListAndAuditAlarmByHandle)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN HANDLE ClientToken,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtOpenObjectAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN ObjectCreation,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtPrivilegeObjectAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

typedef NTSTATUS(NTAPI *tNtCloseObjectAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN BOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtDeleteObjectAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN BOOLEAN GenerateOnClose
    );

typedef NTSTATUS(NTAPI *tNtPrivilegedServiceAuditAlarm)(
    IN PUNICODE_STRING SubsystemName,
    IN PUNICODE_STRING ServiceName,
    IN HANDLE ClientToken,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

typedef NTSTATUS(NTAPI *tNtFilterBootOption)(
    IN FILTER_BOOT_OPTION_OPERATION FilterOperation,
    IN ULONG ObjectType,
    IN ULONG ElementType,
    IN PVOID Data OPTIONAL,
    IN ULONG DataSize
    );


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus