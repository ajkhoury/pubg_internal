#include "Format.h"
#include "Objects.h"
#include "UnrealTypes.h"

#include <sstream>
#include <iterator>


namespace fmt {

std::string MakeValidName(const std::string&& Name)
{
    std::string ValidString(Name);

    for (size_t i = 0; i < Name.length(); ++i) {
        if (ValidString[i] == ' ' ||
            ValidString[i] == '?' ||
            ValidString[i] == '+' ||
            ValidString[i] == '-' ||
            ValidString[i] == ':' ||
            ValidString[i] == '/' ||
            ValidString[i] == '^' ||
            ValidString[i] == '(' ||
            ValidString[i] == ')' ||
            ValidString[i] == '[' ||
            ValidString[i] == ']' ||
            ValidString[i] == '<' ||
            ValidString[i] == '>' ||
            ValidString[i] == '&' ||
            ValidString[i] == '.' ||
            ValidString[i] == '#' ||
            ValidString[i] == '\'' ||
            ValidString[i] == '"' ||
            ValidString[i] == '%') {
            ValidString[i] = '_';
        }
    }

    if (!ValidString.empty()) {
        if (isdigit(ValidString[0])) {
            ValidString = '_' + ValidString;
        }
    }

    return ValidString;
}

std::string MakeUniqueEnumCppName(UEnum const* e)
{
    std::string NameString;

    if (ObjectsProxy().CountObjects<UEnum>(e->GetName()) > 1) {
        const UObject* OuterObj = e->GetOuter();
        if (OuterObj) {
            NameString = MakeValidName(OuterObj->GetName()) + '_';
        }
    }
    NameString += MakeValidName(e->GetName());

    if (!NameString.empty() && NameString[0] != 'E') {
        NameString = 'E' + NameString;
    }
    return NameString;
}

std::string MakeUniqueStructCppName(class UStruct const* s)
{
    std::string NameString;
    if (ObjectsProxy().CountObjects<UStruct>(s->GetName()) > 1) {
        const UObject* OuterObj = s->GetOuter();
        if (OuterObj != nullptr) {
            NameString = MakeValidName(OuterObj->GetNameCPP()) + '_';
        }
    }
    return NameString + MakeValidName(s->GetNameCPP());
}


std::string StringifyFunctionFlags(const uint32_t Flags)
{
    std::vector<const char*> buffer;

    if (Flags & uint32_t(FunctionFlags::Final)) { buffer.push_back("Final"); }
    if (Flags & uint32_t(FunctionFlags::RequiredAPI)) { buffer.push_back("RequiredAPI"); }
    if (Flags & uint32_t(FunctionFlags::BlueprintAuthorityOnly)) { buffer.push_back("BlueprintAuthorityOnly"); }
    if (Flags & uint32_t(FunctionFlags::BlueprintCosmetic)) { buffer.push_back("BlueprintCosmetic"); }
    if (Flags & uint32_t(FunctionFlags::Net)) { buffer.push_back("Net"); }
    if (Flags & uint32_t(FunctionFlags::NetReliable)) { buffer.push_back("NetReliable"); }
    if (Flags & uint32_t(FunctionFlags::NetRequest)) { buffer.push_back("NetRequest"); }
    if (Flags & uint32_t(FunctionFlags::Exec)) { buffer.push_back("Exec"); }
    if (Flags & uint32_t(FunctionFlags::Native)) { buffer.push_back("Native"); }
    if (Flags & uint32_t(FunctionFlags::Event)) { buffer.push_back("Event"); }
    if (Flags & uint32_t(FunctionFlags::NetResponse)) { buffer.push_back("NetResponse"); }
    if (Flags & uint32_t(FunctionFlags::Static)) { buffer.push_back("Static"); }
    if (Flags & uint32_t(FunctionFlags::NetMulticast)) { buffer.push_back("NetMulticast"); }
    if (Flags & uint32_t(FunctionFlags::MulticastDelegate)) { buffer.push_back("MulticastDelegate"); }
    if (Flags & uint32_t(FunctionFlags::Public)) { buffer.push_back("Public"); }
    if (Flags & uint32_t(FunctionFlags::Private)) { buffer.push_back("Private"); }
    if (Flags & uint32_t(FunctionFlags::Protected)) { buffer.push_back("Protected"); }
    if (Flags & uint32_t(FunctionFlags::Delegate)) { buffer.push_back("Delegate"); }
    if (Flags & uint32_t(FunctionFlags::NetServer)) { buffer.push_back("NetServer"); }
    if (Flags & uint32_t(FunctionFlags::HasOutParms)) { buffer.push_back("HasOutParms"); }
    if (Flags & uint32_t(FunctionFlags::HasDefaults)) { buffer.push_back("HasDefaults"); }
    if (Flags & uint32_t(FunctionFlags::NetClient)) { buffer.push_back("NetClient"); }
    if (Flags & uint32_t(FunctionFlags::DLLImport)) { buffer.push_back("DLLImport"); }
    if (Flags & uint32_t(FunctionFlags::BlueprintCallable)) { buffer.push_back("BlueprintCallable"); }
    if (Flags & uint32_t(FunctionFlags::BlueprintEvent)) { buffer.push_back("BlueprintEvent"); }
    if (Flags & uint32_t(FunctionFlags::BlueprintPure)) { buffer.push_back("BlueprintPure"); }
    if (Flags & uint32_t(FunctionFlags::Const)) { buffer.push_back("Const"); }
    if (Flags & uint32_t(FunctionFlags::NetValidate)) { buffer.push_back("NetValidate"); }

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
    case 0: return std::string();
    case 1: return std::string(buffer[0]);
    default:
        std::ostringstream os;
        std::copy(buffer.begin(), buffer.end() - 1, std::ostream_iterator<const char*>(os, ", "));
        os << *buffer.rbegin();
        return os.str();
    }
}

}