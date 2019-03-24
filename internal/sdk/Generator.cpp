#include "Generator.h"
#include "Objects.h"
#include "Properties.h"
#include "Format.h"
#include "FunctionFlags.h"

#include <native/log.h>
#include <utils/xorstr.h>

std::unordered_map<const UPackage*, const GeneratorPackage*> GeneratorPackage::PackageMap;

static std::unordered_map<std::string, int32_t> GeneratorAlignasClasses = {
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

std::string GenerateFileName(const FileContentType type, const UPackage* PackageObject)
{
    const char* name;
    switch (type) {
    case FileContentType::Structs:
        name = _XOR_("%s_%s_structs.h");
        break;
    case FileContentType::Classes:
        name = _XOR_("%s_%s_classes.h");
        break;
    case FileContentType::Functions:
        name = _XOR_("%s_%s_functions.cpp");
        break;
    case FileContentType::FunctionParameters:
        name = _XOR_("%s_%s_parameters.h");
        break;
    default:
        assert(false);
    }

    return tfm::format(name, _XOR_("PUBG"), ObjectProxy(PackageObject).GetName());
}

bool GeneratorParameter::MakeType(PropertyFlags flags, GeneratorParameter::Type& type)
{
    if (flags & PropertyFlags::ReturnParm) {
        type = Type::Return;
    } else if (flags & PropertyFlags::OutParm) {
        //if it is a const parameter make it a default parameter
        if (flags & PropertyFlags::ConstParm) {
            type = Type::Default;
        } else {
            type = Type::Out;
        }
    } else if (flags & PropertyFlags::Parm) {
        type = Type::Default;
    } else {
        return false;
    }
    return true;
}

bool ComparePropertyLess(const PropertyProxy& lhs, const PropertyProxy& rhs)
{
    if (lhs.GetOffset() == rhs.GetOffset() &&
        lhs.IsA<BoolPropertyProxy>() &&
        rhs.IsA<BoolPropertyProxy>()) {
        return lhs.Cast<BoolPropertyProxy>() < rhs.Cast<BoolPropertyProxy>();
    }
    return lhs.GetOffset() < rhs.GetOffset();
}

void GeneratorPackage::GenerateEnum(const EnumProxy& Enum)
{
    GeneratorEnum e;
    e.Name = MakeUniqueCppName(Enum.Get<UEnum>());
    if (e.Name.find(_XOR_("Default__")) != std::string::npos ||
        e.Name.find(_XOR_("PLACEHOLDER-CLASS")) != std::string::npos) {
        return;
    }
    e.FullName = Enum.GetFullName();

    LOG_DBG_PRINT("Enum:          %-98s - instance: 0x%p\n", e.FullName.c_str(), Enum.GetAddress());

    std::unordered_map<std::string, int> conflicts;
    for (auto&& s : Enum.GetNames()) {
        const auto clean = MakeValidName(std::move(s));
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

void GeneratorPackage::GenerateMethods(const ClassProxy& ClassObj, std::vector<GeneratorMethod>& methods) const
{
    //some classes (AnimBlueprintGenerated...) have multiple Members with the same name, so filter them out
    std::unordered_set<std::string> UniqueMethods;

    FieldProxy ClassChildren = ClassObj.GetChildren();
    PropertyProxy Prop = ClassChildren.Cast<PropertyProxy>();
    while (Prop.IsValid()) {

        if (Prop.IsA<FunctionProxy>()) {
            FunctionProxy Function = Prop.Cast<FunctionProxy>();

            GeneratorMethod m;
            m.Index = Function.GetUniqueId();
            m.FullName = Function.GetFullName();
            m.Name = MakeValidName(Function.GetName());

            if (UniqueMethods.find(m.FullName) != std::end(UniqueMethods)) {
                continue;
            }
            UniqueMethods.insert(m.FullName);

            m.IsNative = Function.GetFunctionFlags() & FunctionFlags::Native;
            m.IsStatic = Function.GetFunctionFlags() & FunctionFlags::Static;
            m.FlagsString = StringifyFlags(Function.GetFunctionFlags());

            std::vector<std::pair<PropertyProxy, GeneratorParameter>> Parameters;

            std::unordered_map<std::string, size_t> Unique;
            FieldProxy FunctionChildren = Function.GetChildren();
            PropertyProxy Param = FunctionChildren.Cast<PropertyProxy>();
            while (Param.IsValid()) {
                if (Param.GetElementSize() == 0) {
                    continue;
                }

                const PropertyInfo ParamInfo = Param.GetInfo();
                if (ParamInfo.Type != PropertyType::Unknown) {

                    GeneratorParameter p;

                    if (!GeneratorParameter::MakeType(Param.GetPropertyFlags(), p.ParamType)) {
                        // child isn't a parameter
                        continue;
                    }

                    p.PassByReference = false;
                    p.Name = MakeValidName(Param.GetName());

                    const auto it = Unique.find(p.Name);
                    if (it == std::end(Unique)) {
                        Unique[p.Name] = 1;
                    } else {
                        ++Unique[p.Name];
                        p.Name += tfm::format("%02d", it->second);
                    }

                    p.FlagsString = StringifyFlags(Param.GetPropertyFlags());

                    p.CppType = ParamInfo.CppType;
                    if (Param.IsA<BoolPropertyProxy>()) {
                        p.CppType = _XORSTR_("bool");
                    }
                    switch (p.ParamType) {
                    case GeneratorParameter::Type::Default:
                        if (Prop.GetArrayDim() > 1) {
                            p.CppType = p.CppType + '*';
                        } else if (ParamInfo.CanBeReference) {
                            p.PassByReference = true;
                        }
                        break;
                    }

                    Parameters.emplace_back(std::make_pair(Prop, std::move(p)));
                }

                Param = FieldProxy(Param.GetNext()).Cast<PropertyProxy>();
            }

            std::sort(std::begin(Parameters), std::end(Parameters),
                      [](auto&& lhs, auto&& rhs) { return ComparePropertyLess(lhs.first, rhs.first); });

            for (auto& param : Parameters) {
                m.Parameters.emplace_back(std::move(param.second));
            }

            methods.emplace_back(std::move(m));
        }

        Prop = FieldProxy(Prop.GetNext()).Cast<PropertyProxy>();
    }

}

GeneratorMember CreatePadding(size_t id, int32_t offset, int32_t size, std::string reason)
{
    GeneratorMember m;
    m.Name = tfm::format(_XOR_("UnknownData%02d[0x%X]"), id, size);
    m.Type = _XORSTR_("uint8_t");
    m.Offset = offset;
    m.Size = size;
    m.Comment = std::move(reason);
    return m;
}

GeneratorMember CreateBitfieldPadding(size_t id, int32_t offset, std::string type, size_t bits)
{
    GeneratorMember m;
    m.Name = tfm::format(_XOR_("UnknownData%02d : %d"), id, bits);
    m.Type = std::move(type);
    m.Offset = offset;
    m.Size = 1;
    return m;
}

void GeneratorPackage::GenerateMembers(const StructProxy& Struct, int32_t Offset, const std::vector<PropertyProxy>& Props, std::vector<GeneratorMember>& Members) const
{
    std::unordered_map<std::string, size_t> UniqueMemberNames;
    size_t UnknownDataCounter = 0;
    BoolPropertyProxy PreviousBitfieldProperty(nullptr);

    for (auto&& Prop : Props) {
        if (Offset < Prop.GetOffset()) {
            PreviousBitfieldProperty = BoolPropertyProxy(nullptr);

            const auto size = Prop.GetOffset() - Offset;
            Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, size, _XORSTR_("MISSED OFFSET")));
        }

        const PropertyInfo PropInfo = Prop.GetInfo();
        if (PropInfo.Type != PropertyType::Unknown) {
            GeneratorMember sp;
            sp.Offset = Prop.GetOffset();
            sp.Size = PropInfo.Size;
            sp.Type = PropInfo.CppType;
            sp.Name = MakeValidName(Prop.GetName());

            const auto it = UniqueMemberNames.find(sp.Name);
            if (it == std::end(UniqueMemberNames)) {
                UniqueMemberNames[sp.Name] = 1;
            } else {
                ++UniqueMemberNames[sp.Name];
                sp.Name += tfm::format("%02d", it->second);
            }

            if (Prop.GetArrayDim() > 1) {
                sp.Name += tfm::format("[0x%X]", Prop.GetArrayDim());
            }

            if (Prop.IsA<BoolPropertyProxy>() && Prop.Cast<BoolPropertyProxy>().IsBitfield()) {
                BoolPropertyProxy BoolProp = Prop.Cast<BoolPropertyProxy>();
                const auto MissingBits = BoolProp.GetMissingBitsCount(PreviousBitfieldProperty);
                if (MissingBits[1] != -1) {
                    if (MissingBits[0] > 0) {
                        Members.emplace_back(CreateBitfieldPadding(UnknownDataCounter++, PreviousBitfieldProperty.GetOffset(), PropInfo.CppType, MissingBits[0]));
                    }
                    if (MissingBits[1] > 0) {
                        Members.emplace_back(CreateBitfieldPadding(UnknownDataCounter++, sp.Offset, PropInfo.CppType, MissingBits[1]));
                    }
                } else if (MissingBits[0] > 0) {
                    Members.emplace_back(CreateBitfieldPadding(UnknownDataCounter++, sp.Offset, PropInfo.CppType, MissingBits[0]));
                }

                PreviousBitfieldProperty = BoolProp;

                sp.Name += _XORSTR_(" : 1");
            } else {
                PreviousBitfieldProperty = BoolPropertyProxy(nullptr);
            }

            sp.Flags = static_cast<uint64_t>(Prop.GetPropertyFlags());
            sp.FlagsString = StringifyFlags(Prop.GetPropertyFlags());

            Members.emplace_back(std::move(sp));

            const auto sizeMismatch = static_cast<int>(Prop.GetElementSize() * Prop.GetArrayDim()) - static_cast<int>(PropInfo.Size * Prop.GetArrayDim());
            if (sizeMismatch > 0) {
                Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, sizeMismatch, _XORSTR_("FIX WRONG TYPE SIZE OF PREVIOUS PROPERTY")));
            }
        } else {
            const int32_t size = Prop.GetElementSize() * Prop.GetArrayDim();
            Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, size, _XORSTR_("UNKNOWN PROPERTY: ") + Prop.GetFullName()));
        }

        Offset = Prop.GetOffset() + Prop.GetElementSize() * Prop.GetArrayDim();
    }

    if (Offset < Struct.GetPropertiesSize()) {
        const int32_t size = Struct.GetPropertiesSize() - Offset;
        Members.emplace_back(CreatePadding(UnknownDataCounter++, Offset, size, _XORSTR_("MISSED OFFSET")));
    }
}

