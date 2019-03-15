/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file dispatch.h
 * @author Aidan Khoury (ajkhoury)
 * @date 12/12/2018
 */

#ifndef _BLACKOUT_DRIVER_IOCTL_H_
#define _BLACKOUT_DRIVER_IOCTL_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "driver.h"

_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH BoDeviceCreate; // This function handles the 'create' irp.
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH BoDeviceClose; // This function handles the 'close' irp.
_Dispatch_type_(IRP_MJ_CLEANUP) DRIVER_DISPATCH BoDeviceCleanup; // This function handles the 'cleanup' irp.
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH BoDeviceControl; // This function handles the 'ioctl' irp.

#endif // _BLACKOUT_DRIVER_IOCTL_H_