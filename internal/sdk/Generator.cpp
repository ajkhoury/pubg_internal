#include "Generator.h"
#include "Format.h"
#include "Objects.h"
#include "UnrealTypes.h"

#include <native/log.h>
#include <utils/xorstr.h>
#include <utils/utils.h>

using namespace cpplinq;

// Public static fields of Generator.
std::unordered_map<const UPackage*, const GeneratorPackage*> Generator::PackageMap;
std::unordered_map<std::string, uint32_t> Generator::AlignasClasses = {
    { "ScriptStruct CoreUObject.Plane", 16 },
    { "ScriptStruct CoreUObject.Quat", 16 },
    { "ScriptStruct CoreUObject.Transform", 16 },
    { "ScriptStruct CoreUObject.Vector4", 16 },
    { "ScriptStruct Engine.RootMotionSourceGroup", 8 }
};


void PrintFileHeader(std::ostream& os, const std::vector<std::string>& includes, const bool isHeaderFile)
{
    if (isHeaderFile) {
        os << _XORSTR_("#pragma once\n\n");
    }

    os << tfm::format(_XOR_("// PUBG UE4 SDK\n\n"))
       << tfm::format(_XOR_("#ifdef _MSC_VER\n#pragma pack(push, %d)\n#endif\n\n"), sizeof(size_t));

    if (!includes.empty()) {
        for (auto&& i : includes) {
            if (i[0] != '<' && i[0] != '\"') {
                os << _XORSTR_("#include \"") << i << "\"\n";
            } else {
                os << _XORSTR_("#include ") << i << '\n';
            }
        }
        os << '\n';
    }
}

void PrintFileHeader(std::ostream& os, const bool isHeaderFile)
{
    PrintFileHeader(os, std::vector<std::string>(), isHeaderFile);
}

void PrintFileFooter(std::ostream& os)
{
    os << _XORSTR_("#ifdef _MSC_VER\n#pragma pack(pop)\n#endif\n");
}

void PrintSectionHeader(std::ostream& os, const char* name)
{
    os  << _XORSTR_("//---------------------------------------------------------------------------\n")
        << _XORSTR_("//") << name << '\n'
        << _XORSTR_("//---------------------------------------------------------------------------\n\n");
}

std::string GenerateFileName(const FileContentType Type, const UPackage* PackageObject)
{
    const char* Name;
    switch (Type) {
    case FileContentType::Structs:
        Name = _XOR_("%s_%s_structs.h");
        break;
    case FileContentType::Classes:
        Name = _XOR_("%s_%s_classes.h");
        break;
    case FileContentType::Functions:
        Name = _XOR_("%s_%s_functions.cpp");
        break;
    case FileContentType::FunctionParameters:
        Name = _XOR_("%s_%s_parameters.h");
        break;
    default:
        assert(false);
    }

    return tfm::format(Name, _XOR_("PUBG"), PackageObject->GetName());
}

bool GeneratorParameter::MakeType(const uint64_t flags, GeneratorParameter::Type& type)
{
    if (flags & CPF_ReturnParm) {
        type = Type::Return;
    } else if (flags & CPF_OutParm) {
        //if it is a const parameter make it a default parameter
        if (flags & CPF_ConstParm) {
            type = Type::Default;
        } else {
            type = Type::Out;
        }
    } else if (flags & CPF_Parm) {
        type = Type::Default;
    } else {
        return false;
    }
    return true;
}

bool ComparePropertyLess(const UProperty* lhs, const UProperty* rhs)
{
    if (lhs->GetOffset() == rhs->GetOffset() &&
        lhs->IsA<UBoolProperty>() && rhs->IsA<UBoolProperty>()) {
        return lhs->CastRef<UBoolProperty>() < rhs->CastRef<UBoolProperty>();
    }
    return lhs->GetOffset() < rhs->GetOffset();
}

void GeneratorPackage::GenerateEnum(const UEnum* Enum)
{
    GeneratorEnum e;
    e.Name = fmt::MakeUniqueEnumCppName(Enum);
    if (e.Name.find(_XOR_("Default__")) != std::string::npos ||
        e.Name.find(_XOR_("PLACEHOLDER-CLASS")) != std::string::npos) {
        return;
    }
    e.FullName = Enum->GetFullName();

    LOG_DBG_PRINT("Enum:          %-98s - instance: 0x%p\n", e.FullName.c_str(), Enum);

    std::unordered_map<std::string, int> conflicts;
    for (auto&& s : Enum->GetNames()) {
        const auto clean = fmt::MakeValidName(std::move(s));
        const auto it = conflicts.find(clean);
        if (it == std::end(conflicts)) {
            e.Values.push_back(clean);
            conflicts[clean] = 1;
        } else {
            e.Values.push_back(clean + tfm::format("%02d", it->second));
            conflicts[clean]++;
        }
    }

    Enums.emplace_back(std::move(e));
}

