/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file dispatch.c
 * @author Aidan Khoury (ajkhoury)
 * @date 10/24/2018
 */

#include "dispatch.h"

#include "log.h"

//
// The IOCTL handler routine typedef
//

typedef
NTSTATUS
BO_IOCTL_FUNCTION(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );
typedef BO_IOCTL_FUNCTION *PBO_IOCTL_FUNCTION;

//
// Forward declarations
//

static BO_IOCTL_FUNCTION BoIoctlDummy;
static BO_IOCTL_FUNCTION BoIoctlGetVersion;
static BO_IOCTL_FUNCTION BoIoctlGetNtBuildLab;
static BO_IOCTL_FUNCTION BoIoctlSetTargetPath;
static BO_IOCTL_FUNCTION BoIoctlSetHookModuleData;
static BO_IOCTL_FUNCTION BoIoctlSetHookModulePath;

#if defined(ENABLE_IOCTLS)

//
// IOCTL Handlers
//

static PBO_IOCTL_FUNCTION BoIoctlFunctions[BO_FUNCTION_MAX] = {
    [BO_FUNCTION_GET_VERSION] = BoIoctlGetVersion,
    [BO_FUNCTION_GET_NT_BUILD_LAB] = BoIoctlGetNtBuildLab,
    [BO_FUNCTION_SET_TARGET_PATH] = BoIoctlSetTargetPath,
    [BO_FUNCTION_SET_HOOK_MODULE_DATA] = BoIoctlSetHookModuleData,
    [BO_FUNCTION_SET_HOOK_MODULE_PATH] = BoIoctlSetHookModulePath,
};

static
NTSTATUS
BoIoctlGetVersion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
)
{
    NTSTATUS Status;
    UNICODE_STRING VersionUnicode;
    ULONG EffectiveLength;

    UNREFERENCED_PARAMETER(DeviceObject);

    RtlInitUnicodeString(&VersionUnicode, BO_DRIVER_VERSION);

    EffectiveLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    if (EffectiveLength >= (ULONG)VersionUnicode.Length) {
        EffectiveLength = VersionUnicode.Length;
    }

    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, VersionUnicode.Buffer, EffectiveLength);
    Irp->IoStatus.Information = EffectiveLength;

    Status = STATUS_SUCCESS;

    LOG_INFO("BO_IOCTL_GET_VERSION: exiting - status %08x", Status);

    return Status;
}

static
NTSTATUS
BoIoctlGetNtBuildLab(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
)
{
    NTSTATUS Status;
    ANSI_STRING BuildLabString;
    ULONG EffectiveLength;

    UNREFERENCED_PARAMETER(DeviceObject);

    RtlInitAnsiString(&BuildLabString, BoRuntimeData.KernelBuildLab);

    EffectiveLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    if (EffectiveLength >= (ULONG)BuildLabString.Length) {
        EffectiveLength = BuildLabString.Length;
    }

    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, BuildLabString.Buffer, EffectiveLength);
    Irp->IoStatus.Information = EffectiveLength;

    Status = STATUS_SUCCESS;

    LOG_INFO("BO_IOCTL_GET_NT_BUILD_LAB: exiting - status %08x", Status);

    return Status;
}

static
NTSTATUS
BoIoctlSetTargetPath(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
)
{
    NTSTATUS Status;
    ULONG EffectiveLength;

    UNREFERENCED_PARAMETER(DeviceObject);

    if (Irp->AssociatedIrp.SystemBuffer &&
        IrpSp->Parameters.DeviceIoControl.InputBufferLength > 0) {

        EffectiveLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (EffectiveLength >= sizeof(BoRuntimeData.TargetPath)) {
            EffectiveLength = sizeof(BoRuntimeData.TargetPath);
            Status = STATUS_BUFFER_OVERFLOW;
        } else {
            Status = STATUS_SUCCESS;
        }

        ExAcquireFastMutex(&BoRuntimeData.Lock);

        RtlZeroMemory(BoRuntimeData.TargetPath, sizeof(BoRuntimeData.TargetPath));
        RtlCopyMemory(BoRuntimeData.TargetPath, Irp->AssociatedIrp.SystemBuffer, EffectiveLength);

        ExReleaseFastMutex(&BoRuntimeData.Lock);

    } else {
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }

    LOG_INFO("BO_IOCTL_SET_TARGET_PATH: exiting - status %08x", Status);

    return Status;
}

