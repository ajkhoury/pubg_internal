#include "Names.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

/**
 * Assembly decryption routines.
 */
#ifdef __cplusplus
extern "C" {
#endif
uint64_t DecryptNamesAsm(uint64_t NamesEncrypted);
uint64_t DecryptChunksAsm(uint64_t ChunksEncrypted);
uint64_t DecryptNumElementsAsm(uint64_t ChunksEncrypted);
uint64_t DecryptNameEntryIndexAsm(uint64_t IndexEncrypted);
#ifdef __cplusplus
}
#endif


/** Maximum size of name. */
enum { NAME_SIZE = 1024 };

/** Name index. */
typedef int32_t NAME_INDEX;

/**
 * Mask for index bit used to determine whether string is encoded as TCHAR or ANSICHAR. We don't
 * add an extra bool in order to keep the name size to a minimum and 2 billion names is impractical
 * so there are a few bits left in the index.
 */
#define NAME_WIDE_MASK 0x1
#define NAME_INDEX_SHIFT 1

class FNameEntry {
public:
    int32_t GetIndex() const
    {
        return static_cast<int32_t>(DecryptNameEntryIndexAsm(IndexEncrypted));
    }

    int32_t GetHashIndex() const
    {
        return GetIndex() >> NAME_INDEX_SHIFT;
    }

    bool IsWide() const
    {
        return (GetIndex() & NAME_WIDE_MASK);
    }

    int32_t GetNameLength() const
    {
        if (IsWide()) {
            return static_cast<int32_t>(wcslen(WideName));
        } else {
            return static_cast<int32_t>(strlen(AnsiName));
        }
    }

    inline char const* GetAnsiName() const { return AnsiName; }
    inline wchar_t const* GetWideName() const { return WideName; }

private:
    uint64_t IndexEncrypted;    /** Index of name in hash. */
    FNameEntry *HashNext;
    union {
        char AnsiName[NAME_SIZE];
        wchar_t WideName[NAME_SIZE];
    };
};

template<typename ElementType, int32_t MaxTotalElements, int32_t ElementsPerChunk>
class TStaticIndirectArrayThreadSafeRead {

    enum { ChunkTableSize = (MaxTotalElements + ElementsPerChunk - 1) / ElementsPerChunk };

public:
    int32_t Num() const
    {
        return static_cast<int32_t>(DecryptNumElementsAsm(NumElementsEncrypted));
    }

    //int32_t NumChunks() const
    //{
    //    union CryptValue NumChunksDecrypted;
    //    NumChunksDecrypted.Qword = __PAIR64__(__ROR4__(__ROR4__(NumChunksEncrypted >> 32, 8) + 0x7B437B44, 8) ^ 0x84BC84BC,
    //                                          __ROL4__(__ROL4__(NumChunksEncrypted, 16) + 0x7CFC0CC4, 16) ^ 0x7CFC0CC4u);
    //    return static_cast<int32_t>(NumChunksDecrypted.Qword);
    //}

    bool IsValidIndex(int32_t index) const
    {
        return index >= 0 && index < Num() && GetById(index) != NULL;
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

        ChunksDecrypted.Qword = DecryptChunksAsm(ChunksEncrypted);

        ElementType ***Chunks = static_cast<ElementType ***>(ChunksDecrypted.Pointer);
        ElementType **Chunk = Chunks[ChunkIndex];
        return Chunk + WithinChunkIndex;
    }

    uint64_t ChunksEncrypted;
    uint64_t NumElementsEncrypted;
    uint64_t NumChunksEncrypted;
};

typedef TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16836> TNameEntryArray;
static TNameEntryArray *GNames = NULL;

typedef
uint64_t*
(__fastcall *GetEncryptedNamesFn)(
    OUT uint64_t *Result
    );
//static GetEncryptedNamesFn GetEncryptedNames;

