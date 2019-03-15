#pragma once

#include "Objects.h"
#include "Format.h"
#include <array>


enum class PropertyFlags : uint64_t {
    Edit = 0x0000000000000001,
    ConstParm = 0x0000000000000002,
    BlueprintVisible = 0x0000000000000004,
    ExportObject = 0x0000000000000008,
    BlueprintReadOnly = 0x0000000000000010,
    Net = 0x0000000000000020,
    EditFixedSize = 0x0000000000000040,
    Parm = 0x0000000000000080,
    OutParm = 0x0000000000000100,
    ZeroConstructor = 0x0000000000000200,
    ReturnParm = 0x0000000000000400,
    DisableEditOnTemplate = 0x0000000000000800,
    Transient = 0x0000000000002000,
    Config = 0x0000000000004000,
    DisableEditOnInstance = 0x0000000000010000,
    EditConst = 0x0000000000020000,
    GlobalConfig = 0x0000000000040000,
    InstancedReference = 0x0000000000080000,
    DuplicateTransient = 0x0000000000200000,
    SubobjectReference = 0x0000000000400000,
    SaveGame = 0x0000000001000000,
    NoClear = 0x0000000002000000,
    ReferenceParm = 0x0000000008000000,
    BlueprintAssignable = 0x0000000010000000,
    Deprecated = 0x0000000020000000,
    IsPlainOldData = 0x0000000040000000,
    RepSkip = 0x0000000080000000,
    RepNotify = 0x0000000100000000,
    Interp = 0x0000000200000000,
    NonTransactional = 0x0000000400000000,
    EditorOnly = 0x0000000800000000,
    NoDestructor = 0x0000001000000000,
    AutoWeak = 0x0000004000000000,
    ContainsInstancedReference = 0x0000008000000000,
    AssetRegistrySearchable = 0x0000010000000000,
    SimpleDisplay = 0x0000020000000000,
    AdvancedDisplay = 0x0000040000000000,
    Protected = 0x0000080000000000,
    BlueprintCallable = 0x0000100000000000,
    BlueprintAuthorityOnly = 0x0000200000000000,
    TextExportTransient = 0x0000400000000000,
    NonPIEDuplicateTransient = 0x0000800000000000,
    ExposeOnSpawn = 0x0001000000000000,
    PersistentInstance = 0x0002000000000000,
    UObjectWrapper = 0x0004000000000000,
    HasGetValueTypeHash = 0x0008000000000000,
    NativeAccessSpecifierPublic = 0x0010000000000000,
    NativeAccessSpecifierProtected = 0x0020000000000000,
    NativeAccessSpecifierPrivate = 0x0040000000000000
};

inline bool operator&(PropertyFlags lhs, PropertyFlags rhs)
{
    return (static_cast<std::underlying_type_t<PropertyFlags>>(lhs) & static_cast<std::underlying_type_t<PropertyFlags>>(rhs))
        == static_cast<std::underlying_type_t<PropertyFlags>>(rhs);
}

std::string StringifyFlags(const PropertyFlags Flags);

enum class PropertyType {
    Unknown,
    Primitive,
    PredefinedStruct,
    CustomStruct,
    Container
};

struct PropertyInfo {
    PropertyType Type;
    int32_t Size;
    bool CanBeReference;
    std::string CppType;

    static PropertyInfo Create(PropertyType Type, int32_t Size, bool bCanBeReference, std::string&& CppType)
    {
        PropertyInfo NewInfo;
        NewInfo.Type = Type;
        NewInfo.Size = Size;
        NewInfo.CanBeReference = bCanBeReference;
        NewInfo.CppType = CppType;
        return NewInfo;
    }
};

class PropertyProxy : public FieldProxy {
public:
    typedef UProperty Type;

    PropertyProxy(const UProperty *Property)
        : FieldProxy(Property)
    {
    }

    inline int32_t GetArrayDim() const
    {
        return static_cast<const UProperty*>(GetReference())->ArrayDim;
    }

