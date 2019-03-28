#include "UnrealTypes.h"
#include "Objects.h"
#include "Format.h"

DEFINE_STATIC_CLASS(Property);
DEFINE_STATIC_CLASS(NumericProperty);
DEFINE_STATIC_CLASS(ByteProperty);
DEFINE_STATIC_CLASS(UInt16Property);
DEFINE_STATIC_CLASS(UInt32Property);
DEFINE_STATIC_CLASS(UInt64Property);
DEFINE_STATIC_CLASS(Int8Property);
DEFINE_STATIC_CLASS(Int16Property);
DEFINE_STATIC_CLASS(IntProperty);
DEFINE_STATIC_CLASS(Int64Property);
DEFINE_STATIC_CLASS(FloatProperty);
DEFINE_STATIC_CLASS(DoubleProperty);
DEFINE_STATIC_CLASS(BoolProperty);
DEFINE_STATIC_CLASS(ObjectPropertyBase);
DEFINE_STATIC_CLASS(ObjectProperty);
DEFINE_STATIC_CLASS(ClassProperty);
DEFINE_STATIC_CLASS(InterfaceProperty);
DEFINE_STATIC_CLASS(WeakObjectProperty);
DEFINE_STATIC_CLASS(LazyObjectProperty);
DEFINE_STATIC_CLASS(AssetObjectProperty);
DEFINE_STATIC_CLASS(AssetClassProperty);
DEFINE_STATIC_CLASS(NameProperty);
DEFINE_STATIC_CLASS(StructProperty);
DEFINE_STATIC_CLASS(StrProperty);
DEFINE_STATIC_CLASS(TextProperty);
DEFINE_STATIC_CLASS(ArrayProperty);
DEFINE_STATIC_CLASS(MapProperty);
DEFINE_STATIC_CLASS(DelegateProperty);
DEFINE_STATIC_CLASS(MulticastDelegateProperty);
DEFINE_STATIC_CLASS(EnumProperty);

