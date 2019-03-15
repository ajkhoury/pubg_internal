/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file shared.h
 * @author Aidan Khoury (ajkhoury)
 * @date 8/30/2018
 *
 * @brief Shared header between user mode and kernel mode components.
 */

#ifndef _BLACKOUT_SHARED_H_
#define _BLACKOUT_SHARED_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include <wdm.h>

#if (_MSC_VER > 1000)
#pragma warning(push)
#pragma warning(disable:4214) // bit field types other than int
#pragma warning(disable:4201) // nameless struct/union
#endif

//
// Driver Version
//

#ifndef BO_DRIVER_VERSION
#define BO_DRIVER_VERSION L"Development v1.0"
#endif // OD_DRIVER_VERSION

//
// Driver and device names
// It is important to change the names of the binaries
// in the sample code to be unique for your own use.
//

#define BO_DRIVER_NAME L"BlackoutDriver"
#define BO_DRIVER_NAME_WITH_EXT L"BlackoutDriver.sys"
#define BO_TEST_DRIVER_NAME L"BlackoutDriverTest"
#define BO_TEST_DRIVER_NAME_WITH_EXT L"BlackoutDriverTest.sys"
#define BO_SERVICE_NAME L"BlackoutService"
#define BO_SERVICE_NAME_WITH_EXT L"BlackoutService.exe"

#define BO_NT_DEVICE_NAME L"\\Device\\BlackoutDriver"
#define BO_DOS_DEVICE_NAME L"\\DosDevices\\BlackoutDriver"
#define BO_WIN32_DEVICE_NAME L"\\\\.\\BlackoutDriver"

//
// MAXIMUM_FILENAME_LENGTH is only defined in wdm.h
//
// Define our own equivalent definition for use in usermode.
//

#define BO_MAXIMUM_FILENAME_LENGTH 256
#if defined(_WDM_)
C_ASSERT(BO_MAXIMUM_FILENAME_LENGTH == MAXIMUM_FILENAME_LENGTH);
#endif

//
// IOCTLs exposed by the driver.
//
// Function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for vendor. Note that the vendor-assigned
// values set the Custom bit by being in the range of 2048-4095.
//

#define BO_DEFINE_IOCTL(Function) CTL_CODE( \
    FILE_DEVICE_UNKNOWN, \
    0x800 | Function, \
    METHOD_BUFFERED, \
    FILE_SPECIAL_ACCESS \
    )

#define BO_FUNCTION_GET_VERSION             0
#define BO_FUNCTION_GET_NT_BUILD_LAB        1
#define BO_FUNCTION_SET_TARGET_PATH         2
#define BO_FUNCTION_SET_HOOK_MODULE_DATA    3
#define BO_FUNCTION_SET_HOOK_MODULE_PATH    4
#define BO_FUNCTION_MAX                     5

#define BO_IOCTL_GET_VERSION            BO_DEFINE_IOCTL(BO_FUNCTION_GET_VERSION)
#define BO_IOCTL_GET_NT_BUILD_LAB       BO_DEFINE_IOCTL(BO_FUNCTION_GET_NT_BUILD_LAB)
#define BO_IOCTL_SET_TARGET_PATH        BO_DEFINE_IOCTL(BO_FUNCTION_SET_TARGET_PATH)
#define BO_IOCTL_SET_HOOK_MODULE_DATA   BO_DEFINE_IOCTL(BO_FUNCTION_SET_HOOK_MODULE_DATA)
#define BO_IOCTL_SET_HOOK_MODULE_PATH   BO_DEFINE_IOCTL(BO_FUNCTION_SET_HOOK_MODULE_PATH)


typedef struct _BO_OBJECT_ACCESS_ENTRY {
    HANDLE SourceProcessId;
    HANDLE TargetProcessId;
    BOOLEAN Whitelisted;
    ACCESS_MASK AccessMask;
    UCHAR AccessObjectType;  // 1,2 process, 3,4 thread
    USHORT RepeatAttempts;
    LARGE_INTEGER RepeatAttemptsTime;
    BOOLEAN RedFlagged; // Indicates if this access is flagged for too many repeat attempts.
    WCHAR SourceFile[BO_MAXIMUM_FILENAME_LENGTH];
} BO_OBJECT_ACCESS_ENTRY, *POD_OBJECT_ACCESS_ENTRY;

typedef struct _BO_OBJECT_ACCESS_LIST {
    ULONG EntryCount;
    BO_OBJECT_ACCESS_ENTRY AccessEntries[ANYSIZE_ARRAY];
} BO_OBJECT_ACCESS_LIST, *PBO_OBJECT_ACCESS_LIST;





#if (_MSC_VER > 1000)
#pragma warning(pop)
#endif
#endif // _BLACKOUT_SHARED_H_