void GeneratorPackage::GenerateMethods(const UClass* ClassObj, std::vector<GeneratorMethod>& methods) const
{
    //some classes (AnimBlueprintGenerated...) have multiple Members with the same name, so filter them out
    std::unordered_set<std::string> UniqueMethods;

    UProperty* Prop = static_cast<UProperty*>(ClassObj->GetChildren());
    while (Prop) {

        if (Prop->IsA<UFunction>()) {
            UFunction* Function = Prop->Cast<UFunction>();

            GeneratorMethod m;
            m.Index = Function->GetUniqueId();
            m.FullName = Function->GetFullName();
            m.Name = fmt::MakeValidName(Function->GetName());

            if (UniqueMethods.find(m.FullName) != std::end(UniqueMethods)) {
                continue;
            }
            UniqueMethods.insert(m.FullName);

            m.IsNative = Function->GetFunctionFlags() & uint32_t(FunctionFlags::Native);
            m.IsStatic = Function->GetFunctionFlags() & uint32_t(FunctionFlags::Static);
            m.FlagsString = fmt::StringifyFunctionFlags(Function->GetFunctionFlags());

            std::vector<std::pair<UProperty*, GeneratorParameter>> Parameters;

            std::unordered_map<std::string, size_t> Unique;
            UProperty* Param = static_cast<UProperty*>(Function->GetChildren());
            while (Param) {
                if (Param->GetElementSize() == 0) {
                    continue;
                }

                const UProperty::Info ParamInfo = Param->GetInfo();
                if (ParamInfo.Type != UProperty::PropertyType::Unknown) {

                    GeneratorParameter p;

                    if (!GeneratorParameter::MakeType(Param->GetPropertyFlags(), p.ParamType)) {
                        // child isn't a parameter
                        continue;
                    }

                    p.PassByReference = false;
                    p.Name = fmt::MakeValidName(Param->GetName());

                    const auto it = Unique.find(p.Name);
                    if (it == std::end(Unique)) {
                        Unique[p.Name] = 1;
                    } else {
                        ++Unique[p.Name];
                        p.Name += tfm::format("%02d", it->second);
                    }

                    p.FlagsString = fmt::StringifyPropertyFlags(Param->GetPropertyFlags());

                    p.CppType = ParamInfo.CppType;
                    if (Param->IsA<UBoolProperty>()) {
                        p.CppType = _XORSTR_("bool");
                    }
                    switch (p.ParamType) {
                    case GeneratorParameter::Type::Default:
                        if (Prop->GetArrayDim() > 1) {
                            p.CppType = p.CppType + '*';
                        } else if (ParamInfo.CanBeReference) {
                            p.PassByReference = true;
                        }
                        break;
                    }

                    Parameters.emplace_back(std::make_pair(Prop, std::move(p)));
                }

                Param = static_cast<UProperty*>(Param->GetNext());
            }

            std::sort(std::begin(Parameters), std::end(Parameters),
                      [](auto&& lhs, auto&& rhs) { return ComparePropertyLess(lhs.first, rhs.first); });

            for (auto& param : Parameters) {
                m.Parameters.emplace_back(std::move(param.second));
            }

            methods.emplace_back(std::move(m));
        }

        Prop = static_cast<UProperty*>(Prop->GetNext());
    }
}

GeneratorMember CreatePadding(size_t Id, size_t Offset, size_t Size, std::string Reason)
{
    GeneratorMember m;
    m.Name = tfm::format(_XOR_("UnknownData%02d[0x%X]"), Id, Size);
    m.Type = _XORSTR_("uint8_t");
    m.Offset = Offset;
    m.Size = Size;
    m.Comment = std::move(Reason);
    return m;
}

GeneratorMember CreateBitfieldPadding(size_t Id, size_t Offset, std::string Type, int BitsCount)
{
    GeneratorMember m;
    m.Name = tfm::format(_XOR_("UnknownData%02d : %d"), Id, BitsCount);
    m.Type = std::move(Type);
    m.Offset = Offset;
    m.Size = 1;
    return m;
}

void GeneratorPackage::GenerateMembers(const UStruct* Struct, int32_t Offset, const std::vector<UProperty*>& Props, std::vector<GeneratorMember>& Members) const
{
    std::unordered_map<std::string, size_t> UniqueMemberNames;
    size_t UnknownDataCounter = 0;
    UBoolProperty* PreviousBitfieldProperty = nullptr;

    for (auto&& Prop : Props) {

        if (Offset < Prop->GetOffset()) {
            PreviousBitfieldProperty = nullptr;
            const auto size = Prop->GetOffset() - Offset;
            Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, size, _XORSTR_("MISSED OFFSET")));
        }

        const auto Info = Prop->GetInfo();
        if (Info.Type != UProperty::PropertyType::Unknown) {
            GeneratorMember sp;
            sp.Offset = Prop->GetOffset();
            sp.Size = Info.Size;
            sp.Type = Info.CppType;
            sp.Name = fmt::MakeValidName(Prop->GetName());

            const auto it = UniqueMemberNames.find(sp.Name);
            if (it == std::end(UniqueMemberNames)) {
                UniqueMemberNames[sp.Name] = 1;
            } else {
                ++UniqueMemberNames[sp.Name];
                sp.Name += tfm::format("%02d", it->second);
            }

            if (Prop->GetArrayDim() > 1) {
                sp.Name += tfm::format("[0x%X]", Prop->GetArrayDim());
            }

            if (Prop->IsA<UBoolProperty>() && static_cast<UBoolProperty*>(Prop)->IsBitfield()) {
                UBoolProperty* BoolProp = static_cast<UBoolProperty*>(Prop);
                const auto MissingBits = BoolProp->GetMissingBitsCount(PreviousBitfieldProperty);
                if (MissingBits[1] != -1) {
                    if (MissingBits[0] > 0) {
                        Members.emplace_back(CreateBitfieldPadding(UnknownDataCounter++, PreviousBitfieldProperty->GetOffset(), Info.CppType, MissingBits[0]));
                    }
                    if (MissingBits[1] > 0) {
                        Members.emplace_back(CreateBitfieldPadding(UnknownDataCounter++, sp.Offset, Info.CppType, MissingBits[1]));
                    }
                } else if (MissingBits[0] > 0) {
                    Members.emplace_back(CreateBitfieldPadding(UnknownDataCounter++, sp.Offset, Info.CppType, MissingBits[0]));
                }

                PreviousBitfieldProperty = BoolProp;
                sp.Name += _XORSTR_(" : 1");

            } else {

                PreviousBitfieldProperty = nullptr;
            }

            sp.Flags = Prop->GetPropertyFlags();
            sp.FlagsString = fmt::StringifyPropertyFlags(sp.Flags);

            Members.emplace_back(std::move(sp));

            const auto sizeMismatch = static_cast<int32_t>(Prop->GetElementSize() * Prop->GetArrayDim()) -
                                      static_cast<int32_t>(Info.Size * Prop->GetArrayDim());
            if (sizeMismatch > 0) {
                Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, sizeMismatch, _XORSTR_("FIX WRONG TYPE SIZE OF PREVIOUS PROPERTY")));
            }

        } else {

            const auto size = Prop->GetElementSize() * Prop->GetArrayDim();
            Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, size, _XORSTR_("UNKNOWN PROPERTY: ") + Prop->GetFullName()));
        }

        Offset = Prop->GetOffset() + Prop->GetElementSize() * Prop->GetArrayDim();
    }

    if (Offset < Struct->GetPropertiesSize()) {
        const auto size = Struct->GetPropertiesSize() - Offset;
        Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, size, _XORSTR_("MISSED OFFSET")));
    }
}

