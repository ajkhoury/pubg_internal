/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file ntapi.h
 * @author Aidan Khoury (ajkhoury)
 * @date 8/30/2018
 */

#ifndef _BLOCKOUT_NTAPI_H_
#define _BLOCKOUT_NTAPI_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include <ntifs.h>
#define NTSTRSAFE_NO_CB_FUNCTIONS
#include <ntstrsafe.h>
#include <ntimage.h>
#include <apiset.h>

#include "nttrust.h"
#include "ntbuild.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable : 4214) // nonstandard extension used: bit field types other than int
#endif

//
// Indicates the safe boot init value. Possible values are:
//  0   The operating system is not in Safe Mode.
//  1   SAFEBOOT_MINIMAL
//  2   SAFEBOOT_NETWORK
//  3*  SAFEBOOT_DSREPAIR
//
extern PULONG InitSafeBootMode;


//
// Define macro for appending 'sig' to the beginning of 
// global references.
//
#define NTSIG(name) sig##name


//
// Process Access Flags
//

#define PROCESS_TERMINATE           0x0001
#define PROCESS_CREATE_THREAD       0x0002
#define PROCESS_SET_SESSIONID       0x0004
#define PROCESS_VM_OPERATION        0x0008
#define PROCESS_VM_READ             0x0010
#define PROCESS_VM_WRITE            0x0020
#ifndef PROCESS_DUP_HANDLE
#define PROCESS_DUP_HANDLE          0x0040
#endif
#define PROCESS_CREATE_PROCESS      0x0080
#define PROCESS_SET_QUOTA           0x0100
#define PROCESS_SET_INFORMATION     0x0200
#define PROCESS_QUERY_INFORMATION   0x0400
#define PROCESS_SUSPEND_RESUME      0x0800
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000


//
// Section Flags for NtCreateSection
//

#ifndef SEC_BASED
#define SEC_BASED                                           0x00200000
#endif
#ifndef SEC_NO_CHANGE
#define SEC_NO_CHANGE                                       0x00400000
#endif
#ifndef SEC_FILE
#define SEC_FILE                                            0x00800000
#endif
#ifndef SEC_IMAGE
#define SEC_IMAGE                                           0x01000000
#endif
#ifndef SEC_PROTECTED_IMAGE
#define SEC_PROTECTED_IMAGE                                 0x02000000
#endif
#ifndef SEC_RESERVE
#define SEC_RESERVE                                         0x04000000
#endif
#ifndef SEC_COMMIT
#define SEC_COMMIT                                          0x08000000
#endif
#ifndef SEC_NOCACHE
#define SEC_NOCACHE                                         0x10000000
#endif
#ifndef SEC_WRITECOMBINE
#define SEC_WRITECOMBINE                                    0x40000000
#endif
#ifndef SEC_LARGE_PAGES
#define SEC_LARGE_PAGES                                     0x80000000
#endif

//
// File mapping access attributes
//

#define FILE_MAP_WRITE            SECTION_MAP_WRITE
#define FILE_MAP_READ             SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS       SECTION_ALL_ACCESS

#define FILE_MAP_EXECUTE          SECTION_MAP_EXECUTE_EXPLICIT  // not included in FILE_MAP_ALL_ACCESS

#define FILE_MAP_COPY             0x00000001

#define FILE_MAP_RESERVE          0x80000000
#define FILE_MAP_TARGETS_INVALID  0x40000000
#define FILE_MAP_LARGE_PAGES      0x20000000


//
// Define the security attributes structure.
//

#ifndef _SECURITY_ATTRIBUTES_
#define _SECURITY_ATTRIBUTES_
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    PVOID lpSecurityDescriptor;
    LOGICAL bInheritHandle;
} 	SECURITY_ATTRIBUTES;
typedef struct _SECURITY_ATTRIBUTES *PSECURITY_ATTRIBUTES;
typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;
#endif // !_SECURITY_ATTRIBUTES_


