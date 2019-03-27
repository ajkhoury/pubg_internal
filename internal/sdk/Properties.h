#pragma once

#include "Objects.h"
#include "Format.h"
#include <array>

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
    CPF_InterfaceClearMask = (CPF_ExportObject|CPF_InstancedReference|CPF_ContainsInstancedReference),
    /** all the properties that can be stripped for final release console builds */
    CPF_DevelopmentAssets = (CPF_EditorOnly),
    /** all the properties that should never be loaded or saved */
    CPF_ComputedFlags = (CPF_IsPlainOldData | CPF_NoDestructor | CPF_ZeroConstructor | CPF_HasGetValueTypeHash),
    CPF_AllFlags = 0xFFFFFFFFFFFFFFFF
};

std::string StringifyPropertyFlags(const uint64_t Flags);

class UProperty : public UField {
public:
    int32_t ArrayDim; // 0x0030 (size=0x0004)
    int32_t ElementSize; // 0x0034 (size=0x0004)
    uint64_t PropertyFlags; // 0x0038 (size=0x0008)
    uint8_t UnknownData0x0040[0x10]; // 0x0040 (size=0x0010)
    int32_t Offset_Internal; // 0x0050 (size=0x0004)
    uint8_t UnknownData0x0054[0x24]; // 0x0054 (size=0x0024)

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
        int32_t Size;
        bool CanBeReference;
        std::string CppType;

        static Info Create(PropertyType Type, int32_t Size, bool bCanBeReference, std::string&& CppType)
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
}; // size=0x0078
C_ASSERT(sizeof(UProperty) == 0x78);

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

    UProperty::Info GetInfo() const
    {
        if (IsEnum()) {
            return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "TEnumAsByte<" + MakeUniqueCppName(GetEnum()) + ">");
        }
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "unsigned char");
    }

    DECLARE_STATIC_CLASS(ByteProperty);
};

class UUInt16Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint16_t), false, "uint16_t");
    }

    DECLARE_STATIC_CLASS(UInt16Property);
};

class UUInt32Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint32_t), false, "uint32_t");
    }

    DECLARE_STATIC_CLASS(UInt32Property);
};

class UUInt64Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint64_t), false, "uint64_t");
    }

    DECLARE_STATIC_CLASS(UInt64Property);
};

class UInt8Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(int8_t), false, "int8_t");
    }

    DECLARE_STATIC_CLASS(Int8Property);
};

class UInt16Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(int16_t), false, "int16_t");
    }

    DECLARE_STATIC_CLASS(Int16Property);
};

class UIntProperty : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(int), false, "int");
    }

    DECLARE_STATIC_CLASS(IntProperty);
};

class UInt64Property : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(int64_t), false, "int64_t");
    }

    DECLARE_STATIC_CLASS(Int64Property);
};

class UFloatProperty : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(float), false, "float");
    }

    DECLARE_STATIC_CLASS(FloatProperty);
};

class UDoubleProperty : public UNumericProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(double), false, "double");
    }

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

    inline bool IsNativeBool() const { return GetFieldMask() == 0xFF; }
    inline bool IsBitfield() const { return !IsNativeBool(); }

    UProperty::Info GetInfo() const
    {
        if (IsNativeBool()) {
            return UProperty::Info::Create(PropertyType::Primitive, sizeof(bool), false, "bool");
        }
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(unsigned char), false, "unsigned char");
    }

    std::array<int, 2> GetMissingBitsCount(const UBoolProperty* Other) const
    {
        // If there is no previous bitfield member, just calculate the missing bits.
        if (!Other) {
            return { GetBitPosition(GetByteMask()), -1 };
        }

        // If both bitfield member belong to the same byte, calculate the bit position difference.
        if (GetOffset() == Other->GetOffset()) {
            return { GetBitPosition(GetByteMask()) - GetBitPosition(Other->GetByteMask()) - 1, -1 };
        }

        // If they have different offsets, we need two distances
        // |00001000|00100000|
        // 1.   ^---^
        // 2.       ^--^
        return { std::numeric_limits<uint8_t>::digits - GetBitPosition(Other->GetByteMask()) - 1, GetBitPosition(GetByteMask()) };
    }

    DECLARE_STATIC_CLASS(BoolProperty);

