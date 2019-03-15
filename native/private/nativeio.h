#pragma once

#include "nativecommon.h"
#include "nativeob.h"
#include "nativesec.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * I/O Definitions
 */

//
// Invalid Constants
//
#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE                   ((ULONG)0xFFFFFFFF)
#endif
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER            ((ULONG)-1)
#endif
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES             ((ULONG)-1)
#endif

//
// Standard I/O handles
//
#ifndef STD_INPUT_HANDLE
#define STD_INPUT_HANDLE                    ((ULONG)(ULONG_PTR)-10)
#endif // STD_INPUT_HANDLE
#ifndef STD_OUTPUT_HANDLE
#define STD_OUTPUT_HANDLE                   ((ULONG)(ULONG_PTR)-11)
#endif // STD_OUTPUT_HANDLE
#ifndef STD_ERROR_HANDLE
#define STD_ERROR_HANDLE                    ((ULONG)(ULONG_PTR)-12)
#endif // STD_ERROR_HANDLE

//
// CreateFile Creation Flags & Attributes
//
#ifndef CREATE_NEW
#define CREATE_NEW                          (1)
#endif // CREATE_NEW
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS                       (2)
#endif // CREATE_ALWAYS
#ifndef OPEN_EXISTING
#define OPEN_EXISTING                       (3)
#endif // OPEN_EXISTING
#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS                         (4)
#endif // OPEN_ALWAYS
#ifndef TRUNCATE_EXISTING
#define TRUNCATE_EXISTING                   (5)
#endif // TRUNCATE_EXISTING

//
// File creation flags must start at the high end since they are combined with the attributes
//
// These are flags supported through CreateFile (W7) and CreateFile2 (W8 and beyond)
//
#ifndef FILE_FLAG_WRITE_THROUGH
#define FILE_FLAG_WRITE_THROUGH             (0x80000000)
#endif // FILE_FLAG_WRITE_THROUGH
#ifndef FILE_FLAG_OVERLAPPED
#define FILE_FLAG_OVERLAPPED                (0x40000000)
#endif // FILE_FLAG_OVERLAPPED
#ifndef FILE_FLAG_NO_BUFFERING
#define FILE_FLAG_NO_BUFFERING              (0x20000000)
#endif // FILE_FLAG_NO_BUFFERING
#ifndef FILE_FLAG_RANDOM_ACCESS
#define FILE_FLAG_RANDOM_ACCESS             (0x10000000)
#endif // FILE_FLAG_RANDOM_ACCESS
#ifndef FILE_FLAG_SEQUENTIAL_SCAN
#define FILE_FLAG_SEQUENTIAL_SCAN           (0x08000000)
#endif // FILE_FLAG_SEQUENTIAL_SCAN
#ifndef FILE_FLAG_DELETE_ON_CLOSE
#define FILE_FLAG_DELETE_ON_CLOSE           (0x04000000)
#endif // FILE_FLAG_DELETE_ON_CLOSE
#ifndef FILE_FLAG_BACKUP_SEMANTICS
#define FILE_FLAG_BACKUP_SEMANTICS          (0x02000000)
#endif // FILE_FLAG_BACKUP_SEMANTICS
#ifndef FILE_FLAG_POSIX_SEMANTICS
#define FILE_FLAG_POSIX_SEMANTICS           (0x01000000)
#endif // FILE_FLAG_POSIX_SEMANTICS
#ifndef FILE_FLAG_SESSION_AWARE
#define FILE_FLAG_SESSION_AWARE             (0x00800000)
#endif // FILE_FLAG_SESSION_AWARE
#ifndef FILE_FLAG_OPEN_REPARSE_POINT
#define FILE_FLAG_OPEN_REPARSE_POINT        (0x00200000)
#endif // FILE_FLAG_OPEN_REPARSE_POINT
#ifndef FILE_FLAG_OPEN_NO_RECALL
#define FILE_FLAG_OPEN_NO_RECALL            (0x00100000)
#endif // FILE_FLAG_OPEN_NO_RECALL
#ifndef FILE_FLAG_FIRST_PIPE_INSTANCE
#define FILE_FLAG_FIRST_PIPE_INSTANCE       (0x00080000)
#endif // FILE_FLAG_FIRST_PIPE_INSTANCE

