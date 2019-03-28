#include "UnrealTypes.h"
#include "Names.h"
#include "Objects.h"

#include <sstream>
#include <iterator>

DEFINE_STATIC_CLASS(Object);
DEFINE_STATIC_CLASS(Package);
DEFINE_STATIC_CLASS(Field);
DEFINE_STATIC_CLASS(Enum);
DEFINE_STATIC_CLASS(Struct);
DEFINE_STATIC_CLASS(ScriptStruct);
DEFINE_STATIC_CLASS(Function);
DEFINE_STATIC_CLASS(Class);

int32_t UObject::GetFlags() const
{
    return (int32_t)DecryptObjectFlagsAsm(ObjectFlagsEncrypted);
}

uint32_t UObject::GetUniqueId() const
{
    return (uint32_t)DecryptObjectIndexAsm(InternalIndexEncrypted);
}

UClass* UObject::GetClass() const
{
    return (UClass*)DecryptObjectClassAsm(ClassEncrypted);
}

UObject* UObject::GetOuter() const
{
    return (UObject*)DecryptObjectOuterAsm(OuterEncrypted);
}

FName UObject::GetFName() const
{
    int32_t NameIndex, NameNumber;
    DecryptObjectFNameAsm(NameIndexEncrypted, NameNumberEncrypted, &NameIndex, &NameNumber);
    return FName(NameIndex, NameNumber);
}

const UPackage* UObject::GetOutermost() const
{
    UObject* Top = NULL;
    for (UObject* Outer = GetOuter(); Outer; Outer = Outer->GetOuter()) {
        Top = Outer;
    }
    return static_cast<const UPackage*>(Top);
}

std::string UObject::GetName() const
{
    FName Name = GetFName();
    std::string NameString = NamesProxy().GetById(Name.Index);
    if (Name.Number > 0) {
        NameString += '_' + std::to_string(Name.Number);
    }

    size_t pos = NameString.rfind('/');
    if (pos != std::string::npos) {
        NameString = NameString.substr(pos + 1);
    }

    return NameString;
}

std::string UObject::GetFullName() const
{
    std::string NameString;
    const UClass* Class = GetClass();
    LOG_DEBUG(_XOR_("MADEIT1.1"));
    if (Class) {
        std::string Temp;
        UObject* Outer = GetOuter();
        LOG_DEBUG(_XOR_("MADEIT1.2"));
        while (Outer) {
            Temp = Outer->GetName() + '.' + Temp;
            LOG_DEBUG(_XOR_("MADEIT1.3"));
            Outer = Outer->GetOuter();
        }
        LOG_DEBUG(_XOR_("MADEIT1.4"));
        NameString = Class->GetName();
        LOG_DEBUG(_XOR_("MADEIT1.5"));
        NameString += ' ';
        NameString += Temp;
        NameString += GetName();
        LOG_DEBUG(_XOR_("MADEIT1.6"));
    }
    return NameString;
}

std::string UObject::GetNameCPP() const
{
    std::string NameString;

    if (IsA<UClass>()) {

        const UClass* Class = static_cast<const UClass*>(this);
        while (Class) {

            const std::string ClassName = Class->GetName();
            if (ClassName == "Actor") {
                NameString += "A";
                break;
            }
            if (ClassName == "Object") {
                NameString += "U";
                break;
            }

            Class = static_cast<const UClass*>(Class->GetSuper());
        }

    } else {
        NameString += "F";
    }

    NameString += GetName();
    return NameString;
}

bool UObject::IsA(const UClass* CmpClass) const
{
    UClass* SuperClass = GetClass();
    while (SuperClass != nullptr) {
        if (SuperClass == CmpClass) {
            return true;
        }
        SuperClass = static_cast<UClass*>(SuperClass->GetSuper());
    }
    return false;
}

std::vector<std::string> UEnum::GetNames() const
{
    NamesProxy GlobalNames;
    std::vector<std::string> StringArray;
    const TArray<TPair<FName, int64_t>>& NamesArray = this->Names;

    //LOG_INFO("MADEIT1 - NamesArray.Num = %d", NamesArray.Num());

    for (auto Name : NamesArray) {
        int32_t Index = Name.Key.GetIndex();
        //LOG_INFO("MADEIT2 - NamesArray[%d].Key.GetIndex() = %d", i, Index);
        StringArray.push_back(GlobalNames.GetById(Index));
    }

    //LOG_INFO("MADEIT3");

    return StringArray;
}



std::string StringifyFunctionFlags(const uint32_t Flags)
{
    std::vector<const char*> buffer;

    if (Flags & FunctionFlags::Final) { buffer.push_back("Final"); }
    if (Flags & FunctionFlags::RequiredAPI) { buffer.push_back("RequiredAPI"); }
    if (Flags & FunctionFlags::BlueprintAuthorityOnly) { buffer.push_back("BlueprintAuthorityOnly"); }
    if (Flags & FunctionFlags::BlueprintCosmetic) { buffer.push_back("BlueprintCosmetic"); }
    if (Flags & FunctionFlags::Net) { buffer.push_back("Net"); }
    if (Flags & FunctionFlags::NetReliable) { buffer.push_back("NetReliable"); }
    if (Flags & FunctionFlags::NetRequest) { buffer.push_back("NetRequest"); }
    if (Flags & FunctionFlags::Exec) { buffer.push_back("Exec"); }
    if (Flags & FunctionFlags::Native) { buffer.push_back("Native"); }
    if (Flags & FunctionFlags::Event) { buffer.push_back("Event"); }
    if (Flags & FunctionFlags::NetResponse) { buffer.push_back("NetResponse"); }
    if (Flags & FunctionFlags::Static) { buffer.push_back("Static"); }
    if (Flags & FunctionFlags::NetMulticast) { buffer.push_back("NetMulticast"); }
    if (Flags & FunctionFlags::MulticastDelegate) { buffer.push_back("MulticastDelegate"); }
    if (Flags & FunctionFlags::Public) { buffer.push_back("Public"); }
    if (Flags & FunctionFlags::Private) { buffer.push_back("Private"); }
    if (Flags & FunctionFlags::Protected) { buffer.push_back("Protected"); }
    if (Flags & FunctionFlags::Delegate) { buffer.push_back("Delegate"); }
    if (Flags & FunctionFlags::NetServer) { buffer.push_back("NetServer"); }
    if (Flags & FunctionFlags::HasOutParms) { buffer.push_back("HasOutParms"); }
    if (Flags & FunctionFlags::HasDefaults) { buffer.push_back("HasDefaults"); }
    if (Flags & FunctionFlags::NetClient) { buffer.push_back("NetClient"); }
    if (Flags & FunctionFlags::DLLImport) { buffer.push_back("DLLImport"); }
    if (Flags & FunctionFlags::BlueprintCallable) { buffer.push_back("BlueprintCallable"); }
    if (Flags & FunctionFlags::BlueprintEvent) { buffer.push_back("BlueprintEvent"); }
    if (Flags & FunctionFlags::BlueprintPure) { buffer.push_back("BlueprintPure"); }
    if (Flags & FunctionFlags::Const) { buffer.push_back("Const"); }
    if (Flags & FunctionFlags::NetValidate) { buffer.push_back("NetValidate"); }

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