#include "OffsetDumper.h"
#include "tinyformat.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/disasm.h>
#include <utils/xorstr.h>

#include <algorithm>
#include <sstream>

static const char* RegisterStrings[64] = {
    /* 64-bit registers */
    "RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",     // 0
    "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",     // 8
    /* 32-bit registers */
    "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI",     // 16
    "R8D", "R9D", "R10D","R11D","R12D","R13D","R14D","R15D",    // 24
    /* 16-bit registers */
    "AX",  "CX",  "DX",  "BX",  "SP",  "BP",  "SI",  "DI",      // 32
    "R8W", "R9W", "R10W","R11W","R12W","R13W","R14W","R15W",    // 40
    /* 8-bit registers */
    "AL",  "CL",  "DL",  "BL",  "AH",  "CH",  "DH",  "BH",      // 48
    "R8B", "R9B", "R10B","R11B","R12B","R13B","R14B","R15B",    // 56
};

// Disassembly maximums
#define MAX_DISASM_LENGTH       PAGE_SIZE
#define MAX_INSTRUCTION_COUNT   (MAX_DISASM_LENGTH / 3)

// Disassembly local variable declarations.
#define DECLARE_DISASM_VARS() \
    const uint32_t MaxInstCount = MAX_INSTRUCTION_COUNT; \
    DisData Insts[MaxInstCount]; \
    DisData* Inst, *InstEnd, *NextInst; \
    uint8_t* Ip, *NextIp; \
    uint32_t InstCount, InstIndex; \
    size_t Offset, NextOffset

// For incrementing to the next instruction.
#define INCREMENT_NEXT_INSTRUCTION() do { \
    NextIp += NextInst->size; NextOffset += NextInst->size; ++NextInst; \
} while(0)

// For skipping the decryption fuckery nop calls that look like this:
// .text:7FF7F19532D8 E8 A3 B8 A8 FE    call    tslgame_AK__MemoryMgr__StartProfileThreadUsage
// .text:7FF7F19532DD E8 9E B8 A8 FE    call    tslgame_AK__MemoryMgr__StartProfileThreadUsage
// .text:7FF7F19532E2 E8 99 B8 A8 FE    call    tslgame_AK__MemoryMgr__StartProfileThreadUsage
#define SKIP_FUCKERY_NOP_CALLS() \
    if (NextInst->opcode == I_CALL) do { \
        INCREMENT_NEXT_INSTRUCTION(); \
    } while(NextInst->opcode == I_CALL)


namespace unreal {

ObjectSizeMapType ObjectSizeMap;

int InitializeUnrealObjectSizeMap()
{
    void* ImageBase = utils::GetModuleHandleWIDE(nullptr /*0xC4D8736D TslGame.exe */);
    uint32_t ImageSize = utils::GetModuleSize((HMODULE)ImageBase);
    const uint8_t* SearchEnd = static_cast<uint8_t*>(ImageBase) + ImageSize;
    uint8_t* SearchBase;
    size_t SearchSize;
    const uint8_t *FoundNext;
    ObjectSizeMapType::iterator ObjectSizeIter;

    // Construct a map of Unreal object sizes using the AutoInitialize instrinsic stubs.
    // UStruct__AutoInit proc near
    // .text:7FF7F0277820 45 33 C9              xor     r9d, r9d
    // .text:7FF7F0277823 48 8D 15 AE CF 19 04  lea     rdx, aUstruct       ; "UStruct"
    // .text:7FF7F027782A 41 B8 E0 00 00 00     mov     r8d, 0E0h
    // .text:7FF7F0277830 48 8D 0D 39 A6 99 05  lea     rcx, off_7FF7F5C11E70
    // .text:7FF7F0277837 E9 44 08 6E 01        jmp     sub_7FF7F1958080
    // UStruct__AutoInit endp
    // 45 33 C9 48 8D 15 ? ? ? ? 41 B8 ? ? ? ? 48 8D 0D ? ? ? ? E9 ? ? ? ?
    FoundNext = static_cast<uint8_t*>(ImageBase);
    while (1) {

        // Set the search base and size for this iteration.
        SearchBase = const_cast<uint8_t*>(FoundNext) + 1;
        SearchSize = static_cast<size_t>(SearchEnd - SearchBase);
        // Do the pattern scan for the next match.
        FoundNext = utils::FindPatternIDA(SearchBase,
                                          SearchSize,
                                          _XOR_("45 33 C9 48 8D 15 ?? ?? ?? ?? 41 B8"));
        if (!FoundNext) {
            break;
        }

        // Seek to the LEA instruction that passes the Unreal object name.
        FoundNext += 3;
        // Get the name of the Unreal object from the address passed into RDX.
        std::wstring ObjectName = static_cast<wchar_t*>(utils::GetInstructionTarget(FoundNext, 3));

        // Seek to the MOV instruction that passes the Unreal object size.
        FoundNext += 7;
        // Get the Unreal object size from the instruction immediate operand.
        size_t ObjectSize = static_cast<size_t>(utils::GetInstructionImm32(FoundNext, 2));

        // Insert this Unreal object size into the map.
        ObjectSizeMap[ObjectName] = ObjectSize;
    }

    // The second variation that uses an LEA instruction with the size.
    // .text:7FF7F02777C0                       UField__AutoInit proc near
    // .text:7FF7F02777C0 45 33 C9              xor     r9d, r9d
    // .text:7FF7F02777C3 48 8D 15 5E FA 19 04  lea     rdx, aUfield        ; "UField"
    // .text:7FF7F02777CA 48 8D 0D 77 A6 99 05  lea     rcx, off_7FF7F5C11E48
    // .text:7FF7F02777D1 45 8D 41 38           lea     r8d, [r9+38h]
    // .text:7FF7F02777D5 E9 A6 08 6E 01        jmp     sub_7FF7F1958080
    // .text:7FF7F02777D5                       UField__AutoInit endp
    // 45 33 C9 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? 45 8D 41 ? E9 ? ? ? ?
    FoundNext = static_cast<uint8_t*>(ImageBase);
    while (1) {

        // Set the search base and size for this iteration.
        SearchBase = const_cast<uint8_t*>(FoundNext) + 1;
        SearchSize = static_cast<size_t>(SearchEnd - SearchBase);
        // Do the pattern scan for the next match.
        FoundNext = utils::FindPatternIDA(SearchBase,
                                          SearchSize,
                                          _XOR_("45 33 C9 48 8D 15 ?? ?? ?? ?? 48 8D 0D"));
        if (!FoundNext) {
            break;
        }

        // Seek to the LEA instruction that passes the Unreal object name.
        FoundNext += 3;
        // Get the name of the Unreal object from the address passed into RDX.
        std::wstring ObjectName = static_cast<wchar_t*>(utils::GetInstructionTarget(FoundNext, 3));

        // Seek to the LEA instruction that passes the Unreal object size.
        FoundNext += 14;
        // Get the Unreal object size from the instruction memory operand.
        size_t ObjectSize = static_cast<size_t>(utils::GetInstructionImm8(FoundNext, 3));

        // Insert this Unreal object size into the map.
        ObjectSizeMap[ObjectName] = ObjectSize;
    }

    // The third variation that UObject uses, which looks like this:
    // .text:7FF7F0370B80                       UObject__AutoInit proc near
    // .text:7FF7F0370B80 48 83 EC 28           sub     rsp, 28h
    // .text:7FF7F0370B84 41 B9 B7 75 77 A1     mov     r9d, 0A17775B7h
    // .text:7FF7F0370B8A 41 B8 30 00 00 00     mov     r8d, 30h ; '0'
    // .text:7FF7F0370B90 48 8D 15 99 8C A2 04  lea     rdx, aUobject_0     ; "UObject"
    // .text:7FF7F0370B97 48 8D 0D EA 24 F8 05  lea     rcx, off_7FF7F62F3088
    // .text:7FF7F0370B9E E8 DD AC 81 02        call    sub_7FF7F2B8B880
    // .text:7FF7F0370BA3 48 83 C4 28           add     rsp, 28h
    // .text:7FF7F0370BA7 C3                    retn
    // .text:7FF7F0370BA7                       UObject__AutoInit endp
    // 48 83 EC 28 41 B9 ? ? ? ? 41 B8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ?
    FoundNext = static_cast<uint8_t*>(ImageBase);
    while (1) {

        // Set the search base and size for this iteration.
        SearchBase = const_cast<uint8_t*>(FoundNext) + 1;
        SearchSize = static_cast<size_t>(SearchEnd - SearchBase);
        // Do the pattern scan for the next match.
        FoundNext = utils::FindPatternIDA(
                            SearchBase,
                            SearchSize,
                            _XOR_("48 83 EC 28 41 B9 ?? ?? ?? ?? 41 B8 ?? ?? ?? ?? 48 8D 15"));
        if (!FoundNext) {
            break;
        }

        // Seek to the MOV instruction that passes the Unreal object size.
        FoundNext += 10;
        // Get the Unreal object size from the instruction immediate operand.
        size_t ObjectSize = static_cast<size_t>(utils::GetInstructionImm32(FoundNext, 2));

        // Seek to the LEA instruction that passes the Unreal object name.
        FoundNext += 6;
        // Get the name of the Unreal object from the address passed into RDX.
        std::wstring ObjectName = static_cast<wchar_t*>(utils::GetInstructionTarget(FoundNext, 3));

        // Insert this Unreal object size into the map.
        ObjectSizeMap[ObjectName] = ObjectSize;
    }

    return NOERROR;
}

const size_t GetObjectSize(const std::wstring& ObjectName)
{
    if (!(ObjectSizeMap.empty() && InitializeUnrealObjectSizeMap() != NOERROR)) {
        const auto it = ObjectSizeMap.find(ObjectName);
        if (it != std::end(ObjectSizeMap)) {
            return it->second;
        }
    }
    return 0;
}


/**
 * Objects offsets and inline decryption globals.
 */
size_t ObjectsEncryptedOffset = -1;
int8_t ObjectsEncryptedRegister = -1;
uint8_t* ObjectsDecryptionBegin = nullptr;
uint8_t* ObjectsDecryptionEnd = nullptr;

size_t ObjectFlagsEncryptedOffset = -1;
int8_t ObjectFlagsEncryptedRegister = -1;
uint8_t* ObjectFlagsDecryptionBegin = nullptr;
uint8_t* ObjectFlagsDecryptionEnd = nullptr;

size_t ObjectOuterEncryptedOffset = -1;
int8_t ObjectOuterEncryptedRegister = -1;
uint8_t* ObjectOuterDecryptionBegin = nullptr;
uint8_t* ObjectOuterDecryptionEnd = nullptr;

size_t ObjectInternalIndexEncryptedOffset = -1;
int8_t ObjectInternalIndexEncryptedRegister = -1;
uint8_t* ObjectInternalIndexDecryptionBegin = nullptr;
uint8_t* ObjectInternalIndexDecryptionEnd = nullptr;

size_t ObjectClassEncryptedOffset = -1;
int8_t ObjectClassEncryptedRegister = -1;
uint8_t* ObjectClassDecryptionBegin = nullptr;
uint8_t* ObjectClassDecryptionEnd = nullptr;

size_t ObjectNameIndexEncryptedOffset = -1;
int8_t ObjectNameIndexEncryptedRegister = -1;
size_t ObjectNameNumberEncryptedOffset = -1;
int8_t ObjectNameNumberEncryptedRegister = -1;
uint8_t* ObjectNameDecryptionBegin = nullptr;
uint8_t* ObjectNameDecryptionEnd = nullptr;

size_t StructPropertiesSizeOffset = -1;
size_t StructMinAlignmentOffset = -1;
size_t StructChildrenOffset = -1;
size_t StructSuperStructOffset = -1;

size_t FunctionFlagsOffset = -1;

size_t PropertyElementSizeOffset = -1;
size_t PropertyArrayDimOffset = -1;
size_t PropertyOffsetInternalOffset = -1;
size_t PropertyFlagsOffset = -1;

size_t EnumNamesOffset = -1;
size_t EnumCppFormOffset = -1;

int DumpObjects()
{
    int rc;
    const uint8_t* Found;
    uint64_t TargetAddress;
    DECLARE_DISASM_VARS();

    const void* ImageBase = utils::GetModuleHandleWIDE(nullptr /*0xC4D8736D TslGame.exe */);
    uint32_t ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // Signature inside of FUObjectArray::FreeUObjectIndex to get the
    // UObject::InternalIndexEncrypted field and FUObjectArray::ObjObjects.ObjectsEncrypted
    // inline decryption.
    // F0 4D 0F B1 24 0F
    //const uint8_t FreeUObjectIndexSig[] = {
    //    0xF0, 0x4D, 0x0F, 0xB1, 0x24, 0x0F          /* lock cmpxchg [r15+rcx], r12 */
    //};
    //Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, FreeUObjectIndexSig, sizeof(FreeUObjectIndexSig));
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("F0 4D 0F B1 24 0F"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find FUObjectArray::FreeUObjectIndex signature!"));
        return ERROR_NOT_FOUND;
    }
    rc = utils::FindFunctionStartFromPtr(Found, PAGE_SIZE, &Found);
    if (rc < 0) {
        LOG_ERROR(_XOR_("Failed to backtrack to beginning of FUObjectArray::FreeUObjectIndex!"));
        return rc;
    }

    // Disassemble the FUObjectArray::FreeUObjectIndex routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble FUObjectArray::FreeUObjectIndex!"));
        return rc;
    }