void GeneratorPackage::GenerateClass(const UClass* ClassObj)
{
    if (!ClassObj)
        return;

    GeneratorClass c;
    c.Name = ClassObj->GetName();
    // See if we should filter this class.
    if (c.Name.find(_XOR_("ATslLobby_v3_C")) != std::string::npos) {
        // Skip this class as it's problematic since it has duplicates and has
        // no real important use.
        return;
    }
    c.FullName = ClassObj->GetFullName();

    LOG_DBG_PRINT("Class:          %-98s - instance: 0x%p\n", c.FullName.c_str(), ClassObj);

    c.NameCpp = fmt::MakeValidName(ClassObj->GetNameCPP());
    c.NameCppFull = _XORSTR_("class ") + c.NameCpp;

    c.Size = ClassObj->GetPropertiesSize();
    c.InheritedSize = 0;

    int32_t offset = 0;

    UStruct* SuperStruct = ClassObj->GetSuper();
    if (SuperStruct && SuperStruct != ClassObj) {
        c.InheritedSize = offset = SuperStruct->GetPropertiesSize();
        c.NameCppFull += _XORSTR_(" : public ") + fmt::MakeValidName(SuperStruct->GetNameCPP());
    }

    std::vector<GeneratorPredefinedStaticMember> PredefinedStaticMembers;
    if (Generator->GetPredefinedClassStaticMembers(c.FullName, PredefinedStaticMembers)) {
        for (auto&& Member : PredefinedStaticMembers) {
            GeneratorStaticMember m;
            m.Name = Member.Name;
            m.Type = _XORSTR_("static ") + Member.Type;
            m.Definition = Member.Definition;
            c.StaticMembers.push_back(std::move(m));
        }
    }

    std::vector<GeneratorPredefinedMember> PredefinedMembers;
    c.bMembersPredefined = Generator->GetPredefinedClassMembers(c.FullName, PredefinedMembers, true);
    if (c.bMembersPredefined) {

        for (auto&& Member : PredefinedMembers) {
            GeneratorMember m;
            m.Offset = Member.Offset;
            m.Size = Member.Size;
            m.Name = Member.Name;
            m.Type = Member.Type;
            m.Comment = _XORSTR_("NOT AUTO-GENERATED PROPERTY");
            c.Members.push_back(std::move(m));
        }

    } else {
    
        std::vector<UProperty*> Props;
        UProperty* Prop = static_cast<UProperty*>(ClassObj->GetChildren());
        while (Prop) {
            if (Prop->GetElementSize() > 0 &&
                !Prop->IsA<UScriptStruct>() &&
                !Prop->IsA<UFunction>() &&
                !Prop->IsA<UEnum>() &&
                (!SuperStruct ||
                 (SuperStruct != ClassObj && Prop->GetOffset() >= SuperStruct->GetPropertiesSize()))) {
                Props.push_back(Prop);
            }
            Prop = static_cast<UProperty*>(Prop->GetNext());
        }
        std::sort(std::begin(Props), std::end(Props), ComparePropertyLess);

        // Generate this class object's members/fields.
        GenerateMembers(ClassObj, offset, Props, c.Members);
    }

    // Generate predefined methods.
    Generator->GetPredefinedClassMethods(c.FullName, c.PredefinedMethods);

    // Generate predefined StaticClass routine.
    if (Generator->ShouldUseStrings()) {

        c.PredefinedMethods.push_back(
            GeneratorPredefinedMethod::Default(
                tfm::format(_XOR_("static class UClass* %sClass;\n    static class UClass* StaticClass()"), c.NameCpp),
                tfm::format(_XOR_(
                "UClass* %s::%sClass = nullptr;\n"
                "UClass* %s::StaticClass()\n"
                "{\n"
                "    if (!%sClass)\n"
                "        %sClass = UObject::FindClass(%s);\n"
                "    return %sClass;\n"
                "}"), c.NameCpp, c.NameCpp, c.NameCpp, c.NameCpp, c.NameCpp,
                      Generator->ShouldXorStrings() ? tfm::format("_XOR_(\"%s\")", c.FullName) : tfm::format("\"%s\"", c.FullName),
                      c.NameCpp)
            )
        );

    } else {

        c.PredefinedMethods.push_back(
            GeneratorPredefinedMethod::Default(
                tfm::format(_XOR_("static UClass* %sClass;\n    static UClass* StaticClass()"), c.NameCpp),
                tfm::format(_XOR_(
                    "UClass* %s::%sClass = nullptr;\n"
                    "UClass* %s::StaticClass()\n"
                    "{\n"
                    "    if (!%sClass)\n"
                    "        %sClass = UObject::GetObjectCasted<UClass>(%d);\n"
                    "    return %sClass;\n"
                    "}"), c.NameCpp, c.NameCpp, c.NameCpp, c.NameCpp, c.NameCpp, ClassObj->GetUniqueId(), c.NameCpp)
            )
        );
    }

    // Skip BlueprintGeneratedClass types methods since they seem to be neverending.
    if (c.FullName.find(_XOR_("BlueprintGeneratedClass ")) != std::string::npos) {

        LOG_INFO(_XOR_("BLUEPRINT GENERATED CLASS: %s"), c.Name.c_str());

    } else {

        // Generate this class object's methods.
        GenerateMethods(ClassObj, c.Methods);
    }

    // Search virtual functions.
    std::vector<GeneratorPattern> Patterns;
    if (Generator->GetVirtualFunctionPatterns(c.FullName, Patterns)) {

        // Get the vtable address.
        const void** VTable = *reinterpret_cast<const void** const*>(ClassObj);

        uintptr_t ImageBase = reinterpret_cast<uintptr_t>(utils::GetModuleHandleWIDE(NULL));
        size_t ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

        // Roughly calculate the VTable method count.
        size_t MethodCount = 0;
        while (1) {
            uintptr_t VTableEntry = reinterpret_cast<uintptr_t>(VTable[MethodCount]);
            if (VTableEntry < ImageBase || VTableEntry >= (ImageBase + ImageSize))
                break;
            ++MethodCount;
        }

        LOG_INFO(_XOR_("%s VTable method count: %d"), c.FullName.c_str(), MethodCount);

        // Search for each pattern in each virtual function.
        for (GeneratorPattern& Pattern : Patterns) {
            for (size_t i = 0; i < MethodCount; ++i) {

                size_t Size = Pattern.SearchSize;
                const uint8_t* Base = static_cast<const uint8_t*>(VTable[i]);

                // If this is a thunk, set the search base to the thunk target.
                if (*Base == 0xE9) {
                    Base = static_cast<const uint8_t*>(utils::GetJmpTargetAddress(Base));
                }

                //LOG_INFO(_XOR_("Search pattern \"%s\" in %s VTable[%d]: 0x%p"), Pattern.Pattern.c_str(), c.FullName.c_str(), i, SearchBase);
                //if (i == 92) {
                //    LOG_INFO(_XOR_("instructions = {"));
                //    std::string BufString = utils::FormatBuffer((const uint8_t*)SearchBase, 512);
                //    for (auto Line : utils::SplitString(BufString, '\n'))
                //        LOG_INFO("%s", Line.c_str());
                //    LOG_INFO(_XOR_("};"));
                //}

                // Search for the pattern in this virtual function.
                if (utils::FindPatternIDA(Base, Size, Pattern.Pattern)) {

                    LOG_INFO(_XOR_("FOUND pattern \"%s\" in %s VTable[%d] !!!"), Pattern.Pattern.c_str(), c.FullName.c_str(), i);

                    // Add to the predefined methods for this class if the search was successful.
                    c.PredefinedMethods.push_back(
                        GeneratorPredefinedMethod::Inline(tfm::format(Pattern.Function.c_str(), i)));
                    break;
                }
            }
        }
    }

    // Add generated class.
    Classes.emplace_back(std::move(c));
}

