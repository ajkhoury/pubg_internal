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


std::string StringifyFlags(const PropertyFlags Flags)
{
    std::vector<const char*> buffer;

    if (Flags & PropertyFlags::Edit) { buffer.push_back("Edit"); }
    if (Flags & PropertyFlags::ConstParm) { buffer.push_back("ConstParm"); }
    if (Flags & PropertyFlags::BlueprintVisible) { buffer.push_back("BlueprintVisible"); }
    if (Flags & PropertyFlags::ExportObject) { buffer.push_back("ExportObject"); }
    if (Flags & PropertyFlags::BlueprintReadOnly) { buffer.push_back("BlueprintReadOnly"); }
    if (Flags & PropertyFlags::Net) { buffer.push_back("Net"); }
    if (Flags & PropertyFlags::EditFixedSize) { buffer.push_back("EditFixedSize"); }
    if (Flags & PropertyFlags::Parm) { buffer.push_back("Parm"); }
    if (Flags & PropertyFlags::OutParm) { buffer.push_back("OutParm"); }
    if (Flags & PropertyFlags::ZeroConstructor) { buffer.push_back("ZeroConstructor"); }
    if (Flags & PropertyFlags::ReturnParm) { buffer.push_back("ReturnParm"); }
    if (Flags & PropertyFlags::DisableEditOnTemplate) { buffer.push_back("DisableEditOnTemplate"); }
    if (Flags & PropertyFlags::Transient) { buffer.push_back("Transient"); }
    if (Flags & PropertyFlags::Config) { buffer.push_back("Config"); }
    if (Flags & PropertyFlags::DisableEditOnInstance) { buffer.push_back("DisableEditOnInstance"); }
    if (Flags & PropertyFlags::EditConst) { buffer.push_back("EditConst"); }
    if (Flags & PropertyFlags::GlobalConfig) { buffer.push_back("GlobalConfig"); }
    if (Flags & PropertyFlags::InstancedReference) { buffer.push_back("InstancedReference"); }
    if (Flags & PropertyFlags::DuplicateTransient) { buffer.push_back("DuplicateTransient"); }
    if (Flags & PropertyFlags::SubobjectReference) { buffer.push_back("SubobjectReference"); }
    if (Flags & PropertyFlags::SaveGame) { buffer.push_back("SaveGame"); }
    if (Flags & PropertyFlags::NoClear) { buffer.push_back("NoClear"); }
    if (Flags & PropertyFlags::ReferenceParm) { buffer.push_back("ReferenceParm"); }
    if (Flags & PropertyFlags::BlueprintAssignable) { buffer.push_back("BlueprintAssignable"); }
    if (Flags & PropertyFlags::Deprecated) { buffer.push_back("Deprecated"); }
    if (Flags & PropertyFlags::IsPlainOldData) { buffer.push_back("IsPlainOldData"); }
    if (Flags & PropertyFlags::RepSkip) { buffer.push_back("RepSkip"); }
    if (Flags & PropertyFlags::RepNotify) { buffer.push_back("RepNotify"); }
    if (Flags & PropertyFlags::Interp) { buffer.push_back("Interp"); }
    if (Flags & PropertyFlags::NonTransactional) { buffer.push_back("NonTransactional"); }
    if (Flags & PropertyFlags::EditorOnly) { buffer.push_back("EditorOnly"); }
    if (Flags & PropertyFlags::NoDestructor) { buffer.push_back("NoDestructor"); }
    if (Flags & PropertyFlags::AutoWeak) { buffer.push_back("AutoWeak"); }
    if (Flags & PropertyFlags::ContainsInstancedReference) { buffer.push_back("ContainsInstancedReference"); }
    if (Flags & PropertyFlags::AssetRegistrySearchable) { buffer.push_back("AssetRegistrySearchable"); }
    if (Flags & PropertyFlags::SimpleDisplay) { buffer.push_back("SimpleDisplay"); }
    if (Flags & PropertyFlags::AdvancedDisplay) { buffer.push_back("AdvancedDisplay"); }
    if (Flags & PropertyFlags::Protected) { buffer.push_back("Protected"); }
    if (Flags & PropertyFlags::BlueprintCallable) { buffer.push_back("BlueprintCallable"); }
    if (Flags & PropertyFlags::BlueprintAuthorityOnly) { buffer.push_back("BlueprintAuthorityOnly"); }
    if (Flags & PropertyFlags::TextExportTransient) { buffer.push_back("TextExportTransient"); }
    if (Flags & PropertyFlags::NonPIEDuplicateTransient) { buffer.push_back("NonPIEDuplicateTransient"); }
    if (Flags & PropertyFlags::ExposeOnSpawn) { buffer.push_back("ExposeOnSpawn"); }
    if (Flags & PropertyFlags::PersistentInstance) { buffer.push_back("PersistentInstance"); }
    if (Flags & PropertyFlags::UObjectWrapper) { buffer.push_back("UObjectWrapper"); }
    if (Flags & PropertyFlags::HasGetValueTypeHash) { buffer.push_back("HasGetValueTypeHash"); }
    if (Flags & PropertyFlags::NativeAccessSpecifierPublic) { buffer.push_back("NativeAccessSpecifierPublic"); }
    if (Flags & PropertyFlags::NativeAccessSpecifierProtected) { buffer.push_back("NativeAccessSpecifierProtected"); }
    if (Flags & PropertyFlags::NativeAccessSpecifierPrivate) { buffer.push_back("NativeAccessSpecifierPrivate"); }

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

PropertyInfo PropertyProxy::GetInfo() const
{
    if (IsValid()) {
        if (IsA<BytePropertyProxy>()) {
            return Cast<BytePropertyProxy>().GetInfo();
        }
        if (IsA<UInt16PropertyProxy>()) {
            return Cast<UInt16PropertyProxy>().GetInfo();
        }
        if (IsA<UInt32PropertyProxy>()) {
            return Cast<UInt32PropertyProxy>().GetInfo();
        }
        if (IsA<UInt64PropertyProxy>()) {
            return Cast<UInt64PropertyProxy>().GetInfo();
        }
        if (IsA<Int8PropertyProxy>()) {
            return Cast<Int8PropertyProxy>().GetInfo();
        }
        if (IsA<Int16PropertyProxy>()) {
            return Cast<Int16PropertyProxy>().GetInfo();
        }
        if (IsA<IntPropertyProxy>()) {
            return Cast<IntPropertyProxy>().GetInfo();
        }
        if (IsA<Int64PropertyProxy>()) {
            return Cast<Int64PropertyProxy>().GetInfo();
        }
        if (IsA<FloatPropertyProxy>()) {
            return Cast<FloatPropertyProxy>().GetInfo();
        }
        if (IsA<DoublePropertyProxy>()) {
            return Cast<DoublePropertyProxy>().GetInfo();
        }
        if (IsA<BoolPropertyProxy>()) {
            return Cast<BoolPropertyProxy>().GetInfo();
        }
        if (IsA<ObjectPropertyProxy>()) {
            return Cast<ObjectPropertyProxy>().GetInfo();
        }
        if (IsA<ObjectPropertyBaseProxy>()) {
            return Cast<ObjectPropertyBaseProxy>().GetInfo();
        }
        if (IsA<ClassPropertyProxy>()) {
            return Cast<ClassPropertyProxy>().GetInfo();
        }
        if (IsA<InterfacePropertyProxy>()) {
            return Cast<InterfacePropertyProxy>().GetInfo();
        }
        if (IsA<WeakObjectPropertyProxy>()) {
            return Cast<WeakObjectPropertyProxy>().GetInfo();
        }
        if (IsA<LazyObjectPropertyProxy>()) {
            return Cast<LazyObjectPropertyProxy>().GetInfo();
        }
        if (IsA<AssetObjectPropertyProxy>()) {
            return Cast<AssetObjectPropertyProxy>().GetInfo();
        }
        if (IsA<AssetClassPropertyProxy>()) {
            return Cast<AssetClassPropertyProxy>().GetInfo();
        }
        if (IsA<NamePropertyProxy>()) {
            return Cast<NamePropertyProxy>().GetInfo();
        }
        if (IsA<StructPropertyProxy>()) {
            return Cast<StructPropertyProxy>().GetInfo();
        }
        if (IsA<StrPropertyProxy>()) {
            return Cast<StrPropertyProxy>().GetInfo();
        }
        if (IsA<TextPropertyProxy>()) {
            return Cast<TextPropertyProxy>().GetInfo();
        }
        if (IsA<ArrayPropertyProxy>()) {
            return Cast<ArrayPropertyProxy>().GetInfo();
        }
        if (IsA<MapPropertyProxy>()) {
            return Cast<MapPropertyProxy>().GetInfo();
        }
        if (IsA<DelegatePropertyProxy>()) {
            return Cast<DelegatePropertyProxy>().GetInfo();
        }
        if (IsA<MulticastDelegatePropertyProxy>()) {
            return Cast<MulticastDelegatePropertyProxy>().GetInfo();
        }
        if (IsA<EnumPropertyProxy>()) {
            return Cast<EnumPropertyProxy>().GetInfo();
        }
    }
    return { PropertyType::Unknown };
}
