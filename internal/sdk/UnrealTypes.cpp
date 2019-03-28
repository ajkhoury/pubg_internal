#include "UnrealTypes.h"
#include "Names.h"
#include "Objects.h"

DEFINE_STATIC_CLASS(Object);
DEFINE_STATIC_CLASS(Package);
DEFINE_STATIC_CLASS(Field);
DEFINE_STATIC_CLASS(Enum);
DEFINE_STATIC_CLASS(Struct);
DEFINE_STATIC_CLASS(ScriptStruct);
DEFINE_STATIC_CLASS(Function);
DEFINE_STATIC_CLASS(Class);

#if !defined(ENABLE_SDK)

int32_t UObject::GetFlags() const
{
    return (int32_t)DecryptObjectFlagsAsm(ObjectFlagsEncrypted);
}

uint32_t UObject::GetUniqueId() const
{
    return (uint32_t)DecryptObjectIndexAsm(InternalIndexEncrypted);
}

UClass const* UObject::GetClass() const
{
    return (UClass const*)DecryptObjectClassAsm(ClassEncrypted);
}

UObject const* UObject::GetOuter() const
{
    return (UObject const*)DecryptObjectOuterAsm(OuterEncrypted);
}

FName UObject::GetFName() const
{
    FName Name;
    DecryptObjectFNameAsm(NameIndexEncrypted, NameNumberEncrypted, &Name.Index, &Name.Number);
    return Name;
}

UPackage const* UObject::GetOutermost() const
{
    UObject const* Top = NULL;
    for (UObject const* Outer = GetOuter(); Outer; Outer = Outer->GetOuter()) {
        Top = Outer;
    }
    return static_cast<UPackage const*>(Top);
}

std::string UObject::GetName() const
{
    std::string NameString;

    FName Name = GetFName();
    NameString = NamesProxy().GetById(Name.Index);
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
    UClass const* Class = GetClass();
    if (Class) {
        std::string Temp;

        UObject const* Outer = GetOuter();
        while (Outer) {
            Temp = Outer->GetName() + '.' + Temp;
            Outer = Outer->GetOuter();
        }

        NameString = Class->GetName();
        NameString += ' ';
        NameString += Temp;
        NameString += GetName();
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
    UClass const* SuperClass = GetClass();
    while (SuperClass != nullptr) {
        if (SuperClass == CmpClass) {
            return true;
        }
        SuperClass = static_cast<UClass const*>(SuperClass->GetSuper());
    }
    return false;
}

std::vector<std::string> UEnum::GetNames() const
{
    NamesProxy GlobalNames;
    std::vector<std::string> StringArray;
    const TArray<TPair<FName, int64_t>>& NamesArray = this->Names;

    for (const TPair<FName, int64_t>& Name : NamesArray) {
        int32_t Index = Name.Key.GetIndex();
        StringArray.push_back(GlobalNames.GetById(Index));
    }

    return StringArray;
}

#endif // CORE_UOBJECT_CLASSES