void GeneratorPackage::GenerateScriptStruct(const UScriptStruct* ScriptStruct)
{
    GeneratorScriptStruct ss;
    ss.Name = ScriptStruct->GetName();
    ss.FullName = ScriptStruct->GetFullName();

    LOG_DBG_PRINT("ScriptStruct: %-100s - instance: 0x%p\n", ss.Name.c_str(), ScriptStruct);

    ss.NameCpp = fmt::MakeValidName(ScriptStruct->GetNameCPP());
    ss.NameCppFull = _XORSTR_("struct ");

    // Some classes need special alignment
    const size_t Alignment = Generator->GetClassAlignas(ss.FullName);
    if (Alignment != 0) {
        ss.NameCppFull += tfm::format(_XOR_("alignas(%d) "), Alignment);
    }

    ss.NameCppFull += fmt::MakeUniqueStructCppName(static_cast<const UStruct*>(ScriptStruct));
    ss.Size = ScriptStruct->GetPropertiesSize();
    ss.InheritedSize = 0;

    int32_t Offset = 0;
    const UStruct* SuperStruct = ScriptStruct->GetSuper();
    if (SuperStruct && SuperStruct != ScriptStruct) {
        ss.InheritedSize = Offset = SuperStruct->GetPropertiesSize();
        ss.NameCppFull += _XORSTR_(" : public ") + fmt::MakeUniqueStructCppName(SuperStruct);
    }

    std::vector<UProperty*> Props;
    UProperty* Prop = static_cast<UProperty*>(ScriptStruct->GetChildren());
    while (Prop) {
        if (Prop->GetElementSize() > 0 &&
            !Prop->IsA<UScriptStruct>() &&
            !Prop->IsA<UFunction>() &&
            !Prop->IsA<UEnum>()) {
            Props.push_back(Prop);
        }
        Prop = static_cast<UProperty*>(Prop->GetNext());
    }
    std::sort(std::begin(Props), std::end(Props), ComparePropertyLess);

    GenerateMembers(ScriptStruct, Offset, Props, ss.Members);

    Generator->GetPredefinedClassMethods(ScriptStruct->GetFullName(), ss.PredefinedMethods);

    ScriptStructs.emplace_back(std::move(ss));
}

void GeneratorPackage::GenerateMemberPrerequisites(UProperty const* First, std::unordered_map<UObject const*, bool>& ProcessedObjects)
{
    UProperty* Prop = const_cast<UProperty*>(First);
    while (Prop) {

        const UProperty::Info PropInfo = Prop->GetInfo();

        if (PropInfo.Type == UProperty::PropertyType::Primitive) {

            if (Prop->IsA<UByteProperty>()) {

                UByteProperty* ByteProperty = static_cast<UByteProperty*>(Prop);
                if (ByteProperty->IsEnum()) {
                    UEnum* Enum = ByteProperty->GetEnum();
                    if (Enum) {
                        AddDependency(Enum->GetOutermost());
                    }
                }

            } else if (Prop->IsA<UEnumProperty>()) {

                UEnumProperty* EnumProperty = static_cast<UEnumProperty*>(Prop);
                UEnum* Enum = EnumProperty->GetEnum();
                if (Enum) {
                    AddDependency(Enum->GetOutermost());
                }
            }

        } else if (PropInfo.Type == UProperty::PropertyType::CustomStruct) {

            GeneratePrerequisites(static_cast<UStructProperty*>(Prop)->GetStruct(), ProcessedObjects);

        } else if (PropInfo.Type == UProperty::PropertyType::Container) {

            std::vector<UProperty*> InnerProperties;
            if (Prop->IsA<UArrayProperty>()) {

                UArrayProperty* ArrayProp = static_cast<UArrayProperty*>(Prop);
                InnerProperties.push_back(ArrayProp->GetInner());

            } else if (Prop->IsA<UMapProperty>()) {

                UMapProperty* MapProp = static_cast<UMapProperty*>(Prop);
                InnerProperties.push_back(MapProp->GetKeyProperty());
                InnerProperties.push_back(MapProp->GetValueProperty());
            }

            for (UProperty* InnerProp : from(InnerProperties)
                 >> where([](auto&& p) { return p->GetInfo().Type == UProperty::PropertyType::CustomStruct; })
                 >> experimental::container()) {
                GeneratePrerequisites(static_cast<UStructProperty*>(InnerProp)->GetStruct(), ProcessedObjects);
            }

        } else if (Prop->IsA<UFunction>()) {

            UFunction* Function = Prop->Cast<UFunction>();
            UField* FunctionChildren = Function->GetChildren();
            GenerateMemberPrerequisites(FunctionChildren->Cast<UProperty>(), ProcessedObjects);
        }

        Prop = static_cast<UProperty*>(Prop->GetNext());
    }
}

