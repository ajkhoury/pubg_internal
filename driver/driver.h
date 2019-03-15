/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file driver.c
 * @author Aidan Khoury (ajkhoury)
 * @date 9/4/2018
 */
#ifndef _BLACKOUT_DRIVER_H_
#define _BLACKOUT_DRIVER_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"
#include "shared.h"

//
// Define macros for VMP3 virtualization.
//

#if defined(ENABLE_VMP)
#include <vendor/vmp/VMProtectDDK.h>
#define VM_START(x) VMProtectBeginVirtualization(x)
#define VM_END()    VMProtectEnd()
#else
#define VM_START(x) /* nothing */
#define VM_END()    /* nothing */
#endif


//
// OD_DBG_BREAK
//
// Sets a break point that works only when a debugger is present.
//

#if !defined(BO_DBG_BREAK)
#define BO_DBG_BREAK()              \
  if (KD_DEBUGGER_NOT_PRESENT) {    \
  } else {                          \
    __debugbreak();                 \
  }                                 \
  (void*)(0)
#endif

//
// BO_ASSERT
//
// This macro is identical to NT_ASSERT but works in fre builds as well.
//
// It is used for error checking in the driver in cases where
// we can't easily report the error to the user mode app, or the
// error is so severe that we should break in immediately to
// investigate.
//
// It's better than DbgBreakPoint because it provides additional info
// that can be dumped with .exr -1, and individual asserts can be disabled
// from kd using 'ahi' command.
//

#define BO_ASSERT(_exp) \
    ((!(_exp)) ? \
     (__annotation(L"Debug", L"AssertFail", L#_exp), \
      DbgRaiseAssertionFailure(), FALSE) : \
     TRUE)

//
// Define global runtime data structure.
//

typedef struct _DRIVER_RUNTIME_DATA {
    FAST_MUTEX Lock;
    BOOLEAN Initialized;
    BOOLEAN SymLinkCreated;
    PVOID DriverBase;
    SIZE_T DriverSize;
    PDRIVER_OBJECT DriverObject;
    PDEVICE_OBJECT DeviceObject;
    PVOID KernelBase;
    SIZE_T KernelSize;
    CHAR KernelBuildLab[224];
    USHORT KernelBuild;
    BOOLEAN IsWindows7;
    HANDLE ServiceProcessId;
    HANDLE TargetProcessId;
    WCHAR TargetProcess[BO_MAXIMUM_FILENAME_LENGTH];
    WCHAR HookPath[BO_MAXIMUM_FILENAME_LENGTH];
    PVOID EncryptedHookBuffer;
    SIZE_T EncryptedHookSize;
    HANDLE ManualMapThreadHandle;
    NTSTATUS ManualMapResultStatus;
    HANDLE HookModule;
} DRIVER_RUNTIME_DATA, *PDRIVER_RUNTIME_DATA;

//
// Blackout driver global runtime data structure.
//

extern DRIVER_RUNTIME_DATA BoRuntimeData;

#define RUNTIME_INIT { \
    {0},                    /* Lock */ \
    FALSE,                  /* Initialized */ \
    FALSE,                  /* SymLinkCreated */ \
    NULL,                   /* DriverBase */ \
    0,                      /* DriverSize */ \
    NULL,                   /* DriverObject */ \
    NULL,                   /* DeviceObject */ \
    NULL,                   /* KernelBase */ \
    0,                      /* KernelSize */ \
    {0},                    /* KernelBuildLab */ \
    0,                      /* KernelBuild */ \
    FALSE,                  /* IsWindows7 */ \
    NULL,                   /* ServiceProcessId */ \
    NULL,                   /* TargetProcessId */ \
    {0},                    /* TargetProcess */ \
    {0},                    /* HookPath */ \
    NULL,                   /* EncryptedHookBuffer */ \
    0,                      /* EncryptedHookSize */ \
    NULL,                   /* ManualMapThreadHandle */ \
    0,                      /* ManualMapResultStatus */ \
    NULL,                   /* HookModule */ \
}


#endif // _BLACKOUT_DRIVER_H_