void GeneratorPackage::GenerateClass(const ClassProxy& ClassObj)
{
    GeneratorClass c;
    c.Name = ClassObj.GetName();
    c.FullName = ClassObj.GetFullName();

    LOG_DBG_PRINT("Class:          %-98s - instance: 0x%p\n", c.FullName.c_str(), ClassObj.GetAddress());

    c.NameCpp = MakeValidName(ClassObj.GetNameCPP());
    c.NameCppFull = _XORSTR_("class ") + c.NameCpp;

    c.Size = ClassObj.GetPropertiesSize();
    c.InheritedSize = 0;

    int32_t offset = 0;

    StructProxy SuperStruct = ClassObj.GetSuper();
    if (SuperStruct.IsValid() && SuperStruct != ClassObj) {
        c.InheritedSize = offset = SuperStruct.GetPropertiesSize();
        c.NameCppFull += _XORSTR_(" : public ") + MakeValidName(SuperStruct.GetNameCPP());
    }

    std::vector<PropertyProxy> Props;

    FieldProxy ClassChildren = ClassObj.GetChildren();
    PropertyProxy Prop = ClassChildren.Cast<PropertyProxy>();
    while (Prop.IsValid()) {
        if (Prop.GetElementSize() > 0 &&
            !Prop.IsA<ScriptStructProxy>() &&
            !Prop.IsA<FunctionProxy>() &&
            !Prop.IsA<EnumProxy>() &&
            (!SuperStruct.IsValid() ||
             (SuperStruct != ClassObj && Prop.GetOffset() >= SuperStruct.GetPropertiesSize()))) {
            Props.push_back(Prop);
        }
        Prop = FieldProxy(Prop.GetNext()).Cast<PropertyProxy>();
    }
    std::sort(std::begin(Props), std::end(Props), ComparePropertyLess);

    // Generate this class object's members/fields.
    GenerateMembers(ClassObj, offset, Props, c.Members);

    // Skip BlueprintGeneratedClass types methods since they seem to be neverending.
    if (c.FullName.find(_XOR_("BlueprintGeneratedClass ")) != std::string::npos) {
        LOG_INFO(_XOR_("BLUEPRINT GENERATED CLASS: %s"), c.Name.c_str());
    } else {
        // Generate this class object's methods.
        GenerateMethods(ClassObj, c.Methods);
    }

    Classes.emplace_back(std::move(c));
}

