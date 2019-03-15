#include "World.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

static uint64_t *GWorldEncrypted = NULL;

#ifdef __cplusplus
extern "C" {
#endif
uint64_t DecryptWorldAsm(uint64_t WorldEncrypted);
#ifdef __cplusplus
}
#endif

static bool WorldInitializeGlobal()
{
    PVOID ImageBase;
    ULONG ImageSize;

    if (GWorldEncrypted) {
        return true;
    }

    ImageBase = utils::GetModuleHandleWIDE(NULL);
    ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // 4C 8B FF 4C 8B 05 ? ? ? ?
    static const UINT8 WorldSig[] = {
        0x4C, 0x8B, 0xFF,                           /* mov     r15, rdi */
        0x4C, 0x8B, 0x05/*,0xCC,0xCC,0xCC,0xCC*/    /* mov     r8, cs:GWorld */
    };

    const uint8_t *Found;
    do Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, WorldSig, sizeof(WorldSig));
    while (!Found);
    
    GWorldEncrypted = static_cast<uint64_t*>(utils::GetInstructionTarget(Found, 3 + 3));
    LOG_INFO(_XOR_("GWorldEncrypted = 0x%016llx"), GWorldEncrypted);

    return true;
}

WorldProxy::WorldProxy()
{
    WorldInitializeGlobal();
    WorldEncryptedPtr = GWorldEncrypted;
}

void *WorldProxy::GetAddress() const
{
    union CryptValue WorldDecrypted;
    int64_t WorldEncrypted = *WorldEncryptedPtr;

    LOG_INFO(_XOR_("WorldEncrypted = 0x%016llx"), WorldEncrypted); 
    WorldDecrypted.Qword = DecryptWorldAsm(WorldEncrypted);
    LOG_INFO(_XOR_("WorldDecrypted = 0x%016llx"), WorldDecrypted.Qword);

    return WorldDecrypted.Pointer;
}