//
// Device Types
//
#ifndef FILE_DEVICE_BEEP
#define FILE_DEVICE_BEEP                    1
#endif
#ifndef FILE_DEVICE_CD_ROM
#define FILE_DEVICE_CD_ROM                  2
#endif
#ifndef FILE_DEVICE_CD_ROM_FILE_SYSTEM
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM      3
#endif
#ifndef FILE_DEVICE_CONTROLLER
#define FILE_DEVICE_CONTROLLER              4
#endif
#ifndef FILE_DEVICE_DATALINK
#define FILE_DEVICE_DATALINK                5
#endif
#ifndef FILE_DEVICE_DFS
#define FILE_DEVICE_DFS                     6
#endif
#ifndef FILE_DEVICE_DISK
#define FILE_DEVICE_DISK                    7
#endif
#ifndef FILE_DEVICE_DISK_FILE_SYSTEM
#define FILE_DEVICE_DISK_FILE_SYSTEM        8
#endif
#ifndef FILE_DEVICE_FILE_SYSTEM
#define FILE_DEVICE_FILE_SYSTEM             9
#endif
#ifndef FILE_DEVICE_INPORT_PORT
#define FILE_DEVICE_INPORT_PORT             10
#endif
#ifndef FILE_DEVICE_KEYBOARD
#define FILE_DEVICE_KEYBOARD                11
#endif
#ifndef FILE_DEVICE_MAILSLOT
#define FILE_DEVICE_MAILSLOT                12
#endif
#ifndef FILE_DEVICE_MIDI_IN
#define FILE_DEVICE_MIDI_IN                 13
#endif
#ifndef FILE_DEVICE_MIDI_OUT
#define FILE_DEVICE_MIDI_OUT                14
#endif
#ifndef FILE_DEVICE_MOUSE
#define FILE_DEVICE_MOUSE                   15
#endif
#ifndef FILE_DEVICE_MULTI_UNC_PROVIDER
#define FILE_DEVICE_MULTI_UNC_PROVIDER      16
#endif
#ifndef FILE_DEVICE_NAMED_PIPE
#define FILE_DEVICE_NAMED_PIPE              17
#endif
#ifndef FILE_DEVICE_NETWORK
#define FILE_DEVICE_NETWORK                 18
#endif
#ifndef FILE_DEVICE_NETWORK_BROWSER
#define FILE_DEVICE_NETWORK_BROWSER         19
#endif
#ifndef FILE_DEVICE_NETWORK_FILE_SYSTEM
#define FILE_DEVICE_NETWORK_FILE_SYSTEM     20
#endif
#ifndef FILE_DEVICE_NULL
#define FILE_DEVICE_NULL                    21
#endif
#ifndef FILE_DEVICE_PARALLEL_PORT
#define FILE_DEVICE_PARALLEL_PORT           22
#endif
#ifndef FILE_DEVICE_PHYSICAL_NETCARD
#define FILE_DEVICE_PHYSICAL_NETCARD        23
#endif
#ifndef FILE_DEVICE_PRINTER
#define FILE_DEVICE_PRINTER                 24
#endif
#ifndef FILE_DEVICE_SCANNER
#define FILE_DEVICE_SCANNER                 25
#endif
#ifndef FILE_DEVICE_SERIAL_MOUSE_PORT
#define FILE_DEVICE_SERIAL_MOUSE_PORT       26
#endif
#ifndef FILE_DEVICE_SERIAL_PORT
#define FILE_DEVICE_SERIAL_PORT             27
#endif
#ifndef FILE_DEVICE_SCREEN
#define FILE_DEVICE_SCREEN                  28
#endif
#ifndef FILE_DEVICE_SOUND
#define FILE_DEVICE_SOUND                   29
#endif
#ifndef FILE_DEVICE_STREAMS
#define FILE_DEVICE_STREAMS                 30
#endif
#ifndef FILE_DEVICE_TAPE
#define FILE_DEVICE_TAPE                    31
#endif
#ifndef FILE_DEVICE_TAPE_FILE_SYSTEM
#define FILE_DEVICE_TAPE_FILE_SYSTEM        32
#endif
#ifndef FILE_DEVICE_TRANSPORT
#define FILE_DEVICE_TRANSPORT               33
#endif
#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN                 34
#endif
#ifndef FILE_DEVICE_VIDEO
#define FILE_DEVICE_VIDEO                   35
#endif
#ifndef FILE_DEVICE_VIRTUAL_DISK
#define FILE_DEVICE_VIRTUAL_DISK            36
#endif
#ifndef FILE_DEVICE_WAVE_IN
#define FILE_DEVICE_WAVE_IN                 37
#endif
#ifndef FILE_DEVICE_WAVE_OUT
#define FILE_DEVICE_WAVE_OUT                38
#endif
#ifndef FILE_DEVICE_8042_PORT
#define FILE_DEVICE_8042_PORT               39
#endif
#ifndef FILE_DEVICE_NETWORK_REDIRECTOR
#define FILE_DEVICE_NETWORK_REDIRECTOR      40
#endif
#ifndef FILE_DEVICE_BATTERY
#define FILE_DEVICE_BATTERY                 41
#endif
#ifndef FILE_DEVICE_BUS_EXTENDER
#define FILE_DEVICE_BUS_EXTENDER            42
#endif
#ifndef FILE_DEVICE_MODEM
#define FILE_DEVICE_MODEM                   43
#endif
#ifndef FILE_DEVICE_VDM
#define FILE_DEVICE_VDM                     44
#endif
#ifndef FILE_DEVICE_MASS_STORAGE
#define FILE_DEVICE_MASS_STORAGE            45
#endif
#ifndef FILE_DEVICE_SMB
#define FILE_DEVICE_SMB                     46
#endif
#ifndef FILE_DEVICE_KS
#define FILE_DEVICE_KS                      47
#endif
#ifndef FILE_DEVICE_CHANGER
#define FILE_DEVICE_CHANGER                 48
#endif
#ifndef FILE_DEVICE_SMARTCARD
#define FILE_DEVICE_SMARTCARD               49
#endif
#ifndef FILE_DEVICE_ACPI
#define FILE_DEVICE_ACPI                    50
#endif
#ifndef FILE_DEVICE_DVD
#define FILE_DEVICE_DVD                     51
#endif
#ifndef FILE_DEVICE_FULLSCREEN_VIDEO
#define FILE_DEVICE_FULLSCREEN_VIDEO        52
#endif
#ifndef FILE_DEVICE_DFS_FILE_SYSTEM
#define FILE_DEVICE_DFS_FILE_SYSTEM         53
#endif
#ifndef FILE_DEVICE_DFS_VOLUME
#define FILE_DEVICE_DFS_VOLUME              54
#endif
#ifndef FILE_DEVICE_SERENUM
#define FILE_DEVICE_SERENUM                 55
#endif
#ifndef FILE_DEVICE_TERMSRV
#define FILE_DEVICE_TERMSRV                 56
#endif
#ifndef FILE_DEVICE_KSEC
#define FILE_DEVICE_KSEC                    57
#endif

