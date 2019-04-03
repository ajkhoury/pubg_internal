#include "vthook.h"
#include "utils.h"

SafeVTableHook::SafeVTableHook(void *InObject)
{
    Instance = static_cast<uintptr_t**>(InObject);

    // Roughly calculate the VTable method count.
    uintptr_t ImageBase = reinterpret_cast<uintptr_t>(utils::GetModuleHandleWIDE(NULL));
    size_t ImageSize = utils::GetModuleSize((HMODULE)ImageBase);
    VTableSize = 0;
    while (1) {
        uintptr_t VTableEntry = static_cast<uintptr_t*>(*Instance)[VTableSize];
        if (VTableEntry < ImageBase || VTableEntry >= ImageBase + ImageSize)
            break;
        ++VTableSize;
    }

    OriginalVTable = *Instance;
    CurrentVTable = std::make_unique<std::uintptr_t[]>(VTableSize);

    memcpy(CurrentVTable.get(), OriginalVTable, VTableSize * sizeof(uintptr_t));

    *Instance = CurrentVTable.get();
}