void GeneratorPackage::GeneratePrerequisites(const UObject* Obj, std::unordered_map<const UObject*, bool>& ProcessedObjects)
{
    if (!Obj) {
        return;
    }

    const bool isClass = Obj->IsA<UClass>();
    const bool isScriptStruct = Obj->IsA<UScriptStruct>();
    if (!isClass && !isScriptStruct) {
        return;
    }

    const std::string ObjNameString = Obj->GetName();
    if (ObjNameString.find(_XOR_("Default__")) != std::string::npos ||
        ObjNameString.find(_XOR_("<uninitialized>")) != std::string::npos ||
        ObjNameString.find(_XOR_("PLACEHOLDER-CLASS")) != std::string::npos) {
        return;
    }

    ProcessedObjects[Obj] |= false;

    const UPackage* ClassPackage = Obj->GetOutermost();
    if (!ClassPackage) {
        return;
    }

    if (AddDependency(ClassPackage)) {
        return;
    }

    if (ProcessedObjects[Obj] == false) {
        ProcessedObjects[Obj] = true;

        if (!isScriptStruct) {
            const UObject* Outer = Obj->GetOuter();
            if (Outer && Outer != Obj) {
                GeneratePrerequisites(Outer, ProcessedObjects);
            }
        }

        const UStruct* StructObj = Obj->Cast<UStruct>();
        const UStruct* SuperStruct = StructObj->GetSuper();
        if (SuperStruct && SuperStruct != Obj) {
            GeneratePrerequisites(SuperStruct, ProcessedObjects);
        }

        UField* StructChildren = StructObj->GetChildren();
        GenerateMemberPrerequisites(static_cast<UProperty*>(StructChildren), ProcessedObjects);

        if (isClass) {
            GenerateClass(static_cast<const UClass*>(Obj));
        } else {
            GenerateScriptStruct(static_cast<const UScriptStruct*>(Obj));
        }
    }
}

std::string GeneratorPackage::GetName() const
{
    return PackageObject->GetName();
}

void GeneratorPackage::Process(const ObjectsProxy& Objects, std::unordered_map<const UObject*, bool>& ProcessedObjects)
{
    for (UObject const* Object : Objects) {
        if (Object) {
            const UPackage* Package = Object->GetOutermost();
            if (Package && Package == PackageObject) {

                if (Object->IsA<UEnum>()) {

                    //LOG_INFO("Processing enum \"%s\"...\n", Object->GetFullName().c_str());
                    GenerateEnum(static_cast<UEnum const*>(Object));

                } else if (Object->IsA<UClass>()) {

                    //LOG_INFO("Processing class \"%s\"...\n", Object->GetFullName().c_str());
                    GeneratePrerequisites(Object, ProcessedObjects);

                } else if (Object->IsA<UScriptStruct>()) {

                    //LOG_INFO("Processing struct \"%s\"...\n", Object->GetFullName().c_str());
                    GeneratePrerequisites(Object, ProcessedObjects);
                }
            }
        }
    }
}

void GeneratorPackage::PrintConstant(std::ostream& os, const std::pair<std::string, std::string>& Constant) const
{
    tfm::format(os, _XOR_("#define CONST_%-50s %s\n"), Constant.first, Constant.second);
}

void GeneratorPackage::PrintEnum(std::ostream& os, const GeneratorEnum& Enum) const
{
    os << "// " << Enum.FullName << _XORSTR_("\nenum class ") << Enum.Name << _XORSTR_(" : uint8_t {\n");
    os << (from(Enum.Values)
           >> select([](auto&& name, auto&& i) { return tfm::format(_XOR_("    %-30s = %d"), name, i); })
           >> concatenate(",\n"))
        << _XORSTR_("\n};\n\n");
}

void GeneratorPackage::PrintStruct(std::ostream& os, const GeneratorScriptStruct& ScriptStruct) const
{
    os << "// " << ScriptStruct.FullName << "\n// ";
    if (ScriptStruct.InheritedSize) {
        os << tfm::format(_XOR_("0x%04X (0x%04X - 0x%04X)\n"), ScriptStruct.Size - ScriptStruct.InheritedSize, ScriptStruct.Size, ScriptStruct.InheritedSize);
    } else {
        os << tfm::format(_XOR_("0x%04X\n"), ScriptStruct.Size);
    }
    os << ScriptStruct.NameCppFull << _XORSTR_(" {\n");

    // Members.
    os << (from(ScriptStruct.Members)
        >> select([](auto&& m) {
            return tfm::format(_XOR_("    %-50s %-58s// 0x%04X(0x%04X)"), m.Type, m.Name + ';', m.Offset, m.Size) +
                    (!m.Comment.empty() ? " " + m.Comment : "") +
                    (!m.FlagsString.empty() ? " (" + m.FlagsString + ')' : "");
        }) >> concatenate("\n"))
        << '\n';

    // Predefined methods.
    if (!ScriptStruct.PredefinedMethods.empty()) {
        os << '\n';
        for (auto&& m : ScriptStruct.PredefinedMethods) {
            if (m.MethodType == GeneratorPredefinedMethod::Type::Inline) {
                os << m.Body;
            } else {
                os << _XORSTR_("    ") << m.Signature << ';';
            }
            os << _XORSTR_("\n\n");
        }
    }

    os << _XORSTR_("};\n");
}

