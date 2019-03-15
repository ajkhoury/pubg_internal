#include "service.h"

BOOL
SvcCreateService(
    OUT PHANDLE ServiceHandle,
    IN LPCWSTR ServiceName,
    IN LPCWSTR DisplayName OPTIONAL,
    IN LPCWSTR BinaryPath,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ServiceType,
    IN ULONG StartType,
    IN ULONG ErrorControl
)
{
    SC_HANDLE ScHandle;

    ScHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!ScHandle) {
        return FALSE;
    }

    *ServiceHandle = CreateServiceW(ScHandle,
                                    ServiceName,
                                    DisplayName,
                                    DesiredAccess,
                                    ServiceType,
                                    StartType,
                                    ErrorControl,
                                    BinaryPath,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL
                                    );

    CloseServiceHandle(ScHandle);

    return *ServiceHandle != NULL;
}

BOOL
SvcDeleteService(
    IN HANDLE ServiceHandle
)
{
    return DeleteService(ServiceHandle);
}

BOOL
SvcOpenServiceHandle(
    OUT PHANDLE ServiceHandle,
    IN LPCWSTR ServiceName,
    IN ACCESS_MASK DesiredAccess
)
{
    SC_HANDLE ScHandle;

    ScHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!ScHandle) {
        return FALSE;
    }

    *ServiceHandle = OpenServiceW(ScHandle, ServiceName, DesiredAccess);

    CloseServiceHandle(ScHandle);

    return *ServiceHandle != NULL;
}

BOOL
SvcCloseServiceHandle(
    IN HANDLE ServiceHandle
)
{
    return CloseServiceHandle(ServiceHandle);
}

BOOL
SvcStartService(
    IN HANDLE ServiceHandle
)
{
    return StartServiceW(ServiceHandle, 0, NULL);
}

BOOL
SvcPauseService(
    IN HANDLE ServiceHandle,
    OUT SERVICE_STATUS *ServiceStatus OPTIONAL
)
{
    BOOL Result;
    SERVICE_STATUS Status;

    Result = ControlService(ServiceHandle, SERVICE_CONTROL_PAUSE, &Status);

    if (ServiceStatus) {
        *ServiceStatus = Status;
    }

    return Result;
}

BOOL
SvcResumeService(
    IN HANDLE ServiceHandle,
    OUT SERVICE_STATUS *ServiceStatus OPTIONAL
)
{
    BOOL Result;
    SERVICE_STATUS Status;

    Result = ControlService(ServiceHandle, SERVICE_CONTROL_CONTINUE, &Status);

    if (ServiceStatus) {
        *ServiceStatus = Status;
    }

    return Result;
}

BOOL
SvcStopService(
    IN HANDLE ServiceHandle,
    OUT SERVICE_STATUS *ServiceStatus OPTIONAL
)
{
    BOOL Result;
    SERVICE_STATUS Status;

    Result = ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &Status);

    if (ServiceStatus) {
        *ServiceStatus = Status;
    }

    return Result;
}
