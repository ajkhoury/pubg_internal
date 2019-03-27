#include "Properties.h"

#include <sstream>
#include <iterator>

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


std::string StringifyPropertyFlags(const uint64_t Flags)
{
    std::vector<const char*> buffer;

    if (Flags & CPF_Edit) { buffer.push_back("Edit"); }
    if (Flags & CPF_ConstParm) { buffer.push_back("ConstParm"); }
    if (Flags & CPF_BlueprintVisible) { buffer.push_back("BlueprintVisible"); }
    if (Flags & CPF_ExportObject) { buffer.push_back("ExportObject"); }
    if (Flags & CPF_BlueprintReadOnly) { buffer.push_back("BlueprintReadOnly"); }
    if (Flags & CPF_Net) { buffer.push_back("Net"); }
    if (Flags & CPF_EditFixedSize) { buffer.push_back("EditFixedSize"); }
    if (Flags & CPF_Parm) { buffer.push_back("Parm"); }
    if (Flags & CPF_OutParm) { buffer.push_back("OutParm"); }
    if (Flags & CPF_ZeroConstructor) { buffer.push_back("ZeroConstructor"); }
    if (Flags & CPF_ReturnParm) { buffer.push_back("ReturnParm"); }
    if (Flags & CPF_DisableEditOnTemplate) { buffer.push_back("DisableEditOnTemplate"); }
    if (Flags & CPF_Transient) { buffer.push_back("Transient"); }
    if (Flags & CPF_Config) { buffer.push_back("Config"); }
    if (Flags & CPF_DisableEditOnInstance) { buffer.push_back("DisableEditOnInstance"); }
    if (Flags & CPF_EditConst) { buffer.push_back("EditConst"); }
    if (Flags & CPF_GlobalConfig) { buffer.push_back("GlobalConfig"); }
    if (Flags & CPF_InstancedReference) { buffer.push_back("InstancedReference"); }
    if (Flags & CPF_DuplicateTransient) { buffer.push_back("DuplicateTransient"); }
    if (Flags & CPF_SubobjectReference) { buffer.push_back("SubobjectReference"); }
    if (Flags & CPF_SaveGame) { buffer.push_back("SaveGame"); }
    if (Flags & CPF_NoClear) { buffer.push_back("NoClear"); }
    if (Flags & CPF_ReferenceParm) { buffer.push_back("ReferenceParm"); }
    if (Flags & CPF_BlueprintAssignable) { buffer.push_back("BlueprintAssignable"); }
    if (Flags & CPF_Deprecated) { buffer.push_back("Deprecated"); }
    if (Flags & CPF_IsPlainOldData) { buffer.push_back("IsPlainOldData"); }
    if (Flags & CPF_RepSkip) { buffer.push_back("RepSkip"); }
    if (Flags & CPF_RepNotify) { buffer.push_back("RepNotify"); }
    if (Flags & CPF_Interp) { buffer.push_back("Interp"); }
    if (Flags & CPF_NonTransactional) { buffer.push_back("NonTransactional"); }
    if (Flags & CPF_EditorOnly) { buffer.push_back("EditorOnly"); }
    if (Flags & CPF_NoDestructor) { buffer.push_back("NoDestructor"); }
    if (Flags & CPF_AutoWeak) { buffer.push_back("AutoWeak"); }
    if (Flags & CPF_ContainsInstancedReference) { buffer.push_back("ContainsInstancedReference"); }
    if (Flags & CPF_AssetRegistrySearchable) { buffer.push_back("AssetRegistrySearchable"); }
    if (Flags & CPF_SimpleDisplay) { buffer.push_back("SimpleDisplay"); }
    if (Flags & CPF_AdvancedDisplay) { buffer.push_back("AdvancedDisplay"); }
    if (Flags & CPF_Protected) { buffer.push_back("Protected"); }
    if (Flags & CPF_BlueprintCallable) { buffer.push_back("BlueprintCallable"); }
    if (Flags & CPF_BlueprintAuthorityOnly) { buffer.push_back("BlueprintAuthorityOnly"); }
    if (Flags & CPF_TextExportTransient) { buffer.push_back("TextExportTransient"); }
    if (Flags & CPF_NonPIEDuplicateTransient) { buffer.push_back("NonPIEDuplicateTransient"); }
    if (Flags & CPF_ExposeOnSpawn) { buffer.push_back("ExposeOnSpawn"); }
    if (Flags & CPF_PersistentInstance) { buffer.push_back("PersistentInstance"); }
    if (Flags & CPF_UObjectWrapper) { buffer.push_back("UObjectWrapper"); }
    if (Flags & CPF_HasGetValueTypeHash) { buffer.push_back("HasGetValueTypeHash"); }
    if (Flags & CPF_NativeAccessSpecifierPublic) { buffer.push_back("NativeAccessSpecifierPublic"); }
    if (Flags & CPF_NativeAccessSpecifierProtected) { buffer.push_back("NativeAccessSpecifierProtected"); }
    if (Flags & CPF_NativeAccessSpecifierPrivate) { buffer.push_back("NativeAccessSpecifierPrivate"); }

    switch (buffer.size()) {
    case 0:
        return std::string();
    case 1:
        return std::string(buffer[0]);
    default:
        std::ostringstream os;
        std::copy(buffer.begin(), buffer.end() - 1, std::ostream_iterator<const char*>(os, ", "));
        os << *buffer.rbegin();
        return os.str();
    }
}

