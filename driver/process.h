/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file process.h
 * @author Aidan Khoury (ajkhoury)
 * @date 8/27/2018  - Initial implementation
 */

#ifndef _BLACKOUT_DRIVER_PROCESS_H_
#define _BLACKOUT_DRIVER_PROCESS_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

// Process API calling convention 
#define PSAPI   NTAPI

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201) // nonstandard extension used: nameless struct/union
#endif // _MSC_VER


//
// Public Process API
//

NTSTATUS
PSAPI
PsInitialize(
    IN PVOID KernelBase
    );

#define PsGetPreviousMode() ExGetPreviousMode()

KPROCESSOR_MODE
PSAPI
PsSetPreviousMode(
    IN KPROCESSOR_MODE NewMode
    );

#define PS_PREVIOUS_MODE_KERNEL(old) \
    *(old) = PsSetPreviousMode(KernelMode)

#define PS_PREVIOUS_MODE_USER(old) \
    *(old) = PsSetPreviousMode(UserMode)

#define PS_PREVIOUS_MODE_RESTORE(old) \
    if ((old) != PsGetPreviousMode()) (void)PsSetPreviousMode((old))


NTSTATUS
PSAPI
PsOpenProcessHandle(
    IN PEPROCESS Process,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ProcessHandle
    );

NTSTATUS
PSAPI
PsCloseProcessHandle(
    IN HANDLE ProcessHandle
    );

NTSTATUS
PSAPI
PsCloseThreadHandle(
    IN HANDLE ThreadHandle
    );

NTSTATUS
PSAPI
PsGetProcessFullImageName(
    IN PEPROCESS Process,
    OUT WCHAR *ResultImageName,
    IN USHORT MaxImageNameLength,
    OUT USHORT *ImageNameLengthInBytes OPTIONAL
    );

NTSTATUS
PSAPI
PsGetProcessFullImageNameByProcessId(
    IN HANDLE ProcessId,
    OUT WCHAR *ResultImageName,
    IN USHORT MaxImageNameLength,
    OUT USHORT *ImageNameLengthInBytes OPTIONAL
    );

NTSTATUS
PSAPI
PsQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
PSAPI
PsAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSTATUS
PSAPI
PsFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    );

NTSTATUS
PSAPI
PsProtectVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T NumberOfBytesToProtect,
    IN ULONG NewAccessProtection,
    OUT PULONG OldAccessProtection
    );

NTSTATUS
PSAPI
PsCreateThreadEx(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN HANDLE ProcessHandle,
    IN PUSER_THREAD_START_ROUTINE StartRoutine,
    IN PVOID Parameter,
    IN ULONG Flags,
    IN SIZE_T StackZeroBits,
    IN SIZE_T SizeOfStackCommit,
    IN SIZE_T SizeOfStackReserve,
    IN OUT PPS_ATTRIBUTE_LIST AttributeList
    );

NTSTATUS
PSAPI
PsGetThreadContext(
    IN PETHREAD Thread,
    IN OUT PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
PSAPI
PsSetThreadContext(
    IN PETHREAD Thread,
    IN PCONTEXT ThreadContext,
    IN KPROCESSOR_MODE PreviousMode
    );

NTSTATUS
PSAPI
PsSuspendThread(
    IN PETHREAD Thread,
    OUT PULONG SuspendCount OPTIONAL
    );

NTSTATUS
PSAPI
PsResumeThread(
    IN PETHREAD Thread,
    OUT PULONG SuspendCount OPTIONAL
    );

//
// Disable or enable process protections.
//
// N.B. This subroutine uses a bunch of undocumented windows specific
//      values that need to be carefully monitored upon updates!
//
NTSTATUS
PSAPI
PsSetProcessProtection(
    IN HANDLE ProcessId,
    IN ULONG ProtectDisableOptions,
    IN ULONG ProtectEnableOptions
    );
#define PS_PROTECT_POLICY_PROTECTION 0x01
#define PS_PROTECT_POLICY_DYNAMICODE 0x02
#define PS_PROTECT_POLICY_SIGNATURE 0x04

NTSTATUS
PSAPI
PsFindHijackableThread(
    IN PEPROCESS Process,
    IN BOOLEAN AcceptMainThread,
    OUT PHANDLE HijackableThreadId
    );

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // _MSC_VER
#endif // _BLACKOUT_DRIVER_PROCESS_H_
