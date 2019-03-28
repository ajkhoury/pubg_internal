#pragma once

#include "native.h"
#include <winsvc.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

BOOL
SvcCreateService(
    OUT PHANDLE ServiceHandle,
    IN LPCWSTR ServiceName,
    IN LPCWSTR DisplayName OPTIONAL,
    IN LPCWSTR BinPath,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ServiceType,
    IN ULONG StartType,
    IN ULONG ErrorControl
    );

BOOL 
SvcDeleteService(
    IN HANDLE ServiceHandle
    );

BOOL 
SvcOpenServiceHandle(
    OUT PHANDLE ServiceHandle,
    IN LPCWSTR ServiceName,
    IN ACCESS_MASK DesiredAccess
    );

BOOL 
SvcCloseServiceHandle(
    IN HANDLE ServiceHandle
    );

BOOL 
SvcStartService(
    IN HANDLE ServiceHandle
    );

BOOL
SvcPauseService(
    IN HANDLE ServiceHandle,
    OUT SERVICE_STATUS *ServiceStatus OPTIONAL
    );

BOOL 
SvcResumeService(
    IN HANDLE ServiceHandle,
    OUT SERVICE_STATUS *ServiceStatus OPTIONAL
    );

BOOL 
SvcStopService(
    IN HANDLE ServiceHandle,
    OUT SERVICE_STATUS *ServiceStatus OPTIONAL
    );


#if __cplusplus
} // extern "C"
#endif // __cplusplus