UProperty::Info UProperty::GetInfo() const
{
    if (this != nullptr) {

        //LOG_INFO("IsA<BytePropertyProxy>");
        if (IsA<UByteProperty>()) {
            //LOG_INFO("BytePropertyProxy");
            return Cast<UByteProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<UInt16PropertyProxy>");
        if (IsA<UUInt16Property>()) {
            //LOG_INFO("UInt16PropertyProxy");
            return Cast<UUInt16Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UInt32PropertyProxy>");
        if (IsA<UUInt32Property>()) {
            //LOG_INFO("UInt32PropertyProxy");
            return Cast<UUInt32Property>()->GetInfo();
        }
        //LOG_INFO("IsA<UInt64PropertyProxy>");
        if (IsA<UUInt64Property>()) {
            //LOG_INFO("UInt64PropertyProxy");
            return Cast<UUInt64Property>()->GetInfo();
        }
        //LOG_INFO("IsA<Int8PropertyProxy>");
        if (IsA<UInt8Property>()) {
            //LOG_INFO("Int8PropertyProxy");
            return Cast<UInt8Property>()->GetInfo();
        }
        //LOG_INFO("IsA<Int16PropertyProxy>");
        if (IsA<UInt16Property>()) {
            //LOG_INFO("Int16PropertyProxy");
            return Cast<UInt16Property>()->GetInfo();
        }
        //LOG_INFO("IsA<IntPropertyProxy>");
        if (IsA<UIntProperty>()) {
            //LOG_INFO("IntPropertyProxy");
            return Cast<UIntProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<Int64PropertyProxy>");
        if (IsA<UInt64Property>()) {
            //LOG_INFO("Int64PropertyProxy");
            return Cast<UInt64Property>()->GetInfo();
        }
        //LOG_INFO("IsA<FloatPropertyProxy>");
        if (IsA<UFloatProperty>()) {
            //LOG_INFO("FloatPropertyProxy");
            return Cast<UFloatProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<DoublePropertyProxy>");
        if (IsA<UDoubleProperty>()) {
            //LOG_INFO("DoublePropertyProxy");
            return Cast<UDoubleProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<BoolPropertyProxy>");
        if (IsA<UBoolProperty>()) {
            //LOG_INFO("BoolPropertyProxy");
            return Cast<UBoolProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<ObjectPropertyProxy>");
        if (IsA<UObjectProperty>()) {
            //LOG_INFO("ObjectPropertyProxy");
            return Cast<UObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<ObjectPropertyBaseProxy>");
        if (IsA<UObjectPropertyBase>()) {
            //LOG_INFO("ObjectPropertyBaseProxy");
            return Cast<UObjectPropertyBase>()->GetInfo();
        }
        //LOG_INFO("IsA<ClassPropertyProxy>");
        if (IsA<UClassProperty>()) {
            //LOG_INFO("ClassPropertyProxy");
            return Cast<UClassProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<InterfacePropertyProxy>");
        if (IsA<UInterfaceProperty>()) {
            //LOG_INFO("InterfacePropertyProxy");
            return Cast<UInterfaceProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<WeakObjectPropertyProxy>");
        if (IsA<UWeakObjectProperty>()) {
            //LOG_INFO("WeakObjectPropertyProxy");
            return Cast<UWeakObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<LazyObjectPropertyProxy>");
        if (IsA<ULazyObjectProperty>()) {
            //LOG_INFO("LazyObjectPropertyProxy");
            return Cast<ULazyObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<AssetObjectPropertyProxy>");
        if (IsA<UAssetObjectProperty>()) {
            //LOG_INFO("AssetObjectPropertyProxy");
            return Cast<UAssetObjectProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<AssetClassPropertyProxy>");
        if (IsA<UAssetClassProperty>()) {
            //LOG_INFO("AssetClassPropertyProxy");
            return Cast<UAssetClassProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<NamePropertyProxy>");
        if (IsA<UNameProperty>()) {
            //LOG_INFO("NamePropertyProxy");
            return Cast<UNameProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<StructPropertyProxy>");
        if (IsA<UStructProperty>()) {
            //LOG_INFO("StructPropertyProxy");
            return Cast<UStructProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<StrPropertyProxy>");
        if (IsA<UStrProperty>()) {
            //LOG_INFO("StrPropertyProxy");
            return Cast<UStrProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<TextPropertyProxy>");
        if (IsA<UTextProperty>()) {
            //LOG_INFO("TextPropertyProxy");
            return Cast<UTextProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<ArrayPropertyProxy>");
        if (IsA<UArrayProperty>()) {
            //LOG_INFO("ArrayPropertyProxy");
            return Cast<UArrayProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<MapPropertyProxy>");
        if (IsA<UMapProperty>()) {
            //LOG_INFO("MapPropertyProxy");
            return Cast<UMapProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<DelegatePropertyProxy>");
        if (IsA<UDelegateProperty>()) {
            //LOG_INFO("DelegatePropertyProxy");
            return Cast<UDelegateProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<MulticastDelegatePropertyProxy>");
        if (IsA<UMulticastDelegateProperty>()) {
            //LOG_INFO("MulticastDelegatePropertyProxy");
            return Cast<UMulticastDelegateProperty>()->GetInfo();
        }
        //LOG_INFO("IsA<EnumPropertyProxy>");
        if (IsA<UEnumProperty>()) {
            //LOG_INFO("EnumPropertyProxy");
            return Cast<UEnumProperty>()->GetInfo();
        }
    }
    return { PropertyType::Unknown };
}