static
NTSTATUS
BoIoctlSetHookModuleData(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
)
{
    NTSTATUS Status;
    ULONG InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    UNREFERENCED_PARAMETER(DeviceObject);

    if (Irp->AssociatedIrp.SystemBuffer && InputBufferLength > 0) {

        ExAcquireFastMutex(&BoRuntimeData.Lock);

        //
        // Free previously allocated encrypted hook buffer.
        //
        if (BoRuntimeData.EncryptedHookBuffer) {
            RtlZeroMemory(BoRuntimeData.EncryptedHookBuffer, BoRuntimeData.EncryptedHookSize);
            MmFreeNonPaged(BoRuntimeData.EncryptedHookBuffer);
        }

        //
        // Allocate new non-paged pool for the hook payload buffer.
        //
        BoRuntimeData.EncryptedHookBuffer = MmAllocateNonPaged(InputBufferLength);
        if (BoRuntimeData.EncryptedHookBuffer) {

            BoRuntimeData.EncryptedHookSize = InputBufferLength;
            RtlCopyMemory(BoRuntimeData.EncryptedHookBuffer,
                          Irp->AssociatedIrp.SystemBuffer,
                          BoRuntimeData.EncryptedHookSize
            );

            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        ExReleaseFastMutex(&BoRuntimeData.Lock);

    } else {
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }

    LOG_INFO("BO_IOCTL_SET_HOOK_MODULE_DATA: exiting - status %08x", Status);

    return Status;
}

static
NTSTATUS
BoIoctlSetHookModulePath(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
)
{
    NTSTATUS Status;
    ULONG EffectiveLength;

    UNREFERENCED_PARAMETER(DeviceObject);

    if (Irp->AssociatedIrp.SystemBuffer && IrpSp->Parameters.DeviceIoControl.InputBufferLength > 0) {

        EffectiveLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
        if (EffectiveLength >= sizeof(BoRuntimeData.HookPath)) {
            EffectiveLength = sizeof(BoRuntimeData.HookPath);
            Status = STATUS_BUFFER_OVERFLOW;
        } else {
            Status = STATUS_SUCCESS;
        }
        RtlZeroMemory(BoRuntimeData.HookPath, sizeof(BoRuntimeData.HookPath));
        RtlCopyMemory(BoRuntimeData.HookPath, Irp->AssociatedIrp.SystemBuffer, EffectiveLength);

    } else {
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }

    LOG_INFO("BO_IOCTL_SET_HOOK_MODULE_PATH: exiting - status %08x", Status);

    return Status;
}


//
// IRP handlers
//

NTSTATUS
BoDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    ULONG IoControlCode;
    ULONG FunctionCode;

    UNREFERENCED_PARAMETER(DeviceObject);

    //
    // Get the irp stack location and ioctl code.
    //
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    FunctionCode = (ULONG)((IoControlCode & 0x1FFC) >> 2);

    LOG_INFO("ioctl code %08x (fn %08x)", FunctionCode);

    //
    // Call the IOCTL function handler.
    //
    if (FunctionCode < BO_FUNCTION_MAX) {

        Status = BoIoctlFunctions[FunctionCode](DeviceObject, Irp, IrpSp);

    } else {

        Status = STATUS_IO_DEVICE_ERROR;
        LOG_ERROR("Unexpected ioctl code %08x", IoControlCode);
    }

    //
    // Complete the irp and return.
    //
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
BoDeviceCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
BoDeviceClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
BoDeviceCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

#else

NTSTATUS
BoDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}


NTSTATUS
BoDeviceCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
BoDeviceClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
BoDeviceCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

#endif // ENABLE_IOCTLS