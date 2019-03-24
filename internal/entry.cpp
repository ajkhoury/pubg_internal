#include <native/native.h>
#include <native/log.h>

#include <utils/utils.h>
#include <utils/disasm.h>
#include <utils/xorstr.h>

#include "sdk/Generator.h"
#include "sdk/OffsetDumper.h"
#include "Sdk.h"

static HANDLE MainThread = INVALID_HANDLE_VALUE;

static
NTSTATUS
NTAPI
TestThreadRoutine(
    IN HMODULE hModule
)
{
    int rc = NOERROR;
    LARGE_INTEGER WaitInterval;
    //PVOID Found;
    

    // Output basic image information.
    PVOID ImageBase = utils::GetModuleHandleWIDE(NULL /*0xC4D8736D TslGame.exe */);
    ULONG ImageSize = utils::GetModuleSize((HMODULE)ImageBase);
    LOG_DEBUG(_XOR_("ImageBase = 0x%016llx  ImageSize = 0x%08x"), ImageBase, ImageSize);

    // Wait for VMProtect to completely unpack.
    WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(8000));
    NtDelayExecution(FALSE, &WaitInterval);

    // Dump heuristic search results.
    rc |= dumper::DumpObjects();
    rc |= dumper::DumpNames();
    rc |= dumper::DumpWorld();
    rc |= dumper::DumpStructs();
    if (rc != NOERROR) {
        return STATUS_UNSUCCESSFUL;
    }

    NamesProxy Names;
    if (!Names.GetAddress()) {
        LOG_ERROR(_XOR_("Failed to initialize Names!"));
        return STATUS_UNSUCCESSFUL;
    }
    
    ObjectsProxy Objects;
    if (!Objects.GetAddress()) {
        LOG_ERROR(_XOR_("Failed to initialize Objects!"));
        return STATUS_UNSUCCESSFUL;
    }
    
    WorldProxy World;
    if (!World.GetEncryptedPointerAddress()) {
        LOG_ERROR(_XOR_("Failed to initialize World!"));
        return STATUS_UNSUCCESSFUL;
    }
    
    int32_t LoopCount = 0;
    while(TRUE) {
    
        WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(1000));
        NtDelayExecution(FALSE, &WaitInterval);
    
        LOG_DEBUG(_XOR_("Names address = 0x%016llx"), Names.GetAddress());
        LOG_DEBUG(_XOR_("Names count = %d"), Names.GetNum());
    
        LOG_DEBUG(_XOR_("Objects address = 0x%016llx"), Objects.GetAddress());
        LOG_DEBUG(_XOR_("Objects count = %d"), Objects.GetNum());
    
        LOG_DEBUG(_XOR_("World address = 0x%016llx"), World.GetAddress());
        if (World.GetAddress()) {
            if (++LoopCount > 5)
                break;
        }
    
        //++NameId;
        //const char *NameData = Names.GetById(NameId);
        //if (NameData) {
        //    LOG_DEBUG(_XOR_("Names[%d] = \"%s\""), NameId, Names.GetById(NameId));
        //}
        //void **NamesData = static_cast<void **>(Names.GetAddress());
        //LOG_INFO(_XOR_("NamesData0 = 0x%016llx"), NamesData[0]);
        //LOG_INFO(_XOR_("NamesData1 = 0x%016llx"), NamesData[1]);
    }

    //ClassProxy UEnumStatic = Objects.FindClass("Class CoreUObject.Enum");
    //if (UEnumStatic.IsValid()) {
    //    LOG_DEBUG(_XOR_("UEnumStatic = 0x%p"), UEnumStatic.GetAddress());
    //}

    //for (int32_t ObjectIdx = 0; ObjectIdx < Objects.GetNum(); ObjectIdx++) {
    //    ObjectProxy Object = Objects.GetById(ObjectIdx);
    //    if (Object.IsValid()) {
    //        auto FullName = Object.GetFullName();
    //        if (!FullName.empty()) {
    //            uint32_t uniqueId = Object.GetUniqueId();
    //            LOG_INFO(_XOR_("Object[%d] = %s"), uniqueId, FullName.c_str());
    //        }
    //    }
    //}

    GeneratorProcessPackages(_XOR_("C:\\Users\\Owner\\Desktop\\sdk"));

    return STATUS_SUCCESS;
}

BOOL
WINAPI 
DllMain(
    IN HMODULE hModule,
    IN ULONG dwReason,
    IN OUT PVOID lpReserved
)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ThreadAttributes;

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        LdrDisableThreadCalloutsForDll(hModule);

        //
        // Initialize logger subsystem.
        //
        Status = LogInitialize(LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend,
                               _XOR_(L"C:\\Users\\Owner\\Log.txt"));
        if (Status != NOERROR) {
            return FALSE;
        }

        LOG_DEBUG(_XOR_("hModule = 0x%016llx"), hModule);

        //
        // Create main thread.
        //
        InitializeObjectAttributes(&ThreadAttributes, NULL, 0, NULL, NULL);
        Status = NtCreateThreadEx(&MainThread,
                                  THREAD_ALL_ACCESS,
                                  &ThreadAttributes,
                                  NtCurrentProcess(),
                                  TestThreadRoutine,
                                  hModule,
                                  0,
                                  0,
                                  0x1000,
                                  0x100000,
                                  NULL
                                  );

        if (!NT_SUCCESS(Status)) {
            LOG_DEBUG(_XOR_("Failed to create thread with status %08x"), Status);
            LogDestroy();
            return FALSE;
        }

        LOG_DEBUG(_XOR_("Main thread created %d"), (ULONG)(ULONG_PTR)MainThread);
        return TRUE;

    case DLL_PROCESS_DETACH:
        LogDestroy();
        return TRUE;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return FALSE;
}