void GeneratorPackage::GenerateScriptStruct(const ScriptStructProxy& ScriptStruct)
{
    GeneratorScriptStruct ss;
    ss.Name = ScriptStruct.GetName();
    ss.FullName = ScriptStruct.GetFullName();

    LOG_DBG_PRINT("ScriptStruct: %-100s - instance: 0x%p\n", ss.Name.c_str(), ScriptStruct.GetAddress());

    ss.NameCpp = MakeValidName(ScriptStruct.GetNameCPP());
    ss.NameCppFull = _XORSTR_("struct ");
    // Some classes need special alignment
    auto it = GeneratorAlignasClasses.find(ss.FullName);
    if (it != std::end(GeneratorAlignasClasses)) {
        ss.NameCppFull += tfm::format(_XOR_("alignas(%d) "), it->second);
    }
    ss.NameCppFull += MakeUniqueCppName(ScriptStruct.Get<UStruct>());
    ss.Size = ScriptStruct.GetPropertiesSize();
    ss.InheritedSize = 0;

    int32_t Offset = 0;
    StructProxy SuperStruct = ScriptStruct.GetSuper();
    if (SuperStruct.IsValid() && SuperStruct.GetAddress() != ScriptStruct.GetAddress()) {
        ss.InheritedSize = Offset = SuperStruct.GetPropertiesSize();
        ss.NameCppFull += _XORSTR_(" : public ") + MakeUniqueCppName(SuperStruct.Get<UStruct>());
    }

    std::vector<PropertyProxy> Props;
    FieldProxy ScriptStructChildren = ScriptStruct.GetChildren();
    PropertyProxy Prop = ScriptStructChildren.Cast<PropertyProxy>();
    while (Prop.IsValid()) {
        if (Prop.GetElementSize() > 0 &&
            !Prop.IsA<ScriptStructProxy>() &&
            !Prop.IsA<FunctionProxy>() &&
            !Prop.IsA<EnumProxy>()) {
            Props.push_back(Prop);
        }
        Prop = FieldProxy(Prop.GetNext()).Cast<PropertyProxy>();
    }
    std::sort(std::begin(Props), std::end(Props), ComparePropertyLess);

    GenerateMembers(ScriptStruct, Offset, Props, ss.Members);

    ScriptStructs.emplace_back(std::move(ss));
}

