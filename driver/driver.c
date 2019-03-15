/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file driver.c
 * @author Aidan Khoury (ajkhoury)
 * @date 2/16/2019
 */

#include "driver.h"
#include "log.h"

#include "mm.h"
#include "crypto.h"
#include "util.h"
#include "file.h"
#include "process.h"
#include "loader.h"
#include "inject.h"

//
// Global driver runtime data.
//

DRIVER_RUNTIME_DATA BoRuntimeData = RUNTIME_INIT;

//
// Function declarations
//

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD BoDriverUnload;
#if defined(ENABLE_IOCTLS)
#include "dispatch.h"
#endif // ENABLE_IOCTLS


// Test if the system is one of supported OS versions
inline
BOOLEAN
BoIsSupportedOs(
    VOID
)
{
    NTSTATUS Status;
    RTL_OSVERSIONINFOEXW VersionInfo;

    RtlZeroMemory(&VersionInfo, sizeof(RTL_OSVERSIONINFOEXW));
    VersionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    Status = RtlGetVersion((PRTL_OSVERSIONINFOW)&VersionInfo);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

#if defined(DBG)
    LOG_INFO("dwMajorVersion = %u", VersionInfo.dwMajorVersion);
    LOG_INFO("dwMinorVersion = %u", VersionInfo.dwMinorVersion);
    LOG_INFO("dwBuildNumber = %u", VersionInfo.dwBuildNumber);
    LOG_INFO("dwPlatformId = %u", VersionInfo.dwPlatformId);
    LOG_INFO("szCSDVersion = %ws", VersionInfo.szCSDVersion);
    LOG_INFO("wServicePackMajor = %u", VersionInfo.wServicePackMajor);
    LOG_INFO("wServicePackMinor = %u", VersionInfo.wServicePackMinor);
    LOG_INFO("wSuiteMask = %u", VersionInfo.wSuiteMask);
    LOG_INFO("wProductType = %u", VersionInfo.wProductType);
    LOG_INFO("wReserved = %u", VersionInfo.wReserved);
#endif // DBG

    if (VersionInfo.dwBuildNumber > NT_BUILD_10_BUILD_18348) {
        LOG_ERROR("OS version %d is not supported!", VersionInfo.dwBuildNumber);
        return FALSE;
    }

    return TRUE;
}

