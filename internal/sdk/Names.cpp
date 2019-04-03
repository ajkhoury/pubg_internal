#include "Names.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/xorstr.h>


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

    uint64_t ChunksEncrypted;       // 0x00
    uint64_t NumElementsEncrypted;  // 0x08
    uint64_t NumChunksEncrypted;    // 0x10
};

typedef TStaticIndirectArrayThreadSafeRead<FNameEntry, 2 * 1024 * 1024, 16224> TNameEntryArray;
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

    ImageBase = utils::GetModuleHandleWIDE(NULL);
    ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // Locate signature inside of the FName::GetEncryptedNames routine:
    // .text:7FF6FDBE6181 E8 9A 89 3A FD                call    tslgame_AK__MemoryMgr__StartProfileThreadUsage
    // .text:7FF6FDBE6186 8B 15 94 36 3D 03             mov     edx, cs:dword_7FF700FB9820
    // .text:7FF6FDBE618C 65 48 8B 04 25 58 00 00 00    mov     rax, gs:58h
    // .text:7FF6FDBE6195 B9 18 00 00 00                mov     ecx, 18h
    // .text:7FF6FDBE619A 48 8B 1C D0                   mov     rbx, [rax+rdx*8]
    // .text:7FF6FDBE619E 48 03 D9                      add     rbx, rcx
    // .text:7FF6FDBE61A1 45 33 F6                      xor     r14d, r14d
    // E8 ? ? ? ? 8B 15 ? ? ? ? 65 48 8B 04 25 58 00 00 00 B9 18 00 00 00 48 8B 1C D0 48 03 D9
    const uint8_t *Found;
    do Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("E8 ?? ?? ?? ?? 8B 15 ?? ?? ?? ?? 65 48 8B 04 25 58 00 00 00 B9 18 00 00 00 48 8B 1C D0 48 03 D9"));
    while (!Found);
    
    // Get the function address.
    GetEncryptedNamesFn GetEncryptedNames;
    if (utils::FindFunctionStartFromPtr(Found, 128, (const uint8_t**)&GetEncryptedNames) != NOERROR) {
        return false;
    }
    LOG_INFO(_XOR_("GetEncryptedNames = 0x%016llx"), GetEncryptedNames);

    // Little wait that is needed since we are loaded very early on.
    WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(1000));
    NtDelayExecution(FALSE, &WaitInterval);

    // Get the decrypted Names pointer.
    uint64_t NamesEncrypted;
    union CryptValue NamesDecrypted;

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
            //Str = NameEntry->GetAnsiName();
            if (NameEntry->IsWide()) {
            
                wchar_t const *WideName = NameEntry->GetWideName();
                int WideNameLen = static_cast<int>(wcslen(WideName));
                int size = WideCharToMultiByte(CP_UTF8, 0, WideName, WideNameLen, NULL, 0, NULL, NULL);
                if (size) {
                    Str.resize(size, 0);
                    WideCharToMultiByte(CP_UTF8, 0, WideName, WideNameLen, (LPSTR)Str.data(), size, NULL, NULL);
                }
            
            } else {
                Str = NameEntry->GetAnsiName();
            }
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