std::string GeneratorPackage::BuildMethodSignature(const GeneratorClass* c, const GeneratorMethod& m, bool inHeader, bool makeStaticFuncVar) const
{
    std::ostringstream ss;

    if (c && makeStaticFuncVar) {
        std::string StaticFuncString = c->NameCpp + _XORSTR_("__") + m.Name;
        ss << _XORSTR_("static class UFunction* ") << c->NameCpp << _XORSTR_("__") << m.Name << _XORSTR_(" = nullptr;\n");
    }

    if (m.IsStatic && inHeader && !Generator->ShouldConvertStaticMethods()) {
        ss << _XORSTR_("static ");
    }

    // Return Type
    auto retn = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == GeneratorParameter::Type::Return; });
    if (retn >> any()) {
        ss << (retn >> first()).CppType;
    } else {
        ss << _XORSTR_("void");
    }
    ss << ' ';

    if (!inHeader && c != nullptr) {
        ss << c->NameCpp << _XORSTR_("::");
    }
    if (m.IsStatic && Generator->ShouldConvertStaticMethods()) {
        ss << _XORSTR_("STATIC_");
    }
    ss << m.Name;

    // Parameters
    ss << '(';
    ss << (from(m.Parameters) >>
           where([](auto&& param) { return param.ParamType != GeneratorParameter::Type::Return; }) >>
           orderby([](auto&& param) { return param.ParamType; }) >>
           select([](auto&& param) { return (param.PassByReference ? _XOR_("const ") : "") + 
                                            param.CppType +
                                            (param.PassByReference ? _XOR_("& ") : param.ParamType == GeneratorParameter::Type::Out ? _XOR_("* ") : " ") +
                                            param.Name; }) >>
           concatenate(", "));
    ss << ')';

    return ss.str();
}

std::string GeneratorPackage::BuildMethodBody(const GeneratorClass* c, const GeneratorMethod& m) const
{
    std::ostringstream ss;

    // Function Pointer
    std::string StaticFuncString = c->NameCpp + _XORSTR_("__") + m.Name;
    ss << _XORSTR_("{\n    if(!") << StaticFuncString << _XORSTR_(")\n");
    ss << _XORSTR_("        ") << StaticFuncString;
    if (Generator->ShouldUseStrings()) {

        ss << _XORSTR_(" = UObject::FindObject<UFunction>(");
        if (Generator->ShouldXorStrings()) {
            ss << _XORSTR_("_XOR_(\"") << m.FullName << _XORSTR_("\")");
        } else {
            ss << '\"' << m.FullName << '\"';
        }
        ss << _XORSTR_(");\n");

    } else {

       ss << _XORSTR_(" = UObject::GetObjectCasted<UFunction>(") << m.Index << _XORSTR_(");\n");
    }
    ss << _XORSTR_("    class UFunction* fn = ") << StaticFuncString << _XORSTR_(";\n\n");

    // Parameters
    if (Generator->ShouldGenerateFunctionParametersFile() && c != nullptr) {
    
        ss << _XORSTR_("    ") << c->NameCpp << '_' << m.Name << _XORSTR_("_Params params;\n");
    
    } else {

        ss << _XORSTR_("    struct {\n");
        for (auto&& param : m.Parameters) {
            tfm::format(ss, _XOR_("        %-30s %s;\n"), param.CppType, param.Name);
        }
        ss << _XORSTR_("    } params;\n");
    }

    auto defaultParameters = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == GeneratorParameter::Type::Default; });
    if (defaultParameters >> any()) {
        for (auto&& param : defaultParameters >> experimental::container()) {
            ss << _XORSTR_("    params.") << param.Name << _XORSTR_(" = ") << param.Name << _XORSTR_(";\n");
        }
    }
    ss << '\n';

    // Function Call
    ss << _XORSTR_("    uint32_t flags = fn->FunctionFlags;\n");
    if (m.IsNative) {
        ss << _XORSTR_("    fn->FunctionFlags |= 0x") <<
            tfm::format("%X", static_cast<uint32_t>(FunctionFlags::Native)) << _XORSTR_("; /*Native*/\n");
    }
    ss << '\n';

    if (m.IsStatic && !Generator->ShouldConvertStaticMethods()) {

        ss << _XORSTR_("    /*static*/ auto DefaultObj = StaticClass()->CreateDefaultObject();\n");
        ss << _XORSTR_("    DefaultObj->ProcessEvent(fn, &params);\n\n");

    } else {

        ss << _XORSTR_("    UObject::ProcessEvent(fn, &params);\n\n");
    }
    ss << _XORSTR_("    fn->FunctionFlags = flags;\n");

    // Out Parameters
    auto out = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == GeneratorParameter::Type::Out; });
    if (out >> any()) {
        ss << '\n';
        for (auto&& param : out >> experimental::container()) {
            ss << _XORSTR_("    if (") << param.Name << _XORSTR_(" != nullptr)\n");
            ss << _XORSTR_("        *") << param.Name << _XORSTR_(" = params.") << param.Name << _XORSTR_(";\n");
        }
    }

    // Return Value
    auto retn = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == GeneratorParameter::Type::Return; });
    if (retn >> any()) {
        ss << _XORSTR_("\n    return params.") << (retn >> first()).Name << _XORSTR_(";\n");
    }

    ss << _XORSTR_("}\n");

    return ss.str();
}