private:
    inline int GetBitPosition(uint8_t value) const
    {
        int i4 = !(value & 0xf) << 2;
        value >>= i4;
        int i2 = !(value & 0x3) << 1;
        value >>= i2;
        int i1 = !(value & 0x1);
        int i0 = (value >> i1) & 1 ? 0 : -8;
        return i4 + i2 + i1 + i0;
    }
};

class UObjectPropertyBase : public UProperty {
public:
    UClass* PropertyClass;

    inline UClass* GetPropertyClass() const { return PropertyClass; }

    UProperty::Info GetInfo() const
    {
        UClass* Class = GetPropertyClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(Class->GetNameCPP()) + '*');
        }
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "/*WARNING PropertyClass NULL*/ class !!INVALID!!");
    }

    DECLARE_STATIC_CLASS(ObjectPropertyBase);
};

class UObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const
    {
        UClass* Class = GetPropertyClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(Class->GetNameCPP()) + '*');
        }
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "/*WARNING PropertyClass NULL*/ class !!INVALID!!");
    }

    DECLARE_STATIC_CLASS(ObjectProperty);
};

class UClassProperty : public UObjectProperty {
public:
    UClass* MetaClass;

    inline UClass* GetMetaClass() const { return MetaClass; }

    UProperty::Info GetInfo() const
    {
        UClass* Class = GetMetaClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(Class->GetNameCPP()) + '*');
        }
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "/*WARNING MetaClass NULL*/ class !!INVALID!!");
    }

    DECLARE_STATIC_CLASS(ClassProperty);
};

class UInterfaceProperty : public UProperty {
public:
    UClass* InterfaceClass;

    inline UClass* GetInterfaceClass() const { return InterfaceClass; }

    UProperty::Info GetInfo() const
    {
        UClass* Class = GetInterfaceClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptInterface), true, "TScriptInterface<class " + MakeValidName(Class->GetNameCPP()) + '>');
        }
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptInterface), true, "/*WARNING InterfaceClass NULL*/ TScriptInterface<class !!INVALID!!>");
    }

    DECLARE_STATIC_CLASS(InterfaceProperty);
};

class UWeakObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const
    {
        UClass* Class = GetPropertyClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::Container, sizeof(FWeakObjectPtr), false, "TWeakObjectPtr<class " + MakeValidName(Class->GetNameCPP()) + '>');
        }
        return UProperty::Info::Create(PropertyType::Container, sizeof(FWeakObjectPtr), true, "/*WARNING PropertyClass NULL*/ TWeakObjectPtr<class !!INVALID!!>");
    }

    DECLARE_STATIC_CLASS(WeakObjectProperty);
};

class ULazyObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const
    {
        UClass* Class = GetPropertyClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::Container, sizeof(FLazyObjectPtr), false, "TLazyObjectPtr<class " + MakeValidName(Class->GetNameCPP()) + '>');
        }
        return UProperty::Info::Create(PropertyType::Container, sizeof(FLazyObjectPtr), true, "/*WARNING PropertyClass NULL*/ TLazyObjectPtr<class !!INVALID!!>");
    }

    DECLARE_STATIC_CLASS(LazyObjectProperty);
};

class UAssetObjectProperty : public UObjectPropertyBase {
public:
    UProperty::Info GetInfo() const
    {
        UClass* Class = GetPropertyClass();
        if (Class) {
            return UProperty::Info::Create(PropertyType::Container, sizeof(FAssetPtr), false, "TAssetPtr<class " + MakeValidName(Class->GetNameCPP()) + '>');
        }
        return UProperty::Info::Create(PropertyType::Container, sizeof(FAssetPtr), true, "/*WARNING PropertyClass NULL*/ TAssetPtr<class !!INVALID!!>");
    }

    DECLARE_STATIC_CLASS(AssetObjectProperty);
};

class UAssetClassProperty : public UAssetObjectProperty {
public:
    UClass* MetaClass;