//
// NtQsi information class tpyes.
//

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0x0,
    SystemProcessorInformation = 0x1,
    SystemPerformanceInformation = 0x2,
    SystemTimeOfDayInformation = 0x3,
    SystemPathInformation = 0x4,
    SystemProcessInformation = 0x5,
    SystemCallCountInformation = 0x6,
    SystemDeviceInformation = 0x7,
    SystemProcessorPerformanceInformation = 0x8,
    SystemFlagsInformation = 0x9,
    SystemCallTimeInformation = 0xa,
    SystemModuleInformation = 0xb,
    SystemLocksInformation = 0xc,
    SystemStackTraceInformation = 0xd,
    SystemPagedPoolInformation = 0xe,
    SystemNonPagedPoolInformation = 0xf,
    SystemHandleInformation = 0x10,
    SystemObjectInformation = 0x11,
    SystemPageFileInformation = 0x12,
    SystemVdmInstemulInformation = 0x13,
    SystemVdmBopInformation = 0x14,
    SystemFileCacheInformation = 0x15,
    SystemPoolTagInformation = 0x16,
    SystemInterruptInformation = 0x17,
    SystemDpcBehaviorInformation = 0x18,
    SystemFullMemoryInformation = 0x19,
    SystemLoadGdiDriverInformation = 0x1a,
    SystemUnloadGdiDriverInformation = 0x1b,
    SystemTimeAdjustmentInformation = 0x1c,
    SystemSummaryMemoryInformation = 0x1d,
    SystemMirrorMemoryInformation = 0x1e,
    SystemPerformanceTraceInformation = 0x1f,
    SystemObsolete0 = 0x20,
    SystemExceptionInformation = 0x21,
    SystemCrashDumpStateInformation = 0x22,
    SystemKernelDebuggerInformation = 0x23,
    SystemContextSwitchInformation = 0x24,
    SystemRegistryQuotaInformation = 0x25,
    SystemExtendServiceTableInformation = 0x26,
    SystemPrioritySeperation = 0x27,
    SystemVerifierAddDriverInformation = 0x28,
    SystemVerifierRemoveDriverInformation = 0x29,
    SystemProcessorIdleInformation = 0x2a,
    SystemLegacyDriverInformation = 0x2b,
    SystemCurrentTimeZoneInformation = 0x2c,
    SystemLookasideInformation = 0x2d,
    SystemTimeSlipNotification = 0x2e,
    SystemSessionCreate = 0x2f,
    SystemSessionDetach = 0x30,
    SystemSessionInformation = 0x31,
    SystemRangeStartInformation = 0x32,
    SystemVerifierInformation = 0x33,
    SystemVerifierThunkExtend = 0x34,
    SystemSessionProcessInformation = 0x35,
    SystemLoadGdiDriverInSystemSpace = 0x36,
    SystemNumaProcessorMap = 0x37,
    SystemPrefetcherInformation = 0x38,
    SystemExtendedProcessInformation = 0x39,
    SystemRecommendedSharedDataAlignment = 0x3a,
    SystemComPlusPackage = 0x3b,
    SystemNumaAvailableMemory = 0x3c,
    SystemProcessorPowerInformation = 0x3d,
    SystemEmulationBasicInformation = 0x3e,
    SystemEmulationProcessorInformation = 0x3f,
    SystemExtendedHandleInformation = 0x40,
    SystemLostDelayedWriteInformation = 0x41,
    SystemBigPoolInformation = 0x42,
    SystemSessionPoolTagInformation = 0x43,
    SystemSessionMappedViewInformation = 0x44,
    SystemHotpatchInformation = 0x45,
    SystemObjectSecurityMode = 0x46,
    SystemWatchdogTimerHandler = 0x47,
    SystemWatchdogTimerInformation = 0x48,
    SystemLogicalProcessorInformation = 0x49,
    SystemWow64SharedInformationObsolete = 0x4a,
    SystemRegisterFirmwareTableInformationHandler = 0x4b,
    SystemFirmwareTableInformation = 0x4c,
    SystemModuleInformationEx = 0x4d,
    SystemVerifierTriageInformation = 0x4e,
    SystemSuperfetchInformation = 0x4f,
    SystemMemoryListInformation = 0x50,
    SystemFileCacheInformationEx = 0x51,
    SystemThreadPriorityClientIdInformation = 0x52,
    SystemProcessorIdleCycleTimeInformation = 0x53,
    SystemVerifierCancellationInformation = 0x54,
    SystemProcessorPowerInformationEx = 0x55,
    SystemRefTraceInformation = 0x56,
    SystemSpecialPoolInformation = 0x57,
    SystemProcessIdInformation = 0x58,
    SystemErrorPortInformation = 0x59,
    SystemBootEnvironmentInformation = 0x5a,
    SystemHypervisorInformation = 0x5b,
    SystemVerifierInformationEx = 0x5c,
    SystemTimeZoneInformation = 0x5d,
    SystemImageFileExecutionOptionsInformation = 0x5e,
    SystemCoverageInformation = 0x5f,
    SystemPrefetchPatchInformation = 0x60,
    SystemVerifierFaultsInformation = 0x61,
    SystemSystemPartitionInformation = 0x62,
    SystemSystemDiskInformation = 0x63,
    SystemProcessorPerformanceDistribution = 0x64,
    SystemNumaProximityNodeInformation = 0x65,
    SystemDynamicTimeZoneInformation = 0x66,
    SystemCodeIntegrityInformation = 0x67,
    SystemProcessorMicrocodeUpdateInformation = 0x68,
    SystemProcessorBrandString = 0x69,
    SystemVirtualAddressInformation = 0x6a,
    SystemLogicalProcessorAndGroupInformation = 0x6b,
    SystemProcessorCycleTimeInformation = 0x6c,
    SystemStoreInformation = 0x6d,
    SystemRegistryAppendString = 0x6e,
    SystemAitSamplingValue = 0x6f,
    SystemVhdBootInformation = 0x70,
    SystemCpuQuotaInformation = 0x71,
    SystemNativeBasicInformation = 0x72,
    SystemErrorPortTimeouts = 0x73,
    SystemLowPriorityIoInformation = 0x74,
    SystemBootEntropyInformation = 0x75,
    SystemVerifierCountersInformation = 0x76,
    SystemPagedPoolInformationEx = 0x77,
    SystemSystemPtesInformationEx = 0x78,
    SystemNodeDistanceInformation = 0x79,
    SystemAcpiAuditInformation = 0x7a,
    SystemBasicPerformanceInformation = 0x7b,
    SystemQueryPerformanceCounterInformation = 0x7c,
    SystemSessionBigPoolInformation = 0x7d,
    SystemBootGraphicsInformation = 0x7e,
    SystemScrubPhysicalMemoryInformation = 0x7f,
    SystemBadPageInformation = 0x80,
    SystemProcessorProfileControlArea = 0x81,
    SystemCombinePhysicalMemoryInformation = 0x82,
    SystemEntropyInterruptTimingInformation = 0x83,
    SystemConsoleInformation = 0x84,
    SystemPlatformBinaryInformation = 0x85,
    SystemThrottleNotificationInformation = 0x86,
    SystemHypervisorProcessorCountInformation = 0x87,
    SystemDeviceDataInformation = 0x88,
    SystemDeviceDataEnumerationInformation = 0x89,
    SystemMemoryTopologyInformation = 0x8a,
    SystemMemoryChannelInformation = 0x8b,
    SystemBootLogoInformation = 0x8c,
    SystemProcessorPerformanceInformationEx = 0x8d,
    SystemSpare0 = 0x8e,
    SystemSecureBootPolicyInformation = 0x8f,
    SystemPageFileInformationEx = 0x90,
    SystemSecureBootInformation = 0x91,
    SystemEntropyInterruptTimingRawInformation = 0x92,
    SystemPortableWorkspaceEfiLauncherInformation = 0x93,
    SystemFullProcessInformation = 0x94,
    SystemKernelDebuggerInformationEx = 0x95,
    SystemBootMetadataInformation = 0x96,
    SystemSoftRebootInformation = 0x97,
    SystemElamCertificateInformation = 0x98,
    SystemOfflineDumpConfigInformation = 0x99,
    SystemProcessorFeaturesInformation = 0x9a,
    SystemRegistryReconciliationInformation = 0x9b,
    SystemEdidInformation = 0x9c,
    SystemManufacturingInformation = 0x9d,
    SystemEnergyEstimationConfigInformation = 0x9e,
    SystemHypervisorDetailInformation = 0x9f,
    SystemProcessorCycleStatsInformation = 0xa0,
    SystemVmGenerationCountInformation = 0xa1,
    SystemTrustedPlatformModuleInformation = 0xa2,
    SystemKernelDebuggerFlags = 0xa3,
    SystemCodeIntegrityPolicyInformation = 0xa4,
    SystemIsolatedUserModeInformation = 0xa5,
    SystemHardwareSecurityTestInterfaceResultsInformation = 0xa6,
    SystemSingleModuleInformation = 0xa7,
    SystemAllowedCpuSetsInformation = 0xa8,
    SystemDmaProtectionInformation = 0xa9,
    SystemInterruptCpuSetsInformation = 0xaa,
    SystemSecureBootPolicyFullInformation = 0xab,
    SystemCodeIntegrityPolicyFullInformation = 0xac,
    SystemAffinitizedInterruptProcessorInformation = 0xad,
    SystemRootSiloInformation = 0xae,
    SystemCpuSetInformation = 0xaf,
    SystemCpuSetTagInformation = 0xb0,
    MaxSystemInfoClass = 0xb1
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    ULONG NumberOfPhysicalPages;
    ULONG LowestPhysicalPageNumber;
    ULONG HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG_PTR MinimumUserModeAddress;
    ULONG_PTR MaximumUserModeAddress;
    ULONG_PTR ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_EX {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
    ULONG Spare0;
    LARGE_INTEGER AvailableTime;
    LARGE_INTEGER Spare1;
    LARGE_INTEGER Spare2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_EX, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_EX;

typedef struct _SYSTEM_PROCESSOR_IDLE_INFORMATION {
    ULONGLONG IdleTime;
    ULONGLONG C1Time;
    ULONGLONG C2Time;
    ULONGLONG C3Time;
    ULONG C1Transitions;
    ULONG C2Transitions;
    ULONG C3Transitions;
    ULONG Padding;
} SYSTEM_PROCESSOR_IDLE_INFORMATION, *PSYSTEM_PROCESSOR_IDLE_INFORMATION;

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

typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_EXTENDED_THREAD_INFORMATION {
    SYSTEM_THREAD_INFORMATION ThreadInfo;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID Win32StartAddress;
    PVOID TebBase;
    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;
} SYSTEM_EXTENDED_THREAD_INFORMATION, *PSYSTEM_EXTENDED_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER WorkingSetPrivateSize;
    ULONG HardFaultCount;
    ULONG NumberOfThreadsHighWatermark;
    ULONGLONG CycleTime;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    UINT_PTR UniqueProcessKey; // since VISTA (requires SystemExtendedProcessInformation)
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
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
//  SYSTEM_THREAD_INFORMATION Threads[ANYSIZE_ARRAY]; // When using SystemProcessInformation
//  SYSTEM_EXTENDED_THREAD_INFORMATION + SYSTEM_PROCESS_INFORMATION_EXTENSION; // When using SystemFullProcessInformation
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _PROCESS_DISK_COUNTERS {
    ULONGLONG BytesRead;
    ULONGLONG BytesWritten;
    ULONGLONG ReadOperationCount;
    ULONGLONG WriteOperationCount;
    ULONGLONG FlushOperationCount;
} PROCESS_DISK_COUNTERS, *PPROCESS_DISK_COUNTERS;

typedef union _ENERGY_STATE_DURATION {
    union {
        ULONGLONG Value;
        ULONG LastChangeTime;
    } DUMMYUNIONNAME;
    ULONG Duration : 31;
    ULONG IsInState : 1;
} ENERGY_STATE_DURATION, *PENERGY_STATE_DURATION;

typedef struct _PROCESS_ENERGY_VALUES {
    ULONGLONG Cycles[2][4];
    ULONGLONG DiskEnergy;
    ULONGLONG NetworkTailEnergy;
    ULONGLONG MBBTailEnergy;
    ULONGLONG NetworkTxRxBytes;
    ULONGLONG MBBTxRxBytes;
    union {
        ENERGY_STATE_DURATION Durations[3];
        struct {
            ENERGY_STATE_DURATION ForegroundDuration;
            ENERGY_STATE_DURATION DesktopVisibleDuration;
            ENERGY_STATE_DURATION PSMForegroundDuration;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    ULONG CompositionRendered;
    ULONG CompositionDirtyGenerated;
    ULONG CompositionDirtyPropagated;
    ULONG Reserved1;
    ULONGLONG AttributedCycles[4][2];
    ULONGLONG WorkOnBehalfCycles[4][2];
} PROCESS_ENERGY_VALUES, *PPROCESS_ENERGY_VALUES;

typedef struct _TIMELINE_BITMAP {
    ULONGLONG Value;
    ULONG EndTime;
    ULONG Bitmap;
} TIMELINE_BITMAP, *PTIMELINE_BITMAP;

typedef struct _PROCESS_ENERGY_VALUES_EXTENSION {
    union {
        TIMELINE_BITMAP Timelines[14]; // 9 for REDSTONE2, 14 for REDSTONE3
        struct {
            TIMELINE_BITMAP CpuTimeline;
            TIMELINE_BITMAP DiskTimeline;
            TIMELINE_BITMAP NetworkTimeline;
            TIMELINE_BITMAP MBBTimeline;
            TIMELINE_BITMAP ForegroundTimeline;
            TIMELINE_BITMAP DesktopVisibleTimeline;
            TIMELINE_BITMAP CompositionRenderedTimeline;
            TIMELINE_BITMAP CompositionDirtyGeneratedTimeline;
            TIMELINE_BITMAP CompositionDirtyPropagatedTimeline;
            // winver >= RS3
            TIMELINE_BITMAP InputTimeline;
            TIMELINE_BITMAP AudioInTimeline;
            TIMELINE_BITMAP AudioOutTimeline;
            TIMELINE_BITMAP DisplayRequiredTimeline;
            TIMELINE_BITMAP KeyboardInputTimeline;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    // winver >= RS3
    union {
        ENERGY_STATE_DURATION Durations[5];
        struct {
            ENERGY_STATE_DURATION InputDuration;
            ENERGY_STATE_DURATION AudioInDuration;
            ENERGY_STATE_DURATION AudioOutDuration;
            ENERGY_STATE_DURATION DisplayRequiredDuration;
            ENERGY_STATE_DURATION PSMBackgroundDuration;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME2;

    ULONG KeyboardInput;
    ULONG MouseInput;
} PROCESS_ENERGY_VALUES_EXTENSION, *PPROCESS_ENERGY_VALUES_EXTENSION;

typedef struct _PROCESS_EXTENDED_ENERGY_VALUES {
    PROCESS_ENERGY_VALUES Base;
    PROCESS_ENERGY_VALUES_EXTENSION Extension;
} PROCESS_EXTENDED_ENERGY_VALUES, *PPROCESS_EXTENDED_ENERGY_VALUES;

typedef enum _SYSTEM_PROCESS_CLASSIFICATION {
    SystemProcessClassificationNormal,
    SystemProcessClassificationSystem,
    SystemProcessClassificationSecureSystem,
    SystemProcessClassificationMemCompression,
    SystemProcessClassificationRegistry, // RS4
    SystemProcessClassificationMaximum
} SYSTEM_PROCESS_CLASSIFICATION;

typedef struct _SYSTEM_PROCESS_INFORMATION_EXTENSION {
    PROCESS_DISK_COUNTERS DiskCounters;
    ULONGLONG ContextSwitches;
    union {
        ULONG Flags;
        struct {
            ULONG HasStrongId : 1;
            ULONG Classification : 4; // SYSTEM_PROCESS_CLASSIFICATION
            ULONG BackgroundActivityModerated : 1;
            ULONG Spare : 26;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    ULONG UserSidOffset;
    // winver >= win10
    ULONG PackageFullNameOffset;
    PROCESS_ENERGY_VALUES EnergyValues;
    ULONG AppIdOffset;
    // winver >= th2
    SIZE_T SharedCommitCharge;
    // winver >= rs1
    ULONG JobObjectId;
    ULONG SpareUlong;
    ULONGLONG ProcessSequenceNumber;
} SYSTEM_PROCESS_INFORMATION_EXTENSION, *PSYSTEM_PROCESS_INFORMATION_EXTENSION;


typedef
NTSTATUS
(NTAPI *PNT_QUERY_SYSTEM_INFORMATION)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
//extern PNT_QUERY_SYSTEM_INFORMATION NtQuerySystemInfo;
NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_QUERY_INFORMATION_PROCESS)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef
NTSTATUS
(NTAPI *PNT_QUERY_INFORMATION_THREAD)(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;



//
// NT Syscall Routines.
//

NTSYSAPI
NTSTATUS
NTAPI
NtDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtMapViewOfSection(
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

NTSYSAPI
NTSTATUS
NTAPI
NtWaitForSingleObject(
    IN HANDLE ObjectHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER TimeOut OPTIONAL
    );


//
// Process Attributes
//

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

#define PS_ATTRIBUTE_NUMBER_MASK    0x0000ffff
#define PS_ATTRIBUTE_THREAD         0x00010000 // can be used with threads
#define PS_ATTRIBUTE_INPUT          0x00020000 // input only
#define PS_ATTRIBUTE_UNKNOWN        0x00040000

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

typedef struct _PS_ATTRIBUTE {
    ULONG Attribute;
    SIZE_T Size;
    union {
        ULONG Value;
        PVOID ValuePtr;
    };
    PSIZE_T ReturnLength;
} PS_ATTRIBUTE, *PPS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST {
    SIZE_T TotalLength;
    PS_ATTRIBUTE Attributes[ANYSIZE_ARRAY];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

//
// Thread creation flags
//
#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED        0x00000001
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH      0x00000002
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER      0x00000004
#define THREAD_CREATE_FLAGS_HAS_SECURITY_DESCRIPTOR 0x00000010
#define THREAD_CREATE_FLAGS_ACCESS_CHECK_IN_TARGET  0x00000020
#define THREAD_CREATE_FLAGS_INITIAL_THREAD          0x00000080

//
// NtCreateThreadEx Thread Start Routine
//

typedef
NTSTATUS
(NTAPI *PUSER_THREAD_START_ROUTINE)(
    IN PVOID Parameter
    );

//
// Process Protection Types and Signers
//
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

//
// Process Protection
//
typedef union _PS_PROTECTION {
    UCHAR Level;
    struct {
        UCHAR Type : 3;
        UCHAR Audit : 1;
        UCHAR Signer : 4;
    };
} PS_PROTECTION, *PPS_PROTECTION;


//
// Process Routines.
//

NTKERNELAPI
BOOLEAN
NTAPI
PsIsProtectedProcess(
    IN PEPROCESS Process
    );

NTKERNELAPI
CHAR*
NTAPI
PsGetProcessImageFileName(
    IN PEPROCESS Process
    );

NTKERNELAPI
PPEB
NTAPI
PsGetProcessPeb(
    IN PEPROCESS Process
    );

NTKERNELAPI
PVOID
NTAPI
PsGetProcessWow64Process(
    IN PEPROCESS Process
    );

NTKERNELAPI
PVOID
NTAPI
PsGetCurrentProcessWow64Process(
    VOID
    );

NTKERNELAPI
NTSTATUS
NTAPI
PsGetContextThread(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode
    );

NTKERNELAPI
NTSTATUS
NTAPI
PsResumeProcess(
    IN PEPROCESS Process
    );

NTKERNELAPI
NTSTATUS
NTAPI
PsSuspendProcess(
    IN PEPROCESS Process
    );

//
// The following routines are NOT exported!
//

typedef
PEPROCESS
(NTAPI *PPS_GET_NEXT_PROCESS)(
    IN PEPROCESS Process
    );
// E8 ? ? ? ? 48 85 C0 75 ? 4C 8D 43 ?
#define PS_GET_NEXT_PROCESS_SIGNATURE {                                                             \
/* loc_14060428A: */                                                                                \
    /*0xF0, 0x81, 0xA0, 0xCC, 0xCC, 0xCC, 0xCC,*/   /* lock and dword ptr [rax+2FCh], */            \
    /*0xFF, 0xBF, 0xFF, 0xFF*/                      /*                                0FFFFBFFFh */ \
    /*0x48, 0x8B, 0xC8,*/                           /* mov     rcx, rax */                          \
/* loc_14060428A: */                                                                                \
    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,                   /* call    PsGetNextProcess */                  \
    0x48, 0x85, 0xC0,                               /* test    rax, rax */                          \
    0x75, 0xCC,                                     /* jnz     short loc_14060428A */               \
    0x4C, 0x8D, 0x43 /*, 0xCC,*/                    /* lea     r8, [rbx+58h] */                     \
}
//static PPS_GET_NEXT_PROCESS PsGetNextProcess = NULL;
#define DEFINE_PS_GET_NEXT_PROCESS(name) \
    PPS_GET_NEXT_PROCESS name = NULL; \
    static const UCHAR NTSIG(name)[] = PS_GET_NEXT_PROCESS_SIGNATURE

typedef
ULONG
(NTAPI *PKE_SUSPEND_THREAD)(
    IN PETHREAD Thread
    );
// 65 48 8B 0C 25 88 01 00 00 E8 ? ? ? ? ? EB 02 00 C0 E9 ? ? ? ?
#define KE_SUSPEND_THREAD_SIGNATURE {                                                               \
    0x65, 0x48, 0x8B, 0x0C, 0x25, 0x88, 0x01, 0x00, 0x00,/* mov     rcx, [gs:188h] */               \
    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,                   /* call    KeSuspendThread */                   \
    0xCC, 0xEB, 0x02, 0x00, 0xC0                    /* mov     eax, 0C00002EBh */                   \
}
//static PKE_SUSPEND_THREAD KeSuspendThread = NULL;
#define DEFINE_KE_SUSPEND_THREAD(name) \
    PKE_SUSPEND_THREAD name = NULL; \
    static const UCHAR NTSIG(name)[] = KE_SUSPEND_THREAD_SIGNATURE

typedef
ULONG
(NTAPI *PKE_RESUME_THREAD)(
    IN PETHREAD Thread
    );
// 88 83 ? 02 00 00 75
#define KE_RESUME_THREAD_SIGNATURE {                                                                \
    0x88, 0x83, 0xCC, 0x02, 0x00, 0x00,             /* mov     [rbx+284h], al */                    \
    0x75 /*,0xCC */                                 /* jnz     short loc_14007A5FE */               \
}
//static PKE_RESUME_THREAD KeResumeThread = NULL;
#define DEFINE_KE_RESUME_THREAD(name) \
    PKE_RESUME_THREAD name = NULL; \
    static const UCHAR NTSIG(name)[] = KE_RESUME_THREAD_SIGNATURE

//
// Ke API definitions.
//

typedef enum _KAPC_ENVIRONMENT {
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InsertApcEnvironment
} KAPC_ENVIRONMENT, *PKAPC_ENVIRONMENT;

typedef
VOID
(NTAPI *PKNORMAL_ROUTINE)(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef
VOID
KKERNEL_ROUTINE(
    IN PRKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );
typedef KKERNEL_ROUTINE(NTAPI *PKKERNEL_ROUTINE);

typedef
VOID
(NTAPI *PKRUNDOWN_ROUTINE)(
    IN PRKAPC Apc
    );

NTKERNELAPI
VOID
NTAPI
KeInitializeApc(
    OUT PKAPC Apc,
    IN PETHREAD Thread,
    IN KAPC_ENVIRONMENT Environment,
    IN PKKERNEL_ROUTINE KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
    IN KPROCESSOR_MODE ApcMode OPTIONAL,
    IN PVOID NormalContext OPTIONAL
    );

NTKERNELAPI
BOOLEAN
NTAPI
KeInsertQueueApc(
    IN OUT PKAPC Apc,
    IN PVOID SystemArgument1 OPTIONAL,
    IN PVOID SystemArgument2 OPTIONAL,
    IN KPRIORITY Increment
    );

NTKERNELAPI
BOOLEAN
NTAPI
KeAlertThread(
    IN OUT PKTHREAD Thread,
    IN KPROCESSOR_MODE AlertMode
    );

NTKERNELAPI
BOOLEAN
NTAPI
KeTestAlertThread(
    IN KPROCESSOR_MODE AlertMode
    );


//
// Object Routines
//

NTKERNELAPI
NTSTATUS
NTAPI
ObReferenceObjectByName(
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE AccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PVOID ParseContext,
    OUT PVOID *Object
    );


//
// Runtime Library Structs and Routines.
//

typedef struct _RTL_BALANCED_NODE32 {
    union {
        ULONG Children[2]; // struct _RTL_BALANCED_NODE32 *
        struct {
            ULONG Left; // struct _RTL_BALANCED_NODE32 *
            ULONG Right; // struct _RTL_BALANCED_NODE32 *
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    union {
        UCHAR Red : 1;
        UCHAR Balance : 2;
        ULONG ParentValue; // ULONG_PTR
    } DUMMYUNIONNAME2;
} RTL_BALANCED_NODE32, *PRTL_BALANCED_NODE32;

NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PCUNICODE_STRING GuidString,
    OUT GUID* Guid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN const GUID* Guid,
    OUT PUNICODE_STRING GuidString
    );



//
// System service table descriptor.
//
// N.B. A system service number has a 12-bit service table offset and a
//      3-bit service table number.
//
// N.B. Descriptor table entries must be a power of 2 in size. Currently
//      this is 16 bytes on a 32-bit system and 32 bytes on a 64-bit
//      system.
//

#define NUMBER_SERVICE_TABLES 2
#define NUMBER_SERVICE_TABLES_1607 3 // Added in build 14393 was a new table called the KeServiceDescriptorTableFilter
#define SERVICE_NUMBER_MASK ((1 << 12) -  1)

#define SYSTEM_SERVICE_INDEX 0
#define WIN32K_SERVICE_INDEX 1
#define FILTER_SERVICE_INDEX 2

#define SERVICE_TABLE_SHIFT (12 - 5)
#define SERVICE_TABLE_MASK (((1 << 1) - 1) << 5)
#define SERVICE_TABLE_TEST (WIN32K_SERVICE_INDEX << 5)

#define SERVICE_OFFSET_SHIFT (4)

typedef struct _KSERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;
    PULONG Count;
    ULONG Limit;
    PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

#define SdBase 0x0000
#define SdLimit 0x0010
#define SdNumber 0x0018
#define SdLength 0x0020



//
//  Fusion/sxs thread state information
//

#define ACTIVATION_CONTEXT_STACK_FLAG_QUERIES_DISABLED (0x00000001)

//
// Activation Context
//

typedef struct _ACTIVATION_CONTEXT_DATA {
    ULONG Magic; //'xtcA'
    ULONG HeaderSize;
    ULONG FormatVersion;
    ULONG TotalSize;
    ULONG DefaultTocOffset;
    ULONG ExtendedTocOffset;
    ULONG AssemblyRosterOffset;
    ULONG Flags;
} ACTIVATION_CONTEXT_DATA, *PACTIVATION_CONTEXT_DATA;

typedef struct _ASSEMBLY_STORAGE_MAP_ENTRY {
    ULONG Flags;
    UNICODE_STRING DosPath;
    HANDLE Handle;
} ASSEMBLY_STORAGE_MAP_ENTRY, *PASSEMBLY_STORAGE_MAP_ENTRY;

typedef struct _ASSEMBLY_STORAGE_MAP {
    ULONG Flags;
    ULONG Count;
    ASSEMBLY_STORAGE_MAP_ENTRY *AssemblyArray[ANYSIZE_ARRAY];
} ASSEMBLY_STORAGE_MAP, *PASSEMBLY_STORAGE_MAP;

typedef struct _ACTIVATION_CONTEXT *PACTIVATION_CONTEXT;

typedef VOID(NTAPI *PACTIVATION_CONTEXT_NOTIFY_ROUTINE)(
    IN ULONG NotificationType,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN CONST PVOID ActivationContextData,
    IN PVOID NotificationContext,
    IN PVOID NotificationData,
    IN OUT PBOOLEAN DisableThisNotification
    );

typedef struct _ACTIVATION_CONTEXT {
    ULONG RefCount;
    ULONG Flags;
    LIST_ENTRY Links;
    ACTIVATION_CONTEXT_DATA *ActivationContextData;
    PACTIVATION_CONTEXT_NOTIFY_ROUTINE NotificationRoutine;
    PVOID NotificationContext;
    ULONG SendNotifications[4];
    ULONG DisabledNotifications[4];
    ASSEMBLY_STORAGE_MAP StorageMap;
    ASSEMBLY_STORAGE_MAP_ENTRY *InlineStorageMapEntries;
    ULONG StackTraceIndex;
    PVOID StackTraces[4][4];
} ACTIVATION_CONTEXT;




//
// Process Environment Block
//

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB_LDR_DATA32 {
    ULONG Length;
    BOOLEAN Initialized;
    ULONG SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    ULONG EntryInProgress;
    BOOLEAN ShutdownInProgress;
    ULONG ShutdownThreadId;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _PEB_LDR_DATA64 {
    ULONG Length;
    BOOLEAN Initialized;
    ULONGLONG SsHandle;
    LIST_ENTRY64 InLoadOrderModuleList;
    LIST_ENTRY64 InMemoryOrderModuleList;
    LIST_ENTRY64 InInitializationOrderModuleList;
    ULONGLONG EntryInProgress;
    BOOLEAN ShutdownInProgress;
    ULONGLONG ShutdownThreadId;
} PEB_LDR_DATA64, *PPEB_LDR_DATA64;

//
// Handle tag bits for Peb Stdio File Handles
//

#define PEB_STDIO_HANDLE_NATIVE     0
#define PEB_STDIO_HANDLE_SUBSYS     1
#define PEB_STDIO_HANDLE_PM         2
#define PEB_STDIO_HANDLE_RESERVED   3

#define GDI_HANDLE_BUFFER_SIZE32  34
#define GDI_HANDLE_BUFFER_SIZE64  60

#if !(defined(_M_AMD64) || defined(__x86_64__))
#define GDI_HANDLE_BUFFER_SIZE      GDI_HANDLE_BUFFER_SIZE32
#else
#define GDI_HANDLE_BUFFER_SIZE      GDI_HANDLE_BUFFER_SIZE64
#endif

typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];
typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];
typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];

#define FOREGROUND_BASE_PRIORITY  9
#define NORMAL_BASE_PRIORITY      8

typedef struct _PEB_FREE_BLOCK {
    struct _PEB_FREE_BLOCK *Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;


#if !defined(CLIENT_ID64_DEFINED)

typedef struct _CLIENT_ID64 {
    ULONGLONG UniqueProcess;
    ULONGLONG UniqueThread;
} CLIENT_ID64;
typedef CLIENT_ID64 *PCLIENT_ID64;

typedef struct _CLIENT_ID32 {
    ULONG UniqueProcess;
    ULONG UniqueThread;
} CLIENT_ID32;
typedef CLIENT_ID32 *PCLIENT_ID32;

#define CLIENT_ID64_DEFINED
#endif

#define FLS_MAXIMUM_AVAILABLE 128
#define TLS_MINIMUM_AVAILABLE 64
#define TLS_EXPANSION_SLOTS   1024

typedef
VOID
(*PPS_POST_PROCESS_INIT_ROUTINE) (
    VOID
    );


//
// Thread Environment Block (and portable part of Thread Information Block)
//

//
//  NT_TIB - Thread Information Block - Portable part.
//
//      This is the subsystem portable part of the Thread Information Block.
//      It appears as the first part of the TEB for all threads which have
//      a user mode component.
//
//      This structure MUST MATCH OS/2 V2.0!
//
//      There is another, non-portable part of the TIB which is used
//      for by subsystems, i.e. Os2Tib for OS/2 threads.  SubSystemTib
//      points there.
//
//

#if !defined(_NTDDK_)

typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        ULONG Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;

//
// 32 and 64 bit specific version for wow64 and the debugger
//
typedef struct _NT_TIB32 {
    ULONG ExceptionList;
    ULONG StackBase;
    ULONG StackLimit;
    ULONG SubSystemTib;
    union {
        ULONG FiberData;
        ULONG Version;
    };
    ULONG ArbitraryUserPointer;
    ULONG Self;
} NT_TIB32, *PNT_TIB32;

typedef struct _NT_TIB64 {
    ULONGLONG ExceptionList;
    ULONGLONG StackBase;
    ULONGLONG StackLimit;
    ULONGLONG SubSystemTib;
    union {
        ULONGLONG FiberData;
        ULONG Version;
    };
    ULONGLONG ArbitraryUserPointer;
    ULONGLONG Self;
} NT_TIB64, *PNT_TIB64;

#endif // !_NTDDK_


//
// Gdi command batching
//

#define GDI_BATCH_BUFFER_SIZE 310

typedef struct _GDI_TEB_BATCH {
    ULONG Offset;
    ULONG_PTR HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

typedef struct _GDI_TEB_BATCH64 {
    ULONG Offset;
    ULONGLONG HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH64, *PGDI_TEB_BATCH64;

typedef struct _GDI_TEB_BATCH32 {
    ULONG Offset;
    ULONG HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH32, *PGDI_TEB_BATCH32;

//
// Wx86 thread state information
//

typedef struct _Wx86ThreadState {
    ULONG  *CallBx86Eip;
    PVOID   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    CHAR    OleStubInvoked;
} WX86THREAD, *PWX86THREAD;

//
// WoW64 Process
//

typedef struct _WOW64_PROCESS {
    PVOID Wow64;
} WOW64_PROCESS, *PWOW64_PROCESS;

//
//  TEB - The thread environment block
//

#define STATIC_UNICODE_BUFFER_LENGTH 261
#define WIN32_CLIENT_INFO_LENGTH 62

#define WIN32_CLIENT_INFO_SPIN_COUNT 1

typedef PVOID* PPVOID;

#include "pebteb.h"

#define TYPE32(x)   ULONG
#define TYPE64(x)   ULONGLONG

#define PEBTEB_BITS 32

#include "pebteb.h"

#undef PEBTEB_BITS

#define PEBTEB_BITS 64

#include "pebteb.h"

#undef PEBTEB_BITS


typedef struct _INITIAL_TEB {
    struct {
        PVOID OldStackBase;
        PVOID OldStackLimit;
    } OldInitialTeb;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID StackAllocationBase;
} INITIAL_TEB, *PINITIAL_TEB;


//
//  Define the size of the 80387 save area, which is in the context frame.
//
#define SIZE_OF_80387_REGISTERS      80

typedef struct _FLOATING_SAVE_AREA32 {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[SIZE_OF_80387_REGISTERS];
    ULONG   Spare0;
} FLOATING_SAVE_AREA32, *PFLOATING_SAVE_AREA32;

#ifndef MAXIMUM_SUPPORTED_EXTENSION
#define MAXIMUM_SUPPORTED_EXTENSION     512
#if !defined(__midl) && !defined(MIDL_PASS)
C_ASSERT(sizeof(XSAVE_FORMAT) == MAXIMUM_SUPPORTED_EXTENSION);
#endif
#endif

#ifndef CONTEXT_i386
#define CONTEXT_i386    0x00010000L    // this assumes that i386 and
#define CONTEXT_i486    0x00010000L    // i486 have identical context records
#endif

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) is is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//  The layout of the record conforms to a standard call frame.
//
#if defined(_MSC_VER)
#pragma pack(push,4)
#endif
typedef struct _CONTEXT32 {

    //
    // The flags values within this flag control the contents of
    // a CONTEXT record.
    //
    // If the context record is used as an input parameter, then
    // for each portion of the context record controlled by a flag
    // whose value is set, it is assumed that that portion of the
    // context record contains valid context. If the context record
    // is being used to modify a threads context, then only that
    // portion of the threads context will be modified.
    //
    // If the context record is used as an IN OUT parameter to capture
    // the context of a thread, then only those portions of the thread's
    // context corresponding to set flags will be returned.
    //
    // The context record is never used as an OUT only parameter.
    //

    ULONG ContextFlags;

    //
    // This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
    // set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
    // included in CONTEXT_FULL.
    //

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
    //

    FLOATING_SAVE_AREA32 FloatSave;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_SEGMENTS.
    //

    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_INTEGER.
    //

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_CONTROL.
    //

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;              // MUST BE SANITIZED
    ULONG   EFlags;             // MUST BE SANITIZED
    ULONG   Esp;
    ULONG   SegSs;

    //
    // This section is specified/returned if the ContextFlags word
    // contains the flag CONTEXT_EXTENDED_REGISTERS.
    // The format and contexts are processor specific
    //

    UCHAR    ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} CONTEXT32, *PCONTEXT32;
#if defined(_MSC_VER)
#pragma pack(pop)
#endif



#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // _ODIN_NTAPI_H_