    inline int32_t GetElementSize() const
    {
        return static_cast<const UProperty*>(GetReference())->ElementSize;
    }

    inline PropertyFlags GetPropertyFlags() const
    {
        return static_cast<PropertyFlags>(static_cast<const UProperty*>(GetReference())->PropertyFlags);
    }

    inline int32_t GetOffset() const
    {
        return static_cast<const UProperty*>(GetReference())->Offset_Internal;
    }

    PropertyInfo GetInfo() const;

    DECLARE_STATIC_CLASS(Property);
};

class NumericPropertyProxy : public PropertyProxy {
public:
    typedef UNumericProperty Type;

    NumericPropertyProxy(const UNumericProperty* Property)
        : PropertyProxy(Property)
    {
    }

    DECLARE_STATIC_CLASS(NumericProperty);
};

class BytePropertyProxy : public NumericPropertyProxy {
public:
    typedef UByteProperty Type;

    BytePropertyProxy(const UByteProperty* Property)
        : NumericPropertyProxy(Property)
    {
    }

    inline UEnum* GetEnum() const
    {
        return static_cast<const UByteProperty*>(GetReference())->Enum;
    }

    inline bool IsEnum() const
    {
        return GetEnum() != nullptr;
    }

    PropertyInfo GetInfo() const
    {
        if (IsEnum()) {
            return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint8_t), false, "TEnumAsByte<" + MakeUniqueCppName(GetEnum()) + ">");
        }
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint8_t), false, "unsigned char");
    }

    DECLARE_STATIC_CLASS(ByteProperty);
};

class UInt16PropertyProxy : public NumericPropertyProxy {
public:
    typedef UUInt16Property Type;

    UInt16PropertyProxy(const UUInt16Property* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint16_t), false, "uint16_t");
    }

    DECLARE_STATIC_CLASS(UInt16Property);
};

class UInt32PropertyProxy : public NumericPropertyProxy {
public:
    typedef UUInt32Property Type;

    UInt32PropertyProxy(const UUInt32Property* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint32_t), false, "uint32_t");
    }

    DECLARE_STATIC_CLASS(UInt32Property);
};

class UInt64PropertyProxy : public NumericPropertyProxy {
public:
    typedef UUInt64Property Type;

    UInt64PropertyProxy(const UUInt64Property* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint64_t), false, "uint64_t");
    }

    DECLARE_STATIC_CLASS(UInt64Property);
};

class Int8PropertyProxy : public NumericPropertyProxy {
public:
    typedef UInt8Property Type;

    Int8PropertyProxy(const UInt8Property* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(int8_t), false, "int8_t");
    }

    DECLARE_STATIC_CLASS(Int8Property);
};

class Int16PropertyProxy : public NumericPropertyProxy {
public:
    typedef UInt16Property Type;

    Int16PropertyProxy(const UInt16Property* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(int16_t), false, "int16_t");
    }

    DECLARE_STATIC_CLASS(Int16Property);
};

class IntPropertyProxy : public NumericPropertyProxy {
public:
    typedef UIntProperty Type;

    IntPropertyProxy(const UIntProperty* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(int), false, "int");
    }

    DECLARE_STATIC_CLASS(IntProperty);
};

class Int64PropertyProxy : public NumericPropertyProxy {
public:
    typedef UInt64Property Type;

    Int64PropertyProxy(const UInt64Property* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(int64_t), false, "int64_t");
    }

    DECLARE_STATIC_CLASS(Int64Property);
};

class FloatPropertyProxy : public NumericPropertyProxy {
public:
    typedef UFloatProperty Type;

    FloatPropertyProxy(const UFloatProperty* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(float), false, "float");
    }

    DECLARE_STATIC_CLASS(FloatProperty);
};

class DoublePropertyProxy : public NumericPropertyProxy {
public:
    typedef UDoubleProperty Type;