//
// File Type Flags
//
#ifndef FILE_TYPE_UNKNOWN
#define FILE_TYPE_UNKNOWN                   0
#endif
#ifndef FILE_TYPE_DISK
#define FILE_TYPE_DISK                      1
#endif
#ifndef FILE_TYPE_CHAR
#define FILE_TYPE_CHAR                      2
#endif
#ifndef FILE_TYPE_PIPE
#define FILE_TYPE_PIPE                      3
#endif
#ifndef FILE_TYPE_REMOTE
#define FILE_TYPE_REMOTE                    0x8000
#endif

//
// Define the create disposition values
//
#define FILE_SUPERSEDE                      (0x00000000)
#define FILE_OPEN                           (0x00000001)
#define FILE_CREATE                         (0x00000002)
#define FILE_OPEN_IF                        (0x00000003)
#define FILE_OVERWRITE                      (0x00000004)
#define FILE_OVERWRITE_IF                   (0x00000005)
#define FILE_MAXIMUM_DISPOSITION            (0x00000005)

//
// Define the create/open option flags
//
#define FILE_DIRECTORY_FILE                 (0x00000001)
#define FILE_WRITE_THROUGH                  (0x00000002)
#define FILE_SEQUENTIAL_ONLY                (0x00000004)
#define FILE_NO_INTERMEDIATE_BUFFERING      (0x00000008)
#define FILE_SYNCHRONOUS_IO_ALERT           (0x00000010)
#define FILE_SYNCHRONOUS_IO_NONALERT        (0x00000020)
#define FILE_NON_DIRECTORY_FILE             (0x00000040)
#define FILE_CREATE_TREE_CONNECTION         (0x00000080)
#define FILE_COMPLETE_IF_OPLOCKED           (0x00000100)
#define FILE_NO_EA_KNOWLEDGE                (0x00000200)
#define FILE_OPEN_REMOTE_INSTANCE           (0x00000400)
#define FILE_RANDOM_ACCESS                  (0x00000800)
#define FILE_DELETE_ON_CLOSE                (0x00001000)
#define FILE_OPEN_BY_FILE_ID                (0x00002000)
#define FILE_OPEN_FOR_BACKUP_INTENT         (0x00004000)
#define FILE_NO_COMPRESSION                 (0x00008000)
#define FILE_OPEN_REQUIRING_OPLOCK          (0x00010000)
#define FILE_DISALLOW_EXCLUSIVE             (0x00020000)
#define FILE_SESSION_AWARE                  (0x00040000)
#define FILE_RESERVE_OPFILTER               (0x00100000)
#define FILE_OPEN_REPARSE_POINT             (0x00200000)
#define FILE_OPEN_NO_RECALL                 (0x00400000)
#define FILE_OPEN_FOR_FREE_SPACE_QUERY      (0x00800000)

