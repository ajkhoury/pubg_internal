#pragma once

#include "Objects.h"
#include "Properties.h"
#include "cpplinq.h"
#include "tinyformat.h"

#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <bitset>


void GeneratorProcessPackages(const std::experimental::filesystem::path& path);

enum class FileContentType {
    Structs,
    Classes,
    Functions,
    FunctionParameters
};

struct GeneratorEnum {
    std::string Name;
    std::string FullName;
    std::vector<std::string> Values;
};

struct GeneratorMember {
    std::string Name;
    std::string Type;
    int32_t Offset;
    int32_t Size;
    uint64_t Flags;
    std::string FlagsString;
    std::string Comment;
};

struct GeneratorScriptStruct {
    std::string Name;
    std::string FullName;
    std::string NameCpp;
    std::string NameCppFull;
    int32_t Size;
    int32_t InheritedSize;
    std::vector<GeneratorMember> Members;
};

struct GeneratorParameter {
    enum class Type {
        Default,
        Out,
        Return
    };

    Type ParamType;
    bool PassByReference;
    std::string CppType;
    std::string Name;
    std::string FlagsString;

    /**
     * Generates a valid type of the property flags.
     * @param[in] flags The property flags.
     * @param[out] type The parameter type.
     * @returns true if it is a valid type, else false.
     */
    static bool MakeType(PropertyFlags flags, Type& type);
};

struct GeneratorMethod {
    size_t Index;
    std::string Name;
    std::string FullName;
    std::vector<GeneratorParameter> Parameters;
    std::string FlagsString;
    bool IsNative;
    bool IsStatic;
};

struct GeneratorClass : GeneratorScriptStruct {
    std::vector<std::string> VirtualFunctions;
    std::vector<GeneratorMethod> Methods;
};


class GeneratorPackage {
public:
    GeneratorPackage(const UPackage* PackageObj)
        : PackageObject(const_cast<UPackage*>(PackageObj))
    {
    }

    inline GeneratorPackage& operator=(const UPackage* InPackage) { PackageObject = const_cast<UPackage*>(InPackage); return *this; }
    inline GeneratorPackage& operator=(const GeneratorPackage& InPackage) { PackageObject = InPackage.PackageObject; return *this; }

    inline UPackage* GetPackageObject() const { return PackageObject; }
    inline std::string GetName() const { return ObjectProxy(PackageObject).GetName(); }

    // Process the classes the Package contains.
    void Process(const ObjectsProxy& Objects, std::unordered_map<const UObject*, bool>& ProcessedObjects);

    // Saves the Package classes as C++ code.
    bool Save(const std::experimental::filesystem::path& path) const;


private:
    // Add object to the dependency objects list.
    inline bool AddDependency(const UPackage* Package) const
    {
        if (Package != PackageObject) {
            DependencyObjects.insert(Package);
            return true;
        }
        return false;
    }

    // Checks and generates the prerequisites of the object.
    void GeneratePrerequisites(const ObjectProxy& Obj, std::unordered_map<const UObject*, bool>& ProcessedObjects);
    // Checks and generates the prerequisites of the members.
    void GenerateMemberPrerequisites(const PropertyProxy& First, std::unordered_map<const UObject*, bool>& ProcessedObjects);
    // Generates an enum.
    void GenerateEnum(const EnumProxy& Enum);
    // Generates the methods of a class.
    void GenerateMethods(const ClassProxy& classObj, std::vector<GeneratorMethod>& methods) const;
    // Generates the members of a struct or class.
    void GenerateMembers(const StructProxy& Struct, int32_t Offset, const std::vector<PropertyProxy>& Props, std::vector<GeneratorMember>& Members) const;
    // Generates a class.
    void GenerateClass(const ClassProxy& Class);
    // Generates a script structure.
    void GenerateScriptStruct(const ScriptStructProxy& ScriptStruct);


    // Writes all structs into the appropriate file.
    void SaveStructs(const std::experimental::filesystem::path& path) const;
    // Writes all classes into the appropriate file.
    void SaveClasses(const std::experimental::filesystem::path& path) const;
    // Writes all functions into the appropriate file.
    void SaveFunctions(const std::experimental::filesystem::path& path) const;


    UPackage* PackageObject;
    mutable std::unordered_set<const UPackage*> DependencyObjects;

    // Prints the c++ code of the constant.
    void PrintConstant(std::ostream& os, const std::pair<std::string, std::string>& Constant) const;
    std::unordered_map<std::string, std::string> Constants;

    // Prints the c++ code of the enum.
    void PrintEnum(std::ostream& os, const GeneratorEnum& Enum) const;
    std::vector<GeneratorEnum> Enums;

    // Print the C++ code of the structure.
    void PrintStruct(std::ostream& os, const GeneratorScriptStruct& ScriptStruct) const;
    std::vector<GeneratorScriptStruct> ScriptStructs;

    // Print the C++ code of the class.
    void PrintClass(std::ostream& os, const GeneratorClass& Class) const;
    std::vector<GeneratorClass> Classes;

};

namespace std {
template<>
struct hash<GeneratorPackage> {
    size_t operator()(const GeneratorPackage& package) const {
        return std::hash<void*>()(package.GetPackageObject());
    }
};
}

inline bool operator==(const GeneratorPackage& lhs, const GeneratorPackage& rhs)
{ 
    return rhs.GetPackageObject() == lhs.GetPackageObject();
}

inline bool operator!=(const GeneratorPackage& lhs, const GeneratorPackage& rhs)
{
    return !(lhs == rhs);
}