static HINJECTION BoInjectionHandle = NULL;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status;
    ULONG LogLevel;
    USHORT NtKernelBuild;
    CONST CHAR* NtKernelBuildLab;
    PVOID NtKernelBase;
    ULONG NtKernelSize;
    VM_START("000");
    UNICODE_STRING NtDeviceName = RTL_CONSTANT_STRING(BO_NT_DEVICE_NAME);
    UNICODE_STRING DosDeviceName = RTL_CONSTANT_STRING(BO_DOS_DEVICE_NAME);
    VM_END();
    PDEVICE_OBJECT DeviceObject = NULL;

    UNREFERENCED_PARAMETER(RegistryPath);

    //
    // Make sure we are not running in safe mode!
    //
    if (*InitSafeBootMode != 0) {
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Zero out and intialize runtime data fields.
    //
    RtlZeroMemory(&BoRuntimeData, sizeof(DRIVER_RUNTIME_DATA));
    ExInitializeFastMutex(&BoRuntimeData.Lock);
    BoRuntimeData.DriverObject = DriverObject;
    BoRuntimeData.DriverBase = DriverObject->DriverStart;
    BoRuntimeData.DriverSize = DriverObject->DriverSize;
    BoRuntimeData.ManualMapResultStatus = STATUS_MORE_PROCESSING_REQUIRED;

    //
    // Initialize logger interface.
    //
#if defined(DBG)
    LogLevel = LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend;
#else
    LogLevel = LogPutLevelInfo | LogOptDisableFunctionName | LogOptDisableAppend;
#endif

#if defined(ENABLE_LOG_FILE)
    Status = LogInitialize(LogLevel, L"\\??\\C:\\Log.log");
#else
    Status = LogInitialize(LogLevel, NULL);
#endif

    if (!NT_SUCCESS(Status)) {
        VM_START("001");
        DBGPRINT("Failed to initialize logging interface with status 0x%08x", Status);
        VM_END();
        return Status;
    }

    BO_DBG_BREAK();

    //
    // Check that this OS version is supported.
    //
    if (!BoIsSupportedOs()) {
        LogDestroy();
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Locate kernel base, and grab the size.
    //
    NtKernelBase = UtlGetNtKernelBase(&NtKernelSize);
    if (!NtKernelBase) {

        LOG_ERROR("Failed to find NT kernel base!");

        Status = STATUS_NOT_FOUND;
        goto Exit;
    }

    LOG_INFO("Kernel Base = 0x%llx", NtKernelBase);
    LOG_INFO("Kernel Size = 0x%x", NtKernelSize);

    BoRuntimeData.KernelBase = NtKernelBase;
    BoRuntimeData.KernelSize = NtKernelSize;

    //
    // Get NT build number.
    //
    NtKernelBuild = UtlGetNtKernelBuild(NULL);
    if (!NtKernelBuild) {
        LOG_ERROR("Failed to find NT kernel build number!");
        Status = STATUS_NOT_FOUND;
        goto Exit;
    }

    LOG_INFO("NT build number %d", NtKernelBuild);
    BoRuntimeData.KernelBuild = NtKernelBuild;
    BoRuntimeData.IsWindows7 = (BOOLEAN)(NtKernelBuild >= NT_BUILD_7 && NtKernelBuild < NT_BUILD_8);

    //
    // Get NT build lab string.
    //
    NtKernelBuildLab = UtlGetNtKernelBuildLab();
    if (!NtKernelBuildLab) {
        LOG_ERROR("Failed to find NT kernel build lab string!");
        Status = STATUS_NOT_FOUND;
        goto Exit;
    }

    LOG_INFO("NT build lab %s", NtKernelBuildLab);
    RtlStringCchCopyA(BoRuntimeData.KernelBuildLab,
                      RTL_NUMBER_OF(BoRuntimeData.KernelBuildLab),
                      NtKernelBuildLab);

    //
    // Initialize the Ps API.
    //
    Status = PsInitialize(NtKernelBase);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to initialize Ps API with status %08x", Status);
        goto Exit;
    }

    //
    // Initialize the File API
    //
    Status = FileInitialize(NtKernelBase);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to initialize File API with status %08x", Status);
        goto Exit;
    }

    //
    // Create our device object.
    //
    Status = IoCreateDevice(DriverObject,           // pointer to driver object
                            0,                      // device extension size
                            &NtDeviceName,          // device name
                            FILE_DEVICE_UNKNOWN,    // device type
                            0,                      // device characteristics
                            TRUE,                   // make exclusive fdo
                            &DeviceObject           // returned device object pointer
                            );

    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to driver device object with status %08x", Status);
        goto Exit;
    }
    BO_ASSERT(DeviceObject == DriverObject->DeviceObject);
    BoRuntimeData.DeviceObject = DeviceObject;

    //
    // Set dispatch routines.
    //
#if defined(ENABLE_IOCTLS)
    DriverObject->MajorFunction[IRP_MJ_CREATE] = BoDeviceCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoDeviceClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = BoDeviceCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BoDeviceControl;
#endif
    DriverObject->DriverUnload = BoDriverUnload;

    //
    // Create a link in the Win32 namespace.
    //
    Status = IoCreateSymbolicLink(&DosDeviceName, &NtDeviceName);
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to create symbolic link with status %08x", Status);
        goto Exit;
    }
    BoRuntimeData.SymLinkCreated = TRUE;

    //
    // Initialize the inject API.
    //
    Status = InjInitialize();
    if (!NT_SUCCESS(Status)) {
        LOG_ERROR("Failed to initialize inject API with status %08x", Status);
        goto Exit;
    }

    BO_DBG_BREAK();

    //
    // !!WARNING!! Hardcoding source and target injection paths!
    //