void GeneratorPackage::PrintClass(std::ostream& os, const GeneratorClass& Class) const
{
    os << "// " << Class.FullName << _XORSTR_("\n// ");
    if (Class.InheritedSize) {
        tfm::format(os, _XOR_("0x%04X (0x%04X - 0x%04X)\n"),
                    Class.Size - Class.InheritedSize, Class.Size, Class.InheritedSize);
    } else {
        tfm::format(os, _XOR_("0x%04X\n"), Class.Size);
    }
    os << Class.NameCppFull << _XORSTR_(" {\npublic:\n");

    // Members
    size_t Offset = Class.InheritedSize;
    size_t Size = 0;
    for (auto&& m : Class.Members) {

        size_t PrevOffset = Offset;
        size_t PrevSize = Size;
        Offset = m.Offset;
        Size = m.Size;

        // Print padding field to fill in space, if needed.
        if (Class.bMembersPredefined) {
            if (PrevOffset + PrevSize < Offset) {
                size_t PaddingOffset = PrevOffset + PrevSize;
                size_t PaddingSize = Offset - PaddingOffset;
                std::string PaddingName = tfm::format(_XOR_("UnknownData0x%04X[0x%X]"), PaddingOffset, PaddingSize);
                tfm::format(os, _XOR_("    %-50s %-58s// 0x%04X(0x%04X) PADDING\n"),
                            _XOR_("uint8_t"), PaddingName + ';', PaddingOffset, PaddingSize);
            }
        }

        // Print the field itself.
        if (!m.Size) {
            tfm::format(os, _XOR_("    %-50s %-58s//"), m.Type, m.Name + ';');
        } else {
            tfm::format(os, _XOR_("    %-50s %-58s// 0x%04X(0x%04X)"), m.Type, m.Name + ';', Offset, Size);
        }
        // Print comments.
        if (!m.Comment.empty()) {
            os << ' ' << m.Comment;
        }
        // Print flags string.
        if (!m.FlagsString.empty()) {
            os << " (" << m.FlagsString << ')';
        }
        os << '\n';
    }
    // Print last padding field, if needed.
    if (Class.bMembersPredefined) {
        if (Class.Size > Offset + Size) {
            size_t PaddingOffset = Offset + Size;
            size_t PaddingSize = Class.Size - (Offset + Size);
            std::string PaddingName = tfm::format(_XOR_("UnknownData0x%04X[0x%X]"), PaddingOffset, PaddingSize);
            tfm::format(os, _XOR_("    %-50s %-58s// 0x%04X(0x%04X) PADDING\n"),
                        _XOR_("uint8_t"), PaddingName + ';', PaddingOffset, PaddingSize);
        }
    }

    // Predefined static members.
    if (!Class.StaticMembers.empty()) {
        os << '\n';
        for (auto&& m : Class.StaticMembers) {
            tfm::format(os, _XOR_("    %-50s %-58s// PREDEFINED STATIC FIELD\n"), m.Type, m.Name + ';');
        }
    }

    // Predefined methods.
    if (!Class.PredefinedMethods.empty()) {
        os << '\n';
        for (auto&& m : Class.PredefinedMethods) {
            if (m.MethodType == GeneratorPredefinedMethod::Type::Inline) {
                os << m.Body;
            } else {
                os << _XORSTR_("    ") << m.Signature << ';';
            }
            os << _XORSTR_("\n\n");
        }
    }

    // Methods.
    if (!Class.Methods.empty()) {
        os << '\n';
        for (auto&& m : Class.Methods) {
            os << _XORSTR_("    ") << BuildMethodSignature(nullptr, m, true, false) << _XORSTR_(";\n");
        }
    }

    os << _XORSTR_("};\n\n");
}

void GeneratorPackage::SaveStructs(const std::experimental::filesystem::path& SdkPath) const
{
    std::ofstream os(SdkPath / GenerateFileName(FileContentType::Structs, GetPackageObject()));
    std::vector<std::string> includes{ { _XORSTR_("../Types.h"), _XORSTR_("../Objects.h") } };
    auto dependencyNames = from(DependencyObjects)
        >> select([](auto&& p) { return GenerateFileName(FileContentType::Classes, p); })
        >> experimental::container();
    includes.insert(includes.end(), std::begin(dependencyNames), std::end(dependencyNames));
    PrintFileHeader(os, includes, true);

    if (!Constants.empty()) {
        PrintSectionHeader(os, _XOR_("Constants"));
        for (auto&& c : Constants) {
            PrintConstant(os, c);
        }
        os << '\n';
    }

    if (!Enums.empty()) {
        PrintSectionHeader(os, _XOR_("Enums"));
        for (auto&& e : Enums) {
            PrintEnum(os, e); os << '\n';
        }
        os << '\n';
    }

    if (!ScriptStructs.empty()) {
        PrintSectionHeader(os, _XOR_("Script Structs"));
        for (auto&& s : ScriptStructs) {
            PrintStruct(os, s);
            os << '\n';
        }
    }

    PrintFileFooter(os);
}

void GeneratorPackage::SaveClasses(const std::experimental::filesystem::path& SdkPath) const
{
    std::ofstream os(SdkPath / GenerateFileName(FileContentType::Classes, GetPackageObject()));
    PrintFileHeader(os, { GenerateFileName(FileContentType::Structs, GetPackageObject()) }, true);

    if (!Classes.empty()) {
        PrintSectionHeader(os, _XOR_("Classes"));
        for (auto&& c : Classes) {
            PrintClass(os, c);
            os << '\n';
        }
    }

    PrintFileFooter(os);
}

void GeneratorPackage::SaveFunctions(const std::experimental::filesystem::path& SdkPath) const
{
    if (Generator->ShouldGenerateFunctionParametersFile()) {
        SaveFunctionParameters(SdkPath);
    }

    std::ofstream os(SdkPath / GenerateFileName(FileContentType::Functions, GetPackageObject()));
    PrintFileHeader(os, { GenerateFileName(FileContentType::Classes, GetPackageObject()) }, false);
    PrintSectionHeader(os, _XOR_("Functions"));

    for (auto&& s : ScriptStructs) {

        // Predefined static members.
        for (auto&& m : s.StaticMembers) {
            os << m.Type << ' ' << s.NameCpp << _XORSTR_("::") << m.Name;
            if (!m.Definition.empty()) {
                os << _XORSTR_(" = ") << m.Definition;
            }
            os << _XORSTR_(";\n");
        }

        // Predefined struct methods.
        for (auto&& m : s.PredefinedMethods) {
            if (m.MethodType != GeneratorPredefinedMethod::Type::Inline) {
                os << m.Body << "\n\n";
            }
        }
    }

    for (auto&& c : Classes) {

        // Predefined static members.
        for (auto&& m : c.StaticMembers) {
            os << m.Type << ' ' << c.NameCpp << _XORSTR_("::") << m.Name;
            if (!m.Definition.empty()) {
                os << _XORSTR_(" = ") << m.Definition;
            }
            os << _XORSTR_(";\n");
        }

        // Predefined class methods.
        for (auto&& m : c.PredefinedMethods) {
            if (m.MethodType != GeneratorPredefinedMethod::Type::Inline) {
                os << m.Body << "\n\n";
            }
        }

        // Generate methods.
        for (auto&& m : c.Methods) {
            os << "// " << m.FullName << '\n' << "// (" << m.FlagsString << ')';
            if (!m.Parameters.empty()) {
                os << _XORSTR_("\n// Parameters:");
                for (auto&& param : m.Parameters) {
                    tfm::format(os, _XOR_("\n// %-30s %-30s (%s)"), param.CppType, param.Name, param.FlagsString);
                }
            }
            os << '\n';
            os << BuildMethodSignature(&c, m, false, true) << '\n';
            os << BuildMethodBody(&c, m) << "\n\n";
        }
    }

    PrintFileFooter(os);
}

