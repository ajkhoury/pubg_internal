#pragma once

#include "Types.h"

// Forwarded types
class UObject;
class UClass;
class UProperty;
class UFunction;

#define __XXSTRINGIFY(x) #x
#define _XXSTRINGIFY(x) __XXSTRINGIFY(x)
#define DECLARE_STATIC_CLASS(TClass) \
    static const UClass* StaticClass(); \
    static const UClass* U##TClass##Class
#define DEFINE_STATIC_CLASS(TClass) \
    const UClass* U##TClass::StaticClass() { \
        /* LOG_INFO("StaticClass of " _XXSTRINGIFY(TClass)); */ \
        if (!U##TClass##Class) \
            U##TClass##Class = ObjectsProxy().FindClass(_XORSTR_("Class CoreUObject." _XXSTRINGIFY(TClass))); \
        return U##TClass##Class; \
    } \
    const UClass* U##TClass::U##TClass##Class = nullptr

class UObject {
public:
    void** VTable; // 0x0000 (size=0x0008)
    uint64_t ClassEncrypted; // 0x0008 (size=0x0008)
    int32_t InternalIndexEncrypted; // 0x0010 (size=0x0004)
    int32_t NameIndexEncrypted; // 0x0014 (size=0x0004)
    int32_t NameNumberEncrypted; // 0x0018 (size=0x0004)
    uint8_t UnknownData0x001C[0x4]; // 0x001C (size=0x0004)
    uint64_t OuterEncrypted; // 0x0020 (size=0x0008)
    int32_t ObjectFlagsEncrypted; // 0x0028 (size=0x0004)
    uint8_t UnknownData0x002C[0x4]; // 0x002C (size=0x0004)

    int32_t GetFlags() const;
    uint32_t GetUniqueId() const;
    class UClass *GetClass() const;
    UObject *GetOuter() const;
    FName GetFName() const;

    const class UPackage* GetOutermost() const;

    std::string GetName() const;
    std::string GetFullName() const;

    std::string GetNameCPP() const;

    bool IsA(const UClass* CmpClass) const;
    template<typename T>
    inline bool IsA() const { return IsA(T::StaticClass()); }

    template<typename T>
    inline const T* Cast() const { return static_cast<const T*>(this); }
    template<typename T>
    inline T* Cast() { return static_cast<T*>(this); }

    DECLARE_STATIC_CLASS(Object);

}; // size=0x0030
C_ASSERT(sizeof(UObject) == 0x30);

class UPackage : public UObject {
public:
    bool bDirty;
    bool bHasBeenFullyLoaded;
    FName FolderName;
    float LoadTime;
    FGuid Guid;
    TArray<int32_t> ChunkIDs; 
    FName ForcedExportBasePackageName;
    uint32_t *PackageFlagsPrivate;
    uint8_t padField0[0xE8];

    DECLARE_STATIC_CLASS(Package);
}; // size=0x160 or 0x168

class UField : public UObject {
public:
    UField* Next; // 0x0030 (size=0x0008)

    inline const UField *GetNext() const { return Next; }
    inline UField *GetNext() { return Next; }

    DECLARE_STATIC_CLASS(Field);
}; // size=0x0038
C_ASSERT(sizeof(UField) == 0x38);

class UEnum : public UField {
public:
    FString CppType; // 0x0030 (size=0x0010)
    TArray<TPair<FName, int64_t>> Names; // 0x0040 (size=0x0010)
    uint8_t UnknownData0x0050[0x10]; // 0x0050 (size=0x0010)
    int32_t CppForm; // 0x0060 (size=0x0004)
    uint8_t UnknownData0x0064[0x4]; // 0x0064 (size=0x0004)
    void* EnumDisplayNameFn; // 0x0068  (size=0x0008)

    std::vector<std::string> GetNames() const;

    DECLARE_STATIC_CLASS(Enum);
}; // size=0x0078
C_ASSERT(sizeof(UEnum) == 0x78);

class UStruct : public UField {
public:
    uint8_t UnknownData0x0038[0x10]; // 0x0038 (size=0x0010)
    int32_t PropertiesSize; // 0x0048 (size=0x0004)
    uint8_t UnknownData0x004C[0x6C]; // 0x004C (size=0x006C)
    UField* Children; // 0x00B8 (size=0x0008)
    int32_t MinAlignment; // 0x00C0 (size=0x0004)
    uint8_t UnknownData0x00C4[0x1C]; // 0x00C4 (size=0x001C)
    class UStruct* SuperStruct; // 0x00E0 (size=0x0008)

    inline UStruct* GetSuper() const { return SuperStruct; }
    inline UField* GetChildren() const { return Children; }
    inline int32_t GetPropertiesSize() const { return PropertiesSize; }
    inline int32_t GetMinAlignment() const { return MinAlignment; }

    DECLARE_STATIC_CLASS(Struct);
}; // size=0x00E8
C_ASSERT(sizeof(UStruct) == 0xE8);

class UScriptStruct : public UStruct {
public:
    uint8_t UnknownData0x00E8[0x10]; // 0xE8

    DECLARE_STATIC_CLASS(ScriptStruct);
}; // size=0x00F8
C_ASSERT(sizeof(UScriptStruct) == 0xF8);

class UFunction : public UStruct {
public:
    uint8_t UnknownData0x00E8[0x3C]; // 0x00E8 (size=0x003C)
    uint32_t FunctionFlags; // 0x0124 (size=0x0004)

    inline uint32_t GetFunctionFlags() const { return FunctionFlags; }

    DECLARE_STATIC_CLASS(Function);
}; // size=0x0128
C_ASSERT(sizeof(UFunction) == 0x128);

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

class FClassBaseChain {
public:
    FClassBaseChain** ClassBaseChainArray;  // 0x00
    int32_t NumClassBasesInChainMinusOne;   // 0x08
};

class UClass : public UStruct, public FClassBaseChain {
public:
    uint8_t pad_0x00F8[0x1C0];              // 0xF8

    DECLARE_STATIC_CLASS(Class);
}; // size=0x2B8

