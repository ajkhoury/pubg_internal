#include <windows.h>

#include "../../Engine/NamesStore.h"

#include "EngineClasses.h"

#include "../../../utils.h"

class FNameEntry {
public:
    const char* GetName() const { return AnsiName; }

    int32_t GetIndex() const
    { 
        return (int32_t)__ROR4__(__ROL4__(IndexEncrypted.Dword - 0x3CED7571, 8) - 0x3E5DA210, 16) ^ 0x6F8FD361;
    }

    void SetIndex(int32_t Index)
    {
        IndexEncrypted.LoDword = __ROR4__(__ROL4__(Index ^ 0x6F8FD361, 16) + 0x3E5DA210, 8) + 0x3CED7571;
        IndexEncrypted.HiDword = __ROL4__(__ROL4__(0x113F11FF, 8) + 0x4D8B3C87, 16) - 0x34B3D578;
    }

private:
    union CryptValue IndexEncrypted;
    FNameEntry* HashNext;
    union {
        char AnsiName[1024];
        wchar_t WideName[1024];
    };
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead {
public:
    int32_t Num() const
    {
        return (NumElementsEncrypted.Dword + 0x2BCB1FE5) ^ 0x2BCB1FE5;
    }

    void SetNum(int32_t NumElements)
    {
        NumElementsEncrypted.LoDword = (NumElements ^ 0x2BCB1FE5) - 0x2BCB1FE5;
        NumElementsEncrypted.HiDword = 0xAA76ABF6;
    }

    int32_t NumChunks() const
    {
        return (((NumElementsEncrypted.Dword & 0xFFFF0000 | (unsigned __int16)NumElementsEncrypted.Qword ^ ((unsigned int)NumElementsEncrypted.Qword >> 16)) - 0x707E778) & 0xFFFF0000 | (unsigned __int16)((NumElementsEncrypted.Qword ^ NumElementsEncrypted.Word) + 0x1888) ^ (((NumElementsEncrypted.Qword & 0xFFFF0000 | (unsigned __int16)NumElementsEncrypted.Qword ^ ((unsigned int)NumElementsEncrypted.Qword >> 16)) - 0x707E778) >> 16)) ^ 0xF8F81888;
    }

    void SetNumChunks(int32_t NumChunks)
    {
        union CryptValue NewNumElementsEncrypted1;
        union CryptValue NewNumElementsEncrypted2;
        union CryptValue NewNumElementsEncrypted3;

        NewNumElementsEncrypted1.Qword = ((unsigned __int16)NumChunks ^ 0x1888 ^ ((NumChunks ^ 0xF8F81888) >> 16) | (NumChunks ^ 0xF8F81888) & 0xFFFF0000) + 0x707E778;
        NewNumElementsEncrypted2.LoDword = NewNumElementsEncrypted1.Qword & 0xFFFF0000 | (unsigned __int16)NewNumElementsEncrypted1.Qword ^ (NewNumElementsEncrypted1.Qword >> 16);
        int v16 = (uint16_t)__ROL2__(0x878, 8);
        uint16_t v17 = __ROR2__(0x878, 8);
        NewNumElementsEncrypted3.Qword = ((v16 << 16) | v16 ^ v17) + 0x8780878;
        NewNumElementsEncrypted3.LoWord = __ROR2__((v16 ^ v17) + 0x878, 8);
        uint16_t v19 = __ROL2__(NewNumElementsEncrypted3.HiWord, 8);
        NewNumElementsEncrypted2.HiDword = (v19 << 16) | v19 ^ (unsigned __int16)NewNumElementsEncrypted3.Qword;

        NumChunksEncrypted.Qword = NewNumElementsEncrypted2.Qword;
    }

    bool IsValidIndex(int32_t index) const
    {
        return index >= 0 && index < Num() && GetById(index) != nullptr;
    }

    ElementType const* const& GetById(int32_t index) const
    {
        return *GetItemPtr(index);
    }

private:
    ElementType const* const* GetItemPtr(int32_t Index) const
    {
        union CryptValue ChunksDecrypted;
        int32_t ChunkIndex = Index / ElementsPerChunk;
        int32_t WithinChunkIndex = Index % ElementsPerChunk;

        ChunksDecrypted.LoDword = ((ChunksEncrypted.LoDword ^ 0x674D73EB) - 0x35759DEA) ^ 0xD0C711FD;
        ChunksDecrypted.HiDword = ((ChunksEncrypted.HiDword ^ 0xB4CCA658) - 917124650) ^ 0x51669072;

        ElementType ***Chunks = reinterpret_cast<ElementType ***>(ChunksDecrypted.Qword);
        ElementType **Chunk = Chunks[ChunkIndex];
        return Chunk + WithinChunkIndex;
    }

    enum {
        ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk
    };

    union CryptValue ChunksEncrypted;
    union CryptValue NumElementsEncrypted;
    union CryptValue NumChunksEncrypted;
};

using TNameEntryArray = TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16752>;
TNameEntryArray *GlobalNames = nullptr;

typedef
union CryptValue*
(__fastcall *GetNamesFn)(
    OUT union CryptValue *Result
    );

bool NamesStore::Initialize()
{
    // E8 ? ? ? ? 4C 8B 15 ? ? ? ? 45 33 FF
    static const UINT8 NamesSig[] = {
        0xE8, 0xCC, 0xCC, 0xCC, 0xCC,             /* call    FName__GetNames */
        0x4C, 0x8B, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, /* mov     r10, qword ptr cs:aXenuinesdkCarv_169 */
        0x45, 0x33, 0xFF                          /* xor     r15d, r15d */
    };

    PVOID ImageBase = utils::GetModuleHandleWIDE(NULL);
    ULONG ImageSize = utils::GetModuleSize((HMODULE)ImageBase);
    PVOID Found;
    do Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, NamesSig, sizeof(NamesSig));
    while(!Found);
 