//
// The FILE_VALID_OPTION_FLAGS mask cannot be expanded to include the
// highest 8 bits of the DWORD because those are used to represent the
// create disposition in the IO Request Packet when sending information
// to the file system
//
#define FILE_VALID_OPTION_FLAGS             (0x00FFFFFF)
#define FILE_VALID_PIPE_OPTION_FLAGS        (0x00000032)
#define FILE_VALID_MAILSLOT_OPTION_FLAGS    (0x00000032)
#define FILE_VALID_SET_FLAGS                (0x00000036)

//
// Define the I/O status information return values for NtCreateFile/NtOpenFile
//
#define FILE_SUPERSEDED                     (0x00000000)
#define FILE_OPENED                         (0x00000001)
#define FILE_CREATED                        (0x00000002)
#define FILE_OVERWRITTEN                    (0x00000003)
#define FILE_EXISTS                         (0x00000004)
#define FILE_DOES_NOT_EXIST                 (0x00000005)

//
// Checks for valid flags
//
#define FILE_ATTRIBUTE_VALID_FLAGS          (0x00007FB7)
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      (0x000031A7)

//
// SetFilePointer move methods
//
#define FILE_BEGIN                          0
#define FILE_CURRENT                        1
#define FILE_END                            2

//
// Flush Flags
//
#define FLUSH_FLAGS_FILE_DATA_ONLY          0x00000001
#define FLUSH_FLAGS_NO_SYNC                 0x00000002


//
// File system control flags
//
#define FILE_VC_QUOTA_NONE                  0x00000000
#define FILE_VC_QUOTA_TRACK                 0x00000001
#define FILE_VC_QUOTA_ENFORCE               0x00000002
#define FILE_VC_QUOTA_MASK                  0x00000003
#define FILE_VC_CONTENT_INDEX_DISABLED      0x00000008
#define FILE_VC_LOG_QUOTA_THRESHOLD         0x00000010
#define FILE_VC_LOG_QUOTA_LIMIT             0x00000020
#define FILE_VC_LOG_VOLUME_THRESHOLD        0x00000040
#define FILE_VC_LOG_VOLUME_LIMIT            0x00000080
#define FILE_VC_QUOTAS_INCOMPLETE           0x00000100
#define FILE_VC_QUOTAS_REBUILDING           0x00000200
#define FILE_VC_VALID_MASK                  0x000003FF

//
// Define Volume Parameter Block (VPB) flags.
//
#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004
#define VPB_REMOVE_PENDING              0x00000008
#define VPB_RAW_MOUNT                   0x00000010
#define VPB_DIRECT_WRITES_ALLOWED       0x00000020

//
// Define File Object (FO) flags
//
#define FO_FILE_OPEN                    0x00000001
#define FO_SYNCHRONOUS_IO               0x00000002
#define FO_ALERTABLE_IO                 0x00000004
#define FO_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FO_WRITE_THROUGH                0x00000010
#define FO_SEQUENTIAL_ONLY              0x00000020
#define FO_CACHE_SUPPORTED              0x00000040
#define FO_NAMED_PIPE                   0x00000080
#define FO_STREAM_FILE                  0x00000100
#define FO_MAILSLOT                     0x00000200
#define FO_GENERATE_AUDIT_ON_CLOSE      0x00000400
#define FO_QUEUE_IRP_TO_THREAD          FO_GENERATE_AUDIT_ON_CLOSE
#define FO_DIRECT_DEVICE_OPEN           0x00000800
#define FO_FILE_MODIFIED                0x00001000
#define FO_FILE_SIZE_CHANGED            0x00002000
#define FO_CLEANUP_COMPLETE             0x00004000
#define FO_TEMPORARY_FILE               0x00008000
#define FO_DELETE_ON_CLOSE              0x00010000
#define FO_OPENED_CASE_SENSITIVE        0x00020000
#define FO_HANDLE_CREATED               0x00040000
#define FO_FILE_FAST_IO_READ            0x00080000
#define FO_RANDOM_ACCESS                0x00100000
#define FO_FILE_OPEN_CANCELLED          0x00200000
#define FO_VOLUME_OPEN                  0x00400000
#define FO_REMOTE_ORIGIN                0x01000000
#define FO_DISALLOW_EXCLUSIVE           0x02000000
#define FO_SKIP_COMPLETION_PORT         FO_DISALLOW_EXCLUSIVE
#define FO_SKIP_SET_EVENT               0x04000000
#define FO_SKIP_SET_FAST_IO             0x08000000
#define FO_INDIRECT_WAIT_OBJECT         0x10000000
#define FO_SECTION_MINSTORE_TREATMENT   0x20000000