void GeneratorPackage::SaveFunctionParameters(const std::experimental::filesystem::path& path) const
{
    std::ofstream os(path / GenerateFileName(FileContentType::FunctionParameters, GetPackageObject()));
    PrintFileHeader(os, { _XORSTR_("\"../sdk.h\"") }, true);
    PrintSectionHeader(os, _XOR_("Parameters"));

    for (auto&& c : Classes) {
        for (auto&& m : c.Methods) {
            os << "// " << m.FullName << "\n";
            tfm::format(os, "struct %s_%s_Params\n{\n", c.NameCpp, m.Name);
            for (auto&& param : m.Parameters) {
                tfm::format(os, "\t%-50s %-58s// (%s)\n", param.CppType, param.Name + ";", param.FlagsString);
            }
            os << "};\n\n";
        }
    }

    PrintFileFooter(os);
}

bool GeneratorPackage::Save() const
{
    std::experimental::filesystem::path SdkPath = Generator->GetPath() / "gen";

    if ((from(Enums) >> where([](auto&& e) { return !e.Values.empty(); }) >> any()) ||
        (from(ScriptStructs) >> where([](auto&& s) { return !s.Members.empty() || !s.StaticMembers.empty() || !s.PredefinedMethods.empty(); }) >> any()) ||
        (from(Classes) >> where([](auto&& c) { return !c.Members.empty() || !c.StaticMembers.empty() || !c.PredefinedMethods.empty() || !c.Methods.empty(); }) >> any()))
    {
        SaveStructs(SdkPath);
        SaveClasses(SdkPath);
        SaveFunctions(SdkPath);
        return true;
    }

    //LOG_DBG_PRINT("Skipped empty Package: %s\n", ObjectProxy(PackageObject).GetFullName().c_str());
    return false;
}

void Generator::SaveSdkHeader(
    const std::unordered_map<const UObject*, bool>& ProcessedObjects,
    const std::vector<std::unique_ptr<GeneratorPackage>>& Packages
)
{
    const std::experimental::filesystem::path& path = GetPath();
    std::ofstream os(path / "PUBG_SDK.h");
    os << _XORSTR_("#pragma once\n\n") << tfm::format(_XOR_("// PUBG UE4 SDK\n\n"));

    //check for missing structs
    const auto missing = from(ProcessedObjects) >> where([](auto&& kv) { return kv.second == false; });
    if (missing >> any()) {
        std::ofstream os2(path / "gen" / tfm::format(_XOR_("PUBG_MISSING.h")));
        PrintFileHeader(os2, true);
        for (auto&& s : missing >> select([](auto&& kv) { return static_cast<const UStruct*>(kv.first); }) >> experimental::container()) {
            os2 << "// " << s->GetFullName() << _XORSTR_("\n// ");
            os2 << tfm::format("0x%04X\n", s->GetPropertiesSize());
            os2 << _XORSTR_("struct ") << fmt::MakeValidName(s->GetNameCPP()) << "\n{\n";
            os2 << _XORSTR_("    uint8_t UnknownData[0x") << tfm::format("%X", s->GetPropertiesSize()) << _XORSTR_("];\n};\n\n");
        }
        PrintFileFooter(os2);
        os << _XORSTR_("\n#include \"gen/") << tfm::format(_XOR_("PUBG_MISSING.h")) << "\"\n";
    }
    os << '\n';
    for (auto&& Package : Packages) {
        os << _XORSTR_("#include \"gen/") << GenerateFileName(FileContentType::Classes, Package->GetPackageObject()) << "\"\n";
    }
}

bool Generator::Generate()
{
    // Create the generated sdk 
    std::experimental::filesystem::create_directories(GetPath() / "gen");

    std::vector<std::unique_ptr<GeneratorPackage>> Packages;
    std::unordered_map<const UObject*, bool> ProcessedObjects;

    ObjectsProxy Objects;
    std::vector<const UPackage*> PackageObjects = from(Objects)
        >> select([](auto&& o) { return o->GetOutermost(); })
        >> where([](auto&& o) { return o != nullptr; })
        >> distinct()
        >> to_vector();

    //for (int32_t i = 0; i < Objects.GetNum(); ++i) {
    //    ObjectProxy Object = Objects.GetById(i);
    //    if (Object.IsValid()) {
    //        const UPackage* Outermost = Object.GetOutermost();
    //        if (Outermost) {
    //            PackageObjects.push_back(Outermost);
    //        }
    //    }
    //}
    //LOG_INFO(_XOR_("MADEIT0"));
    //for (const UPackage* PackageObj : PackageObjects) {
    //    ObjectProxy Object = PackageObj;
    //    LOG_DBG_PRINT("Package object %d = %s\n",
    //                  Object.GetUniqueId(), Object.GetFullName().c_str());
    //}
    //LOG_INFO(_XOR_("MADEIT4"));
    //LOG_DBG_PRINT("DONE\n");

    // Generate classes, structs, and enums for each package object.
    for (const UPackage* PackageObject : PackageObjects) {

        // Create new generator package object.
        std::unique_ptr<GeneratorPackage> Package = std::make_unique<GeneratorPackage>(
                                                                    this, PackageObject);
        // Process this package
        Package->Process(Objects, ProcessedObjects);

        // Save the generated classes, structs, and enums of this package.
        if (Package->Save()) {
            Generator::PackageMap[PackageObject] = Package.get();
            Packages.emplace_back(std::move(Package));
        }
    }

    LOG_INFO(_XOR_("Donezo."));

    // Sort the packages.
    if (!Packages.empty()) {
        // std::sort doesn't work, so use a simple bubble sort
        const GeneratorPackageDependencyComparer comparer;
        for (size_t i = 0; i < Packages.size() - 1; ++i) {
            for (size_t j = 0; j < Packages.size() - i - 1; ++j) {
                if (!comparer(Packages[j], Packages[j + 1])) {
                    std::swap(Packages[j], Packages[j + 1]);
                }
            }
        }
    }

    // Save the SDK header.
    SaveSdkHeader(ProcessedObjects, Packages);

    return true;
}