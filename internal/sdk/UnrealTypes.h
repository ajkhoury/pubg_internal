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
    int32_t NameIndexEncrypted; // 0x0008 (size=0x0004)
    int32_t NameNumberEncrypted; // 0x000C (size=0x0004)
    uint64_t OuterEncrypted; // 0x0010 (size=0x0008)
    int32_t ObjectFlagsEncrypted; // 0x0018 (size=0x0004)
    int32_t InternalIndexEncrypted; // 0x001C (size=0x0004)
    uint64_t ClassEncrypted; // 0x0020 (size=0x0008)

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

}; // size=0x0028
C_ASSERT(sizeof(UObject) == 0x28);

class UPackage : public UObject {
public:
    bool bDirty;                            // 0x30
    bool bHasBeenFullyLoaded;               // 0x31
    FName FolderName;                       // 0x38
    float LoadTime;                         // 0x40
    FGuid Guid;                             // 0x48
    TArray<int32_t> ChunkIDs;               // 0x58
    FName ForcedExportBasePackageName;      // 0x68
    uint32_t *PackageFlagsPrivate;          // 0x70
    uint8_t pad_0x0078[0xE8];               // 0x78

    DECLARE_STATIC_CLASS(Package);
}; // size=0x160

class UField : public UObject {
public:
    UField* Next; // 0x0028 (size=0x0008)

    inline const UField *GetNext() const { return Next; }
    inline UField *GetNext() { return Next; }

    DECLARE_STATIC_CLASS(Field);
}; // size=0x0030
C_ASSERT(sizeof(UField) == 0x30);

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
}; // size=0x0070
C_ASSERT(sizeof(UEnum) == 0x70);

class UStruct : public UField {
public:
    uint8_t UnknownData0x0030[0x10]; // 0x0030 (size=0x0010)
    int32_t MinAlignment; // 0x0040 (size=0x0004)
    uint8_t UnknownData0x0044[0x7C]; // 0x0044 (size=0x007C)
    class UStruct* SuperStruct; // 0x00C0 (size=0x0008)
    UField* Children; // 0x00C8 (size=0x0008)
    uint8_t UnknownData0x00D0[0x8]; // 0x00D0 (size=0x0008)
    int32_t PropertiesSize; // 0x00D8 (size=0x0004)
    uint8_t UnknownData0x00DC[0x4]; // 0x00DC (size=0x0004)

    inline UStruct* GetSuper() const { return SuperStruct; }
    inline UField* GetChildren() const { return Children; }
    inline int32_t GetPropertiesSize() const { return PropertiesSize; }
    inline int32_t GetMinAlignment() const { return MinAlignment; }

    DECLARE_STATIC_CLASS(Struct);
}; // size=0x00E0
C_ASSERT(sizeof(UStruct) == 0xE0);

class UScriptStruct : public UStruct {
public:
    uint8_t UnknownData0x00E0[0x10]; // 0xE0

    DECLARE_STATIC_CLASS(ScriptStruct);
}; // size=0xF0
C_ASSERT(sizeof(UScriptStruct) == 0xF0);

class UFunction : public UStruct {
public:
    uint8_t UnknownData0x00E0[0x38]; // 0x00E0 (size=0x0038)
    uint32_t FunctionFlags; // 0x0118 (size=0x0004)
    uint8_t UnknownData0x011C[0x14]; // 0x011C (size=0x0014)

    inline uint32_t GetFunctionFlags() const { return FunctionFlags; }

    DECLARE_STATIC_CLASS(Function);
}; // size=0x0130
C_ASSERT(sizeof(UFunction) == 0x130);

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

