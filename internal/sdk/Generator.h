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

struct GeneratorPredefinedMember {
    std::string Type;
    std::string Name;
};

struct GeneratorPredefinedMethod {
    enum class Type {
        Default,
        Inline
    };

    std::string Signature;
    std::string Body;
    Type MethodType;

    // Adds a predefined method which gets split in declaration and definition.
    static GeneratorPredefinedMethod Default(std::string&& signature, std::string&& body) { return { signature, body, Type::Default }; }
    // Adds a predefined method which gets included as an inline method.
    static GeneratorPredefinedMethod Inline(std::string&& body) { return { std::string(), body, Type::Inline }; }
};

struct GeneratorScriptStruct {
    std::string Name;
    std::string FullName;
    std::string NameCpp;
    std::string NameCppFull;
    int32_t Size;
    int32_t InheritedSize;
    std::vector<GeneratorMember> Members;
    std::vector<GeneratorPredefinedMethod> PredefinedMethods;
};

struct GeneratorClass : GeneratorScriptStruct {
    std::vector<std::string> VirtualFunctions;
    std::vector<GeneratorMethod> Methods;
};

typedef std::vector<std::tuple<size_t, const char*, const char*>> GeneratorPattern;

class GeneratorPackage;

class Generator {
public:
    // Constructor which sets generator options.
    Generator(const std::experimental::filesystem::path& InPath,
              bool InShouldUseStrings = false,
              bool InShouldXorStrings = false,
              bool InConvertStaticMethods = false,
              bool InGenerateFunctionParametersFile = false)
        : Path(InPath)
        , bShouldUseStrings(InShouldUseStrings)
        , bShouldXorStrings(InShouldXorStrings)
        , bConvertStaticMethods(InConvertStaticMethods)
        , bGenerateFunctionParametersFile(InGenerateFunctionParametersFile)
    {
    }

    // Check if the generated classes should use strings to identify objects.
    // If false the generated classes use the object index.
    // Warning: The object index may change for non default classes.
    bool ShouldUseStrings() const { return bShouldUseStrings; }

    // Check if strings should be xor encoded.
    bool ShouldXorStrings() const { return bShouldXorStrings; }

    // Check if static methods should get converted to normal methods.
    // Static methods require a CreateDefaultObject() method in the UObject class.
    bool ShouldConvertStaticMethods() const { return bConvertStaticMethods; }

    // Check if we should generate a function parameters file.
    // Otherwise the parameters are declared inside the function body.
    // If hooks with access to the parameters are need, this method should return true.
    bool ShouldGenerateFunctionParametersFile() const { return bGenerateFunctionParametersFile; }

    // Process the packages.
    void ProcessPackages(const std::experimental::filesystem::path& path);

    inline const std::experimental::filesystem::path& GetPath() const { return Path; }

public:

    // Public static fields.
    static std::unordered_map<const UPackage*, const GeneratorPackage*> PackageMap;
    static std::unordered_map<std::string, int32_t> AlignasClasses;

    // Gets alignas size for the specific class.
    static inline size_t GetClassAlignas(const std::string& name)
    {
        auto it = AlignasClasses.find(name);
        if (it != std::end(AlignasClasses)) {
            return it->second;
        }
        return 0;
    }

    // Gets the predefined members of the specific class.
    static bool GetPredefinedClassMembers(const std::string& name,
                                          std::vector<GeneratorPredefinedMember>& members)
    {
        auto it = PredefinedMembers.find(name);
        if (it != std::end(PredefinedMembers)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(members));
            return true;
        }
        return false;
    }

    // Gets the static predefined members of the specific class.
    static bool GetPredefinedClassStaticMembers(const std::string& name,
                                                std::vector<GeneratorPredefinedMember>& members)
    {
        auto it = PredefinedStaticMembers.find(name);
        if (it != std::end(PredefinedStaticMembers)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(members));
            return true;
        }
        return false;
    }

    // Gets the predefined methods of the specific class.
    static bool GetPredefinedClassMethods(const std::string& name,
                                          std::vector<GeneratorPredefinedMethod>& methods)
    {
        auto it = PredefinedMethods.find(name);
        if (it != std::end(PredefinedMethods)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(methods));
            return true;
        }
        return false;
    }

    // Gets the patterns of virtual functions of the specific class.
    // The generator loops the virtual functions of the class and adds a class method if the pattern matches.
    static bool GetVirtualFunctionPatterns(const std::string& name, GeneratorPattern& patterns)
    {
        auto it = VirtualFunctionPatterns.find(name);
        if (it != std::end(VirtualFunctionPatterns)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(patterns));
            return true;
        }
        return false;
    }