UProperty::Info UProperty::GetInfo() const
{
    if (this != nullptr) {

        //LOG_INFO("IsA<UByteProperty>");
        if (IsA<UByteProperty>()) {
            //LOG_INFO("UByteProperty");
            return Cast<UByteProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UUInt16Property>");
        if (IsA<UUInt16Property>()) {
            //LOG_INFO("UUInt16Property");
            return Cast<UUInt16Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UUInt32Property>");
        if (IsA<UUInt32Property>()) {
            //LOG_INFO("UUInt32Property");
            return Cast<UUInt32Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UUInt64Property>");
        if (IsA<UUInt64Property>()) {
            //LOG_INFO("UUInt64Property");
            return Cast<UUInt64Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UInt8Property>");
        if (IsA<UInt8Property>()) {
            //LOG_INFO("UInt8Property");
            return Cast<UInt8Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UInt16Property>");
        if (IsA<UInt16Property>()) {
            //LOG_INFO("UInt16Property");
            return Cast<UInt16Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UIntProperty>");
        if (IsA<UIntProperty>()) {
            //LOG_INFO("UIntProperty");
            return Cast<UIntProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UInt64Property>");
        if (IsA<UInt64Property>()) {
            //LOG_INFO("UInt64Property");
            return Cast<UInt64Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UFloatProperty>");
        if (IsA<UFloatProperty>()) {
            //LOG_INFO("UFloatProperty");
            return Cast<UFloatProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UDoubleProperty>");
        if (IsA<UDoubleProperty>()) {
            //LOG_INFO("UDoubleProperty");
            return Cast<UDoubleProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UBoolProperty>");
        if (IsA<UBoolProperty>()) {
            //LOG_INFO("UBoolProperty");
            return Cast<UBoolProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UObjectProperty>");
        if (IsA<UObjectProperty>()) {
            //LOG_INFO("UObjectProperty");
            return Cast<UObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UObjectPropertyBase>");
        if (IsA<UObjectPropertyBase>()) {
            //LOG_INFO("UObjectPropertyBase");
            return Cast<UObjectPropertyBase>()->GetInfo();
        }
        //LOG_INFO("IsA<UClassProperty>");
        if (IsA<UClassProperty>()) {
            //LOG_INFO("UClassProperty");
            return Cast<UClassProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UInterfaceProperty>");
        if (IsA<UInterfaceProperty>()) {
            //LOG_INFO("UInterfaceProperty");
            return Cast<UInterfaceProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UWeakObjectProperty>");
        if (IsA<UWeakObjectProperty>()) {
            //LOG_INFO("UWeakObjectProperty");
            return Cast<UWeakObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<ULazyObjectProperty>");
        if (IsA<ULazyObjectProperty>()) {
            //LOG_INFO("ULazyObjectProperty");
            return Cast<ULazyObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UAssetObjectProperty>");
        if (IsA<UAssetObjectProperty>()) {
            //LOG_INFO("UAssetObjectProperty");
            return Cast<UAssetObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UAssetClassProperty>");
        if (IsA<UAssetClassProperty>()) {
            //LOG_INFO("UAssetClassProperty");
            return Cast<UAssetClassProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UNameProperty>");
        if (IsA<UNameProperty>()) {
            //LOG_INFO("UNameProperty");
            return Cast<UNameProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UStructProperty>");
        if (IsA<UStructProperty>()) {
            //LOG_INFO("UStructProperty");
            return Cast<UStructProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UStrProperty>");
        if (IsA<UStrProperty>()) {
            //LOG_INFO("UStrProperty");
            return Cast<UStrProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UTextProperty>");
        if (IsA<UTextProperty>()) {
            //LOG_INFO("UTextProperty");
            return Cast<UTextProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UArrayProperty>");
        if (IsA<UArrayProperty>()) {
            //LOG_INFO("UArrayProperty");
            return Cast<UArrayProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UMapProperty>");
        if (IsA<UMapProperty>()) {
            //LOG_INFO("UMapProperty");
            return Cast<UMapProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UDelegateProperty>");
        if (IsA<UDelegateProperty>()) {
            //LOG_INFO("UDelegateProperty");
            return Cast<UDelegateProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UMulticastDelegateProperty>");
        if (IsA<UMulticastDelegateProperty>()) {
            //LOG_INFO("UMulticastDelegateProperty");
            return Cast<UMulticastDelegateProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UEnumProperty>");
        if (IsA<UEnumProperty>()) {
            //LOG_INFO("UEnumProperty");
            return Cast<UEnumProperty>()->GetInfo();
        }
    }
    return { PropertyType::Unknown };
}

UProperty::Info UByteProperty::GetInfo() const
{
    if (IsEnum()) {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "TEnumAsByte<" + fmt::MakeUniqueEnumCppName(GetEnum()) + ">");
    }
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "unsigned char");
}

UProperty::Info UUInt16Property::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint16_t), false, "uint16_t");
}

UProperty::Info UUInt32Property::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint32_t), false, "uint32_t");
}

UProperty::Info UUInt64Property::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint64_t), false, "uint64_t");
}

UProperty::Info UInt8Property::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(int8_t), false, "int8_t");
}

UProperty::Info UInt16Property::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(int16_t), false, "int16_t");
}

UProperty::Info UIntProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(int), false, "int");
}

UProperty::Info UInt64Property::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(int64_t), false, "int64_t");
}

UProperty::Info UFloatProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(float), false, "float");
}

UProperty::Info UDoubleProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(double), false, "double");
}

UProperty::Info UBoolProperty::GetInfo() const
{
    if (IsNativeBool()) {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(bool), false, "bool");
    }
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(unsigned char), false, "unsigned char");
}

inline int GetBitPosition(uint8_t value)
{
    int i4 = !(value & 0xf) << 2;
    value >>= i4;
    int i2 = !(value & 0x3) << 1;
    value >>= i2;
    int i1 = !(value & 0x1);
    int i0 = (value >> i1) & 1 ? 0 : -8;
    return i4 + i2 + i1 + i0;
}

UBoolProperty::MissingBitsCount UBoolProperty::GetMissingBitsCount(const UBoolProperty* Other) const
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

UProperty::Info UObjectPropertyBase::GetInfo() const
{
    UClass* Class = GetPropertyClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + fmt::MakeValidName(Class->GetNameCPP()) + '*');
    }
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, _XORSTR_("/*WARNING PropertyClass NULL*/ class !!INVALID!!"));
}