    DoublePropertyProxy(const UDoubleProperty* Property)
        : NumericPropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(double), false, "double");
    }

    DECLARE_STATIC_CLASS(DoubleProperty);
};

__forceinline int GetBitPosition(uint8_t value)
{
    int i4 = !(value & 0xf) << 2;
    value >>= i4;
    int i2 = !(value & 0x3) << 1;
    value >>= i2;
    int i1 = !(value & 0x1);
    int i0 = (value >> i1) & 1 ? 0 : -8;
    return i4 + i2 + i1 + i0;
}

class BoolPropertyProxy : public PropertyProxy {
public:
    typedef UBoolProperty Type;

    BoolPropertyProxy(const UBoolProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline uint8_t GetFieldSize() const
    {
        return static_cast<const UBoolProperty*>(GetReference())->FieldSize;
    }

    inline uint8_t GetByteOffset() const
    {
        return static_cast<const UBoolProperty*>(GetReference())->ByteOffset;
    }

    inline uint8_t GetByteMask() const
    {
        return static_cast<const UBoolProperty*>(GetReference())->ByteMask;
    }

    inline uint8_t GetFieldMask() const
    {
        return static_cast<const UBoolProperty*>(GetReference())->FieldMask;
    }

    inline bool IsNativeBool() const
    {
        return GetFieldMask() == 0xFF;
    }

    inline bool IsBitfield() const
    {
        return !IsNativeBool();
    }

    std::array<int, 2> GetMissingBitsCount(const BoolPropertyProxy& Other) const
    {
        // If there is no previous bitfield member, just calculate the missing bits.
        if (!Other.IsValid()) {
            return { GetBitPosition(GetByteMask()), -1 };
        }

        // If both bitfield member belong to the same byte, calculate the bit position difference.
        if (GetOffset() == Other.GetOffset()) {
            return { GetBitPosition(GetByteMask()) - GetBitPosition(Other.GetByteMask()) - 1, -1 };
        }

        // If they have different offsets, we need two distances
        // |00001000|00100000|
        // 1.   ^---^
        // 2.       ^--^
        return { std::numeric_limits<uint8_t>::digits - GetBitPosition(Other.GetByteMask()) - 1, GetBitPosition(GetByteMask()) };
    }

    PropertyInfo GetInfo() const
    {
        if (IsNativeBool()) {
            return PropertyInfo::Create(PropertyType::Primitive, sizeof(bool), false, "bool");
        }
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(unsigned char), false, "unsigned char");
    }

    DECLARE_STATIC_CLASS(BoolProperty);
};

class ObjectPropertyBaseProxy : public PropertyProxy {
public:
    typedef UObjectPropertyBase Type;

    ObjectPropertyBaseProxy(const UObjectPropertyBase* Property)
        : PropertyProxy(Property)
    {
    }

    inline UClass* GetPropertyClass() const
    {
        return static_cast<const UObjectPropertyBase*>(GetReference())->PropertyClass;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(ObjectProxy(GetPropertyClass()).GetNameCPP()) + "*");
    }

    DECLARE_STATIC_CLASS(ObjectPropertyBase);
};

class ObjectPropertyProxy : public ObjectPropertyBaseProxy {
public:
    typedef UObjectProperty Type;

    ObjectPropertyProxy(const UObjectProperty* Property)
        : ObjectPropertyBaseProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(ObjectProxy(GetPropertyClass()).GetNameCPP()) + "*");
    }

    DECLARE_STATIC_CLASS(ObjectProperty);
};

class ClassPropertyProxy : public ObjectPropertyProxy {
public:
    typedef UClassProperty Type;

    ClassPropertyProxy(const UClassProperty* Property)
        : ObjectPropertyProxy(Property)
    {
    }

    inline UClass* GetMetaClass() const
    {
        return static_cast<const UClassProperty*>(GetReference())->MetaClass;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(void*), false, "class " + MakeValidName(ObjectProxy(GetMetaClass()).GetNameCPP()) + "*");
    }

    DECLARE_STATIC_CLASS(ClassProperty);
};

