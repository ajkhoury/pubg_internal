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

    //// 4C 8B FF 4C 8B 05 ? ? ? ?
    //static const UINT8 WorldSig[] = {
    //    0x4C, 0x8B, 0xFF,                           /* mov     r15, rdi */
    //    0x4C, 0x8B, 0x05/*,0xCC,0xCC,0xCC,0xCC*/    /* mov     r8, cs:GWorld */
    //};
    //const uint8_t *Found;
    //do Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, WorldSig, sizeof(WorldSig));
    //while (!Found);

    // Search for the GWorld address inside an unknown routine:
    // .text:7FF6FCBF16CC                       loc_7FF6FCBF16CC:               ; CODE XREF: GWorldAndGEngineFunction+115^j
    // .text:7FF6FCBF16CC 48 8B F7              mov     rsi, rdi
    // .text:7FF6FCBF16CF                       loc_7FF6FCBF16CF:               ; CODE XREF: GWorldAndGEngineFunction+11A^j
    // .text:7FF6FCBF16CF 48 8B 0D 52 72 16 04  mov     rcx, cs:GWorld
    // .text:7FF6FCBF16D6 4C 39 2D 23 14 A2 02  cmp     qword ptr cs:aXenuinesdkCarv_95, r13 ; "XENUINESDK_CARVE"
    // .text:7FF6FCBF16DD 75 1A                 jnz     short loc_7FF6FCBF16F9  ; Decryption starts here
    // 48 8B F7 48 8B 0D ? ? ? ?
    const uint8_t *Found;
    do Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8B F7 48 8B 0D"));
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

    //LOG_INFO(_XOR_("WorldEncrypted = 0x%016llx"), WorldEncrypted); 
    WorldDecrypted.Qword = DecryptWorldAsm(WorldEncrypted);
    //LOG_INFO(_XOR_("WorldDecrypted = 0x%016llx"), WorldDecrypted.Qword);

    return WorldDecrypted.Pointer;
}