void GeneratorPackage::GenerateMemberPrerequisites(const PropertyProxy& First, std::unordered_map<const UObject*, bool>& ProcessedObjects)
{
    using namespace cpplinq;

    PropertyProxy Prop = First;
    while (Prop.IsValid()) {

        const PropertyInfo PropInfo = Prop.GetInfo();

        if (PropInfo.Type == PropertyType::Primitive) {

            if (Prop.IsA<BytePropertyProxy>()) {

                BytePropertyProxy ByteProperty = Prop.Cast<BytePropertyProxy>();
                if (ByteProperty.IsEnum()) {
                    ObjectProxy Enum = ByteProperty.GetEnum();
                    AddDependency(Enum.GetOutermost());
                }

            } else if (Prop.IsA<EnumPropertyProxy>()) {

                EnumPropertyProxy EnumProperty = Prop.Cast<EnumPropertyProxy>();
                ObjectProxy Enum = EnumProperty.GetEnum();
                AddDependency(Enum.GetOutermost());
            }

        } else if (PropInfo.Type == PropertyType::CustomStruct) {

            GeneratePrerequisites(Prop.Cast<StructPropertyProxy>().GetStruct(), ProcessedObjects);

        } else if (PropInfo.Type == PropertyType::Container) {

            std::vector<PropertyProxy> InnerProperties;
            if (Prop.IsA<ArrayPropertyProxy>()) {
                InnerProperties.push_back(Prop.Cast<ArrayPropertyProxy>().GetInner());
            } else if (Prop.IsA<MapPropertyProxy>()) {
                MapPropertyProxy MapProp = Prop.Cast<MapPropertyProxy>();
                InnerProperties.push_back(MapProp.GetKeyProperty());
                InnerProperties.push_back(MapProp.GetValueProperty());
            }

            for (PropertyProxy InnerProp : from(InnerProperties)
                 >> where([](auto&& p) { return p.GetInfo().Type == PropertyType::CustomStruct; })
                 >> experimental::container()) {
                GeneratePrerequisites(InnerProp.Cast<StructPropertyProxy>().GetStruct(), ProcessedObjects);
            }

        } else if (Prop.IsA<FunctionProxy>()) {

            FunctionProxy Function = Prop.Cast<FunctionProxy>();
            FieldProxy FunctionChildren = Function.GetChildren();
            GenerateMemberPrerequisites(FunctionChildren.Cast<PropertyProxy>(), ProcessedObjects);
        }

        Prop = FieldProxy(Prop.GetNext()).Cast<PropertyProxy>();
    }
}