//
// This mask allows us to re-use flags that are not present during a create
// operation for uses that are only valid for the duration of the create.
//
#define FO_FLAGS_VALID_ONLY_DURING_CREATE FO_DISALLOW_EXCLUSIVE




///
/// < I/O Enums >
///

typedef enum _FS_INFORMATION_CLASS {
    FileFsVolumeInformation = 1,
    FileFsLabelInformation,
    FileFsSizeInformation,
    FileFsDeviceInformation,
    FileFsAttributeInformation,
    FileFsControlInformation,
    FileFsFullSizeInformation,
    FileFsObjectIdInformation,  // FILE_FS_OBJECTID_INFORMATION
    FileFsDriverPathInformation,
    FileFsVolumeFlagsInformation,
    FileFsSectorSizeInformation, // since WIN8
    FileFsDataCopyInformation,
    FileFsMetadataSizeInformation, // since THRESHOLD
    FileFsMaximumInformation
} FS_INFORMATION_CLASS;

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation, // 10
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation, // 20
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileObjectIdInformation,
    FileCompletionInformation, // 30
    FileMoveClusterInformation,
    FileQuotaInformation,
    FileReparsePointInformation,
    FileNetworkOpenInformation,
    FileAttributeTagInformation,
    FileTrackingInformation,
    FileIdBothDirectoryInformation,
    FileIdFullDirectoryInformation,
    FileValidDataLengthInformation,
    FileShortNameInformation, // 40
    FileIoCompletionNotificationInformation,
    FileIoStatusBlockRangeInformation,
    FileIoPriorityHintInformation,
    FileSfioReserveInformation,
    FileSfioVolumeInformation,
    FileHardLinkInformation,
    FileProcessIdsUsingFileInformation,
    FileNormalizedNameInformation,
    FileNetworkPhysicalNameInformation,
    FileIdGlobalTxDirectoryInformation, // 50
    FileIsRemoteDeviceInformation,
    FileUnusedInformation,
    FileNumaNodeInformation,
    FileStandardLinkInformation,
    FileRemoteProtocolInformation,
    FileRenameInformationBypassAccessCheck, // (kernel-mode only) // since WIN8
    FileLinkInformationBypassAccessCheck, // (kernel-mode only)
    FileIntegrityStreamInformation,
    FileVolumeNameInformation,
    FileIdInformation,
    FileIdExtdDirectoryInformation,
    FileReplaceCompletionInformation, // since WINBLUE
    FileHardLinkFullIdInformation,
    FileIdExtdBothDirectoryInformation, // since 10
    FileMaximumInformation
} FILE_INFORMATION_CLASS;


/// 
/// < I/O Structures & Typedefs >
/// 