static bool NamesInitializeGlobal()
{
    LARGE_INTEGER WaitInterval;
    PVOID ImageBase;
    ULONG ImageSize;

    if (GNames) {
        return true;
    }

    // E8 ? ? ? ? 4C 8B 15 ? ? ? ? 45 33 FF
    static const UINT8 NamesSig[] = {
        0xE8, 0xCC, 0xCC, 0xCC, 0xCC,             /* call    FName__GetNamesEncrypted */
        0x4C, 0x8B, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, /* mov     r10, qword ptr cs:aXenuinesdkCarv_101 */
        0x45, 0x33, 0xFF                          /* xor     r15d, r15d */
    };
    ImageBase = utils::GetModuleHandleWIDE(NULL);
    ImageSize = utils::GetModuleSize((HMODULE)ImageBase);
    
    const uint8_t *Found;
    do Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, NamesSig, sizeof(NamesSig));
    while (!Found);

    GetEncryptedNamesFn GetEncryptedNames;
    uint64_t NamesEncrypted;
    union CryptValue NamesDecrypted;

    GetEncryptedNames = reinterpret_cast<GetEncryptedNamesFn>(utils::GetCallTargetAddress(Found));
    LOG_INFO(_XOR_("GetEncryptedNames = 0x%016llx"), GetEncryptedNames);

    // Little wait that is needed since we are loaded very early on.
    WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(1000));
    NtDelayExecution(FALSE, &WaitInterval);

    GetEncryptedNames(&NamesEncrypted);
    LOG_INFO(_XOR_("NamesEncrypted = 0x%016llx"), NamesEncrypted);

    NamesDecrypted.Qword = DecryptNamesAsm(NamesEncrypted);
    LOG_INFO(_XOR_("NamesDecrypted = 0x%016llx"), NamesDecrypted.Qword);

    GNames = static_cast<TNameEntryArray *>(NamesDecrypted.Pointer);
    LOG_INFO(_XOR_("GNames = 0x%016llx"), GNames);

    return true;
}

NamesProxy::NamesProxy()
{
    if (!GNames) {
        NamesInitializeGlobal();
    }

    Names = static_cast<void *>(GNames);
}

int32_t NamesProxy::GetNum() const
{
    if (!Names)
        return 0;
    return static_cast<TNameEntryArray *>(Names)->Num();
}

bool NamesProxy::IsValid(int32_t id) const
{ 
    if (!Names)
        return false;
    return static_cast<TNameEntryArray *>(Names)->IsValidIndex(id);
}

std::string NamesProxy::GetById(int32_t id) const
{
    std::string Str;

    if (Names) {
        
        const FNameEntry *NameEntry = static_cast<TNameEntryArray *>(Names)->GetById(id);
        if (NameEntry) {
            Str = NameEntry->GetAnsiName();
            //if (NameEntry->IsWide()) {
            //
            //    wchar_t const *WideName = NameEntry->GetWideName();
            //    int WideNameLen = static_cast<int>(wcslen(WideName));
            //    int size = WideCharToMultiByte(CP_UTF8, 0, WideName, WideNameLen, NULL, 0, NULL, NULL);
            //    if (size) {
            //        Str.assign(size, 0);
            //        WideCharToMultiByte(CP_UTF8, 0, WideName, WideNameLen, (LPSTR)Str.data(), size, NULL, NULL);
            //    }
            //
            //} else {
            //    Str.assign(NameEntry->GetAnsiName());
            //}
        }
    }

    return Str;
}

std::string FName::ToString() const
{
    std::string Str;
    NamesProxy NamesProxy;
    TNameEntryArray *Names = static_cast<TNameEntryArray *>(NamesProxy.GetAddress());

    if (Names) {
    
        const FNameEntry *NameEntry = Names->GetById(GetIndex());
        if (NameEntry) {
            if (NameEntry->IsWide()) {

                wchar_t const *WideName = NameEntry->GetWideName();
                int WideNameLen = static_cast<int>(wcslen(WideName));
                int size = WideCharToMultiByte(CP_UTF8, 0, WideName, WideNameLen, NULL, 0, NULL, NULL);
                if (size) {
                    Str.assign(size, 0);
                    WideCharToMultiByte(CP_UTF8, 0, WideName, WideNameLen, (LPSTR)Str.data(), size, NULL, NULL);
                }

            } else {
                Str.assign(NameEntry->GetAnsiName());
            }
        }
    }

    return Str;
}