UProperty::Info UObjectProperty::GetInfo() const
{
    UClass* Class = GetPropertyClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + fmt::MakeValidName(Class->GetNameCPP()) + '*');
    }
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, _XORSTR_("/*WARNING PropertyClass NULL*/ class !!INVALID!!"));
}

UProperty::Info UClassProperty::GetInfo() const
{
    UClass* Class = GetMetaClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, "class " + fmt::MakeValidName(Class->GetNameCPP()) + '*');
    }
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(void*), false, _XORSTR_("/*WARNING MetaClass NULL*/ class !!INVALID!!"));
}

UProperty::Info UInterfaceProperty::GetInfo() const
{
    UClass* Class = GetInterfaceClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptInterface), true, "TScriptInterface<class " + fmt::MakeValidName(Class->GetNameCPP()) + '>');
    }
    return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptInterface), true, _XORSTR_("/*WARNING InterfaceClass NULL*/ TScriptInterface<class !!INVALID!!>"));
}

UProperty::Info UWeakObjectProperty::GetInfo() const
{
    UClass* Class = GetPropertyClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::Container, sizeof(FWeakObjectPtr), false, "TWeakObjectPtr<class " + fmt::MakeValidName(Class->GetNameCPP()) + '>');
    }
    return UProperty::Info::Create(PropertyType::Container, sizeof(FWeakObjectPtr), true, _XORSTR_("/*WARNING PropertyClass NULL*/ TWeakObjectPtr<class !!INVALID!!>"));
}

UProperty::Info ULazyObjectProperty::GetInfo() const
{
    UClass* Class = GetPropertyClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::Container, sizeof(FLazyObjectPtr), false, "TLazyObjectPtr<class " + fmt::MakeValidName(Class->GetNameCPP()) + '>');
    }
    return UProperty::Info::Create(PropertyType::Container, sizeof(FLazyObjectPtr), true, _XORSTR_("/*WARNING PropertyClass NULL*/ TLazyObjectPtr<class !!INVALID!!>"));
}

UProperty::Info UAssetObjectProperty::GetInfo() const
{
    UClass* Class = GetPropertyClass();
    if (Class) {
        return UProperty::Info::Create(PropertyType::Container, sizeof(FAssetPtr), false, "TAssetPtr<class " + fmt::MakeValidName(Class->GetNameCPP()) + '>');
    }
    return UProperty::Info::Create(PropertyType::Container, sizeof(FAssetPtr), true, _XORSTR_("/*WARNING PropertyClass NULL*/ TAssetPtr<class !!INVALID!!>"));
}

UProperty::Info UAssetClassProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, "");
}

UProperty::Info UNameProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FName), true, "struct FName");
}

UProperty::Info UStructProperty::GetInfo() const
{
    UScriptStruct* ScriptStruct = GetStruct();
    if (ScriptStruct) {
        return UProperty::Info::Create(PropertyType::CustomStruct, GetElementSize(), true, "struct " + fmt::MakeUniqueStructCppName(ScriptStruct));
    }
    return UProperty::Info::Create(PropertyType::CustomStruct, GetElementSize(), true, _XORSTR_("/*WARNING Struct NULL*/ struct !!INVALID!!"));
}

UProperty::Info UStrProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FString), true, "FString");
}

UProperty::Info UTextProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FText), true, "FText");
}

UProperty::Info UArrayProperty::GetInfo() const
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

UProperty::Info UMapProperty::GetInfo() const
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

UProperty::Info UDelegateProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FScriptDelegate), true, _XORSTR_("FScriptDelegate"));
}

UProperty::Info UMulticastDelegateProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::PredefinedStruct, sizeof(FMulticastScriptDelegate), true, _XORSTR_("FMulticastScriptDelegate"));
}

UProperty::Info UEnumProperty::GetInfo() const
{
    return UProperty::Info::Create(PropertyType::Primitive, sizeof(uint8_t), false, fmt::MakeUniqueEnumCppName(GetEnum()));
}