    // Heuristically search for offsets and inline decryption.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the beginning of the UObject::InternalIndexEncrypted decryption, which
        // is near the beginning within the first 10 instructions.
        /* mov     ebx, [rdx+28h] ; Object->InternalIndexEncrypted */
        if (InstIndex < 12 &&
            Inst->opcode == I_MOV &&
            Inst->ops[0].type == O_REG &&
            Inst->ops[1].type == O_SMEM) {

            // We found the beginning of the UObject::InternalIndexEncrypted decryption!
            ObjectInternalIndexEncryptedOffset = static_cast<size_t>(Inst->disp);
            ObjectInternalIndexEncryptedRegister = Inst->ops[0].index;
            LOG_INFO(_XOR_("Found UObject::InternalIndexEncrypted offset: 0x%X (%s)"),
                     ObjectInternalIndexEncryptedOffset,
                     RegisterStrings[ObjectInternalIndexEncryptedRegister]);

            // Increment next instruction twice.
            INCREMENT_NEXT_INSTRUCTION();
            INCREMENT_NEXT_INSTRUCTION();
            // Shave off any decryption fuckery nop calls.
            SKIP_FUCKERY_NOP_CALLS();

            // Make sure this is the instruction that gets the value of the
            // FUObjectArray::ObjObjects.ObjectsEncrypted field.
            /* mov     rcx, [rdi+10h] */
            if (NextInst->opcode == I_MOV &&
                NextInst->ops[0].type == O_REG &&
                NextInst->ops[1].type == O_SMEM) {

                // Found the ObjObjects.ObjectsEncrypted offset and register!
                ObjectsEncryptedOffset = static_cast<size_t>(NextInst->disp);
                ObjectsEncryptedRegister = NextInst->ops[0].index;
                LOG_INFO(_XOR_("Found FUObjectArray::ObjObjects.ObjectsEncrypted offset: 0x%X (%s)"),
                         ObjectsEncryptedOffset, RegisterStrings[ObjectsEncryptedRegister]);

                // Increment instruction.
                INCREMENT_NEXT_INSTRUCTION();

            } else {
                LOG_ERROR(_XOR_("Failed to find instruction that gets ObjectsEncrypted!"));
            }

            // The next series of instructions is the UObject::InternalIndexEncrypted decryption.
            ObjectInternalIndexDecryptionBegin = NextIp;
            LOG_INFO(_XOR_("Found the UObject::InternalIndexEncrypted decryption beginning: 0x%p"),
                     ObjectInternalIndexDecryptionBegin);

            // Handle the next instruction.
            goto Next1;
        }

        // Search for the end of the UObject::InternalIndexEncrypted decryption.
        if (ObjectInternalIndexDecryptionBegin && !ObjectInternalIndexDecryptionEnd) {

            /* cmp     qword ptr cs:aXenuinesdkCarv_159, r12 ; "XENUINESDK_CARVE"
             * jnz     short loc_7FF785416CE0
             */
            if (Inst->opcode == I_CMP && META_GET_FC(NextInst->meta) == FC_CND_BRANCH) {

                // We found the ending of the UObject::InternalIndexEncrypted decryption!
                ObjectInternalIndexDecryptionEnd = Ip;
                LOG_INFO(_XOR_("Found the UObject::InternalIndexEncrypted decryption ending: 0x%p"),
                         ObjectInternalIndexDecryptionEnd);

                // The instructions at the jnz target is the FUObjectArray::ObjObjects.ObjectsEncrypted decryption:
                // .text:7FF785416CC4 4C 39 25 35 AD DA 01  cmp     qword ptr cs:aXenuinesdkCarv_159, r12 ; "XENUINESDK_CARVE"
                // .text:7FF785416CCB 75 13                 jnz     short loc_7FF785416CE0
                // .text:7FF785416CCD 48 8B D1              mov     rdx, rcx                        ; _QWORD
                // .text:7FF785416CD0 B9 D1 DD 3E 48        mov     ecx, 483EDDD1h                  ; _QWORD
                // .text:7FF785416CD5 FF 15 4D AD DA 01     call    cs:Xenuine__Decrypt_159
                // .text:7FF785416CDB 48 8B C8              mov     rcx, rax
                // .text:7FF785416CDE EB 35                 jmp     short loc_7FF785416D15          ; <-- This jumps to the end of decryption
                // .text:7FF785416CE0                       ; ---------------------------------------------------------------------------
                // .text:7FF785416CE0                       loc_7FF785416CE0:                       ; CODE XREF: FUObjectArray__FreeUObjectIndex+4B^j
                // .text:7FF785416CE0 8D 81 7A 4E E6 A6     lea     eax, [rcx-5919B186h]            ; <-- Decryption starts here.
                TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
                ObjectsDecryptionBegin = reinterpret_cast<uint8_t*>(TargetAddress);
                LOG_INFO(_XOR_("Found the FUObjectArray::ObjObjects.ObjectsEncrypted decryption beginning: 0x%p"),
                         ObjectsDecryptionBegin);
                INCREMENT_NEXT_INSTRUCTION();
                DisData* PrevInst = NextInst;
                while (NextInst->addr != TargetAddress) {
                    PrevInst = NextInst;
                    INCREMENT_NEXT_INSTRUCTION();
                }
                TargetAddress = INSTRUCTION_GET_TARGET(PrevInst);
                ObjectsDecryptionEnd = reinterpret_cast<uint8_t*>(TargetAddress);
                LOG_INFO(_XOR_("Found the FUObjectArray::ObjObjects.ObjectsEncrypted decryption ending: 0x%p"),
                         ObjectsDecryptionEnd);

                // We are done!
                break;
            }
        }