class InterfacePropertyProxy : public PropertyProxy {
public:
    typedef UInterfaceProperty Type;

    InterfacePropertyProxy(const UInterfaceProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UClass* GetInterfaceClass() const
    {
        return static_cast<const UInterfaceProperty*>(GetReference())->InterfaceClass;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::PredefinedStruct, sizeof(FScriptInterface), true, "TScriptInterface<class " + MakeValidName(ObjectProxy(GetInterfaceClass()).GetNameCPP()) + ">");
    }

    DECLARE_STATIC_CLASS(InterfaceProperty);
};

class WeakObjectPropertyProxy : public ObjectPropertyBaseProxy {
public:
    typedef UWeakObjectProperty Type;

    WeakObjectPropertyProxy(const UWeakObjectProperty* Property)
        : ObjectPropertyBaseProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Container, sizeof(FWeakObjectPtr), false, "TWeakObjectPtr<class " + MakeValidName(ObjectProxy(GetPropertyClass()).GetNameCPP()) + ">");
    }

    DECLARE_STATIC_CLASS(WeakObjectProperty);
};

class LazyObjectPropertyProxy : public ObjectPropertyBaseProxy {
public:
    typedef ULazyObjectProperty Type;

    LazyObjectPropertyProxy(const ULazyObjectProperty* Property)
        : ObjectPropertyBaseProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Container, sizeof(FLazyObjectPtr), false, "TLazyObjectPtr<class " + MakeValidName(ObjectProxy(GetPropertyClass()).GetNameCPP()) + ">");
    }

    DECLARE_STATIC_CLASS(LazyObjectProperty);
};

class AssetObjectPropertyProxy : public ObjectPropertyBaseProxy {
public:
    typedef UAssetObjectProperty Type;

    AssetObjectPropertyProxy(const UAssetObjectProperty* Property)
        : ObjectPropertyBaseProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Container, sizeof(FAssetPtr), false, "TAssetPtr<class " + MakeValidName(ObjectProxy(GetPropertyClass()).GetNameCPP()) + ">");
    }

    DECLARE_STATIC_CLASS(AssetObjectProperty);
};

class AssetClassPropertyProxy : public AssetObjectPropertyProxy {
public:
    typedef UAssetClassProperty Type;

    AssetClassPropertyProxy(const UAssetClassProperty* Property)
        : AssetObjectPropertyProxy(Property)
    {
    }

    inline UClass* GetMetaClass() const
    {
        return static_cast<const UAssetClassProperty*>(GetReference())->MetaClass;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint8_t), false, "");
    }

    DECLARE_STATIC_CLASS(AssetClassProperty);
};

class NamePropertyProxy : public PropertyProxy {
public:
    typedef UNameProperty Type;

    NamePropertyProxy(const UNameProperty* Property)
        : PropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::PredefinedStruct, sizeof(FName), true, "struct FName");
    }

    DECLARE_STATIC_CLASS(NameProperty);
};

class StructPropertyProxy : public PropertyProxy {
public:
    typedef UStructProperty Type;

    StructPropertyProxy(const UStructProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UScriptStruct* GetStruct() const
    {
        return static_cast<const UStructProperty*>(GetReference())->Struct;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::CustomStruct, GetElementSize(), true, "struct " + MakeUniqueCppName(GetStruct()));
    }

    DECLARE_STATIC_CLASS(StructProperty);
};

class StrPropertyProxy : public PropertyProxy {
public:
    typedef UStrProperty Type;

    StrPropertyProxy(const UStrProperty* Property)
        : PropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::PredefinedStruct, sizeof(FString), true, "struct FString");
    }

    DECLARE_STATIC_CLASS(StrProperty);
};

class TextPropertyProxy : public PropertyProxy {
public:
    typedef UTextProperty Type;

    TextPropertyProxy(const UTextProperty* Property)
        : PropertyProxy(Property)
    {
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::PredefinedStruct, sizeof(FText), true, "struct FText");
    }

    DECLARE_STATIC_CLASS(TextProperty);
};

