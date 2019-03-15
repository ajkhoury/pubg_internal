#include <native/service.h>
#include <native/rtl.h>
#include <native/log.h>
#include <utils/xorstr.h>

void help() {
    fputs(_XOR_("Usage:\n  program start [bin] | stop\n\npress any key to exit..."), stderr);
    getchar();
}

int main(int argc, char **argv)
{
    HANDLE SvcHandle;
    WCHAR DriverPath[MAX_PATH];
    WCHAR BinaryPath[MAX_PATH+2];
    int Operation;
    int rc;

    rc = LogInitialize(LogPutLevelDebug | LogOptDisableFunctionName,
                       _XOR_(L"C:\\Users\\Owner\\svclog.txt"));
    if (rc != NOERROR) {
        return rc;
    }

    if (argc < 2) {
        LOG_WARN(_XOR_("No arguments supplied"));
        help();
        rc = -1;
        goto Exit;
    }

    if (_stricmp(argv[1], _XOR_("start")) == 0) {
        Operation = 1;
    } else if (_stricmp(argv[1], _XOR_("stop")) == 0) {
        Operation = 2;
    } else {
        LOG_WARN(_XOR_("Invalid argument supplied"));
        help();
        rc = -1;
        goto Exit;
    }

    //
    // Start operation
    //
    if (Operation == 1) {

        wcscpy_s(DriverPath, SharedUserData->NtSystemRoot);
        wcscat_s(DriverPath, _XOR_(L"\\System32\\drivers\\nvdid.sys"));
        wcscpy_s(BinaryPath, L"\"");
        wcscat_s(BinaryPath, DriverPath);
        wcscat_s(BinaryPath, L"\"");

        if (!SvcCreateService(&SvcHandle,
                              _XOR_(L"nvdid"),
                              _XOR_(L"nvdid"),
                              DriverPath,
                              SERVICE_ALL_ACCESS,
                              SERVICE_KERNEL_DRIVER,
                              SERVICE_DEMAND_START,
                              SERVICE_ERROR_NORMAL))
        {
            rc = (int)RtlGetLastWin32Error();
            if (rc == ERROR_SERVICE_EXISTS) {

                //
                // Since it already exists, try to open a handle to the
                // existing service.
                //
                if (!SvcOpenServiceHandle(&SvcHandle, _XOR_(L"nvdid"), SERVICE_ALL_ACCESS)) {
                    rc = (int)RtlGetLastWin32Error();
                    LOG_ERROR(_XOR_("Failed to open service with error %d"), rc);
                    goto Exit;
                }
                rc = NOERROR;

            } else {
                LOG_ERROR(_XOR_("Failed to create service with error %d"), rc);
                goto Exit;
            }
        }

        if (!SvcStartService(SvcHandle)) {
            rc = (int)RtlGetLastWin32Error();
            if (rc != ERROR_SERVICE_ALREADY_RUNNING) {
                SvcDeleteService(SvcHandle);
                LOG_ERROR(_XOR_("Service failed to start with error %d"), rc);
                goto Exit;
            }
        }
    }

    //
    // Stop operation
    //
    else {

        if (!SvcOpenServiceHandle(&SvcHandle, _XOR_(L"nvdid"), SERVICE_ALL_ACCESS)) {
            rc = (int)RtlGetLastWin32Error();
            if (rc != ERROR_SERVICE_DOES_NOT_EXIST) {
                LOG_ERROR(_XOR_("Failed to open service with error %d"), rc);
            } else {
                rc = NOERROR;
            }
            goto Exit;
        }

        if (!SvcStopService(SvcHandle, NULL)) {
            rc = (int)RtlGetLastWin32Error();
            if (rc != ERROR_SERVICE_NOT_ACTIVE) {
                LOG_ERROR(_XOR_("Failed to stop service with error %d"), rc);
            } else {
                rc = NOERROR;
            }
            goto Exit;
        }

        if (!SvcDeleteService(SvcHandle)) {
            rc = (int)RtlGetLastWin32Error();
            if (rc != ERROR_SERVICE_MARKED_FOR_DELETE) {
                LOG_ERROR(_XOR_("Failed to stop service with error %d"), rc);
                goto Exit;
            }
        }
    }

Exit:
    LogDestroy();

    return rc;
}