    union CryptValue NamesEncrypted;
    union CryptValue NamesDecrypted;

    GetNamesFn GetNames = reinterpret_cast<GetNamesFn>(utils::GetCallTargetAddress(Found));
    GetNames(&NamesEncrypted);

    NamesDecrypted.LoDword = (0x38128A8E - (NamesEncrypted.LoDword - 0x1B24D0AC)) ^ 0x7212461C;
    NamesDecrypted.HiWord = (0x1AB3D577 - NamesEncrypted.HiWord - 0x624E374B) ^ 0xC020CC2;

    //NamesEncryptedHigh.Dword = NamesEncrypted.HiDword;
    //NamesEncryptedLow.Dword = (NamesEncrypted.Qword & 0xFFFF0000 |
    //                           __ROR2__(NamesEncrypted.Word ^ NamesEncrypted.Word1, 8)) - 0x5111C90E;
    //
    //NamesDecrypted.LoDword = (__ROR2__((unsigned __int16)(NamesEncryptedLow.Dword ^ NamesEncryptedLow.HiWord), 8) | __ROR2__(NamesEncryptedLow.HiWord, 8) << 16) ^ -0x5111C90E;
    //
    //NamesEncryptedLow.LoWord = __ROR2__(NamesEncrypted.HiWord, 8);
    //
    //NamesDecrypted.HiDword = ((unsigned __int16)(((NamesEncryptedHigh.Word1 ^ NamesEncrypted.Word2) - 0x52CE) ^ ((unsigned int)(((NamesEncryptedHigh.Word1 ^ NamesEncrypted.Word2) | (NamesEncryptedLow.LoWord << 16)) - 0x524E52CE) >> 16)) |
    //    (((NamesEncryptedHigh.Word1 ^ NamesEncrypted.Word2) | (NamesEncryptedLow.Word << 16)) - 0x524E52CE) & 0xFFFF0000) ^ 0x524E52CE;

    GlobalNames = reinterpret_cast<decltype(GlobalNames)>(NamesDecrypted.Qword);

    return true;
}

void* NamesStore::GetAddress()
{
    return GlobalNames;
}

size_t NamesStore::GetNamesNum() const
{
    return GlobalNames->Num();
}

bool NamesStore::IsValid(size_t id) const
{
    return GlobalNames->IsValidIndex(static_cast<int32_t>(id));
}

std::string NamesStore::GetById(size_t id) const
{
    return GlobalNames->GetById(static_cast<int32_t>(id))->GetName();
}