void GeneratorPackage::GeneratePrerequisites(const ObjectProxy& Obj, std::unordered_map<const UObject*, bool>& ProcessedObjects)
{
    if (!Obj.IsValid()) {
        return;
    }

    const bool isClass = Obj.IsA<ClassProxy>();
    const bool isScriptStruct = Obj.IsA<ScriptStructProxy>();
    if (!isClass && !isScriptStruct) {
        return;
    }

    const std::string ObjNameString = Obj.GetName();
    if (ObjNameString.find(_XOR_("Default__")) != std::string::npos ||
        ObjNameString.find(_XOR_("<uninitialized>")) != std::string::npos ||
        ObjNameString.find(_XOR_("PLACEHOLDER-CLASS")) != std::string::npos) {
        return;
    }

    ProcessedObjects[Obj.GetConstPtr()] |= false;

    const UPackage* ClassPackage = Obj.GetOutermost();
    if (!ClassPackage) {
        return;
    }

    if (AddDependency(ClassPackage)) {
        return;
    }

    if (ProcessedObjects[Obj.GetConstPtr()] == false) {
        ProcessedObjects[Obj.GetConstPtr()] = true;

        if (!isScriptStruct) {
            ObjectProxy Outer = Obj.GetOuter();
            if (Outer.IsValid() && Outer != Obj) {
                GeneratePrerequisites(Outer, ProcessedObjects);
            }
        }

        StructProxy StructObj = Obj.Cast<StructProxy>();
        StructProxy SuperStruct = StructObj.GetSuper();
        if (SuperStruct.IsValid() && SuperStruct != Obj) {
            GeneratePrerequisites(SuperStruct, ProcessedObjects);
        }

        FieldProxy StructChildren = StructObj.GetChildren();
        GenerateMemberPrerequisites(StructChildren.Cast<PropertyProxy>(), ProcessedObjects);

        if (isClass) {
            GenerateClass(Obj.Cast<ClassProxy>());
        } else {
            GenerateScriptStruct(Obj.Cast<ScriptStructProxy>());
        }
    }
}

