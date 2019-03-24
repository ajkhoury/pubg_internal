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
    void* ImageBase;
    uint32_t ImageSize;

    if (GWorldEncrypted) {
        return true;
    }

    ImageBase = utils::GetModuleHandleWIDE(NULL);
    ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // Search for the GWorld address inside the HandleCSVProfileCommand routine:
    // .text:7FF6FC3954C7 48 8B D9                  mov     rbx, rcx
    // .text:7FF6FC3954CA 48 8B 05 57 34 9C 04      mov     rax, cs:GWorld
    // .text:7FF6FC3954D1 48 89 84 24 80 00 00 00   mov     [rsp+68h+arg_10], rax
    // .text:7FF6FC3954D9 33 F6                     xor     esi, esi
    // .text:7FF6FC3954DB 48 39 35 1E 7C 04 03      cmp     qword ptr cs:aXenuinesdkCarv_97, rsi ; "XENUINESDK_CARVE"
    // .text:7FF6FC3954E2 75 10                     jnz     short loc_7FF6FC3954F4
    // 48 8B D9 48 8B 05 ? ? ? ? 48 89 84 24 ? ? ? ? 33 F6
    const uint8_t *Found;
    do Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8B D9 48 8B 05 ?? ?? ?? ?? 48 89 84 24 ?? ?? ?? ?? 33 F6"));
    while (!Found);
    
    GWorldEncrypted = static_cast<uint64_t*>(utils::GetInstructionTarget(Found, 3 + 3));
    LOG_INFO(_XOR_("GWorldEncrypted = 0x%016llx"), GWorldEncrypted);

    return true;
}

WorldProxy::WorldProxy()
{
    if (!GWorldEncrypted) {
        WorldInitializeGlobal();
    }
    
    WorldEncryptedPtr = GWorldEncrypted;
}

void *WorldProxy::GetAddress() const
{
    union CryptValue WorldDecrypted;
    uint64_t WorldEncrypted = *WorldEncryptedPtr;

    //LOG_INFO(_XOR_("WorldEncrypted = 0x%016llx"), WorldEncrypted); 
    WorldDecrypted.Qword = DecryptWorldAsm(WorldEncrypted);
    //LOG_INFO(_XOR_("WorldDecrypted = 0x%016llx"), WorldDecrypted.Qword);

    return WorldDecrypted.Pointer;
}



