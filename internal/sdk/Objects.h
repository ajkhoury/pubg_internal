#pragma once

#include "Engine.h"
#include "Names.h"

#include <unordered_map>
#include <type_traits>

#define __XXSTRINGIFY(x) #x
#define _XXSTRINGIFY(x) __XXSTRINGIFY(x)
#define DECLARE_STATIC_CLASS(TClass) \
    static const UClass* StaticClass() { \
        if (!TClass##Class) \
            TClass##Class = ObjectsProxy().FindClass("Class CoreUObject." _XXSTRINGIFY(TClass)); \
        return TClass##Class; \
    } \
    static const UClass* TClass##Class

#define DEFINE_STATIC_CLASS(TClass) \
    const UClass* TClass##Proxy::TClass##Class = nullptr

class ObjectsProxy {
public:
    ObjectsProxy();

    inline void *GetAddress() const { return ObjectArray; }

    int32_t GetNum() const;
    int64_t GetMax() const;
    UObject *GetById(int32_t Index) const;

    template<typename T>
    const T* FindObject(const std::string& name) const;

    template<class T>
    int32_t CountObjects(const std::string& name) const;

    inline const UClass* FindClass(const std::string& name) const { return FindObject<UClass>(name); }

private:
    void *ObjectArray;
};

class ObjectProxy {
public:
    ObjectProxy()
        : Object(nullptr)
    {
    }

    ObjectProxy(const UObject* InObject)
        : Object(const_cast<UObject*>(InObject))
    {
    }

    inline UObject* operator->() { return Object; }
    inline const UObject* operator->() const { return Object; }

    inline UObject& operator*() { return *Object; }
    inline const UObject& operator*() const { return *Object; }

    inline ObjectProxy& operator=(UObject* InObject) { Object = InObject; return *this; }
    inline ObjectProxy& operator=(const ObjectProxy& InProxy) { Object = InProxy.Object; return *this; }

    inline operator UObject*() const { return Object; }

    inline const UObject* GetReference() const { return const_cast<const UObject*>(Object); }
    inline void *GetAddress() const { return (void *)Object; }
    inline bool IsValid() const { return GetAddress() != nullptr; }

    int32_t GetFlags() const;
    uint32_t GetUniqueId() const;
    UClass *GetClass() const;
    UObject *GetOuter() const;
    FName GetFName() const;

    const UPackage* GetOutermost() const;

    std::string GetName() const;
    std::string GetFullName() const;

    std::string GetNameCPP() const;

    template<typename Type>
    const Type* Get() const
    {
        static_assert(std::is_base_of<UObject, Type>::value == true, "Not a base of UObject!");
        return static_cast<const Type*>(GetReference());
    }

    template<typename Type>
    Type* Get()
    {
        static_assert(std::is_base_of<UObject, Type>::value == true, "Not a base of UObject!");
        return static_cast<Type*>(Object);
    }

    template<typename ProxyBase>
    ProxyBase Cast() const { return ProxyBase(Get<ProxyBase::Type>()); }

    template<typename T>
    bool IsA() const
    {
        const UClass* CmpClass = T::StaticClass();
        if (!CmpClass) {
            return false;
        }

        for (UClass* SuperClass = GetClass();
             SuperClass != nullptr;
             SuperClass = static_cast<UClass*>(SuperClass->SuperStruct)) {
            if (SuperClass == CmpClass) {
                return true;
            }
        }

        return false;
    }

    DECLARE_STATIC_CLASS(Object);

private:
    UObject *Object;
};

namespace std {
template<>
struct hash<ObjectProxy> {
    size_t operator()(const ObjectProxy& obj) const
    {
        return std::hash<void*>()(obj.GetAddress());
    }
};
}

inline bool operator==(const ObjectProxy& lhs, const ObjectProxy& rhs) { return rhs.GetAddress() == lhs.GetAddress(); }
inline bool operator!=(const ObjectProxy& lhs, const ObjectProxy& rhs) { return !(lhs == rhs); }

template<typename T>
const T* ObjectsProxy::FindObject(const std::string& name) const
{
    for (int32_t i = 0; i < GetNum(); ++i) {
        ObjectProxy Object = GetById(i);
        if (Object.IsValid()) {
            if (Object.GetFullName() == name) {
                return static_cast<const T*>(Object.GetReference());
            }
        }
    }
    return NULL;
}

static std::unordered_map<std::string, int32_t> ObjectsCacheMap;

template<class T>
int32_t ObjectsProxy::CountObjects(const std::string& name) const
{
    auto it = ObjectsCacheMap.find(name);
    if (it != std::end(ObjectsCacheMap)) {
        return it->second;
    }

    int32_t Count = 0;
    for (int32_t i = 0; i < GetNum(); ++i) {
        ObjectProxy Object = GetById(i);
        if (Object.IsValid()) {
            if (Object.IsA<T>() && Object.GetName() == name) {
                ++Count;
            }
        }
    }

    ObjectsCacheMap[name] = Count;
    return Count;
}

class FieldProxy : public ObjectProxy {
public:
    typedef UField Type;

    FieldProxy(const UField* InField)
        : ObjectProxy(InField)
    {
    }

    inline UField *GetNext() const
    {
        return static_cast<const UField*>(GetReference())->Next;
    }

    DECLARE_STATIC_CLASS(Field);
};

class EnumProxy : public FieldProxy {
public:
    typedef UEnum Type;

    EnumProxy(const UEnum* InEnum)
        : FieldProxy(InEnum)
    {
    }

    inline std::vector<std::string> GetNames() const
    {
        NamesProxy Names;
        std::vector<std::string> StringArray;
        const TArray<TPair<FName, uint64_t>>& NamesArray = static_cast<const UEnum*>(GetReference())->Names;

        for (int32_t i = 0; i < NamesArray.Num(); ++i) {
            StringArray.push_back(Names.GetById(NamesArray[i].Key.GetIndex()));
        }

        return StringArray;
    }

    DECLARE_STATIC_CLASS(Enum);
};

class StructProxy : public FieldProxy {
public:
    typedef UStruct Type;

    StructProxy(const UStruct* InStruct)
        : FieldProxy(InStruct)
    {
    }

    inline UStruct* GetSuper() const
    {
        return static_cast<const UStruct*>(GetReference())->SuperStruct;
    }

    inline UField* GetChildren() const
    {
        return static_cast<const UStruct*>(GetReference())->Children;
    }

    inline int32_t GetPropertiesSize() const
    {
        return static_cast<const UStruct*>(GetReference())->PropertiesSize;
    }

    inline int32_t GetMinAlignment() const
    {
        return static_cast<const UStruct*>(GetReference())->MinAlignment;
    }

    DECLARE_STATIC_CLASS(Struct);
};

class ScriptStructProxy : public StructProxy {
public:
    typedef UScriptStruct Type;

    ScriptStructProxy(const UScriptStruct* InFunc)
        : StructProxy(InFunc)
    {
    }

    DECLARE_STATIC_CLASS(ScriptStruct);
};

enum class FunctionFlags : uint32_t {
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

inline bool operator&(FunctionFlags lhs, FunctionFlags rhs)
{
    return (static_cast<std::underlying_type_t<FunctionFlags>>(lhs) & static_cast<std::underlying_type_t<FunctionFlags>>(rhs))
        == static_cast<std::underlying_type_t<FunctionFlags>>(rhs);
}

std::string StringifyFlags(const FunctionFlags Flags);

class FunctionProxy : public StructProxy {
public:
    typedef UFunction Type;

    FunctionProxy(const UFunction *InFunc)
        : StructProxy(InFunc)
    {
    }

    inline FunctionFlags GetFunctionFlags() const
    {
        return static_cast<FunctionFlags>(static_cast<const UFunction*>(GetReference())->FunctionFlags);
    }

    DECLARE_STATIC_CLASS(Function);
};

class ClassProxy : public StructProxy {
public:
    typedef UClass Type;

    ClassProxy(const UClass *InClass)
        : StructProxy(InClass)
    {
    }

    DECLARE_STATIC_CLASS(Class);
};