// Define the base asynchronous I/O argument types
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } DUMMYUNIONNAME;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _IO_STATUS_BLOCK32 {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK32, *PIO_STATUS_BLOCK32;

typedef VOID( NTAPI *PIO_APC_ROUTINE )(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

//
// Volume Parameter Block (VPB)
//
typedef struct _VPB {
    CSHORT Type;
    CSHORT Size;
    USHORT Flags;
    USHORT VolumeLabelLength; // in bytes
    struct _DEVICE_OBJECT *DeviceObject;
    struct _DEVICE_OBJECT *RealDevice;
    ULONG SerialNumber;
    ULONG ReferenceCount;
#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR)) // 32 characters
    WCHAR VolumeLabel[MAXIMUM_VOLUME_LABEL_LENGTH / sizeof( WCHAR )];
} VPB, *PVPB;

//
// The following structure is pointed to by the SectionObject pointer field
// of a file object, and is allocated by the various NT file systems.
//
typedef struct _SECTION_OBJECT_POINTERS {
    PVOID DataSectionObject;
    PVOID SharedCacheMap;
    PVOID ImageSectionObject;
} SECTION_OBJECT_POINTERS;
typedef SECTION_OBJECT_POINTERS *PSECTION_OBJECT_POINTERS;

//
// Define the format of a completion message.
//
typedef struct _IO_COMPLETION_CONTEXT {
    PVOID Port;
    PVOID Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;

//
// Define File Object (FO)
//
typedef struct _FILE_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PVOID DeviceObject;
    PVPB Vpb;
    PVOID FsContext;
    PVOID FsContext2;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;
    PVOID PrivateCacheMap;
    NTSTATUS FinalStatus;
    struct _FILE_OBJECT *RelatedFileObject;
    BOOLEAN LockOperation;
    BOOLEAN DeletePending;
    BOOLEAN ReadAccess;
    BOOLEAN WriteAccess;
    BOOLEAN DeleteAccess;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN SharedDelete;
    ULONG Flags;
    UNICODE_STRING FileName;
    LARGE_INTEGER CurrentByteOffset;
    __volatile ULONG Waiters;
    __volatile ULONG Busy;
    PVOID LastLock;
    KEVENT Lock;
    KEVENT Event;
    __volatile PIO_COMPLETION_CONTEXT CompletionContext;
    KSPIN_LOCK IrpListLock;
    LIST_ENTRY IrpList;
    __volatile PVOID FileObjectExtension;
} FILE_OBJECT;
typedef struct _FILE_OBJECT *PFILE_OBJECT;


//
// NtQuery(Set)InformationFile return types:
//
//      FILE_BASIC_INFORMATION
//      FILE_STANDARD_INFORMATION
//      FILE_INTERNAL_INFORMATION
//      FILE_EA_INFORMATION
//      FILE_ACCESS_INFORMATION
//      FILE_POSITION_INFORMATION
//      FILE_MODE_INFORMATION
//      FILE_ALIGNMENT_INFORMATION
//      FILE_NAME_INFORMATION
//      FILE_ALL_INFORMATION
//
//      FILE_NETWORK_OPEN_INFORMATION
//
//      FILE_ALLOCATION_INFORMATION
//      FILE_COMPRESSION_INFORMATION
//      FILE_DISPOSITION_INFORMATION
//      FILE_END_OF_FILE_INFORMATION
//      FILE_LINK_INFORMATION
//      FILE_MOVE_CLUSTER_INFORMATION
//      FILE_RENAME_INFORMATION
//      FILE_SHORT_NAME_INFORMATION
//      FILE_STREAM_INFORMATION
//      FILE_COMPLETION_INFORMATION
//
//      FILE_PIPE_INFORMATION
//      FILE_PIPE_LOCAL_INFORMATION
//      FILE_PIPE_REMOTE_INFORMATION
//
//      FILE_MAILSLOT_QUERY_INFORMATION
//      FILE_MAILSLOT_SET_INFORMATION
//      FILE_REPARSE_POINT_INFORMATION
//
typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION {
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
    ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {
    LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _FILE_MODE_INFORMATION {
    ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {
    ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION {
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
    FILE_INTERNAL_INFORMATION InternalInformation;
    FILE_EA_INFORMATION EaInformation;
    FILE_ACCESS_INFORMATION AccessInformation;
    FILE_POSITION_INFORMATION PositionInformation;
    FILE_MODE_INFORMATION ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

typedef struct _FILE_NETWORK_OPEN_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {
    ULONG FileAttributes;
    ULONG ReparseTag;
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;

typedef struct _FILE_ALLOCATION_INFORMATION {
    LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFORMATION, *PFILE_ALLOCATION_INFORMATION;

typedef struct _FILE_COMPRESSION_INFORMATION {
    LARGE_INTEGER CompressedFileSize;
    USHORT CompressionFormat;
    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved[3];
} FILE_COMPRESSION_INFORMATION, *PFILE_COMPRESSION_INFORMATION;

typedef struct _FILE_DISPOSITION_INFORMATION {
    BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

typedef struct _FILE_END_OF_FILE_INFORMATION {
    LARGE_INTEGER EndOfFile;
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION;

typedef struct _FILE_VALID_DATA_LENGTH_INFORMATION {
    LARGE_INTEGER ValidDataLength;
} FILE_VALID_DATA_LENGTH_INFORMATION, *PFILE_VALID_DATA_LENGTH_INFORMATION;

typedef struct _FILE_LINK_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;

typedef struct _FILE_MOVE_CLUSTER_INFORMATION {
    ULONG ClusterCount;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_MOVE_CLUSTER_INFORMATION, *PFILE_MOVE_CLUSTER_INFORMATION;

typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

typedef struct _FILE_STREAM_INFORMATION {
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _FILE_TRACKING_INFORMATION {
    HANDLE DestinationFile;
    ULONG ObjectInformationLength;
    CHAR ObjectInformation[1];
} FILE_TRACKING_INFORMATION, *PFILE_TRACKING_INFORMATION;

typedef struct _FILE_COMPLETION_INFORMATION {
    HANDLE Port;
    PVOID Key;
} FILE_COMPLETION_INFORMATION, *PFILE_COMPLETION_INFORMATION;

typedef struct _FILE_PIPE_INFORMATION {
    ULONG ReadMode;
    ULONG CompletionMode;
} FILE_PIPE_INFORMATION, *PFILE_PIPE_INFORMATION;

typedef struct _FILE_PIPE_LOCAL_INFORMATION {
    ULONG NamedPipeType;
    ULONG NamedPipeConfiguration;
    ULONG MaximumInstances;
    ULONG CurrentInstances;
    ULONG InboundQuota;
    ULONG ReadDataAvailable;
    ULONG OutboundQuota;
    ULONG WriteQuotaAvailable;
    ULONG NamedPipeState;
    ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

typedef struct _FILE_PIPE_REMOTE_INFORMATION {
    LARGE_INTEGER CollectDataTime;
    ULONG MaximumCollectionCount;
} FILE_PIPE_REMOTE_INFORMATION, *PFILE_PIPE_REMOTE_INFORMATION;

typedef struct _FILE_MAILSLOT_QUERY_INFORMATION {
    ULONG MaximumMessageSize;
    ULONG MailslotQuota;
    ULONG NextMessageSize;
    ULONG MessagesAvailable;
    LARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_QUERY_INFORMATION, *PFILE_MAILSLOT_QUERY_INFORMATION;

typedef struct _FILE_MAILSLOT_SET_INFORMATION {
    PLARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_SET_INFORMATION, *PFILE_MAILSLOT_SET_INFORMATION;

typedef struct _FILE_REPARSE_POINT_INFORMATION {
    LONGLONG FileReference;
    ULONG Tag;
} FILE_REPARSE_POINT_INFORMATION, *PFILE_REPARSE_POINT_INFORMATION;

//
// NtQuery(Set)EaFile
//
// The offset for the start of EaValue is EaName[EaNameLength + 1]
//
typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

typedef struct _FILE_GET_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR EaNameLength;
    CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

//
// NtQuery(Set)QuotaInformationFile
//
typedef struct _FILE_GET_QUOTA_INFORMATION {
    ULONG NextEntryOffset;
    ULONG SidLength;
    SID Sid;
} FILE_GET_QUOTA_INFORMATION, *PFILE_GET_QUOTA_INFORMATION;

typedef struct _FILE_QUOTA_INFORMATION {
    ULONG NextEntryOffset;
    ULONG SidLength;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER QuotaUsed;
    LARGE_INTEGER QuotaThreshold;
    LARGE_INTEGER QuotaLimit;
    SID Sid;
} FILE_QUOTA_INFORMATION, *PFILE_QUOTA_INFORMATION;

//
// NtQuery[Set]VolumeInformationFile types:
//
//  FILE_FS_LABEL_INFORMATION
//  FILE_FS_VOLUME_INFORMATION
//  FILE_FS_SIZE_INFORMATION
//  FILE_FS_DEVICE_INFORMATION
//  FILE_FS_ATTRIBUTE_INFORMATION
//  FILE_FS_CONTROL_INFORMATION
//  FILE_FS_OBJECTID_INFORMATION
//
typedef struct _FILE_FS_LABEL_INFORMATION {
    ULONG VolumeLabelLength;
    WCHAR VolumeLabel[1];
} FILE_FS_LABEL_INFORMATION, *PFILE_FS_LABEL_INFORMATION;

typedef struct _FILE_FS_VOLUME_INFORMATION {
    LARGE_INTEGER VolumeCreationTime;
    ULONG VolumeSerialNumber;
    ULONG VolumeLabelLength;
    BOOLEAN SupportsObjects;
    WCHAR VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;

typedef struct _FILE_FS_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER AvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

typedef struct _FILE_FS_FULL_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER CallerAvailableAllocationUnits;
    LARGE_INTEGER ActualAvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_FULL_SIZE_INFORMATION, *PFILE_FS_FULL_SIZE_INFORMATION;

typedef struct _FILE_FS_OBJECTID_INFORMATION {
    UCHAR ObjectId[16];
    UCHAR ExtendedInfo[48];
} FILE_FS_OBJECTID_INFORMATION, *PFILE_FS_OBJECTID_INFORMATION;

typedef struct _FILE_FS_DEVICE_INFORMATION {
    DEVICE_TYPE DeviceType;
    ULONG Characteristics;
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;

typedef struct _FILE_FS_ATTRIBUTE_INFORMATION {
    ULONG FileSystemAttributes;
    LONG MaximumComponentNameLength;
    ULONG FileSystemNameLength;
    WCHAR FileSystemName[1];
} FILE_FS_ATTRIBUTE_INFORMATION, *PFILE_FS_ATTRIBUTE_INFORMATION;

typedef struct _FILE_FS_DRIVER_PATH_INFORMATION {
    BOOLEAN DriverInPath;
    ULONG DriverNameLength;
    WCHAR DriverName[1];
} FILE_FS_DRIVER_PATH_INFORMATION, *PFILE_FS_DRIVER_PATH_INFORMATION;

typedef struct _FILE_FS_CONTROL_INFORMATION {
    LARGE_INTEGER FreeSpaceStartFiltering;
    LARGE_INTEGER FreeSpaceThreshold;
    LARGE_INTEGER FreeSpaceStopFiltering;
    LARGE_INTEGER DefaultQuotaThreshold;
    LARGE_INTEGER DefaultQuotaLimit;
    ULONG FileSystemControlFlags;
} FILE_FS_CONTROL_INFORMATION, *PFILE_FS_CONTROL_INFORMATION;

// Define segment buffer structure for scatter/gather read/write.
//typedef union _FILE_SEGMENT_ELEMENT {
//    PVOID64 Buffer;
//    ULONGLONG Alignment;
//} FILE_SEGMENT_ELEMENT, *PFILE_SEGMENT_ELEMENT;





///
/// < I/O Routines >
///

typedef NTSTATUS( NTAPI *tNtFlushBuffersFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );
NTSYSAPI
NTSTATUS
NTAPI
NtFlushBuffersFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

typedef NTSTATUS( NTAPI *tNtNotifyChangeDirectoryFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree
    );

typedef NTSTATUS( NTAPI *tNtQueryAttributesFile )(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
    );

typedef NTSTATUS( NTAPI *tNtQueryFullAttributesFile )(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
    );

typedef NTSTATUS( NTAPI *tNtQueryEaFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID EaList OPTIONAL,
    IN ULONG EaListLength,
    IN PULONG EaIndex OPTIONAL,
    IN BOOLEAN RestartScan
    );

typedef NTSTATUS( NTAPI *tNtCreateFile )(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );
NTSYSAPI
NTSTATUS
NTAPI
NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

typedef NTSTATUS( NTAPI *tNtDeviceIoControlFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
NtDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

typedef NTSTATUS( NTAPI *tNtFsControlFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FsControlCode,
    IN PVOID InputBufferOPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

typedef NTSTATUS( NTAPI *tNtLockFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    );
NTSYSAPI
NTSTATUS
NTAPI
NtLockFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    );

typedef NTSTATUS( NTAPI *tNtOpenFile )(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );
NTSYSAPI
NTSTATUS
NTAPI
NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

typedef NTSTATUS( NTAPI *tNtQueryDirectoryFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    );

typedef NTSTATUS( NTAPI *tNtQueryInformationFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

typedef NTSTATUS( NTAPI *tNtQueryQuotaInformationFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID SidList OPTIONAL,
    IN ULONG SidListLength,
    IN PSID StartSid OPTIONAL,
    IN BOOLEAN RestartScan
    );

typedef NTSTATUS( NTAPI *tNtQueryVolumeInformationFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass 
    );

typedef NTSTATUS( NTAPI *tNtReadFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtSetInformationFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

typedef NTSTATUS( NTAPI *tNtSetQuotaInformationFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef NTSTATUS( NTAPI *tNtSetVolumeInformationFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

typedef NTSTATUS( NTAPI *tNtWriteFile )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
NtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

typedef NTSTATUS(NTAPI *tNtDeleteFile)(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
NtDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


typedef NTSTATUS( NTAPI *tNtUnlockFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    );
NTSYSAPI
NTSTATUS
NTAPI
NtUnlockFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    );


typedef NTSTATUS( NTAPI *tNtReadFileScatter )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtSetEaFile )(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef NTSTATUS( NTAPI *tNtWriteFileGather )(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

typedef NTSTATUS( NTAPI *tNtLoadDriver )(
    IN PUNICODE_STRING DriverServiceName
    );

typedef NTSTATUS( NTAPI *tNtUnloadDriver )(
    IN PUNICODE_STRING DriverServiceName
    );


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus