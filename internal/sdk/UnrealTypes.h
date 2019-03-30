#pragma once

#include "Types.h"

#include <array>

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
    uint64_t OuterEncrypted; // 0x0020 (size=0x0008)
    int32_t ObjectFlagsEncrypted; // 0x0028 (size=0x0004)

    int32_t GetFlags() const;
    uint32_t GetUniqueId() const;
    class UClass const* GetClass() const;
    UObject const* GetOuter() const;
    FName GetFName() const;

    class UPackage const* GetOutermost() const;

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

    template<typename T>
    inline const T& CastRef() const { return *static_cast<const T*>(this); }
    template<typename T>
    inline T& CastRef() { return *static_cast<T*>(this); }

    DECLARE_STATIC_CLASS(Object);

}; // size=0x0030
C_ASSERT(sizeof(UObject) == 0x30);

class UPackage : public UObject {
public:
    bool bDirty;
    bool bHasBeenFullyLoaded;
    FName FolderName;
    float LoadTime;
    UGuid Guid;
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
ENUM_CLASS_FLAGS(FunctionFlags);

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



class UProperty : public UField {
public:
    int32_t ArrayDim; // 0x0038 (size=0x0004)
    int32_t ElementSize; // 0x003C (size=0x0004)
    uint64_t PropertyFlags; // 0x0040 (size=0x0008)
    uint8_t UnknownData0x0048[0x10]; // 0x0048 (size=0x0010)
    int32_t Offset_Internal; // 0x0058 (size=0x0004)
    uint8_t UnknownData0x005C[0x24]; // 0x005C (size=0x0024)

    inline int32_t GetArrayDim() const { return ArrayDim; }
    inline int32_t GetElementSize() const { return ElementSize; }
    inline uint64_t GetPropertyFlags() const { return PropertyFlags; }
    inline int32_t GetOffset() const { return Offset_Internal; }

    enum class PropertyType {
        Unknown,
        Primitive,
        PredefinedStruct,
        CustomStruct,
        Container
    };

    struct Info {
        PropertyType Type;
        uint32_t Size;
        bool CanBeReference;
        std::string CppType;

        static Info Create(PropertyType Type, uint32_t Size, bool bCanBeReference, std::string&& CppType)
        {
            Info NewInfo;
            NewInfo.Type = Type;
            NewInfo.Size = Size;
            NewInfo.CanBeReference = bCanBeReference;
            NewInfo.CppType = CppType;
            return NewInfo;
        }
    };

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(Property);
}; // size=0x0080
C_ASSERT(sizeof(UProperty) == 0x80);

enum PropertyFlags : uint64_t {
    CPF_Edit = 0x0000000000000001,
    CPF_ConstParm = 0x0000000000000002,
    CPF_BlueprintVisible = 0x0000000000000004,
    CPF_ExportObject = 0x0000000000000008,
    CPF_BlueprintReadOnly = 0x0000000000000010,
    CPF_Net = 0x0000000000000020,
    CPF_EditFixedSize = 0x0000000000000040,
    CPF_Parm = 0x0000000000000080,
    CPF_OutParm = 0x0000000000000100,
    CPF_ZeroConstructor = 0x0000000000000200,
    CPF_ReturnParm = 0x0000000000000400,
    CPF_DisableEditOnTemplate = 0x0000000000000800,
    CPF_Transient = 0x0000000000002000,
    CPF_Config = 0x0000000000004000,
    CPF_DisableEditOnInstance = 0x0000000000010000,
    CPF_EditConst = 0x0000000000020000,
    CPF_GlobalConfig = 0x0000000000040000,
    CPF_InstancedReference = 0x0000000000080000,
    CPF_DuplicateTransient = 0x0000000000200000,
    CPF_SubobjectReference = 0x0000000000400000,
    CPF_SaveGame = 0x0000000001000000,
    CPF_NoClear = 0x0000000002000000,
    CPF_ReferenceParm = 0x0000000008000000,
    CPF_BlueprintAssignable = 0x0000000010000000,
    CPF_Deprecated = 0x0000000020000000,
    CPF_IsPlainOldData = 0x0000000040000000,
    CPF_RepSkip = 0x0000000080000000,
    CPF_RepNotify = 0x0000000100000000,
    CPF_Interp = 0x0000000200000000,
    CPF_NonTransactional = 0x0000000400000000,
    CPF_EditorOnly = 0x0000000800000000,
    CPF_NoDestructor = 0x0000001000000000,
    CPF_AutoWeak = 0x0000004000000000,
    CPF_ContainsInstancedReference = 0x0000008000000000,
    CPF_AssetRegistrySearchable = 0x0000010000000000,
    CPF_SimpleDisplay = 0x0000020000000000,
    CPF_AdvancedDisplay = 0x0000040000000000,
    CPF_Protected = 0x0000080000000000,
    CPF_BlueprintCallable = 0x0000100000000000,
    CPF_BlueprintAuthorityOnly = 0x0000200000000000,
    CPF_TextExportTransient = 0x0000400000000000,
    CPF_NonPIEDuplicateTransient = 0x0000800000000000,
    CPF_ExposeOnSpawn = 0x0001000000000000,
    CPF_PersistentInstance = 0x0002000000000000,
    CPF_UObjectWrapper = 0x0004000000000000,
    CPF_HasGetValueTypeHash = 0x0008000000000000,
    CPF_NativeAccessSpecifierPublic = 0x0010000000000000,
    CPF_NativeAccessSpecifierProtected = 0x0020000000000000,
    CPF_NativeAccessSpecifierPrivate = 0x0040000000000000,

    /** Combinations flags */
    CPF_NativeAccessSpecifiers = (CPF_NativeAccessSpecifierPublic | CPF_NativeAccessSpecifierProtected | CPF_NativeAccessSpecifierPrivate),
    CPF_ParmFlags = (CPF_Parm | CPF_OutParm | CPF_ReturnParm | CPF_ReferenceParm | CPF_ConstParm),
    CPF_PropagateToArrayInner = (CPF_ExportObject | CPF_PersistentInstance | CPF_InstancedReference | CPF_ContainsInstancedReference | CPF_Config | CPF_EditConst | CPF_Deprecated | CPF_EditorOnly | CPF_AutoWeak | CPF_UObjectWrapper),
    CPF_PropagateToMapValue = (CPF_ExportObject | CPF_PersistentInstance | CPF_InstancedReference | CPF_ContainsInstancedReference | CPF_Config | CPF_EditConst | CPF_Deprecated | CPF_EditorOnly | CPF_AutoWeak | CPF_UObjectWrapper | CPF_Edit),
    CPF_PropagateToMapKey = (CPF_ExportObject | CPF_PersistentInstance | CPF_InstancedReference | CPF_ContainsInstancedReference | CPF_Config | CPF_EditConst | CPF_Deprecated | CPF_EditorOnly | CPF_AutoWeak | CPF_UObjectWrapper | CPF_Edit),
    CPF_PropagateToSetElement = (CPF_ExportObject | CPF_PersistentInstance | CPF_InstancedReference | CPF_ContainsInstancedReference | CPF_Config | CPF_EditConst | CPF_Deprecated | CPF_EditorOnly | CPF_AutoWeak | CPF_UObjectWrapper | CPF_Edit),
    /** the flags that should never be set on interface properties */
    CPF_InterfaceClearMask = (CPF_ExportObject | CPF_InstancedReference | CPF_ContainsInstancedReference),
    /** all the properties that can be stripped for final release console builds */
    CPF_DevelopmentAssets = (CPF_EditorOnly),
    /** all the properties that should never be loaded or saved */
    CPF_ComputedFlags = (CPF_IsPlainOldData | CPF_NoDestructor | CPF_ZeroConstructor | CPF_HasGetValueTypeHash),
    CPF_AllFlags = 0xFFFFFFFFFFFFFFFF
};

class UNumericProperty : public UProperty {
public:
    DECLARE_STATIC_CLASS(NumericProperty);
};

class UByteProperty : public UNumericProperty {
private:
    UEnum *Enum;

public:
    inline UEnum* GetEnum() const { return Enum; }
    inline bool IsEnum() const { return GetEnum() != nullptr; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(ByteProperty);
};

class UUInt16Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(UInt16Property);
};

class UUInt32Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(UInt32Property);
};

class UUInt64Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(UInt64Property);
};

class UInt8Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(Int8Property);
};

class UInt16Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(Int16Property);
};

class UIntProperty : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(IntProperty);
};

class UInt64Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(Int64Property);
};

class UFloatProperty : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(FloatProperty);
};

class UDoubleProperty : public UNumericProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(DoubleProperty);
};

class UBoolProperty : public UProperty {
public:
    uint8_t FieldSize;
    uint8_t ByteOffset;
    uint8_t ByteMask;
    uint8_t FieldMask;

    inline uint8_t GetFieldSize() const { return FieldSize; }
    inline uint8_t GetByteOffset() const { return ByteOffset; }
    inline uint8_t GetByteMask() const { return ByteMask; }
    inline uint8_t GetFieldMask() const { return FieldMask; }

    bool IsNativeBool() const { return GetFieldMask() == 0xFF; }
    bool IsBitfield() const { return !IsNativeBool(); }

    struct MissingBitsCount {
        int Count0, Count1;
        inline int const& operator[](int32_t Index) const { return *((int *)this + Index); }
        inline int& operator[](int32_t Index) { return *((int *)this + Index); }
    };
    MissingBitsCount GetMissingBitsCount(const UBoolProperty* Other) const;

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(BoolProperty);
};

inline bool operator<(const UBoolProperty& lhs, const UBoolProperty& rhs)
{
    if (lhs.GetByteOffset() == rhs.GetByteOffset())
        return lhs.GetByteMask() < rhs.GetByteMask();
    return lhs.GetByteOffset() < rhs.GetByteOffset();
}


class UObjectPropertyBase : public UProperty {
public:
    UClass* PropertyClass;

    inline UClass* GetPropertyClass() const { return PropertyClass; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(ObjectPropertyBase);
};

class UObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(ObjectProperty);
};

class UClassProperty : public UObjectProperty {
public:
    UClass* MetaClass;

    inline UClass* GetMetaClass() const { return MetaClass; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(ClassProperty);
};

class UInterfaceProperty : public UProperty {
public:
    UClass* InterfaceClass;

    inline UClass* GetInterfaceClass() const { return InterfaceClass; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(InterfaceProperty);
};

class UWeakObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(WeakObjectProperty);
};

class ULazyObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(LazyObjectProperty);
};

class UAssetObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(AssetObjectProperty);
};

class UAssetClassProperty : public UAssetObjectProperty {
public:
    UClass* MetaClass;

    inline UClass* GetMetaClass() const { return MetaClass; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(AssetClassProperty);
};

class UNameProperty : public UProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(NameProperty);
};

class UStructProperty : public UProperty {
public:
    UScriptStruct* Struct;

    inline UScriptStruct* GetStruct() const { return Struct; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(StructProperty);
};

class UStrProperty : public UProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(StrProperty);
};

class UTextProperty : public UProperty {
public:
    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(TextProperty);
};

class UArrayProperty : public UProperty {
public:
    UProperty* Inner;

    inline UProperty* GetInner() const { return Inner; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(ArrayProperty);
};

class UMapProperty : public UProperty {
public:
    UProperty* KeyProp;
    UProperty* ValueProp;

    inline UProperty* GetKeyProperty() const { return KeyProp; }
    inline UProperty* GetValueProperty() const { return ValueProp; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(MapProperty);
};

class UDelegateProperty : public UProperty {
public:
    UFunction* SignatureFunction;

    inline UFunction* GetSignatureFunction() const { return SignatureFunction; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(DelegateProperty);
};

class UMulticastDelegateProperty : public UProperty {
public:
    UFunction* SignatureFunction;

    inline UFunction* GetSignatureFunction() const { return SignatureFunction; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(MulticastDelegateProperty);
};

class UEnumProperty : public UProperty {
public:
    class UNumericProperty* UnderlyingProp;
    class UEnum* Enum;

    inline UNumericProperty* GetUnderlyingProperty() const { return UnderlyingProp; }
    inline UEnum* GetEnum() const { return Enum; }

    UProperty::Info GetInfo() const;

    DECLARE_STATIC_CLASS(EnumProperty);
};

