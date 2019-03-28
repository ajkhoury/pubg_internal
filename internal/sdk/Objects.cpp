#include "Objects.h"
#include "Names.h"

#include "UnrealTypes.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

std::unordered_map<std::string, int32_t> ObjectsProxy::ObjectsCacheMap;

class TUObjectArray {
public:
    uint64_t ObjectsEncrypted; // 0x00 FUObjectItem *
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

static FUObjectArray *GUObjectArray = NULL;

static bool ObjectsInitializeGlobal()
{
    PVOID ImageBase;
    ULONG ImageSize;

    if (GUObjectArray) {
        return true;
    }

    ImageBase = utils::GetModuleHandleWIDE(NULL);
    ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // 48 8D 0D ? ? ? ? E8 ? ? ? ? 90 48 8B 5C 24 ? 48 83 C4 30
    //static const UINT8 ObjectsSig[] = {
    //    0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC,   /* lea     rcx, GUObjectArray */
    //    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,               /* call    FUObjectArray__FreeUObjectIndex */
    //    0x90,                                       /* nop */
    //                                                /* loc_7FF785413906: */
    //    0x48, 0x8B, 0x5C, 0x24, 0xCC,               /* mov     rbx, [rsp+48h] */
    //    0x48, 0x83, 0xC4, 0x30                      /* add     rsp, 30h */
    //};
    //const uint8_t *Found;
    //do Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, ObjectsSig, sizeof(ObjectsSig));
    //while (!Found);

    // Search for the GUObjectArray address inside the UObjectBase::Deconstructor routine:
    // .text:7FF7F194D035 90                    nop
    // .text:7FF7F194D036 48 8B D7              mov     rdx, rdi                        ; Object
    // .text:7FF7F194D039 48 8D 0D 88 41 92 04  lea     rcx, GUObjectArray              ; <---- GObjectsArray
    // .text:7FF7F194D040 E8 5B 3A 00 00        call    FUObjectArray__FreeUObjectIndex
    // .text:7FF7F194D045 90                    nop
    // .text:7FF7F194D046                       loc_7FF7F194D046:                       ; CODE XREF: UObjectBase__Deconstructor+28^j
    // .text:7FF7F194D046                                                               ; UObjectBase__Deconstructor+6C^j ...
    // .text:7FF7F194D046 48 8B 5C 24 48        mov     rbx, [rsp+38h+arg_8]
    // .text:7FF7F194D04B 48 83 C4 30           add     rsp, 30h
    // 48 8D 0D ? ? ? ? E8 ? ? ? ? 90 48 8B 5C 24 ? 48 83 C4 30
    const uint8_t *Found;
    do Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 90 48 8B 5C 24 ?? 48 83 C4 30"));
    while (!Found);

    GUObjectArray = static_cast<FUObjectArray *>(utils::GetInstructionTarget(Found, 3));
    LOG_INFO(_XOR_("GUObjectArray = 0x%016llx"), GUObjectArray);

    return true;
}

ObjectsProxy::ObjectsProxy()
{
    if (!GUObjectArray) {
        ObjectsInitializeGlobal();
    }
    ObjectArray = static_cast<void *>(GUObjectArray);
}

int32_t ObjectsProxy::GetNum() const
{
    return static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.NumElements;
}

int64_t ObjectsProxy::GetMax() const
{
    return static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.MaxElements;
}

FUObjectItem *ObjectsProxy::GetObjectsPrivate() const
{
    union CryptValue ObjectsDecrypted;
    uint64_t ObjectsEncrypted = static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.ObjectsEncrypted;

    ObjectsDecrypted.Qword = DecryptObjectsAsm(ObjectsEncrypted);

    return (FUObjectItem *)ObjectsDecrypted.Pointer;
}

UObject *ObjectsProxy::GetById(int32_t Index) const
{
    union CryptValue ObjectsDecrypted;
    uint64_t ObjectsEncrypted = static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.ObjectsEncrypted;

    ObjectsDecrypted.Qword = DecryptObjectsAsm(ObjectsEncrypted);

    FUObjectItem *Objects = static_cast<FUObjectItem *>(ObjectsDecrypted.Pointer);

    return Objects[Index].Object;
}

UObject* ObjectsProxy::FindObject(const std::string& name) const
{
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject* Object = GetById(i);
        if (Object != nullptr) {
            if (Object->GetFullName() == name) {
                return Object;
            }
        }
    }
    return NULL;
}

int32_t ObjectsProxy::CountObjects(const UClass* CmpClass, const std::string& Name) const
{
    auto it = ObjectsCacheMap.find(Name);
    if (it != std::end(ObjectsCacheMap)) {
        return it->second;
    }

    int32_t Count = 0;
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject* Object = GetById(i);
        if (Object) {
            if (Object->IsA(CmpClass) && Object->GetName() == Name) {
                ++Count;
            }
        }
    }

    ObjectsCacheMap[Name] = Count;
    return Count;
}