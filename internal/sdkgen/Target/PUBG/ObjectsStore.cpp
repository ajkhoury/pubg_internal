#include <windows.h>

#include "../../Engine/ObjectsStore.h"

#include "EngineClasses.h"

#include "../../../utils.h"

class FUObjectItem {
public:
    UObject *Object; // 0x00
    int32_t Flags; // 0x08
    int32_t ClusterIndex; // 0x0C
    int32_t SerialNumber; // 0x10
}; // size=0x18

class TUObjectArray {
public:
    union CryptValue ObjectsEncrypted; // 0x00 FUObjectItem*
    int64_t MaxElements; // 0x08
    int32_t NumElements; // 0x10
};

class FUObjectArray {
public:
    int32_t ObjFirstGCIndex; // 0x00
    int32_t ObjLastNonGCIndex; // 0x04
    int32_t MaxObjectsNotConsideredByGC; // 0x08
    bool OpenForDisregardForGC; // 0x0C

    TUObjectArray ObjObjects; // 0x10

    FCriticalSection ObjObjectsCritical; // 0x28
    FThreadSafeCounter ObjAvailableCount; // 0x50
    TArray<void *> UObjectCreateListeners; // 0x58
    TArray<void *> UObjectDeleteListeners; // 0x68

    // .... 
};

FUObjectArray *GUObjectArray = nullptr;

bool ObjectsStore::Initialize()
{
    PVOID ImageBase = utils::GetModuleHandleWIDE(NULL);
    ULONG ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // 48 8D 0D ? ? ? ? E8 ? ? ? ? 90 48 8B 5C 24 ? 48 83 C4 30
    static const UINT8 ObjectsSig[] = {
        0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC,   /* lea     rcx, GUObjectArray */
        0xE8, 0xCC, 0xCC, 0xCC, 0xCC,               /* call    FUObjectArray__FreeUObjectIndex */
        0x90,                                       /* nop */
                                                    /* loc_7FF785413906: */
        0x48, 0x8B, 0x5C, 0x24, 0xCC,               /* mov     rbx, [rsp+48h] */
        0x48, 0x83, 0xC4, 0x30                      /* add     rsp, 30h */
    };

    PVOID Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, ObjectsSig, sizeof(ObjectsSig));
    if (!Found) {
        return false;
    }

    GUObjectArray = reinterpret_cast<FUObjectArray *>(utils::GetInstructionTarget(Found, 3));

    return true;
}

void* ObjectsStore::GetAddress()
{
    return GUObjectArray;
}

size_t ObjectsStore::GetObjectsNum() const
{
    return GUObjectArray->ObjObjects.NumElements;
}

UEObject ObjectsStore::GetById(size_t id) const
{
    union CryptValue ObjectsDecrypted;
    union CryptValue ObjectsEncrypted = GUObjectArray->ObjObjects.ObjectsEncrypted;

    ObjectsDecrypted.LoDword = (ObjectsEncrypted.LoDword + 0x7685D8E5) ^ 0x85E5D16B;
    ObjectsDecrypted.HiDword = (ObjectsEncrypted.HiDword - 0x40766DE5) ^ 0xFB75FBB5;

    FUObjectItem *Objects = reinterpret_cast<FUObjectItem *>(ObjectsDecrypted.Qword);

    return Objects[id].Object;
}