    inline UClass* GetMetaClass() const { return MetaClass; }

    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "");
    }

    DECLARE_STATIC_CLASS(AssetClassProperty);
};

class UNameProperty : public UProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FName), true, "struct FName");
    }

    DECLARE_STATIC_CLASS(NameProperty);
};

class UStructProperty : public UProperty {
public:
    UScriptStruct* Struct;

    inline UScriptStruct* GetStruct() const { return Struct; }

    UProperty::Info GetInfo() const
    {
        UScriptStruct* ScriptStruct = GetStruct();
        if (ScriptStruct) {
            return UProperty::Info::Create(PropertyType::CustomStruct, GetElementSize(), true, "struct " + MakeUniqueCppName(ScriptStruct));
        }
        return UProperty::Info::Create(PropertyType::CustomStruct, GetElementSize(), true, "/*WARNING Struct NULL*/ struct !!INVALID!!");
    }

    DECLARE_STATIC_CLASS(StructProperty);
};

class UStrProperty : public UProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FString), true, "struct FString");
    }

    DECLARE_STATIC_CLASS(StrProperty);
};

class UTextProperty : public UProperty {
public:
    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FText), true, "struct FText");
    }

    DECLARE_STATIC_CLASS(TextProperty);
};

class UArrayProperty : public UProperty {
public:
    UProperty* Inner;

    inline UProperty* GetInner() const { return Inner; }

    UProperty::Info GetInfo() const
    {
        UProperty* InnerProperty = GetInner();
        if (InnerProperty) {
            UProperty::Info InnerInfo = InnerProperty->GetInfo();
            if (InnerInfo.Type != PropertyType::Unknown) {
                return UProperty::Info::Create(PropertyType::Container, sizeof(TArray<void*>), false, "TArray<" + InnerInfo.CppType + ">");
            }
        }

        return { PropertyType::Unknown };
    }

    DECLARE_STATIC_CLASS(ArrayProperty);
};

class UMapProperty : public UProperty {
public:
    UProperty* KeyProp;
    UProperty* ValueProp;

    inline UProperty* GetKeyProperty() const { return KeyProp; }
    inline UProperty* GetValueProperty() const { return ValueProp; }

    UProperty::Info GetInfo() const
    {
        UProperty* KeyProperty = GetKeyProperty();
        UProperty* ValueProperty = GetValueProperty();
        if (KeyProperty && ValueProperty) {
            UProperty::Info KeyInfo = KeyProperty->GetInfo();
            UProperty::Info ValueInfo = ValueProperty->GetInfo();
            if (KeyInfo.Type != PropertyType::Unknown && ValueInfo.Type != PropertyType::Unknown) {
                return UProperty::Info::Create(PropertyType::Container, sizeof(TMap<void*, void*>), false, "TMap<" + KeyInfo.CppType + ", " + ValueInfo.CppType + ">");
            }
        }

        return { PropertyType::Unknown };
    }

    DECLARE_STATIC_CLASS(MapProperty);
};

class UDelegateProperty : public UProperty {
public:
    UFunction* SignatureFunction;

    inline UFunction* GetSignatureFunction() const { return SignatureFunction; }

    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptDelegate), true, "struct FScriptDelegate");
    }

    DECLARE_STATIC_CLASS(DelegateProperty);
};

class UMulticastDelegateProperty : public UProperty {
public:
    UFunction* SignatureFunction;

    inline UFunction* GetSignatureFunction() const { return SignatureFunction; }

    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptMulticastDelegate), true, "struct FScriptMulticastDelegate");
    }

    DECLARE_STATIC_CLASS(MulticastDelegateProperty);
};

class UEnumProperty : public UProperty {
public:
    class UNumericProperty* UnderlyingProp;
    class UEnum* Enum;

    inline UNumericProperty* GetUnderlyingProperty() const { return UnderlyingProp; }
    inline UEnum* GetEnum() const { return Enum; }

    UProperty::Info GetInfo() const
    {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, MakeUniqueCppName(GetEnum()));
    }

    DECLARE_STATIC_CLASS(EnumProperty);
};
