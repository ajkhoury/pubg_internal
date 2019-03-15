/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file inject.h
 * @author Aidan Khoury (ajkhoury)
 * @date 2/19/2019
 */

#ifndef _BLACKOUT_DRIVER_INJECT_H_
#define _BLACKOUT_DRIVER_INJECT_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

// Inject API calling convention
#define INJAPI NTAPI

// Opaque Injection Handle
typedef PVOID HINJECTION, *PHINJECTION;


NTSTATUS
INJAPI
InjInitialize(
    VOID
    );

VOID
INJAPI
InjDestroy(
    VOID
    );

NTSTATUS
INJAPI
InjRegisterInjection(
    OUT PHINJECTION Handle,
    IN PVOID SourceImageBase OPTIONAL,
    IN PWCHAR SourceImagePath,
    IN HANDLE TargetProcessId OPTIONAL,
    IN PWCHAR TargetProcessName
    );

NTSTATUS
INJAPI
InjQueueApc(
    IN PETHREAD Thread,
    IN KPROCESSOR_MODE ApcMode,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
INJAPI
InjForceApc(
    IN PETHREAD Thread,
    IN KPROCESSOR_MODE ApcMode,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#endif // _BLACKOUT_DRIVER_INJECT_H_