private:
    void SaveSdkHeader(const std::experimental::filesystem::path& Path,
                       const std::unordered_map<const UObject*, bool>& ProcessedObjects,
                       const std::vector<std::unique_ptr<GeneratorPackage>>& Packages);

    // Private fields.
    std::experimental::filesystem::path Path;
    bool bShouldUseStrings;
    bool bShouldXorStrings;
    bool bConvertStaticMethods;
    bool bGenerateFunctionParametersFile;

    // Private static fields.
    static std::unordered_map<std::string, std::vector<GeneratorPredefinedMember>> PredefinedMembers;
    static std::unordered_map<std::string, std::vector<GeneratorPredefinedMember>> PredefinedStaticMembers;
    static std::unordered_map<std::string, std::vector<GeneratorPredefinedMethod>> PredefinedMethods;
    static std::unordered_map<std::string, GeneratorPattern> VirtualFunctionPatterns;
};

class GeneratorPackage {

    friend struct std::hash<GeneratorPackage>;
    friend bool operator==(const GeneratorPackage& lhs, const GeneratorPackage& rhs);
    friend struct GeneratorPackageDependencyComparer;

public:
    GeneratorPackage(const Generator* InGenerator, const UPackage* PackageObj)
        : Generator(InGenerator)
        , PackageObject(const_cast<UPackage*>(PackageObj))
    {
    }

    inline GeneratorPackage& operator=(const UPackage* InPackage) { PackageObject = const_cast<UPackage*>(InPackage); return *this; }
    inline GeneratorPackage& operator=(const GeneratorPackage& InPackage) { PackageObject = InPackage.PackageObject; return *this; }

    //inline operator const UPackage*() const { return PackageObject; }
    inline UPackage* GetPackageObject() const { return PackageObject; }
    inline std::string GetName() const { return ObjectProxy(PackageObject).GetName(); }

    // Process the classes the Package contains.
    void Process(const ObjectsProxy& Objects, std::unordered_map<const UObject*, bool>& ProcessedObjects);

    // Saves the Package classes as C++ code.
    bool Save(const std::experimental::filesystem::path& SdkPath) const;

private:
    // Add object to the dependency objects list.
    inline bool AddDependency(const UPackage* Package) const
    {
        if (Package && Package != PackageObject) {
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
    void SaveStructs(const std::experimental::filesystem::path& SdkPath) const;
    // Writes all classes into the appropriate file.
    void SaveClasses(const std::experimental::filesystem::path& SdkPath) const;
    // Writes all functions into the appropriate file.
    void SaveFunctions(const std::experimental::filesystem::path& SdkPath) const;
    // Writes all function parameters into the appropriate file.
    void SaveFunctionParameters(const std::experimental::filesystem::path& SdkPath) const;

    // The parent generator instance.
    const Generator* Generator;

    // The package object describing this generator package.
    UPackage* PackageObject;
    // Dependency objects of this package object.
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

struct GeneratorPackageDependencyComparer {
    bool operator()(const std::unique_ptr<GeneratorPackage>& lhs,
                    const std::unique_ptr<GeneratorPackage>& rhs) const
    {
        return operator()(*lhs, *rhs);
    }

    bool operator()(const GeneratorPackage* lhs, const GeneratorPackage* rhs) const
    {
        return operator()(*lhs, *rhs);
    }

    bool operator()(const GeneratorPackage& lhs, const GeneratorPackage& rhs) const
    {
        if (rhs.DependencyObjects.empty()) {
            return false;
        }

        if (std::find(std::begin(rhs.DependencyObjects),
                      std::end(rhs.DependencyObjects),
                      lhs.PackageObject) != std::end(rhs.DependencyObjects)) {
            return true;
        }

        for (const auto dep : rhs.DependencyObjects) {
            const auto Package = Generator::PackageMap[dep];
            if (!Package)
                continue; // Missing package, should not occur...
            if (operator()(lhs, *Package))
                return true;
        }

        return false;
    }
};