void GeneratorPackage::Process(const ObjectsProxy& Objects, std::unordered_map<const UObject*, bool>& ProcessedObjects)
{
    for (ObjectProxy Object : Objects) {
        if (Object.IsValid()) {
            const UPackage* Package = Object.GetOutermost();
            if (PackageObject == Package) {

                if (Object.IsA<EnumProxy>()) {

                    //LOG_INFO("Processing enum \"%s\"...\n", Object.GetFullName().c_str());
                    GenerateEnum(Object.Cast<EnumProxy>());

                } else if (Object.IsA<ClassProxy>()) {

                    //LOG_INFO("Processing class \"%s\"...\n", Object.GetFullName().c_str());
                    GeneratePrerequisites(Object, ProcessedObjects);

                } else if (Object.IsA<ScriptStructProxy>()) {

                    //LOG_INFO("Processing struct \"%s\"...\n", Object.GetFullName().c_str());
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
    using namespace cpplinq;
    os << "// " << Enum.FullName << _XORSTR_("\nenum class ") << Enum.Name << _XORSTR_(" : uint8_t {\n");
    os << (from(Enum.Values)
           >> select([](auto&& name, auto&& i) { return tfm::format(_XOR_("    %-30s = %d"), name, i); })
           >> concatenate(",\n"))
        << _XORSTR_("\n};\n\n");
}

void GeneratorPackage::PrintStruct(std::ostream& os, const GeneratorScriptStruct& ScriptStruct) const
{
    using namespace cpplinq;

    os << "// " << ScriptStruct.FullName << "\n// ";
    if (ScriptStruct.InheritedSize) {
        os << tfm::format(_XOR_("0x%04X (0x%04X - 0x%04X)\n"), ScriptStruct.Size - ScriptStruct.InheritedSize, ScriptStruct.Size, ScriptStruct.InheritedSize);
    } else {
        os << tfm::format(_XOR_("0x%04X\n"), ScriptStruct.Size);
    }
    os << ScriptStruct.NameCppFull << _XORSTR_(" {\n");

    // Members
    os << (from(ScriptStruct.Members)
        >> select([](auto&& m) {
            return tfm::format(_XOR_("    %-50s %-58s// 0x%04X(0x%04X)"), m.Type, m.Name + ';', m.Offset, m.Size) +
                    (!m.Comment.empty() ? " " + m.Comment : "") +
                    (!m.FlagsString.empty() ? " (" + m.FlagsString + ')' : "");
        }) >> concatenate("\n"))
        << '\n';

    os << _XORSTR_("};\n");
}

std::string BuildMethodSignature(const GeneratorMethod& m, const GeneratorClass& c, bool inHeader)
{
    using namespace cpplinq;

    std::ostringstream ss;

    if (m.IsStatic && inHeader) {
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

    if (!inHeader) {
        ss << c.NameCpp << _XORSTR_("::");
    }
    if (m.IsStatic) {
        ss << _XORSTR_("STATIC_");
    }
    ss << m.Name;

    // Parameters
    ss << '(';
    ss << (from(m.Parameters)
           >> where([](auto&& param) { return param.ParamType != GeneratorParameter::Type::Return; })
           >> orderby([](auto&& param) { return param.ParamType; })
           >> select([](auto&& param) { return (param.PassByReference ? _XOR_("const ") : "") + param.CppType + (param.PassByReference ? "& " : param.ParamType == GeneratorParameter::Type::Out ? "* " : " ") + param.Name; })
           >> concatenate(", "));
    ss << ')';

    return ss.str();
}

std::string BuildMethodBody(const GeneratorClass& c, const GeneratorMethod& m)
{
    using namespace cpplinq;

    std::ostringstream ss;

    // Function Pointer
    ss << _XORSTR_("{\n    /*static*/ auto fn");

//if (generator->ShouldUseStrings()) {
    ss << _XORSTR_(" = UObject::FindObject<UFunction>(");
    ss << '\"' << m.FullName << '\"';
    ss << _XORSTR_(");\n\n");
//} else {
//    ss << _XORSTR_(" = UObject::GetObjectCasted<UFunction>(") << m.Index << _XORSTR_(");\n\n");
//}

    // Parameters
    ss << _XORSTR_("    struct {\n");
    for (auto&& param : m.Parameters) {
        tfm::format(ss, _XOR_("        %-30s %s;\n"), param.CppType, param.Name);
    }
    ss << _XORSTR_("    } params;\n");

    auto defaultParameters = from(m.Parameters) >> where([](auto&& param) { return param.ParamType == GeneratorParameter::Type::Default; });
    if (defaultParameters >> any()) {
        for (auto&& param : defaultParameters >> experimental::container()) {
            ss << _XORSTR_("    params.") << param.Name << " = " << param.Name << ";\n";
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

    if (m.IsStatic) {
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
    using namespace cpplinq;

    os << "// " << Class.FullName << "\n// ";
    if (Class.InheritedSize) {
        tfm::format(os, _XOR_("0x%04X (0x%04X - 0x%04X)\n"),
                    Class.Size - Class.InheritedSize, Class.Size, Class.InheritedSize);
    } else {
        tfm::format(os, _XOR_("0x%04X\n"), Class.Size);
    }
    os << Class.NameCppFull << _XORSTR_(" {\npublic:\n");

    // Members
    for (auto&& m : Class.Members) {
        tfm::format(os, _XOR_("    %-50s %-58s// 0x%04X(0x%04X)"), m.Type, m.Name + ';', m.Offset, m.Size);
        if (!m.Comment.empty()) {
            os << " " << m.Comment;
        }
        if (!m.FlagsString.empty()) {
            os << " (" << m.FlagsString << ')';
        }
        os << '\n';
    }

    // Methods
    if (!Class.Methods.empty()) {
        os << '\n';
        for (auto&& m : Class.Methods) {
            os << _XORSTR_("    ") << BuildMethodSignature(m, {}, true) << _XORSTR_(";\n");
        }
    }

    os << _XORSTR_("};\n\n");
}

void GeneratorPackage::SaveStructs(const std::experimental::filesystem::path& path) const
{
    using namespace cpplinq;

    std::ofstream os(path / GenerateFileName(FileContentType::Structs, GetPackageObject()));
    std::vector<std::string> includes{ { tfm::format(_XOR_("PUBG_Basic.h")) } };
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

void GeneratorPackage::SaveClasses(const std::experimental::filesystem::path& path) const
{
    std::ofstream os(path / GenerateFileName(FileContentType::Classes, GetPackageObject()));
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

void GeneratorPackage::SaveFunctions(const std::experimental::filesystem::path& path) const
{
    //SaveFunctionParameters(path);
    std::ofstream os(path / GenerateFileName(FileContentType::Functions, GetPackageObject()));
    PrintFileHeader(os, { GenerateFileName(FileContentType::Classes, GetPackageObject()) }, false);
    PrintSectionHeader(os, _XOR_("Functions"));

    for (auto&& s : ScriptStructs) {
        // Predefined struct methods.
        for (auto&& m : s.PredefinedMethods) {
            if (m.MethodType != GeneratorPredefinedMethod::Type::Inline) {
                os << m.Body << "\n\n";
            }
        }
    }

    for (auto&& c : Classes) {
        // Predefined class methods.
        for (auto&& m : c.PredefinedMethods) {
            if (m.MethodType != GeneratorPredefinedMethod::Type::Inline) {
                os << m.Body << "\n\n";
            }
        }
        // Generate methods.
        for (auto&& m : c.Methods) {
            os << "// " << m.FullName << '\n' << "// (" << m.FlagsString << ")\n";
            if (!m.Parameters.empty()) {
                os << _XORSTR_("// Parameters:\n");
                for (auto&& param : m.Parameters) {
                    tfm::format(os, _XOR_("// %-30s %-30s (%s)\n"), param.CppType, param.Name, param.FlagsString);
                }
            }
            os << '\n';
            os << BuildMethodSignature(m, c, false) << '\n';
            os << BuildMethodBody(c, m) << "\n\n";
        }
    }

    PrintFileFooter(os);
}

void GeneratorPackage::SaveFunctionParameters(const std::experimental::filesystem::path& path) const
{
    using namespace cpplinq;

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

bool GeneratorPackage::Save(const std::experimental::filesystem::path& path) const
{
    using namespace cpplinq;

    if ((from(Enums) >> where([](auto&& e) { return !e.Values.empty(); }) >> any()
         || from(ScriptStructs) >> where([](auto&& s) { return !s.Members.empty(); }) >> any()
         || from(Classes) >> where([](auto&& c) { return !c.Members.empty() || !c.Methods.empty(); }) >> any())
        ) {
        SaveStructs(path);
        SaveClasses(path);
        SaveFunctions(path);
        return true;
    }

    //LOG_DBG_PRINT("Skipped empty Package: %s\n", ObjectProxy(PackageObject).GetFullName().c_str());
    return false;
}

void GeneratorSaveSdkHeader(
    const std::experimental::filesystem::path& Path,
    const std::unordered_map<const UObject*, bool>& ProcessedObjects,
    const std::vector<std::unique_ptr<GeneratorPackage>>& Packages
)
{
    std::ofstream os(Path / "sdk.h");
    os << _XORSTR_("#pragma once\n\n") << tfm::format(_XOR_("// PUBG UE4 SDK\n\n"));

    using namespace cpplinq;

    //check for missing structs
    const auto missing = from(ProcessedObjects) >> where([](auto&& kv) { return kv.second == false; });
    if (missing >> any()) {
        std::ofstream os2(Path / "gen" / tfm::format(_XOR_("PUBG_MISSING.h")));
        PrintFileHeader(os2, true);
        for (auto&& s : missing >> select([](auto&& kv) { return ObjectProxy(kv.first).Cast<StructProxy>(); }) >> experimental::container()) {
            os2 << "// " << s.GetFullName() << _XORSTR_("\n// ");
            os2 << tfm::format("0x%04X\n", s.GetPropertiesSize());
            os2 << _XORSTR_("struct ") << MakeValidName(s.GetNameCPP()) << "\n{\n";
            os2 << _XORSTR_("    uint8_t UnknownData[0x") << tfm::format("%X", s.GetPropertiesSize()) << _XORSTR_("];\n};\n\n");
        }
        PrintFileFooter(os2);
        os << _XORSTR_("\n#include \"gen/") << tfm::format(_XOR_("PUBG_MISSING.h")) << "\"\n";
    }
    os << '\n';
    for (auto&& Package : Packages) {
        os << _XORSTR_("#include \"gen/") << GenerateFileName(FileContentType::Classes, Package->GetPackageObject()) << "\"\n";
    }
}

void GeneratorProcessPackages(const std::experimental::filesystem::path& path)
{
    using namespace cpplinq;

    const auto SdkPath = path / "gen";
    std::experimental::filesystem::create_directories(SdkPath);

    std::vector<std::unique_ptr<GeneratorPackage>> Packages;
    std::unordered_map<const UObject*, bool> ProcessedObjects;

    ObjectsProxy Objects;
    std::vector<const UPackage*> PackageObjects = from(Objects)
        >> select([](auto&& o) { return o.GetOutermost(); })
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

    for (auto PackageObject : PackageObjects) {
        //GeneratorPackage Package(PackageObject);
        auto Package = std::make_unique<GeneratorPackage>(PackageObject);
        Package->Process(Objects, ProcessedObjects);
        if (Package->Save(SdkPath)) {
            GeneratorPackage::PackageMap[PackageObject] = Package.get();
            Packages.emplace_back(std::move(Package));
        }
    }

    LOG_INFO(_XOR_("Donezo."));

    if (!Packages.empty()) {
        // std::sort doesn't work, so use a simple bubble sort
        //std::sort(std::begin(packages), std::end(packages), GeneratorPackageDependencyComparer());
        const GeneratorPackageDependencyComparer comparer;
        for (auto i = 0u; i < Packages.size() - 1; ++i) {
            for (auto j = 0u; j < Packages.size() - i - 1; ++j) {
                if (!comparer(Packages[j], Packages[j + 1])) {
                    std::swap(Packages[j], Packages[j + 1]);
                }
            }
        }
    }

    GeneratorSaveSdkHeader(path, ProcessedObjects, Packages);
}