#if defined(DBG)
    UNICODE_STRING HookPath = RTL_CONSTANT_STRING(BoRuntimeData.HookPath);
    UtlGetSystemRootPath(&HookPath);
    RtlUnicodeStringCatString(&HookPath, L"\\System32\\nvdid.dll");
    UNICODE_STRING TargetProcess = RTL_CONSTANT_STRING(L"notepad.exe");
    RtlCopyMemory(BoRuntimeData.TargetProcess, TargetProcess.Buffer, TargetProcess.Length);
    LOG_INFO("Hook path = %wZ", &HookPath);
    LOG_INFO("Target process = %wZ", &TargetProcess);
#else
    VM_START("002");
    UNICODE_STRING HookPath = RTL_CONSTANT_STRING(BoRuntimeData.HookPath);
    UtlGetSystemRootPath(&HookPath);
    RtlUnicodeStringCatString(&HookPath, L"\\System32\\nvdid.dll");
    UNICODE_STRING TargetProcess = RTL_CONSTANT_STRING(L"TslGame.exe");
    RtlCopyMemory(BoRuntimeData.TargetProcess, TargetProcess.Buffer, TargetProcess.Length);
    VM_END();
#endif // DBG

    //
    // Register the injection.
    //
    InjRegisterInjection(&BoInjectionHandle, NULL, HookPath.Buffer, NULL, TargetProcess.Buffer);

    //
    // Indicate the runtime data as initialized!
    //
    BoRuntimeData.Initialized = TRUE;

Exit:
    if (!NT_SUCCESS(Status)) {

        //
        // Destroy the inject API.
        //
        InjDestroy();

        //
        // Delete the driver symbolic link.
        //
        if (BoRuntimeData.SymLinkCreated) {
            IoDeleteSymbolicLink(&DosDeviceName);
            BoRuntimeData.SymLinkCreated = FALSE;
        }

        //
        // Delete the driver device object.
        //
        if (BoRuntimeData.DeviceObject != NULL) {
            IoDeleteDevice(BoRuntimeData.DeviceObject);
            BoRuntimeData.DeviceObject = NULL;
        }

        //
        // Destroy the log interface.
        //
        LogDestroy();
    }

    return Status;
}

VOID
BoDriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
    NTSTATUS Status;
    UNICODE_STRING DosDevicesName = RTL_CONSTANT_STRING(BO_DOS_DEVICE_NAME);

    //
    // Free the encrypted hook module data if needed.
    //
    if (BoRuntimeData.EncryptedHookBuffer) {
        RtlZeroMemory(BoRuntimeData.EncryptedHookBuffer, BoRuntimeData.EncryptedHookSize);
        MmFreeNonPaged(BoRuntimeData.EncryptedHookBuffer);
    }

    //
    // Destroy the inject API.
    //
    InjDestroy();

    LOG_INFO("Deleting symlinks and device");

    //
    // Delete the link from our device name to a name in the Win32 namespace.
    //
    if (BoRuntimeData.SymLinkCreated) {
        Status = IoDeleteSymbolicLink(&DosDevicesName);
        if (Status != STATUS_INSUFFICIENT_RESOURCES) {
            //
            // IoDeleteSymbolicLink can fail with STATUS_INSUFFICIENT_RESOURCES.
            //
            BO_ASSERT(NT_SUCCESS(Status));
        }
        BoRuntimeData.SymLinkCreated = FALSE;
    }

    //
    // Delete our device object.
    //
    if (BoRuntimeData.DeviceObject) {
        BO_ASSERT(DriverObject->DeviceObject == BoRuntimeData.DeviceObject);
        IoDeleteDevice(BoRuntimeData.DeviceObject);
        BoRuntimeData.DeviceObject = NULL;
    }

    BoRuntimeData.Initialized = FALSE;

    LOG_INFO("DeviceUnload finished");

    //
    // Destroy the log interface.
    //
    LogDestroy();

#if defined(DBG)
    DBGPRINT("Byeee!");
#endif // DBG
}