class ArrayPropertyProxy : public PropertyProxy {
public:
    typedef UArrayProperty Type;

    ArrayPropertyProxy(const UArrayProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UProperty* GetInner() const
    {
        return static_cast<const UArrayProperty*>(GetReference())->Inner;
    }

    PropertyInfo GetInfo() const
    {
        PropertyProxy Inner(GetInner());

        if (Inner.IsValid()) {
            PropertyInfo InnerInfo = Inner.GetInfo();
            if (Inner.GetInfo().Type != PropertyType::Unknown) {
                return PropertyInfo::Create(PropertyType::Container, sizeof(TArray<void*>), false, "TArray<" + InnerInfo.CppType + ">");
            }
        }

        return { PropertyType::Unknown };
    }

    DECLARE_STATIC_CLASS(ArrayProperty);
};

class MapPropertyProxy : public PropertyProxy {
public:
    typedef UMapProperty Type;

    MapPropertyProxy(const UMapProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UProperty* GetKeyProperty() const
    {
        return static_cast<const UMapProperty*>(GetReference())->KeyProp;
    }

    inline UProperty* GetValueProperty() const
    {
        return static_cast<const UMapProperty*>(GetReference())->ValueProp;
    }

    PropertyInfo GetInfo() const
    {
        PropertyProxy KeyProp(GetKeyProperty());
        PropertyProxy ValueProp(GetValueProperty());

        if (KeyProp.IsValid() && ValueProp.IsValid()) {

            PropertyInfo KeyInfo = KeyProp.GetInfo();
            PropertyInfo ValueInfo = ValueProp.GetInfo();
            if (KeyInfo.Type != PropertyType::Unknown && ValueInfo.Type != PropertyType::Unknown) {
                return PropertyInfo::Create(PropertyType::Container, sizeof(TMap<void*,void*>), false, "TMap<" + KeyInfo.CppType + ", " + ValueInfo.CppType + ">");
            }
        }

        return { PropertyType::Unknown };
    }

    DECLARE_STATIC_CLASS(MapProperty);
};

class DelegatePropertyProxy : public PropertyProxy {
public:
    typedef UDelegateProperty Type;

    DelegatePropertyProxy(const UDelegateProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UFunction* GetSignatureFunction() const
    {
        return static_cast<const UDelegateProperty*>(GetReference())->SignatureFunction;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::PredefinedStruct, sizeof(FScriptDelegate), true, "struct FScriptDelegate");
    }

    DECLARE_STATIC_CLASS(DelegateProperty);
};

class MulticastDelegatePropertyProxy : public PropertyProxy {
public:
    typedef UMulticastDelegateProperty Type;

    MulticastDelegatePropertyProxy(const UMulticastDelegateProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UFunction* GetSignatureFunction() const
    {
        return static_cast<const UMulticastDelegateProperty*>(GetReference())->SignatureFunction;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::PredefinedStruct, sizeof(FScriptMulticastDelegate), true, "struct FScriptMulticastDelegate");
    }

    DECLARE_STATIC_CLASS(MulticastDelegateProperty);
};

class EnumPropertyProxy : public PropertyProxy {
public:
    typedef UEnumProperty Type;

    EnumPropertyProxy(const UEnumProperty* Property)
        : PropertyProxy(Property)
    {
    }

    inline UNumericProperty* GetUnderlyingProperty() const
    {
        return static_cast<const UEnumProperty*>(GetReference())->UnderlyingProp;
    }

    inline UEnum* GetEnum() const
    {
        return static_cast<const UEnumProperty*>(GetReference())->Enum;
    }

    PropertyInfo GetInfo() const
    {
        return PropertyInfo::Create(PropertyType::Primitive, sizeof(uint8_t), false, MakeUniqueCppName(GetEnum()));
    }

    DECLARE_STATIC_CLASS(EnumProperty);
};



