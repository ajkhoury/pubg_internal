#include "Objects.h"
#include "Names.h"

#include "UnrealTypes.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

std::unordered_map<std::string, int32_t> ObjectsProxy::ObjectsCacheMap;

class FFixedUObjectArray {
public:
    uint64_t ObjectsEncrypted;  // 0x00 FUObjectItem*
    int64_t MaxElements;        // 0x08
    int32_t NumElements;        // 0x10

    inline FUObjectItem const* GetObjectPtr(int32_t Index) const
    {
        if (!IsValidIndex(Index)) {
            return nullptr;
        }
        FUObjectItem const* Objects = (FUObjectItem const*)DecryptObjectsAsm(ObjectsEncrypted);
        return &Objects[Index];
    }

    inline FUObjectItem* GetObjectPtr(int32_t Index)
    {
        if (!IsValidIndex(Index)) {
            return nullptr;
        }
        FUObjectItem* Objects = (FUObjectItem*)DecryptObjectsAsm(ObjectsEncrypted);
        return &Objects[Index];
    }

    /**
    * Return the number of elements in the array
    * Thread safe, but you know, someone might have added more elements before this even returns
    * @return   the number of elements in the array
    **/
    inline int32_t Num() const { return NumElements; }

    /**
    * Return if this index is valid
    * Thread safe, if it is valid now, it is valid forever. Other threads might be adding during this call.
    * @param    Index   Index to test
    * @return   true, if this is a valid
    **/
    inline bool IsValidIndex(int32_t Index) const { return Index < Num() && Index >= 0; }

    ///**
    //* Return a reference to an element
    //* @param    Index   Index to return
    //* @return   a reference to the pointer to the element
    //* Thread safe, if it is valid now, it is valid forever. This might return nullptr, but by then, some other thread might have made it non-nullptr.
    //**/
    //inline FUObjectItem const& operator[](int32_t Index) const
    //{
    //    FUObjectItem const* ItemPtr = GetObjectPtr(Index);
    //    return *ItemPtr;
    //}
    //
    //inline FUObjectItem& operator[](int32_t Index)
    //{
    //    FUObjectItem* ItemPtr = GetObjectPtr(Index);
    //    return *ItemPtr;
    //}
};

class FUObjectArray {
public:
    int32_t ObjFirstGCIndex; // 0x00
    int32_t ObjLastNonGCIndex; // 0x04
    int32_t MaxObjectsNotConsideredByGC; // 0x08
    bool OpenForDisregardForGC; // 0x0C

    FFixedUObjectArray ObjObjects; // 0x10

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
    return static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.Num();
}

int64_t ObjectsProxy::GetMax() const
{
    return static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.MaxElements;
}

FUObjectItem *ObjectsProxy::GetObjectsPrivate() const
{
    union CryptValue ObjectsDecrypted;
    uint64_t ObjectsEncrypted = static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.ObjectsEncrypted;

    LOG_DEBUG(_XOR_("ObjectsEncrypted = 0x%p"), (void*)ObjectsEncrypted);

    ObjectsDecrypted.Qword = DecryptObjectsAsm(ObjectsEncrypted);

    LOG_DEBUG(_XOR_("ObjectsDecrypted = 0x%p"), ObjectsDecrypted.Pointer);

    return static_cast<FUObjectItem *>(ObjectsDecrypted.Pointer);
}


UObject const* ObjectsProxy::GetById(int32_t Index) const
{
    FUObjectItem const* Object = static_cast<FUObjectArray*>(ObjectArray)->ObjObjects.GetObjectPtr(Index);
    if (!Object) {
        return nullptr;
    }
    return Object->Object;
}

UObject* ObjectsProxy::GetById(int32_t Index)
{
    FUObjectItem const* Object = static_cast<FUObjectArray*>(ObjectArray)->ObjObjects.GetObjectPtr(Index);
    if (!Object) {
        return nullptr;
    }
    return Object->Object;
}

UObject* ObjectsProxy::FindObject(const std::string& name)
{
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject* Object = GetById(i);
        if (Object && Object->GetFullName() == name) {
            return Object;
        }
    }
    return NULL;
}

UObject const* ObjectsProxy::FindObject(const std::string& name) const
{
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject const* Object = GetById(i);
        if (Object && Object->GetFullName() == name) {
            return Object;
        }
    }
    return NULL;
}

UClass* ObjectsProxy::FindClass(const std::string& Name)
{
    return static_cast<UClass*>(FindObject(Name));
}

UClass const* ObjectsProxy::FindClass(const std::string& Name) const
{
    return static_cast<UClass const*>(FindObject(Name));
}

int32_t ObjectsProxy::CountObjects(const UClass* CmpClass, const std::string& Name) const
{
    auto it = ObjectsCacheMap.find(Name);
    if (it != std::end(ObjectsCacheMap)) {
        return it->second;
    }

    int32_t Count = 0;
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject const* Object = GetById(i);
        if (Object && Object->IsA(CmpClass) && Object->GetName() == Name) {
            ++Count;
        }
    }

    ObjectsCacheMap[Name] = Count;
    return Count;
}