    Next1:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    if (ObjectsEncryptedOffset != -1) {
        LOG_INFO(_XOR_("FUObjectArray::ObjObjects.Objects offset = 0x%08x"), ObjectsEncryptedOffset);
        LOG_INFO(_XOR_("FUObjectArray::ObjObjects.Objects register = %s"), RegisterStrings[ObjectsEncryptedRegister]);
        LOG_INFO(_XOR_("FUObjectArray::ObjObjects.Objects decryption = {"));
        std::string BufferString = utils::FormatBuffer(ObjectsDecryptionBegin,
                                                       __MIN(static_cast<size_t>(ObjectsDecryptionEnd - ObjectsDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    // Output the hueristic search results.
    if (ObjectInternalIndexEncryptedOffset != -1) {
        LOG_INFO(_XOR_("UObject::InternalIndexEncrypted offset = 0x%08x"), ObjectInternalIndexEncryptedOffset);
        LOG_INFO(_XOR_("UObject::InternalIndexEncrypted register = %s"), RegisterStrings[ObjectInternalIndexEncryptedRegister]);
        LOG_INFO(_XOR_("UObject::InternalIndexEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(ObjectInternalIndexDecryptionBegin,
                                                       __MIN(static_cast<size_t>(ObjectInternalIndexDecryptionEnd - ObjectInternalIndexDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }



    // UObjectBaseUtility::MarkPackageDirty
    // E8 ? ? ? ? 4C 63 63 ? 4C 63 73
    //const uint8_t MarkPackageDirty[] = {
    //    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,               /* call    UObjectBaseUtility__MarkPackageDirty */
    //                                                /* loc_7FF7F13FBCF4: */
    //    0x4C, 0x63, 0x63, 0x40                      /* movsxd  r12, dword ptr [rbx+40h] */
    //    /*4C 63 73 40*/                             /* movsxd  r14, dword ptr [rbx+40h] */
    //};
    //Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, MarkPackageDirty, sizeof(MarkPackageDirty));
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("E8 ?? ?? ?? ?? 4C 63 63 ?? 4C 63 73"));
    if (Found)
        Found = static_cast<const uint8_t *>(utils::GetCallTargetAddress(Found));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UObjectBaseUtility::MarkPackageDirty!"));
        return ERROR_NOT_FOUND;
    }

    // Disassemble the UObjectBaseUtility::MarkPackageDirty routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble UObjectBaseUtility::MarkPackageDirty!"));
        return rc;
    }

    // Heuristically search for offsets and inline decryption.
    Ip = const_cast<uint8_t*>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the beginning of the ObjectFlagsEncrypted decryption, which
        // is near the beginning within the first 6 instructions.
        /* mov     ebx, [rcx+20h] ; this->ObjectFlagsEncrypted */
        if (InstIndex < 6 &&
            Inst->opcode == I_MOV &&
            Inst->ops[0].type == O_REG &&
            Inst->ops[1].type == O_SMEM) {

            // We found the UObject::ObjectFlagsEncrypted offset!
            ObjectFlagsEncryptedOffset = static_cast<size_t>(Inst->disp);
            ObjectFlagsEncryptedRegister = Inst->ops[0].index;
            LOG_INFO(_XOR_("Found UObject::ObjectFlagsEncrypted offset: 0x%X (%s)"),
                     ObjectFlagsEncryptedOffset, RegisterStrings[ObjectFlagsEncryptedRegister]);

            // Increment next instruction.
            INCREMENT_NEXT_INSTRUCTION();
            // Shave off any decryption fuckery nop calls.
            SKIP_FUCKERY_NOP_CALLS();

            // We found the beginning of the UObject::ObjectFlagsEncrypted decryption.
            ObjectFlagsDecryptionBegin = NextIp;
            LOG_INFO(_XOR_("Found the UObject::ObjectFlagsEncrypted decryption beginning: 0x%p"),
                     ObjectFlagsDecryptionBegin);
            // Search for the ending of the UObject::ObjectFlagsEncrypted inline decryption.
            DisData* PrevInst = NextInst;
            uint8_t* PrevIp = NextIp;
            while (!(PrevInst->opcode == I_SHR &&
                     NextInst->opcode == I_TEST)) {
                PrevInst = NextInst;
                PrevIp = NextIp;
                INCREMENT_NEXT_INSTRUCTION();
            }
            // We found the ending of the UObject::ObjectFlagsEncrypted inline encryption.
            ObjectFlagsDecryptionEnd = PrevIp;
            LOG_INFO(_XOR_("Found the UObject::ObjectFlagsEncrypted decryption ending: 0x%p"),
                     ObjectFlagsDecryptionEnd);

            // Handle the next instruction.
            goto Next2;
        }

        // The first call instruction is a call to the UObjectBaseUtility::GetOutermost routine.
        if (Inst->opcode == I_CALL) {
            // Make sure this is the correct call instruction!
            if (Inst->ops[0].type == O_PC) {
                // Get the address of the UObjectBaseUtility::GetOutermost routine.
                TargetAddress = INSTRUCTION_GET_TARGET(Inst);
                LOG_INFO(_XOR_("Found the UObjectBaseUtility::GetOutermost routine: 0x%p"),
                         TargetAddress);
                // We're done!
                break;
            } else {
                LOG_WARN(_XOR_("Not the correct call to UObjectBaseUtility::GetOutermost routine!"));
            }

            // Handle the next instruction.
            goto Next2;
        }

    Next2:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    // Check if we found the UObjectBaseUtility::GetOutermost routine address.
    if (!TargetAddress) {
        LOG_ERROR(_XOR_("Failed to find UObjectBaseUtility::GetOutermost!"));
        return ERROR_NOT_FOUND;
    }

    // Set the Found address to UObjectBaseUtility::GetOutermost.
    Found = reinterpret_cast<const uint8_t *>(TargetAddress);
    //LOG_INFO(_XOR_("UObjectBaseUtility::GetOutermost code = {"));
    //std::string GetOutermostString = utils::FormatBuffer(Found, 256);
    //for (auto&& Line : utils::SplitString(GetOutermostString, '\n')) {
    //    LOG_INFO(Line.c_str());
    //}
    //LOG_INFO(_XOR_("};"));

    // Disassemble the UObjectBaseUtility::GetOutermost routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble UObjectBaseUtility::GetOutermost!"));
        return rc;
    }

    // Heuristically search for offsets and inline decryption.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the beginning of the OuterEncrypted decryption, which
        // is near the beginning within the first 7 instructions.
        /* mov     rbx, [rcx+10h] ; Object->OuterEncrypted */
        if (InstIndex < 10 &&
            Inst->opcode == I_MOV &&
            Inst->ops[0].type == O_REG &&
            Inst->ops[1].type == O_SMEM) {

            // We found the UObject::OuterEncrypted offset!
            ObjectOuterEncryptedOffset = static_cast<size_t>(Inst->disp);
            ObjectOuterEncryptedRegister = Inst->ops[0].index;
            LOG_INFO(_XOR_("Found UObject::OuterEncrypted offset: 0x%X (%s)"),
                     ObjectOuterEncryptedOffset, RegisterStrings[ObjectOuterEncryptedRegister]);

            // Increment next instruction.
            INCREMENT_NEXT_INSTRUCTION();
            // Shave off any decryption fuckery nop calls.
            SKIP_FUCKERY_NOP_CALLS();

            // We found the beginning of the UObject::OuterEncrypted inline decryption.
            ObjectOuterDecryptionBegin = NextIp;
            LOG_INFO(_XOR_("Found the UObject::OuterEncrypted decryption beginning: 0x%p"),
                     ObjectOuterDecryptionBegin);
            // Search for the end of the UObject::OuterEncrypted inline decryption.
            INCREMENT_NEXT_INSTRUCTION();
            while (META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            // We found the ending of the UObject::OuterEncrypted inline decryption.
            ObjectOuterDecryptionEnd = NextIp;
            LOG_INFO(_XOR_("Found the UObject::OuterEncrypted decryption ending: 0x%p"),
                     ObjectOuterDecryptionEnd);

            // We are done!
            break;
        }

//  Next3:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    if (ObjectFlagsEncryptedOffset != -1) {
        LOG_INFO(_XOR_("UObject::ObjectFlagsEncrypted offset = 0x%08x"), ObjectFlagsEncryptedOffset);
        LOG_INFO(_XOR_("UObject::ObjectFlagsEncrypted register = %s"), RegisterStrings[ObjectFlagsEncryptedRegister]);
        LOG_INFO(_XOR_("UObject::ObjectFlagsEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(ObjectFlagsDecryptionBegin,
                                                       __MIN(static_cast<size_t>(ObjectFlagsDecryptionEnd - ObjectFlagsDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    // Output the hueristic search results.
    if (ObjectOuterEncryptedOffset != -1) {
        LOG_INFO(_XOR_("UObject::OuterEncrypted offset = 0x%08x"), ObjectOuterEncryptedOffset);
        LOG_INFO(_XOR_("UObject::OuterEncrypted register = %s"), RegisterStrings[ObjectOuterEncryptedRegister]);
        LOG_INFO(_XOR_("UObject::OuterEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(ObjectOuterDecryptionBegin,
                                                       __MIN(static_cast<size_t>(ObjectOuterDecryptionEnd - ObjectOuterDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }


    // Signature inside of ObjectBaseUtility::GetFullName to get the
    // ClassEncrypted and NameEncrypted fields of UObject.
    // BA 80 00 00 00 E8 ? ? ? ? 48 8B 7D ?
    //const uint8_t GetFullNameSig[] = {
    //    0xBA, 0x80, 0x00, 0x00, 0x00,               /* mov     edx, 80h */
    //    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,               /* call    FString__Empty */
    //    0x48, 0x8B, 0x7D /*,0xCC*/                  /* mov     rdi, [rbp+20h] ; Object->ClassEncrypted */
    //    // This one also works:
    //    //0xE8, 0xCC, 0xCC, 0xCC, 0xCC,             /* call    UObjectBaseUtility__GetFullName */
    //    //0x4C, 0x8B, 0xF8,                         /* mov     r15, rax */
    //    //0x48, 0x8D, 0x55, 0xD8                    /* lea     rdx, [rbp-28h] */
    //};
    //Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, GetFullNameSig, sizeof(GetFullNameSig));
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("BA 80 00 00 00 E8 ?? ?? ?? ?? 48 8B 7D"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UObjectBaseUtility::GetFullName signature!"));
        return ERROR_NOT_FOUND;
    }

    // Disassemble the UObjectBaseUtility::GetFullName routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble UObjectBaseUtility::GetFullName!"));
        return rc;
    }

    // Heuristically search for offsets and inline decryption.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the beginning of the ClassEncrypted decryption.
        if (Inst->opcode == I_MOV &&
            Inst->ops[0].type == O_REG &&
            Inst->ops[1].type == O_IMM &&
            Inst->imm.dword == 0x80 &&
            NextInst->opcode == I_CALL) {

            // Skip the FString::Empty call.
            INCREMENT_NEXT_INSTRUCTION();

            /* mov     rdi, [rbp+20h] ; Object->ClassEncrypted */
            if (NextInst->opcode == I_MOV &&
                NextInst->ops[0].type == O_REG &&
                NextInst->ops[1].type == O_SMEM) {

                // We found the beginning of the ClassEncrypted decryption!
                ObjectClassEncryptedOffset = static_cast<size_t>(NextInst->disp);
                ObjectClassEncryptedRegister = NextInst->ops[0].index;
                INCREMENT_NEXT_INSTRUCTION();

                // Shave off the decryption fuckery nop calls.
                SKIP_FUCKERY_NOP_CALLS();
                // The next series of instructions is the ClassEncrypted decryption.
                ObjectClassDecryptionBegin = NextIp;
                LOG_INFO(_XOR_("Found the UObject::ClassEncrypted decryption beginning: 0x%p"),
                         ObjectClassDecryptionBegin);

                // This should now mark the beginning of the UObject::ClassEncrypted decryption.
                // Keep iterating next instruction until we hit the end of the decryption.
                INCREMENT_NEXT_INSTRUCTION();
                // Search for the decryption terminating instruction:
                /*mov     edi, [rbx+0Ch] ; _EDI = Class->NameEncrypted.NumberEncrypted */
                while (!(NextInst->opcode == I_MOV &&
                         NextInst->ops[1].type == O_SMEM)) {
                    INCREMENT_NEXT_INSTRUCTION();
                }
                // We found the end of the UObject::ClassEncrypted inline decryption!
                ObjectClassDecryptionEnd = NextIp;
                LOG_INFO(_XOR_("Found the UObject::ClassEncrypted decryption ending: 0x%p"),
                         ObjectClassDecryptionEnd);

                // The beginning of the UObject::NameEncrypted decryption is the end of the
                // UObject::ClassEncrypted decryption. Handle this as such:
                /* xor     rbx, rdi         ; <--- ClassEncrypted decryption ends here      */
                /* mov     edi, [rbx+0Ch]   ; _EDI = Class->NameEncrypted.NumberEncrypted   */
                /* call    NopSubroutine                                                    */
                /* call    NopSubroutine                                                    */
                /* call    NopSubroutine                                                    */
                /* mov     ebx, [rbx+8]     ; _EBX = Class->NameEncrypted.IndexEncrypted    */
                /* call    NopSubroutine                                                    */
                /* call    NopSubroutine                                                    */
                /* call    NopSubroutine                                                    */
                /* xor     ebx, 0DF808891h  ; <--- NameEncrypted decryption begins here     */
                ObjectNameNumberEncryptedOffset = static_cast<size_t>(NextInst->disp);
                ObjectNameNumberEncryptedRegister = NextInst->ops[0].index;
                // Increment the next instruction.
                INCREMENT_NEXT_INSTRUCTION();
                 // Shave off any decryption fuckery nop calls.
                SKIP_FUCKERY_NOP_CALLS();
                ObjectNameIndexEncryptedOffset = static_cast<size_t>(NextInst->disp);
                ObjectNameIndexEncryptedRegister = NextInst->ops[0].index;
                // Increment instruction.
                INCREMENT_NEXT_INSTRUCTION();
                // Shave off any decryption fuckery nop calls.
                SKIP_FUCKERY_NOP_CALLS();
                // The next series of instructions is the NameEncrypted decryption.
                ObjectNameDecryptionBegin = NextIp;
                LOG_INFO(_XOR_("Found the UObject::NameEncrypted decryption beginning: 0x%p"),
                         ObjectNameDecryptionBegin);
                // The call to FName::AppendString marks the end of the NameEncrypted decryption:
                /* lea     rcx, [rsp+58h+DecryptedName] ; Decrypted name */
                /* call    FName__AppendString ; End of the name decryption */
                INCREMENT_NEXT_INSTRUCTION();
                while (NextInst->opcode != I_LEA) {
                    INCREMENT_NEXT_INSTRUCTION();
                }
                INCREMENT_NEXT_INSTRUCTION();
                 // We found the end of the NameEncrypted decryption!
                ObjectNameDecryptionEnd = NextIp;
                LOG_INFO(_XOR_("Found the UObject::NameEncrypted decryption ending: 0x%p"),
                         ObjectNameDecryptionEnd);

                // We're done!
                break;

            } else {
                LOG_ERROR(_XOR_("Failed to find ClassEncrypted offset and decryption!"));
            }

            // Handle the next instruction.
            goto Next4;
        }

    Next4:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    // Output the hueristic search results.
    if (ObjectClassEncryptedOffset != -1) {
        LOG_INFO(_XOR_("UObject::ClassEncrypted offset = 0x%08x"), ObjectClassEncryptedOffset);
        LOG_INFO(_XOR_("UObject::ClassEncrypted register = %s"), RegisterStrings[ObjectClassEncryptedRegister]);
        LOG_INFO(_XOR_("UObject::ClassEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(ObjectClassDecryptionBegin,
                                                       __MIN(static_cast<size_t>(ObjectClassDecryptionEnd - ObjectClassDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    if (ObjectNameIndexEncryptedOffset != -1) {
        LOG_INFO(_XOR_("UObject::NameEncrypted.Index offset = 0x%08x"), ObjectNameIndexEncryptedOffset);
        LOG_INFO(_XOR_("UObject::NameEncrypted.Index register = %s"), RegisterStrings[ObjectNameIndexEncryptedRegister]);
        LOG_INFO(_XOR_("UObject::NameEncrypted.Number offset = 0x%08x"), ObjectNameNumberEncryptedOffset);
        LOG_INFO(_XOR_("UObject::NameEncrypted.Number register = %s"), RegisterStrings[ObjectNameNumberEncryptedRegister]);
        LOG_INFO(_XOR_("UObject::NameEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(ObjectNameDecryptionBegin,
                                                       __MIN(static_cast<size_t>(ObjectNameDecryptionEnd - ObjectNameDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }


    // Search for the following sequence inside of UStruct::Link:
    // .text:7FF6E834CEBD E8 CE 08 00 00        call    UScriptStruct__PrepareCppStructOps
    // .text:7FF6E834CEC2 49 8B 8E F0 00 00 00  mov     rcx, [r14+0F0h]
    // .text:7FF6E834CEC9 45 33 ED              xor     r13d, r13d
    // .text:7FF6E834CECC 48 85 C9              test    rcx, rcx
    // .text:7FF6E834CECF 0F 84 F9 00 00 00     jz      loc_7FF6E834CFCE
    // .text:7FF6E834CED5 8B 41 0C              mov     eax, [rcx+0Ch]      ; EAX = CppStructOps->Alignment
    // .text:7FF6E834CED8 41 89 86 90 00 00 00  mov     [r14+90h], eax      ; this->MinAlignment = EAX
    // .text:7FF6E834CEDF 8B 41 08              mov     eax, [rcx+8]        ; EAX = CppStructOps->Size
    // .text:7FF6E834CEE2 41 89 86 C0 00 00 00  mov     [r14+0C0h], eax     ; this->PropertiesSize = EAX
    // 48 85 C9 0F 84 ? ? ? ? 8B 41 0C 41 89
    //const uint8_t UStructLinkSig[] = {
    //    0x48, 0x85, 0xC9,
    //    0x0F, 0x84, 0xCC, 0xCC, 0xCC, 0xCC,
    //    0x8B, 0x41, 0x0C,
    //    0x41, 0x89 /*,0xCC,0xCC,0xCC,0xCC,0xCC*/
    //};
    //Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, UStructLinkSig, sizeof(UStructLinkSig));
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 85 C9 0F 84 ?? ?? ?? ?? 8B 41 0C 41 89"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find Link signature!"));
        return ERROR_NOT_FOUND;
    }

    // Disassemble the UStruct::Link routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble Link!"));
        return rc;
    }

    // Heuristically search for offsets.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the following instructions:
        /* mov     [r14+90h], eax   ; this->MinAlignment = EAX */
        /* mov     [r14+0C0h], eax  ; this->PropertiesSize = EAX */
        if (Inst->opcode == I_MOV &&
            Inst->ops[0].type == O_SMEM &&
            Inst->ops[1].type == O_REG) {

            if (StructMinAlignmentOffset == -1) {

                StructMinAlignmentOffset = static_cast<size_t>(Inst->disp);
                LOG_INFO(_XOR_("Found UStruct::MinAlignment offset: 0x%08x"),
                         StructMinAlignmentOffset);

            } else if (StructPropertiesSizeOffset == -1) {

                StructPropertiesSizeOffset = static_cast<size_t>(Inst->disp);
                LOG_INFO(_XOR_("Found UStruct::PropertiesSize offset: 0x%08x"),
                         StructPropertiesSizeOffset);
            }

            // Handle the next instruction.
            goto Next5;
        }

        /* mov     rsi, [rcx+0A0h]  ; RSI = this->Children */
        /* test    rsi, rsi */
        if (Inst->opcode == I_MOV &&
            Inst->ops[0].type == O_REG &&
            Inst->ops[1].type == O_SMEM &&
            NextInst->opcode == I_TEST) {

            StructChildrenOffset = static_cast<size_t>(Inst->disp);
            LOG_INFO(_XOR_("Found UStruct::Children offset: 0x%08x"),
                     StructChildrenOffset);

            break; // We are done!
        }

    Next5:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    // Search for the following sequence inside of UClass::CreateDefaultObject:
    // .text:7FF6E83473F0 48 8B 99 B8 00 00 00  mov     rbx, [rcx+0B8h]     ; RBX = this->SuperStruct
    // .text:7FF6E83473F7 45 33 E4              xor     r12d, r12d
    // .text:7FF6E83473FA 4C 89 64 24 60        mov     [rsp+210h+ParentDefaultObject], r12
    // .text:7FF6E83473FF 48 85 DB              test    rbx, rbx
    // .text:7FF6E8347402 74 36                 jz      short loc_7FF6E834743A
    // 48 8B 99 ? ? ? ? 45 33 E4 4C 89 64 24 ?
    // 48 8B 59 ? 45 33 E4 4C 89 64 24 ?
    //const uint8_t CreateDefaultObjectSig0[] = {
    //    0x48, 0x8B, 0x99, 0xCC, 0xCC, 0xCC, 0xCC,   /* mov     rbx, [rcx+0B8h] */
    //    0x45, 0x33, 0xE4,                           /* xor     r12d, r12d */
    //    0x4C, 0x89, 0x64, 0x24 /*,0xCC*/            /* mov     [rsp+210h+ParentDefaultObject], r12 */
    //};
    //const uint8_t CreateDefaultObjectSig1[] = {
    //    0x48, 0x8B, 0x59, 0xCC,                     /* mov     rbx, [rcx+38h] */
    //    0x45, 0x33, 0xE4,                           /* xor     r12d, r12d */
    //    0x4C, 0x89, 0x64, 0x24 /*,0xCC*/            /* mov     [rsp+210h+ParentDefaultObject], r12 */
    //};
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8B 99 ?? ?? ?? ?? 45 33 E4 4C 89 64 24"));
    if (!Found) {
        Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8B 59 ?? 45 33 E4 4C 89 64 24"));
    }
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UClass::CreateDefaultObject signature!"));
        return ERROR_NOT_FOUND;
    }
    LOG_INFO(_XOR_("Found the UClass::CreateDefaultObject signature: 0x%p"), Found);
    // Disassemble the instruction.
    Inst = Insts;
    if (DisInstruction(Found, Inst, DISASM_X64)) {

        StructSuperStructOffset = static_cast<size_t>(Inst->disp);
        LOG_INFO(_XOR_("Found UStruct::SuperStruct offset: 0x%08x"),
                 StructSuperStructOffset);

    } else {
        LOG_ERROR(_XOR_("Failed to disassemble UClass::CreateDefaultObject instruction!"));
        return RtlNtStatusToDosError(STATUS_ILLEGAL_INSTRUCTION);
    }

    // Search for the following sequence inside of UObject::ProcessEvent:
    // .text:7FF7F1956879 F7 86 E0 00 00 00 00 00 40 00 test    dword ptr [rsi+0E0h], 400000h
    // .text:7FF7F1956883 74 64                         jz      short loc_7FF7F19568E9
    // .text:7FF7F1956885 4C 8D 85 98 00 00 00          lea     r8, [rbp+100h+var_70+8]
    // F7 86 ? ? ? ? 00 00 40 00 74 ?
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("F7 86 ?? ?? ?? ?? 00 00 40 00 74"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UObject::ProcessEvent signature!"));
        return ERROR_NOT_FOUND;
    }
    LOG_INFO(_XOR_("Found the UObject::ProcessEvent signature: 0x%p"), Found);

    // Disassemble the instruction.
    Inst = Insts;
    if (DisInstruction(Found, Inst, DISASM_X64)) {

        FunctionFlagsOffset = static_cast<size_t>(Inst->disp);
        LOG_INFO(_XOR_("Found UFunction::FunctionFlags offset: 0x%08x"),
                 FunctionFlagsOffset);

    } else {
        LOG_ERROR(_XOR_("Failed to disassemble UObject::ProcessEvent instruction!"));
        return RtlNtStatusToDosError(STATUS_ILLEGAL_INSTRUCTION);
    }

    // Output the hueristic search results.
    if (StructMinAlignmentOffset != -1) {
        LOG_INFO(_XOR_("UStruct::MinAlignment offset = 0x%08x"), StructMinAlignmentOffset);
    }
    if (StructPropertiesSizeOffset != -1) {
        LOG_INFO(_XOR_("UStruct::PropertiesSize offset = 0x%08x"), StructPropertiesSizeOffset);
    }
    if (StructChildrenOffset != -1) {
        LOG_INFO(_XOR_("UStruct::Children offset = 0x%08x"), StructChildrenOffset);
    }
    if (StructSuperStructOffset != -1) {
        LOG_INFO(_XOR_("UStruct::SuperStruct offset = 0x%08x"), StructSuperStructOffset);
    }
    if (FunctionFlagsOffset != -1) {
        LOG_INFO(_XOR_("UFunction::FunctionFlags offset = 0x%08x"), FunctionFlagsOffset);
    }


    // Search for sequence of instructions inside of UProperty::SetupOffset:
    // .text:7FF7F1E22334 FF C8         dec     eax
    // .text:7FF7F1E22336 F7 D0         not     eax
    // .text:7FF7F1E22338 23 C8         and     ecx, eax
    // .text:7FF7F1E2233A 8B 47 3C      mov     eax, [rdi+3Ch]  ; _EAX = this->ElementSize
    // .text:7FF7F1E2233D 0F AF 47 38   imul    eax, [rdi+38h]  ; _EAX = this->ArrayDim * this->ElementSize
    // .text:7FF7F1E22341 89 4F 58      mov     [rdi+58h], ecx  ; this->Offset_Internal = NewOffset
    // .text:7FF7F1E22344 03 C1         add     eax, ecx        ; return NewOffset + Size
    // 8B 47 ? 0F AF 47 ? 89 4F ?
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("8B 47 ?? 0F AF 47 ?? 89 4F"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UProperty::SetupOffset signature!"));
        return ERROR_NOT_FOUND;
    }
    // Disassemble the UProperty::SetupOffset instructions.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MAX_INSTRUCTION_COUNT,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble UProperty::SetupOffset code!"));
        return rc;
    }

    // Get the offsets.
    if (InstCount >= 3) {

        PropertyElementSizeOffset = Insts[0].disp;
        PropertyArrayDimOffset = Insts[1].disp;
        PropertyOffsetInternalOffset = Insts[2].disp;

    } else {
        LOG_ERROR(_XOR_("Failed to find UProperty offsets inside UProperty::SetupOffset!"));
    }

    // Search for sequence of instructions inside of UEnumProperty::LinkInternal:
    // .text:7FF7F210777B 48 8B CB                      mov     rcx, rbx                ; this
    // .text:7FF7F210777E E8 CD AA D1 FF                call    UProperty__SetupOffset  
    // .text:7FF7F210777E
    // .text:7FF7F2107783 48 8B 96 80 00 00 00          mov     rdx, [rsi+80h]          ; _RDX = this->UnderlyingProp
    // .text:7FF7F210778A 48 B9 00 00 00 00 00 00 08 00 mov     rcx, 8000000000000h     ; _RCX = 0x8000000000000
    // .text:7FF7F2107794 48 8B 5C 24 30                mov     rbx, [rsp+30h]
    // .text:7FF7F2107799 8B 42 3C                      mov     eax, [rdx+3Ch]          ; _EAX = UnderlyingProp->ElementSize
    // .text:7FF7F210779C 89 46 3C                      mov     [rsi+3Ch], eax          ; this->ElementSize = _EAX
    // .text:7FF7F210779F 48 B8 00 02 00 40 10 00 00 00 mov     rax, 1040000200h        ; _RAX = 0x1040000200
    // .text:7FF7F21077A9 48 09 46 40                   or      [rsi+40h], rax          ; this->PropertyFlags |= 0x1040000200
    // .text:7FF7F21077AD 48 8B 42 40                   mov     rax, [rdx+40h]          ; _RAX = UnderlyingProp->PropertyFlags
    // .text:7FF7F21077B1 48 23 C1                      and     rax, rcx                ; _RAX &= 0x8000000000000
    // .text:7FF7F21077B4 48 09 46 40                   or      [rsi+40h], rax          ; this->PropertyFlags |= UnderlyingProp->PropertyFlags & 0x8000000000000
    // 48 B8 00 02 00 40 10 00 00 00 48 09 46 ? 48 8B 42 ? 48 23 C1
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 B8 00 02 00 40 10 00 00 00 48 09 46 ?? 48 8B 42"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UEnumProperty::LinkInternal signature!"));
        return ERROR_NOT_FOUND;
    }
    // Disassemble the UEnumProperty::LinkInternal instruction.
    Inst = Insts;
    if (!DisInstruction(Found + 10, Inst, DISASM_X64)) {
        LOG_ERROR(_XOR_("Failed to disassemble UEnumProperty::LinkInternal instruction!"));
        return rc;
    }
    // Get the offset.
    if (Inst->opcode == I_OR && Inst->dispOffset > 0) {
        PropertyFlagsOffset = Inst->disp;
    }

    // Output the search results.
    if (PropertyElementSizeOffset != -1) {
        LOG_INFO(_XOR_("UProperty::ElementSize offset = 0x%08x"), PropertyElementSizeOffset);
    }
    if (PropertyArrayDimOffset != -1) {
        LOG_INFO(_XOR_("UProperty::ArrayDim offset = 0x%08x"), PropertyArrayDimOffset);
    }
    if (PropertyOffsetInternalOffset != -1) {
        LOG_INFO(_XOR_("UProperty::Offset_Internal offset = 0x%08x"), PropertyOffsetInternalOffset);
    }
    if (PropertyFlagsOffset != -1) {
        LOG_INFO(_XOR_("UProperty::PropertyFlags offset = 0x%08x"), PropertyFlagsOffset);
    }



    // Search for the UEnum::CppForm instruction inside of UEnum::SetEnums.
    // .text:7FF7F21BE776 44 89 7B 68   mov     [rbx+68h], r15d ; this->CppForm = InCppForm
    // .text:7FF7F21BE77A 45 84 F6      test    r14b, r14b
    // 44 89 7B ? 45 84 F6
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("44 89 7B ?? 45 84 F6"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UEnum::SetEnums signature!"));
        return ERROR_NOT_FOUND;
    }
    // Get the function start address.
    rc = utils::FindFunctionStartFromPtr(Found, 1024, &Found);
    if (rc < 0) {
        LOG_ERROR(_XOR_("Failed to backtrack to beginning of UEnum::SetEnums!"));
        return rc;
    }

    // Disassemble the UEnum::SetEnums instruction.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MAX_INSTRUCTION_COUNT,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble UEnum::SetEnums code!"));
        return rc;
    }

    // Heuristically search for offsets.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the UEnum::Names field offset.
        if (Inst->opcode == I_LEA && NextInst->opcode == I_CMP) {
            EnumNamesOffset = static_cast<size_t>(Inst->disp);
            goto Next7;
        }
        // Search for the UEnum::CppForm field offset.
        if (Inst->opcode == I_MOV && 
            Inst->ops[0].type == O_SMEM &&
            Inst->ops[0].index == R_RBX &&
            NextInst->opcode == I_TEST) {
            EnumCppFormOffset = static_cast<size_t>(Inst->disp);
            // We are done!
            break;
        }

    Next7:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    // Output the search results.
    if (EnumNamesOffset != -1) {
        LOG_INFO(_XOR_("UEnum::Names offset = 0x%08x"), EnumNamesOffset);
    }
    if (EnumCppFormOffset != -1) {
        LOG_INFO(_XOR_("UEnum::CppForm offset = 0x%08x"), EnumCppFormOffset);
    }


    return rc;
}

int32_t GenerateStructString(_Out_ std::string* OutString,
                             _In_ std::wstring Name,
                             _In_opt_ std::vector<std::wstring>* InheritedStructs,
                             _In_opt_ FieldMapType* FieldMap)
{
    size_t Offset = 0;
    size_t Size = 0;
    size_t StructSize = 0;
    ObjectSizeMapType::iterator SizeIter;
    FieldMapType::iterator FieldIter;

    std::ostringstream oss;

    std::string NameString = utils::wstring_to_string(Name);

    if (!InheritedStructs) {

        oss << _XORSTR_("class ") << NameString << _XORSTR_(" {\n");

    } else {

        std::vector<std::wstring>::iterator InheritedStructIter = InheritedStructs->begin();

        // Add the first inherited struct to the inherit string.
        std::wstring InheritList = _XORSTR_(L": public ") + *InheritedStructIter;

        // Adjust the offset.
        SizeIter = ObjectSizeMap.find(*InheritedStructIter);
        if (SizeIter != ObjectSizeMap.end()) {
            Offset += SizeIter->second;
        } else {
            LOG_ERROR(_XOR_("COULD NOT FIND INHERITED STRUCT \"%ws\" SIZE!"),
                      InheritedStructIter->c_str());
            return E_NOT_SET;
        }

        // Iterate to the second inherited struct.
        ++InheritedStructIter;
        // Iterate through the remaining inherited structs.
        while (InheritedStructIter != InheritedStructs->end()) {
            // Add inherited struct name to the inherit string.
            InheritList += _XORSTR_(L", public ") + *InheritedStructIter;

            // Adjust the offset.
            SizeIter = ObjectSizeMap.find(*InheritedStructIter);
            if (SizeIter != ObjectSizeMap.end()) {
                Offset += SizeIter->second;
            } else {
                LOG_ERROR(_XOR_("COULD NOT FIND INHERITED STRUCT \"%ws\" SIZE!"),
                          InheritedStructIter->c_str());
                return E_NOT_SET;
            }

            // Iterate to the next inherited struct.
            ++InheritedStructIter;
        }

        oss << _XORSTR_("class ") << NameString << ' ' <<
            utils::wstring_to_string(InheritList) << _XORSTR_(" {\n");
    }
    oss << _XORSTR_("public:\n");

    if (FieldMap) {

        // Dump the fields.
        FieldIter = FieldMap->begin();
        while (FieldIter != FieldMap->end()) {

            // Get this field map entry.
            FieldMapEntry Entry = FieldIter->second;

            size_t LastOffset = Offset;
            size_t LastSize = Size;
            Offset = FieldIter->first;
            Size = Entry.Size;

            // Dump padding field to fill in space, if needed.
            if (LastOffset + LastSize < Offset) {
                size_t PaddingSize = Offset - (LastOffset + LastSize);
                oss << tfm::format(_XOR_("    uint8_t UnknownData0x%04X[0x%X]; // 0x%04X (size=0x%04X)\n"),
                                   LastOffset + LastSize, PaddingSize, LastOffset + LastSize, PaddingSize);
            }

            // Dump the field.
            oss << tfm::format(_XOR_("    %s %s; // 0x%04X (size=0x%04X)\n"),
                               Entry.TypeName, Entry.Name, Offset, Size);

            // Iterate to the next field.
            ++FieldIter;
        }
    } 

    // Dump the struct size.
    SizeIter = ObjectSizeMap.find(Name);
    if (SizeIter != ObjectSizeMap.end()) {

        StructSize = SizeIter->second;
        if (StructSize > Offset + Size) {
            size_t PaddingSize = StructSize - (Offset + Size);
            oss << tfm::format(_XOR_("    uint8_t UnknownData0x%04X[0x%X]; // 0x%04X (size=0x%04X)\n"),
                               Offset + Size, PaddingSize, Offset + Size, PaddingSize);
        }
        oss << tfm::format(_XOR_("}; // size=0x%04X\n"), StructSize);

    } else {

        StructSize = Offset + Size;
        oss << tfm::format(_XOR_("}; // guessed size=0x%04X"), StructSize);
    }

    // Dump a C_ASSERT for verification or struct size.
    oss << tfm::format(_XOR_("C_ASSERT(sizeof(%s) == 0x%X);\n"), NameString.c_str(), StructSize);

    // Return resulting output string.
    *OutString = oss.str();

    // Return struct size, indicating success.
    return static_cast<int32_t>(StructSize);
}

int DumpStructs()
{
    int rc;
    size_t Offset;
    FieldMapType::iterator FieldIter;
    ObjectSizeMapType::iterator SizeIter;
    size_t UObjectSize, UFieldSize, UStructSize, UFunctionSize, UPropertySize, UEnumSize;

    // Get the needed Unreal object struct sizes.
    rc = InitializeUnrealObjectSizeMap();
    if (rc != NOERROR) {
        LOG_ERROR(_XOR_("Failed to initialize Unreal object size map with rc %d"), rc);
        return rc;
    }

#if defined(_DEBUG)
    // Dump the Unreal object struct sizes.
    LOG_INFO(_XOR_("Unreal Object Sizes:"));
    for (auto it : UnrealObjectSizeMap) {
        LOG_INFO(_XOR_("%ws size = 0x%08x"), it.first.c_str(), it.second);
    }
#endif // _DEBUG

    //
    // Dump the UObject fields.
    //
    Offset = 0;
    std::string UObjectOutput;
    FieldMapType UObjectFieldMap;
    UObjectFieldMap[Offset] = FIELD_MAP_ENTRY("VTable", void**);
    UObjectFieldMap[ObjectFlagsEncryptedOffset] = FIELD_MAP_ENTRY("ObjectFlagsEncrypted", int32_t);
    UObjectFieldMap[ObjectOuterEncryptedOffset] = FIELD_MAP_ENTRY("OuterEncrypted", uint64_t);
    UObjectFieldMap[ObjectInternalIndexEncryptedOffset] = FIELD_MAP_ENTRY("InternalIndexEncrypted", int32_t);
    UObjectFieldMap[ObjectClassEncryptedOffset] = FIELD_MAP_ENTRY("ClassEncrypted", uint64_t);
    UObjectFieldMap[ObjectNameIndexEncryptedOffset] = FIELD_MAP_ENTRY("NameIndexEncrypted", int32_t);
    UObjectFieldMap[ObjectNameNumberEncryptedOffset] = FIELD_MAP_ENTRY("NameNumberEncrypted", int32_t);
    UObjectSize = GenerateStructString(&UObjectOutput, _XORSTR_(L"UObject"), nullptr, &UObjectFieldMap);
    if (UObjectSize <= 0) {
        LOG_ERROR(_XOR_("Failed to dump UObject rc %d"), rc);
        return rc < 0 ? rc : E_FAIL;
    }
    for (auto&& Line : utils::SplitString(UObjectOutput, '\n')) {
        LOG_INFO("%s", Line.c_str());
    }

    //
    // Dump the UField fields.
    //
    Offset = UObjectSize;
    std::string UFieldOutput;
    FieldMapType UFieldFieldMap;
    UFieldFieldMap[Offset] = FieldMapEntry(_XOR_("Next"), _XOR_("UField*"), sizeof(void*));
    std::vector<std::wstring> UFieldInherited;
    UFieldInherited.push_back(_XORSTR_(L"UObject"));
    UFieldSize = GenerateStructString(&UFieldOutput, _XORSTR_(L"UField"), &UFieldInherited, &UFieldFieldMap);
    if (UFieldSize <= 0) {
        LOG_ERROR(_XOR_("Failed to dump UField rc %d"), rc);
        return rc < 0 ? rc : E_FAIL;
    }
    for (auto&& Line : utils::SplitString(UFieldOutput, '\n')) {
        LOG_INFO("%s", Line.c_str());
    }

    //
    // Dump the UStruct fields.
    //
    Offset = UFieldSize;
    std::string UStructOutput;
    FieldMapType UStructFieldMap;
    UStructFieldMap[StructPropertiesSizeOffset] = FIELD_MAP_ENTRY("PropertiesSize", int32_t);
    UStructFieldMap[StructMinAlignmentOffset] = FIELD_MAP_ENTRY("MinAlignment", int32_t);
    UStructFieldMap[StructChildrenOffset] = FieldMapEntry("Children", "UField*", sizeof(void*));
    UStructFieldMap[StructSuperStructOffset] = FieldMapEntry("SuperStruct", "class UStruct*", sizeof(void*));
    std::vector<std::wstring> UStructInherited;
    UStructInherited.push_back(_XORSTR_(L"UField"));
    UStructSize = GenerateStructString(&UStructOutput, _XORSTR_(L"UStruct"), &UStructInherited, &UStructFieldMap);
    if (UStructSize <= 0) {
        LOG_ERROR(_XOR_("Failed to dump UStruct rc %d"), rc);
        return rc < 0 ? rc : E_FAIL;
    }
    for (auto&& Line : utils::SplitString(UStructOutput, '\n')) {
        LOG_INFO("%s", Line.c_str());
    }

    //
    // Dump the UFunction fields.
    //
    Offset = UStructSize;
    std::string UFunctionOutput;
    FieldMapType UFunctionFieldMap;
    UFunctionFieldMap[FunctionFlagsOffset] = FIELD_MAP_ENTRY("FunctionFlags", uint32_t);
    std::vector<std::wstring> UFunctionInherited;
    UFunctionInherited.push_back(_XORSTR_(L"UStruct"));
    UFunctionSize = GenerateStructString(&UFunctionOutput, _XORSTR_(L"UFunction"), &UFunctionInherited, &UFunctionFieldMap);
    if (UFunctionSize <= 0) {
        LOG_ERROR(_XOR_("Failed to dump UFunction rc %d"), rc);
        return rc < 0 ? rc : E_FAIL;
    }
    for (auto&& Line : utils::SplitString(UFunctionOutput, '\n')) {
        LOG_INFO("%s", Line.c_str());
    }

    //
    // Dump the UProperty fields.
    //
    Offset = UFieldSize;
    std::string UPropertyOutput;
    FieldMapType UPropertyFieldMap;
    UPropertyFieldMap[PropertyElementSizeOffset] = FIELD_MAP_ENTRY("ElementSize", int32_t);
    UPropertyFieldMap[PropertyArrayDimOffset] = FIELD_MAP_ENTRY("ArrayDim", int32_t);
    UPropertyFieldMap[PropertyOffsetInternalOffset] = FIELD_MAP_ENTRY("Offset_Internal", int32_t);
    UPropertyFieldMap[PropertyFlagsOffset] = FIELD_MAP_ENTRY("PropertyFlags", uint64_t);
    std::vector<std::wstring> UPropertyInherited;
    UPropertyInherited.push_back(_XORSTR_(L"UField"));
    UPropertySize = GenerateStructString(&UPropertyOutput, _XORSTR_(L"UProperty"), &UPropertyInherited, &UPropertyFieldMap);
    if (UPropertySize <= 0) {
        LOG_ERROR(_XOR_("Failed to dump UProperty rc %d"), rc);
        return rc < 0 ? rc : E_FAIL;
    }
    for (auto&& Line : utils::SplitString(UPropertyOutput, '\n')) {
        LOG_INFO("%s", Line.c_str());
    }


    //
    // Dump the UEnum fields.
    //
    std::string UEnumOutput;
    FieldMapType UEnumFieldMap;
    UEnumFieldMap[EnumNamesOffset] = FieldMapEntry(_XOR_("Names"), _XOR_("TArray<TPair<FName, int64_t>>"), 16);
    UEnumFieldMap[EnumCppFormOffset] = FIELD_MAP_ENTRY("CppForm", int32_t);
    std::vector<std::wstring> UEnumInherited;
    UEnumInherited.push_back(_XORSTR_(L"UField"));
    UEnumSize = GenerateStructString(&UEnumOutput, _XORSTR_(L"UEnum"), &UEnumInherited, &UEnumFieldMap);
    if (UEnumSize <= 0) {
        LOG_ERROR(_XOR_("Failed to dump UEnum rc %d"), rc);
        return rc < 0 ? rc : E_FAIL;
    }
    for(auto&& Line : utils::SplitString(UEnumOutput, '\n')) {
        LOG_INFO("%s", Line.c_str());
    }

    return NOERROR;
}





/**
 * Names offsets and inline decryption globals.
 */
int8_t NamesHashEntryRegister = -1;
size_t NamesEntryHashNextOffset = -1;
size_t NamesEntryIndexEncryptedOffset = -1;
int8_t NamesEntryIndexEncryptedRegister = -1;
uint8_t* NamesEntryIndexDecryptionBegin = nullptr;
uint8_t* NamesEntryIndexDecryptionEnd = nullptr;

uint8_t* NamesGetEncryptedNamesRoutine = nullptr;
int8_t NamesEncryptedRegister = -1;
uint8_t* NamesDecryptionBegin = nullptr;
uint8_t* NamesDecryptionEnd = nullptr;

uint8_t* NamesEntryArrayAddZeroedRoutine = nullptr;
size_t NamesEntryArrayElementsPerChunk = -1;
size_t NamesEntryArrayChunksEncryptedOffset = -1;
int8_t NamesEntryArrayChunksEncryptedRegister = -1;
uint8_t* NamesEntryArrayChunksDecryptionBegin = nullptr;
uint8_t* NamesEntryArrayChunksDecryptionEnd = nullptr;

size_t NamesEntryArrayNumElementsEncryptedOffset = -1;
int8_t NamesEntryArrayNumElementsEncryptedRegister = -1;
uint8_t* NamesEntryArrayNumElementsDecryptionBegin = nullptr;
uint8_t* NamesEntryArrayNumElementsDecryptionEnd = nullptr;

int DumpNames()
{
    int rc = 0;
    const uint8_t* Found;
    uint64_t TargetAddress;
    DECLARE_DISASM_VARS();

    void* ImageBase = utils::GetModuleHandleWIDE(nullptr /*0xC4D8736D TslGame.exe */);
    uint32_t ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // Search for the call to FName::InitInternal_FindOrAddNameEntry:
    // .text:7FF6FDBDF91A 48 8B 8C 24 C8 00 00 00   mov     rcx, [rsp+108h+InName]  ; InName
    // .text:7FF6FDBDF922 E8 79 00 00 00            call    FName__InitInternal_FindOrAddNameEntry
    // .text:7FF6FDBDF927 0F B6 F8                  movzx   edi, al
    // 48 8B 8C 24 ? ? ? ? E8 ? ? ? ? 0F B6 F8
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8B 8C 24 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 F8"));
    if (Found)
        Found = static_cast<const uint8_t*>(utils::GetCallTargetAddress(Found + 8));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find FName::InitInternal_FindOrAddNameEntry signature!"));
        return ERROR_NOT_FOUND;
    }
    LOG_INFO(_XOR_("Found the FName::InitInternal_FindOrAddNameEntry signature: 0x%p"), Found);

    //LOG_INFO(_XOR_("FName::InitInternal_FindOrAddNameEntry instruction bytes = {"));
    //for (auto&& Line : utils::SplitString(utils::FormatBuffer(Found, 2048), '\n')) {
    //    LOG_INFO(_XOR_("%s"), Line.c_str());
    //}
    //LOG_INFO(_XOR_("};"));

    // Disassemble the FName::InitInternal_FindOrAddNameEntry routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble FName::InitInternal_FindOrAddNameEntry!"));
        return rc;
    }



    // Heuristically search for offsets and inline decryption.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the name hash entry register.
        if (NamesHashEntryRegister == -1) {
            
            // Search for the following instruction sequence which are the instructions
            // which get the name hash entry from the hash table:
            /* movzx   eax, r9w */
            /* mov     rbx, [rcx+rax*8+611B120h] */
            /* test    rbx, rbx */
            if (Inst->opcode == I_MOV &&
                Inst->ops[1].type == O_MEM &&
                NextInst->opcode == I_TEST) {

                // We found the name hash entry register
                NamesHashEntryRegister = Inst->ops[0].index;
                LOG_INFO(_XOR_("Found the name hash entry register: %s"),
                         RegisterStrings[NamesHashEntryRegister]);

                // Make sure this is the right test instruction!
                if (NextInst->ops[0].index != NamesHashEntryRegister) {
                    LOG_ERROR(_XOR_("Name hash entry heuristic failed!"));
                }

                // Increment to the next instruction.
                INCREMENT_NEXT_INSTRUCTION();
            }

            // Handle the next instruction.
            goto Next6;
        }

        // Search for the FNameEntry::HashNext offset.
        if (NamesEntryHashNextOffset == -1 && NamesHashEntryRegister != -1) {

            // Check for the following instruction sequences:
            /* lea     rsi, [rbx+8] */
            /* mov     rax, [rsi] */
            /* prefetcht0 byte ptr [rax] */
            //          OR
            /* mov     rax, [rbx+8] */
            /* prefetcht0 byte ptr [rax] */
            if ((Inst->opcode == I_MOV && NextInst->opcode == I_PREFETCHT0) ||
                (Inst->opcode == I_LEA && NextInst[1].opcode == I_PREFETCHT0)) {

                // We found the FNameEntry::HashNext offset!
                NamesEntryHashNextOffset = static_cast<size_t>(Inst->disp);
                LOG_INFO(_XOR_("Found the FNameEntry::HashNext offset: %d"),
                         NamesEntryHashNextOffset);

                // Increment to the next instruction.
                INCREMENT_NEXT_INSTRUCTION();
            }

            // Handle the next instruction.
            goto Next6;
        }

        // Search for the FNameEntry::IndexEncrypted offset and inline decryption.
        if (NamesEntryIndexEncryptedOffset == -1) {

            // Check for the following instruction sequence:
            /* mov     rcx, [rbx] */
            /* cmp     qword ptr cs:aXenuinesdkCarv_93, 0 ; "XENUINESDK_CARVE" */
            if (Inst->opcode == I_MOV &&
                Inst->ops[1].type == O_SMEM &&
                Inst->ops[1].index == NamesHashEntryRegister) {

                // Make sure this is the right instruction!
                if (NextInst->opcode == I_CMP &&
                    NextInst->ops[1].type == O_IMM &&
                    NextInst->imm.dword == 0) {
                    
                    // We found the FNameEntry::IndexEncrypted offset!
                    NamesEntryIndexEncryptedOffset = static_cast<size_t>(Inst->disp);
                    NamesEntryIndexEncryptedRegister = Inst->ops[0].index;

                    LOG_INFO(_XOR_("Found the FNameEntry::IndexEncrypted offset: %d"), NamesEntryIndexEncryptedOffset);
                    LOG_INFO(_XOR_("Found the FNameEntry::IndexEncrypted register: %s"), RegisterStrings[NamesEntryIndexEncryptedRegister]);

                    // Increment to the next instruction.
                    INCREMENT_NEXT_INSTRUCTION();

                    // The jump target is the beginning of the decryption.
                    if (META_GET_FC(NextInst->meta) == FC_CND_BRANCH) {

                        // Skip instructions in between jump and decryption.
                        TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
                        while (NextInst->addr != TargetAddress) {
                            INCREMENT_NEXT_INSTRUCTION();
                        }
                        // We found the beginning of the FNameEntry::IndexEncrypted decryption!
                        NamesEntryIndexDecryptionBegin = NextIp;
                        LOG_INFO(_XOR_("Found the FNameEntry::IndexEncrypted decryption beginning: 0x%p"),
                                 NamesEntryIndexDecryptionBegin);

                        // This should now mark the beginning of the FNameEntry::IndexEncrypted
                        // decryption. Keep iterating next instruction until we hit the end of
                        // the decryption.
                        INCREMENT_NEXT_INSTRUCTION();
                        // Search for the decryption terminating instruction:
                        /* mov     rax, [rbp+4Fh+Decrypted] */
                        while (!(NextInst->opcode == I_MOV &&
                                 NextInst->ops[1].type == O_SMEM)) {
                            INCREMENT_NEXT_INSTRUCTION();
                        }
                        INCREMENT_NEXT_INSTRUCTION();
                        // We found the end of the FNameEntry::IndexEncrypted inline decryption!
                        NamesEntryIndexDecryptionEnd = NextIp;
                        LOG_INFO(_XOR_("Found the FNameEntry::IndexEncrypted decryption ending: 0x%p"),
                                 NamesEntryIndexDecryptionEnd);

                        // Increment to the next instruction.
                        INCREMENT_NEXT_INSTRUCTION();

                    } else {
                        LOG_ERROR(_XOR_("FNameEntry::IndexEncrypted decryption heuristic failed!"));
                    }
                }
            }

            // Handle the next instruction.
            goto Next6;
        }

        // Search for the FName::GetEncryptedNames subroutine.
        if (!NamesGetEncryptedNamesRoutine) {
        
            // Search for the following instruction sequence:
            /* lea     rcx, [rbp+4Fh+var_70] */
            /* call    FName__GetEncryptedNames */
            /* mov     rcx, [rax] */
            /* cmp     qword ptr cs:aXenuinesdkCarv_93, 0 ; "XENUINESDK_CARVE" */
            if (Inst->opcode == I_LEA &&
                NextInst->opcode == I_CALL &&
                NextInst[1].opcode == I_MOV &&
                NextInst[1].ops[1].type == O_SMEM) {

                // We found the FName::GetEncryptedNames subroutine!
                TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
                NamesGetEncryptedNamesRoutine = reinterpret_cast<uint8_t *>(TargetAddress);
                LOG_INFO(_XOR_("Found the FName::GetEncryptedNames routine: 0x%p"),
                         NamesGetEncryptedNamesRoutine);

                // Increment to the next instruction.
                INCREMENT_NEXT_INSTRUCTION();

                // Get the encrypted names register.
                NamesEncryptedRegister = NextInst->ops[0].index;

                // Find the jump instruction to the FName::NamesEncrypted inline decryption.
                INCREMENT_NEXT_INSTRUCTION();
                while(META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                    INCREMENT_NEXT_INSTRUCTION();
                }
                // The jump target address is the beginning of the FName::NamesEncrypted
                // inline decryption.
                TargetAddress = INSTRUCTION_GET_TARGET(NextInst);

                // Skip instructions in between jump and decryption.
                while (NextInst->addr != TargetAddress) {
                    INCREMENT_NEXT_INSTRUCTION();
                }
                // We found the beginning of the FName::NamesEncrypted decryption!
                NamesDecryptionBegin = reinterpret_cast<uint8_t *>(TargetAddress);
                LOG_INFO(_XOR_("Found the FName::NamesEncrypted decryption beginning: 0x%p"),
                         NamesDecryptionBegin);

                // This should now mark the beginning of the FName::NamesEncrypted decryption.
                // Keep iterating next instruction until we hit the end of the decryption.
                INCREMENT_NEXT_INSTRUCTION();
                // Search for the decryption terminating instruction:
                /* mov     rax, [rsp+120h+NamesDecrypted] */
                while (!(NextInst->opcode == I_MOV &&
                         NextInst->ops[1].type == O_SMEM)) {
                    INCREMENT_NEXT_INSTRUCTION();
                }
                INCREMENT_NEXT_INSTRUCTION();
                // We found the end of the FName::NamesEncryptedinline decryption!
                NamesDecryptionEnd = NextIp;
                LOG_INFO(_XOR_("Found the FName::NamesEncrypted decryption ending: 0x%p"),
                         NamesDecryptionEnd);

                // Increment to the next instruction.
                INCREMENT_NEXT_INSTRUCTION();
            }

            // Handle the next instruction.
            goto Next6;
        }

        // Search for the TNameEntryArray::AddZeroed subroutine.
        if (!NamesEntryArrayAddZeroedRoutine) {

            // Search for the following instruction sequence:
            /* mov     edx, 1 */
            /* mov     rcx, rax */
            /* call    TNameEntryArray__AddZeroed */
            if (Inst->opcode == I_MOV &&
                Inst->ops[1].type == O_IMM &&
                Inst->imm.dword == 1) {

                // Make sure this is the right instruction sequence!
                if (NextInst->opcode == I_MOV &&
                    NextInst[1].opcode == I_CALL &&
                    NextInst[1].ops[0].type == O_PC) {

                    // Increment to the next instruction.
                    INCREMENT_NEXT_INSTRUCTION();

                    // We found the address of the TNameEntryArray::AddZeroed subroutine!
                    TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
                    NamesEntryArrayAddZeroedRoutine = reinterpret_cast<uint8_t *>(TargetAddress);
                    LOG_INFO(_XOR_("Found the TNameEntryArray::AddZeroed routine: 0x%p"),
                             NamesEntryArrayAddZeroedRoutine);

                    // Increment to the next instruction.
                    INCREMENT_NEXT_INSTRUCTION();

                } else {
                    LOG_ERROR(_XOR_("TNameEntryArray::AddZeroed heuristic failed!"));
                }
            }

            // Handle the next instruction.
            goto Next6;
        }

        // Search for the TNameEntryArray::ElementsPerChunk value and the offset and inline
        // decryption of TNameEntryArray::ChunksEncrypted.
        if (NamesEntryArrayElementsPerChunk == -1) {
        
            // Search for the following instruction sequence:
            /* imul    eax, ebx, 15892  ; ChunkIndex = *OutIndex / 15892 */
            /* sub     edi, eax         ; ChunkIndex = *OutIndex % 15892 */
            if (Inst->opcode == I_IMUL &&
                Inst->ops[2].type == O_IMM) {
            
                // Make sure this is the correct sequence of instructions!
                if (NextInst->opcode == I_SUB) {

                    // We found the TNameEntryArray::ElementsPerChunk value!
                    NamesEntryArrayElementsPerChunk = utils::read_bytes(&Inst->imm,
                                                                        Inst->ops[2].size);
                    LOG_INFO(_XOR_("Found the TNameEntryArray::ElementsPerChunk value: %d"),
                             NamesEntryArrayElementsPerChunk);

                    // Skip instructions until we find the TNameEntryArray::ChunksEncrypted
                    // register, offset, and inline decryption.
                    // So look for the following sequence of instructions:
                    /* mov     rcx, [r13+0] */
                    /* cmp     qword ptr cs:aXenuinesdkCarv_93, 0 ; "XENUINESDK_CARVE" */
                    /* jnz     short loc_7FF6FDBDFDDB ; <--- Jump to decryption here */
                    INCREMENT_NEXT_INSTRUCTION();
                    while(!(NextInst->opcode == I_MOV &&
                            NextInst->ops[1].type == O_SMEM)) {
                        INCREMENT_NEXT_INSTRUCTION();
                    }
                    // We found the TNameEntryArray::ChunksEncrypted offset and decryption!
                    NamesEntryArrayChunksEncryptedOffset = static_cast<size_t>(NextInst->disp);
                    NamesEntryArrayChunksEncryptedRegister = NextInst->ops[0].index;

                    // Skip instructions until we find the jump to the inline decryption.
                    INCREMENT_NEXT_INSTRUCTION();
                    while (META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                        INCREMENT_NEXT_INSTRUCTION();
                    }
                    // Skip instructions in between jump and decryption.
                    TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
                    while (NextInst->addr != TargetAddress) {
                        INCREMENT_NEXT_INSTRUCTION();
                    }
                    // We found the beginning of the TNameEntryArray::ChunksEncrypted
                    // inline decryption!
                    NamesEntryArrayChunksDecryptionBegin = NextIp;
                    LOG_INFO(_XOR_("Found the TNameEntryArray::ChunksEncrypted decryption beginning: 0x%p"),
                             NamesEntryArrayChunksDecryptionBegin);

                    // This should now mark the beginning of the TNameEntryArray::ChunksEncrypted
                    // decryption. Keep iterating next instruction until we hit the end of
                    // the decryption.
                    INCREMENT_NEXT_INSTRUCTION();
                    // Search for the decryption terminating instruction:
                    /* mov     rcx, [rbp+4Fh+ChunksDecrypted] */
                    while (!(NextInst->opcode == I_MOV &&
                             NextInst->ops[1].type == O_SMEM)) {
                        INCREMENT_NEXT_INSTRUCTION();
                    }
                    INCREMENT_NEXT_INSTRUCTION();
                    // We found the end of the TNameEntryArray::ChunksEncrypted inline decryption!
                    NamesEntryArrayChunksDecryptionEnd = NextIp;
                    LOG_INFO(_XOR_("Found the TNameEntryArray::ChunksEncrypted decryption ending: 0x%p"),
                             NamesEntryArrayChunksDecryptionEnd);

                    // We are done!
                    break;

                } else {
                    LOG_ERROR(_XOR_("TNameEntryArray::ElementsPerChunk heuristic failed!"));
                }
            }

            // Handle the next instruction.
            goto Next6;
        }

    Next6:
        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
        //LOG_INFO(_XOR_("InstIndex = %d"), InstIndex);
    }

    //LOG_INFO(_XOR_("LOL DONE"));

    // Output the hueristic search results.
    if (NamesEntryHashNextOffset != -1) {
        LOG_INFO(_XOR_("HashEntry register = %s"), RegisterStrings[NamesHashEntryRegister]);
        LOG_INFO(_XOR_("FNameEntry::HashNext offset = 0x%08x"), NamesEntryHashNextOffset);
    }

    if (NamesEntryArrayElementsPerChunk != -1) {
        LOG_INFO(_XOR_("TNameEntryArray::ElementsPerChunk value = %d"), NamesEntryArrayElementsPerChunk);
    }

    if (NamesGetEncryptedNamesRoutine) {
        LOG_INFO(_XOR_("FName::GetEncryptedNames address = 0x%p"), NamesGetEncryptedNamesRoutine);
        LOG_INFO(_XOR_("NamesEncrypted register = %s"), RegisterStrings[NamesEncryptedRegister]);
    }

    if (NamesDecryptionBegin) {
        LOG_INFO(_XOR_("NamesEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(NamesDecryptionBegin,
                                                       __MIN(static_cast<size_t>(NamesDecryptionEnd - NamesDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    if (NamesEntryIndexEncryptedOffset != -1) {
        LOG_INFO(_XOR_("FNameEntry::IndexEncrypted offset = 0x%08x"), NamesEntryIndexEncryptedOffset);
        LOG_INFO(_XOR_("FNameEntry::IndexEncrypted register = %s"), RegisterStrings[NamesEntryIndexEncryptedRegister]);
        LOG_INFO(_XOR_("FNameEntry::IndexEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(NamesEntryIndexDecryptionBegin,
                                                       __MIN(static_cast<size_t>(NamesEntryIndexDecryptionEnd - NamesEntryIndexDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    if (NamesEntryArrayChunksEncryptedOffset != -1) {
        LOG_INFO(_XOR_("TNameEntryArray::ChunksEncrypted offset = 0x%08x"), NamesEntryArrayChunksEncryptedOffset);
        LOG_INFO(_XOR_("TNameEntryArray::ChunksEncrypted register = %s"), RegisterStrings[NamesEntryArrayChunksEncryptedRegister]);
    }

    if (NamesEntryArrayChunksDecryptionBegin) {
        LOG_INFO(_XOR_("TNameEntryArray::ChunksEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(NamesEntryArrayChunksDecryptionBegin,
                                                       __MIN(static_cast<size_t>(NamesEntryArrayChunksDecryptionEnd - NamesEntryArrayChunksDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    if (NamesEntryArrayAddZeroedRoutine) {
        LOG_INFO(_XOR_("TNameEntryArray::AddZeroed address = 0x%p"), NamesEntryArrayAddZeroedRoutine);
    }


    // Search for the call to FArchive::operator<<(FArchive& Ar, FPropertyTag& Tag):
    // .text:7FF6FB83065F 0F B6 43 28           movzx   eax, byte ptr [rbx+28h]
    // .text:7FF6FB830663 A8 02                 test    al, 2
    // .text:7FF6FB830665 0F 84 95 00 00 00     jz      loc_7FF6FB830700
    // .text:7FF6FB83066B 48 8D 54 24 50        lea     rdx, [rsp+130h+InnerTag]        ; Tag
    // .text:7FF6FB830670 48 8B CB              mov     rcx, rbx                        ; this
    // .text:7FF6FB830673 E8 28 F1 F5 01        call    FArchive__PropertyTagOperator
    // A8 02 0F 84 ? ? ? ? 48 8D 54 24 ? 48 8B CB E8 ? ? ? ?
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("A8 02 0F 84 ?? ?? ?? ?? 48 8D 54 24 ?? 48 8B CB"));
    if (Found)
        Found = static_cast<const uint8_t*>(utils::GetCallTargetAddress(Found + 16));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find FArchive::PropertyTagOperator signature!"));
        return ERROR_NOT_FOUND;
    }
    LOG_INFO(_XOR_("Found FArchive::PropertyTagOperator routine at 0x%p"), Found);

    // Disassemble the FArchive::PropertyTagOperator routine.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble FArchive::PropertyTagOperator!"));
        return rc;
    }

    // Heuristically search for offsets and inline decryption.
    int8_t NamesRegister = 0;
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the FName::GetEncryptedNames routine call:
        /* lea     rcx, [rsp+48h+arg_8] */
        /* call    FName__GetEncryptedNames */
        /* mov     r10, qword ptr cs:aXenuinesdkCarv_94 ; "XENUINESDK_CARVE"
        /* mov     rcx, [rax]
        /* test    r10, r10
        /* jnz     short loc_7FF6FD78F809 */
        if (Inst->opcode == I_LEA &&
            NextInst->opcode == I_CALL &&
            NextInst->ops[0].type == O_PC) {
        
            // We probably found the FName::GetEncryptedNames routine.
            TargetAddress = INSTRUCTION_GET_TARGET(NextInst);

            // Validate the target address of this call.
            if (TargetAddress != (uint64_t)NamesGetEncryptedNamesRoutine) {
                LOG_ERROR(_XOR_("Call target does not match NamesGetEncryptedNamesRoutine: %p"),
                          NamesGetEncryptedNamesRoutine);
            }

            // Keep iterating next instruction until we hit the jcc.
            INCREMENT_NEXT_INSTRUCTION();
            while (META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                INCREMENT_NEXT_INSTRUCTION();
            }

            // The target address of the jcc points to the Names decryption.
            TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
            // Skip the instructions in between jump and decryption.
            INCREMENT_NEXT_INSTRUCTION();
            while (NextInst->addr != TargetAddress) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            // This should now mark the beginning of the Names decryption.
            // Keep iterating next instruction until we hit the end of the
            // decryption, which should mark near the beginning of the
            // NumElements decryption.
            INCREMENT_NEXT_INSTRUCTION();
            // Search for the decryption terminating instruction:
            /* mov     r15, [rsp+48h+NamesDecrypted] */
            while (!(NextInst->opcode == I_MOV &&
                     NextInst->ops[1].type == O_SMEM)) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            // Get the decrypted Names register.
            NamesRegister = NextInst->ops[0].index;
            // Search for the dereferencing of the Name register, which marks
            // the beginning of the NumElements decryption prologue.
            INCREMENT_NEXT_INSTRUCTION();
            /* mov     rdx, [r15+8] */
            while (!(NextInst->opcode == I_MOV &&
                     NextInst->ops[1].type == O_SMEM &&
                     NextInst->ops[1].index == NamesRegister)) {
                INCREMENT_NEXT_INSTRUCTION();
            }

            // Get the NumElementsEncrypted offset and register.
            NamesEntryArrayNumElementsEncryptedOffset = static_cast<size_t>(NextInst->disp);
            NamesEntryArrayNumElementsEncryptedRegister = NextInst->ops[0].index;

            // Determine the beginning of the NumElementsEncrypted inline decryption.
            INCREMENT_NEXT_INSTRUCTION();
            while (META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                INCREMENT_NEXT_INSTRUCTION();
            }

            // We found the beginning of the NumElementsEncrypted inline decryption.
            TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
            // Skip the instructions in between jump and decryption.
            INCREMENT_NEXT_INSTRUCTION();
            while (NextInst->addr != TargetAddress) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            NamesEntryArrayNumElementsDecryptionBegin = reinterpret_cast<uint8_t*>(TargetAddress);
            LOG_INFO(_XOR_("Found the TNameEntryArray::NumElementsEncrypted decryption beginning: 0x%p"),
                     NamesEntryArrayNumElementsDecryptionBegin);

            // This should now mark the beginning of the NumElements decryption.
            // Keep iterating next instruction until we hit the end of the decryption.
            INCREMENT_NEXT_INSTRUCTION();
            // Search for the decryption terminating instruction:
            /* mov     rax, [rsp+48h+Decrypted] */
            while (!(NextInst->opcode == I_MOV &&
                     NextInst->ops[1].type == O_SMEM)) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            INCREMENT_NEXT_INSTRUCTION();
            NamesEntryArrayNumElementsDecryptionEnd = NextIp;
            LOG_INFO(_XOR_("Found the TNameEntryArray::NumElementsEncrypted decryption ending: 0x%p"),
                     NamesEntryArrayNumElementsDecryptionEnd);

            // We are done!
            break;
        }

        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    if (NamesEntryArrayNumElementsEncryptedOffset != -1) {
        LOG_INFO(_XOR_("TNameEntryArray::NumElementsEncrypted offset = 0x%08x"), NamesEntryArrayNumElementsEncryptedOffset);
        LOG_INFO(_XOR_("TNameEntryArray::NumElementsEncrypted register = %s"), RegisterStrings[NamesEntryArrayNumElementsEncryptedRegister]);
        LOG_INFO(_XOR_("TNameEntryArray::NumElementsEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(NamesEntryArrayNumElementsDecryptionBegin,
                                                       __MIN(static_cast<size_t>(NamesEntryArrayNumElementsDecryptionEnd - NamesEntryArrayNumElementsDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }

    return rc;
}



/**
 * UWorld offsets and inline decryption globals.
 */
size_t WorldEncryptedOffset = -1;
int8_t WorldEncryptedRegister = -1;
size_t WorldEncryptedStackOffset = -1;
int8_t WorldEncryptedStackRegister = -1;
uint8_t* WorldDecryptionBegin = nullptr;
uint8_t* WorldDecryptionEnd = nullptr;

size_t WorldPersistentLevelEncryptedOffset = -1;
int8_t WorldPersistentLevelEncryptedRegister = -1;
uint8_t* WorldPersistentLevelDecryptionBegin = nullptr;
uint8_t* WorldPersistentLevelDecryptionEnd = nullptr;

size_t LevelActorsEncryptedOffset = -1;
int8_t LevelActorsEncryptedRegister = -1;
uint8_t* LevelActorsDecryptionBegin = nullptr;
uint8_t* LevelActorsDecryptionEnd = nullptr;


int DumpWorld()
{
    int rc = NOERROR;
    const uint8_t* Found;
    uint64_t TargetAddress;
    DECLARE_DISASM_VARS();

    void* ImageBase = utils::GetModuleHandleWIDE(nullptr /*0xC4D8736D TslGame.exe */);
    uint32_t ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    //// Search for the GWorld address inside the HandleCSVProfileCommand routine:
    //// .text:7FF6FC3954C7 48 8B D9                  mov     rbx, rcx
    //// .text:7FF6FC3954CA 48 8B 05 57 34 9C 04      mov     rax, cs:GWorld
    //// .text:7FF6FC3954D1 48 89 84 24 80 00 00 00   mov     [rsp+68h+arg_10], rax
    //// .text:7FF6FC3954D9 33 F6                     xor     esi, esi
    //// .text:7FF6FC3954DB 48 39 35 1E 7C 04 03      cmp     qword ptr cs:aXenuinesdkCarv_97, rsi ; "XENUINESDK_CARVE"
    //// .text:7FF6FC3954E2 75 10                     jnz     short loc_7FF6FC3954F4
    //// 48 8B D9 48 8B 05 ? ? ? ? 48 89 84 24 ? ? ? ? 33 F6
    //Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8B D9 48 8B 05 ?? ?? ?? ?? 48 89 84 24"));
    //if (!Found) {
    //    LOG_ERROR(_XOR_("Failed to find GWorld decryption signature!"));
    //    return ERROR_NOT_FOUND;
    //}
    //LOG_INFO(_XOR_("Found GWorld signature at 0x%p"), Found);
    //
    //// Disassemble the code at the found GWorld signature.
    //rc = DisDecompose(Found,
    //                  MAX_DISASM_LENGTH,
    //                  (uint64_t)Found,
    //                  Insts,
    //                  MaxInstCount,
    //                  &InstCount,
    //                  NULL,
    //                  DISASM_X64
    //                  );
    //if (rc != DIS_SUCCESS) {
    //    LOG_ERROR(_XOR_("Failed to disassemble code at GWorld signature!"));
    //    return rc;
    //}
    //
    //// Heuristically search for the GWorld inline decryption.
    //Ip = const_cast<uint8_t *>(Found);
    //TargetAddress = 0;
    //Offset = 0;
    //InstIndex = 0;
    //Inst = Insts;
    //InstEnd = Insts + InstCount;
    //while (Inst < InstEnd) {
    //    NextInst = Inst + 1;
    //    NextIp = Ip + Inst->size;
    //    NextOffset = Offset + Inst->size;
    //
    //    // Find the the GWorld register mov instruction which leads to the inline decryption.
    //    // Search for the GWorld mov instruction:
    //    /* mov     rax, cs:GWorld */
    //    /* mov     [rsp+80h], rax  */
    //    /* xor     esi, esi  */
    //    if (InstIndex == 1 &&
    //        Inst->opcode == I_MOV &&
    //        Inst->ops[1].type == O_SMEM &&
    //        NextInst->opcode == I_MOV &&
    //        NextInst->ops[0].type == O_SMEM &&
    //        Inst->ops[0].index == NextInst->ops[1].index) {
    //
    //        // We found the GWorld offset and register!
    //        WorldEncryptedOffset = static_cast<size_t>(Inst->disp);
    //        WorldEncryptedRegister = Inst->ops[0].index;
    //        WorldEncryptedStackOffset = static_cast<size_t>(NextInst->disp);
    //        WorldEncryptedStackRegister = NextInst->ops[0].index;
    //
    //        // Search for the beginning of the GWorldEncrypted inline decryption:
    //        INCREMENT_NEXT_INSTRUCTION();
    //        while(META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
    //            INCREMENT_NEXT_INSTRUCTION();
    //        }
    //        TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
    //        // Skip the instructions in between jump and decryption.
    //        INCREMENT_NEXT_INSTRUCTION();
    //        while (NextInst->addr != TargetAddress) {
    //            INCREMENT_NEXT_INSTRUCTION();
    //        }
    //
    //        // We found the beginning of the GWorldEncrypted inline decryption!
    //        WorldDecryptionBegin = reinterpret_cast<uint8_t*>(TargetAddress);
    //        LOG_INFO(_XOR_("Found the GWorldEncrypted decryption beginning: 0x%p"),
    //                 WorldDecryptionBegin);
    //
    //        // Keep iterating next instruction until we hit the end of the decryption.
    //        INCREMENT_NEXT_INSTRUCTION();
    //        // Search for the decryption terminating instruction:
    //        /* mov     rax, [rsp+68h+Decrypted] */
    //        while (!(NextInst->opcode == I_MOV &&
    //                 NextInst->ops[1].type == O_SMEM &&
    //                 NextInst->ops[1].size == sizeof(uint64_t))) {
    //            INCREMENT_NEXT_INSTRUCTION();
    //        }
    //        //LOG_INFO(_XOR_("NextInst->ops[1].size: %d  NextInst->dispSize: %d"),
    //        //         NextInst->ops[1].size, NextInst->dispSize);
    //        INCREMENT_NEXT_INSTRUCTION();
    //        // We found the end of the GWorldEncrypted inline decryption!
    //        WorldDecryptionEnd = NextIp;
    //        LOG_INFO(_XOR_("Found the GWorldEncrypted decryption ending: 0x%p"),
    //                 WorldDecryptionEnd);
    //
    //        // We are done!
    //        break;
    //    }
    //
    //    // Increment to the next instruction.
    //    Inst = NextInst;
    //    Ip = NextIp;
    //    Offset = NextOffset;
    //    InstIndex = static_cast<uint32_t>(NextInst - Insts);
    //}
    //
    //// Output the heauristic search results.
    //if (WorldEncryptedOffset != -1) {
    //    LOG_INFO(_XOR_("GWorld offset = 0x%08x"), WorldEncryptedOffset);
    //    LOG_INFO(_XOR_("GWorld register = %s"), RegisterStrings[WorldEncryptedRegister]);
    //    LOG_INFO(_XOR_("GWorld stack offset = 0x%08x"), WorldEncryptedStackOffset);
    //    LOG_INFO(_XOR_("GWorld stack register = %s"), RegisterStrings[WorldEncryptedStackRegister]);
    //    LOG_INFO(_XOR_("GWorld decryption = {"));
    //    std::string BufferString = utils::FormatBuffer(WorldDecryptionBegin,
    //                                                   __MIN(static_cast<size_t>(WorldDecryptionEnd - WorldDecryptionBegin), 1024));
    //    for (auto&& Line : utils::SplitString(BufferString, '\n')) {
    //        LOG_INFO(Line.c_str());
    //    }
    //    LOG_INFO(_XOR_("}; "));
    //}

    // Search for the UWorld::Serialize routine:
    // .text:7FF6BF348010                       UWorld__Serialize proc near
    // .text:7FF6BF348010                       .....
    // .text:7FF6BF3480E1                       loc_7FF6BF3480E1:           ; CODE XREF: UWorld__Serialize+9B^j
    // .text:7FF6BF3480E1 48 89 86 F0 09 00 00  mov     [rsi+9F0h], rax
    // .text:7FF6BF3480E8 4C 39 35 11 76 A0 02  cmp     qword ptr cs:aXenuinesdkCarv_1, r14 ; "XENUINESDK_CARVE"
    // .text:7FF6BF3480EF 75 0E                 jnz     short loc_7FF6BF3480FF
    // .text:7FF6BF3480F1 48 8B D0              mov     rdx, rax            ; _QWORD
    // .text:7FF6BF3480F4 B9 84 C2 3E 48        mov     ecx, 483EC284h      ; _QWORD
    // .text:7FF6BF3480F9 FF 15 29 76 A0 02     call    cs:XenuineDecrypt_1
    // .text:7FF6BF3480FF                       loc_7FF6BF3480FF:           ; CODE XREF: UWorld__Serialize+DF^j
    // .text:7FF6BF3480FF 8B 47 40              mov     eax, [rdi+40h]      ; <---- Byte pattern signature here.
    // .text:7FF6BF348102 3D C0 01 00 00        cmp     eax, 1C0h           ; <---- Byte pattern signature here.
    // .text:7FF6BF348107 7D 5A                 jge     short loc_7FF6BF348163
    // .text:7FF6BF348109 BB 04 00 00 00        mov     ebx, 4
    // .text:7FF6BF34810E 66 90                 xchg    ax, ax
    // 8B 47 ? 3D C0 01 00 00
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("8B 47 ?? 3D C0 01 00 00"));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find UWorld::Serialize signature!"));
        return ERROR_NOT_FOUND;
    }
    LOG_INFO(_XOR_("Found UWorld::Serialize signature at 0x%p"), Found);
    rc = utils::FindFunctionStartFromPtr(Found, 1024, &Found);
    if (rc < 0) {
        LOG_ERROR(_XOR_("Failed to backtrack to beginning of UWorld::Serialize!"));
        return rc;
    }
    LOG_INFO(_XOR_("Found UWorld::Serialize function address at 0x%p"), Found);

    //LOG_INFO(_XOR_("UWorld::Serialize code = {"));
    //std::string UWorldSerializeString = utils::FormatBuffer(Found, 256);
    //for (auto&& Line : utils::SplitString(UWorldSerializeString, '\n')) {
    //    LOG_INFO("%s", Line.c_str());
    //}
    //LOG_INFO(_XOR_("};"));

    // Disassemble the code at the found UWorld::Serialize address.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble code at GWorld signature!"));
        return rc;
    }

    // Heuristically search for the UWorld::PersistentLevel inline decryption.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the instruction that gets the UWorld::PersistentLevelEncrypted value.
        /* mov     rcx, [rsi+9F0h]  ; RCX = this->PersistentLevelEncrypted */
        if (Inst->opcode == I_MOV &&
            Inst->ops[1].type == O_SMEM) {

            // Found the UWorld::PersistentLevelEncrypted field offset and register!
            WorldPersistentLevelEncryptedOffset = static_cast<size_t>(Inst->disp);
            WorldPersistentLevelEncryptedRegister = Inst->ops[0].index;

            // The next conditional jump should jump to the beginning of the
            // UWorld::PersistentLevelEncrypted field inline decryption.
            while(META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
            // Skip the instructions in between jump and decryption.
            INCREMENT_NEXT_INSTRUCTION();
            while (NextInst->addr != TargetAddress) {
                INCREMENT_NEXT_INSTRUCTION();
            }

            // We found the beginning of the UWorld::PersistentLevelEncrypted inline decryption!
            WorldPersistentLevelDecryptionBegin = reinterpret_cast<uint8_t*>(TargetAddress);
            LOG_INFO(_XOR_("Found the UWorld::PersistentLevelEncrypted decryption beginning: 0x%p"),
                     WorldPersistentLevelDecryptionBegin);

            // Keep iterating next instruction until we hit the end of the decryption.
            INCREMENT_NEXT_INSTRUCTION();
            // Search for the decryption terminating instruction:
            /* mov     rax, [rsp+58h+Decrypted] */
            while (!(NextInst->opcode == I_MOV &&
                     NextInst->ops[1].type == O_SMEM &&
                     NextInst->ops[1].size == sizeof(uint64_t))) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            INCREMENT_NEXT_INSTRUCTION();
            // We found the end of the UWorld::PersistentLevelEncrypted inline decryption!
            WorldPersistentLevelDecryptionEnd = NextIp;
            LOG_INFO(_XOR_("Found the UWorld::PersistentLevelEncrypted decryption ending: 0x%p"),
                     WorldPersistentLevelDecryptionEnd);

            // We are done!
            break;
        }

        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    // Output the heauristic search results.
    if (WorldPersistentLevelEncryptedOffset != -1) {
        LOG_INFO(_XOR_("UWorld::PersistentLevelEncrypted offset = 0x%08x"), WorldPersistentLevelEncryptedOffset);
        LOG_INFO(_XOR_("UWorld::PersistentLevelEncrypted register = %s"), RegisterStrings[WorldPersistentLevelEncryptedRegister]);
        LOG_INFO(_XOR_("UWorld::PersistentLevelEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(WorldPersistentLevelDecryptionBegin,
                                                       __MIN(static_cast<size_t>(WorldPersistentLevelDecryptionEnd - WorldPersistentLevelDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }


    // Search for the ULevel::IncrementalUpdateComponents call inside of UWorld::AddToWorld:
    // .text:7FF6BF335670                           loc_7FF6BF335670:               ; CODE XREF: UWorld__AddToWorld+322^j
    // .text:7FF6BF335670 41 8B D5                  mov     edx, r13d               ; NumComponentsToUpdate
    // .text:7FF6BF335673                           loc_7FF6BF335673:               ; CODE XREF: UWorld__AddToWorld+32E^j
    // .text:7FF6BF335673 45 33 C0                  xor     r8d, r8d                ; bRerunConstructionScripts
    // .text:7FF6BF335676 48 8B CE                  mov     rcx, rsi                ; this
    // .text:7FF6BF335679 E8 02 03 06 FF            call    ULevel__IncrementalUpdateComponents <---- Call instruction right here
    // .text:7FF6BF33567E 41 0F B6 07               movzx   eax, byte ptr [r15]
    // .text:7FF6BF335682 A8 01                     test    al, 1
    // .text:7FF6BF335684 75 2A                     jnz     short loc_7FF6BF3356B0
    // .text:7FF6BF335686 40 84 FF                  test    dil, dil
    // .text:7FF6BF335689 74 CD                     jz      short loc_7FF6BF335658
    // .text:7FF6BF33568B F3 0F 10 1D 15 96 BB 03   movss   xmm3, cs:GLevelStreamingActorsUpdateTimeLimit
    // .text:7FF6BF335693 0F 5A DB                  cvtps2pd xmm3, xmm3             ; TimeLimit
    // .text:7FF6BF335696 4C 8B C6                  mov     r8, rsi                 ; Level
    // .text:7FF6BF335699 0F 28 CE                  movaps  xmm1, xmm6              ; StartTime
    // .text:7FF6BF33569C 48 8D 0D 0D 99 A1 02      lea     rcx, aUpdatingCompon    ; "updating components"
    // .text:7FF6BF3356A3 E8 E8 B3 00 00            call    IsTimeLimitExceeded
    // E8 ? ? ? ? 41 0F B6 07 A8 01
    Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("E8 ?? ?? ?? ?? 41 0F B6 07 A8 01"));
    if (Found)
        Found = static_cast<const uint8_t*>(utils::GetCallTargetAddress(Found));
    if (!Found) {
        LOG_ERROR(_XOR_("Failed to find ULevel::IncrementalUpdateComponents address!"));
        return ERROR_NOT_FOUND;
    }

    // Disassemble the code at the found ULevel::IncrementalUpdateComponents address.
    rc = DisDecompose(Found,
                      MAX_DISASM_LENGTH,
                      (uint64_t)Found,
                      Insts,
                      MaxInstCount,
                      &InstCount,
                      NULL,
                      DISASM_X64
                      );
    if (rc != DIS_SUCCESS) {
        LOG_ERROR(_XOR_("Failed to disassemble code at ULevel::IncrementalUpdateComponents!"));
        return rc;
    }

    // Heuristically search for the ULevel::Actors offset and inline decryption.
    Ip = const_cast<uint8_t *>(Found);
    TargetAddress = 0;
    Offset = 0;
    InstIndex = 0;
    Inst = Insts;
    InstEnd = Insts + InstCount;
    while (Inst < InstEnd) {
        NextInst = Inst + 1;
        NextIp = Ip + Inst->size;
        NextOffset = Offset + Inst->size;

        // Search for the instruction that gets the ULevel::ActorsEncrypted value.
        /* mov     rcx, [rdi+1C8h]  ; RCX = thisptr->ActorsEncrypted */
        if (Inst->opcode == I_MOV &&
            Inst->ops[1].type == O_SMEM &&
            Inst->ops[1].size == sizeof(uint64_t)) {

            // Found the ULevel::ActorsEncrypted field offset and register!
            LevelActorsEncryptedOffset = static_cast<size_t>(Inst->disp);
            LevelActorsEncryptedRegister = Inst->ops[0].index;

            // The next conditional jump should jump to the beginning of the
            // ULevel::ActorsEncrypted field inline decryption.
            while (META_GET_FC(NextInst->meta) != FC_CND_BRANCH) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            TargetAddress = INSTRUCTION_GET_TARGET(NextInst);
            // Skip the instructions in between jump and decryption.
            INCREMENT_NEXT_INSTRUCTION();
            while (NextInst->addr != TargetAddress) {
                INCREMENT_NEXT_INSTRUCTION();
            }

            // We found the beginning of the ULevel::ActorsEncrypted inline decryption!
            LevelActorsDecryptionBegin = reinterpret_cast<uint8_t*>(TargetAddress);
            LOG_INFO(_XOR_("Found the ULevel::ActorsEncrypted decryption beginning: 0x%p"),
                     LevelActorsDecryptionBegin);

            // Keep iterating next instruction until we hit the end of the decryption.
            INCREMENT_NEXT_INSTRUCTION();
            // Search for the decryption terminating instruction:
            /* mov     rax, [rsp+48h+Decrypted] */
            while (!(NextInst->opcode == I_MOV &&
                     NextInst->ops[1].type == O_SMEM &&
                     NextInst->ops[1].size == sizeof(uint64_t))) {
                INCREMENT_NEXT_INSTRUCTION();
            }
            INCREMENT_NEXT_INSTRUCTION();
            // We found the end of the ULevel::ActorsEncrypted inline decryption!
            LevelActorsDecryptionEnd = NextIp;
            LOG_INFO(_XOR_("Found the ULevel::ActorsEncrypted decryption ending: 0x%p"),
                     LevelActorsDecryptionEnd);

            // We are done!
            break;
        }

        // Increment to the next instruction.
        Inst = NextInst;
        Ip = NextIp;
        Offset = NextOffset;
        InstIndex = static_cast<uint32_t>(NextInst - Insts);
    }

    // Output the heauristic search results.
    if (LevelActorsEncryptedOffset != -1) {
        LOG_INFO(_XOR_("ULevel::ActorsEncrypted offset = 0x%08x"), LevelActorsEncryptedOffset);
        LOG_INFO(_XOR_("ULevel::ActorsEncrypted register = %s"), RegisterStrings[LevelActorsEncryptedRegister]);
        LOG_INFO(_XOR_("ULevel::ActorsEncrypted decryption = {"));
        std::string BufferString = utils::FormatBuffer(LevelActorsDecryptionBegin,
                                                       __MIN(static_cast<size_t>(LevelActorsDecryptionEnd - LevelActorsDecryptionBegin), 1024));
        for (auto&& Line : utils::SplitString(BufferString, '\n')) {
            LOG_INFO(Line.c_str());
        }
        LOG_INFO(_XOR_("}; "));
    }


    return rc;
}

}