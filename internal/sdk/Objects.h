#pragma once

#include "UnrealTypes.h"
#include "Names.h"

#include <native/log.h>

#include <unordered_map>
#include <type_traits>

class ObjectsProxy {
    friend class ObjectIterator;
public:
    ObjectsProxy();

    inline void *GetAddress() const { return ObjectArray; }

    int32_t GetNum() const;
    int64_t GetMax() const;
    UObject *GetById(int32_t Index) const;

    template<typename T>
    T* FindObject(const std::string& name) const;

    template<class T>
    int32_t CountObjects(const std::string& name) const;

    inline const UClass* FindClass(const std::string& name) const {
        return FindObject<UClass>(name);
    }

    class ObjectIterator {
    public:
        ObjectIterator(const class ObjectsProxy* InObjects, const int32_t InObjectIndex) : Index(InObjectIndex), Objects(InObjects) {}
        ObjectIterator(const ObjectIterator& Other) : Index(Other.Index), Objects(Other.Objects) {}
        ObjectIterator(ObjectIterator&& Other) noexcept : Index(Other.Index), Objects(Other.Objects) {}

        inline ObjectIterator& operator=(const ObjectIterator& rhs) { Index = rhs.Index; return *this; }
        inline void swap(ObjectIterator& Other) noexcept { std::swap(Index, Other.Index); }

        inline bool operator==(const ObjectIterator& rhs) const { return Index == rhs.Index; }
        inline bool operator!=(const ObjectIterator& rhs) const { return Index != rhs.Index; }

        inline ObjectIterator& operator++() { ++Index; return *this; }
        inline ObjectIterator operator++(int) { auto tmp(*this); ++(*this); return tmp; }

        inline UObject* operator*() const { return Objects->GetById(Index); }
        inline UObject* operator->() const { return Objects->GetById(Index); }

    private:
        int32_t Index;
        const class ObjectsProxy* Objects;
    };

    inline ObjectIterator begin() { return ObjectIterator(this, 0); }
    inline const ObjectIterator begin() const { return ObjectIterator(this, 0); }
    inline ObjectIterator end() { return ObjectIterator(this, GetNum()); }
    inline const ObjectIterator end() const { return ObjectIterator(this, GetNum()); }

private:
    FUObjectItem *GetObjectsPrivate() const;

    void *ObjectArray;

    static std::unordered_map<std::string, int32_t> ObjectsCacheMap;
};

template<typename T>
T* ObjectsProxy::FindObject(const std::string& name) const
{
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject* Object = GetById(i);
        if (Object != nullptr) {
            if (Object->GetFullName() == name) {
                return static_cast<T*>(Object);
            }
        }
    }
    return NULL;
}

template<class T>
int32_t ObjectsProxy::CountObjects(const std::string& name) const
{
    auto it = ObjectsCacheMap.find(name);
    if (it != std::end(ObjectsCacheMap)) {
        return it->second;
    }

    int32_t Count = 0;
    for (int32_t i = 0; i < GetNum(); ++i) {
        UObject* Object = GetById(i);
        if (Object != nullptr) {
            if (Object->IsA<T>() && Object->GetName() == name) {
                ++Count;
            }
        }
    }

    ObjectsCacheMap[name] = Count;
    return Count;
}

enum FunctionFlags : uint32_t {
    Final = 0x00000001,
    RequiredAPI = 0x00000002,
    BlueprintAuthorityOnly = 0x00000004,
    BlueprintCosmetic = 0x00000008,
    Net = 0x00000040,
    NetReliable = 0x00000080,
    NetRequest = 0x00000100,
    Exec = 0x00000200,
    Native = 0x00000400,
    Event = 0x00000800,
    NetResponse = 0x00001000,
    Static = 0x00002000,
    NetMulticast = 0x00004000,
    MulticastDelegate = 0x00010000,
    Public = 0x00020000,
    Private = 0x00040000,
    Protected = 0x00080000,
    Delegate = 0x00100000,
    NetServer = 0x00200000,
    HasOutParms = 0x00400000,
    HasDefaults = 0x00800000,
    NetClient = 0x01000000,
    DLLImport = 0x02000000,
    BlueprintCallable = 0x04000000,
    BlueprintEvent = 0x08000000,
    BlueprintPure = 0x10000000,
    Const = 0x40000000,
    NetValidate = 0x80000000
};

std::string StringifyFunctionFlags(const uint32_t Flags);
