#pragma once

#include "Objects.h"
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
    size_t Offset;
    size_t Size;
    uint64_t Flags;
    std::string FlagsString;
    std::string Comment;
};

struct GeneratorStaticMember {
    std::string Name;
    std::string Type;
    std::string Definition;
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
    static bool MakeType(const uint64_t flags, Type& type);
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

struct GeneratorPredefinedStaticMember {

    std::string Type;
    std::string Name;
    std::string Definition;

    static GeneratorPredefinedStaticMember Inline(const std::string&& Type,
                                                  const std::string&& Name) { return { Type, Name, std::string() }; }
    static GeneratorPredefinedStaticMember Static(const std::string&& Type,
                                                  const std::string&& Name,
                                                  const std::string&& Definition) { return { Type, Name, Definition }; }
};

struct GeneratorPredefinedMember {

    std::string Type;
    std::string Name;
    size_t Size;
    size_t Offset;

    static GeneratorPredefinedMember Inline(const std::string&& Type, const std::string&& Name) { return { Type, Name, 0, (uint32_t)-1 }; }

    inline bool operator<(const GeneratorPredefinedMember& rhs) const { return Offset < rhs.Offset; }
    inline bool operator>(const GeneratorPredefinedMember& rhs) const { return Offset > rhs.Offset; }
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
    static GeneratorPredefinedMethod Default(const std::string&& Signature, const std::string&& Body) { return { Signature, Body, Type::Default }; }
    // Adds a predefined method which gets included as an inline method.
    static GeneratorPredefinedMethod Inline(const std::string&& Body) { return { std::string(), Body, Type::Inline }; }
};

struct GeneratorScriptStruct {
    std::string Name;
    std::string FullName;
    std::string NameCpp;
    std::string NameCppFull;
    uint32_t Size;
    uint32_t InheritedSize;
    bool bMembersPredefined;
    std::vector<GeneratorMember> Members;
    std::vector<GeneratorStaticMember> StaticMembers;
    std::vector<GeneratorPredefinedMethod> PredefinedMethods;
};

struct GeneratorClass : GeneratorScriptStruct {
    std::vector<std::string> VirtualFunctions;
    std::vector<GeneratorMethod> Methods;
};

struct GeneratorPattern {
    size_t SearchSize;
    std::string Pattern;
    std::string Function;
};

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
    bool Generate();

    // Get the sdk generator path.
    inline const std::experimental::filesystem::path& GetPath() const { return Path; }

    // Add a predefined core member for the specified object.
    inline void AddPredefinedClassMembers(const std::string& ObjectName,
                                          const std::vector<GeneratorPredefinedMember>& Members)
    {
        std::copy(std::begin(Members),
                  std::end(Members),
                  std::back_inserter(PredefinedMembers[ObjectName]));
    }

    // Add a predefined core member for the specified object.
    inline void AddPredefinedClassStaticMembers(const std::string& ObjectName,
                                                const std::vector<GeneratorPredefinedStaticMember>& Members)
    {
        std::copy(std::begin(Members),
                  std::end(Members),
                  std::back_inserter(PredefinedStaticMembers[ObjectName]));
    }

    // Add a predefined method for the specified object.
    inline void AddPredefinedMethods(const std::string& ObjectName,
                                     const std::vector<GeneratorPredefinedMethod>& Methods)
    {
        std::copy(std::begin(Methods),
                  std::end(Methods),
                  std::back_inserter(PredefinedMethods[ObjectName]));
    }

    inline void AddPredefinedMethod(const std::string& ObjectName,
                                    const GeneratorPredefinedMethod&& PredefinedMethod)
    {
        PredefinedMethods[ObjectName].push_back(PredefinedMethod);
    }

    // Add a virtual function pattern to search for the specified object.
    inline void AddVirtualFunctionPattern(const std::string& ObjectName,
                                          const size_t SearchSize,
                                          const std::string& Pattern,
                                          const std::string& Function)
    {
        VirtualFunctionPatterns[ObjectName].push_back({SearchSize, Pattern, Function});
    }

    // Gets the predefined members of the specific class.
    bool GetPredefinedClassMembers(const std::string& ClassName,
                                   std::vector<GeneratorPredefinedMember>& Members,
                                   bool bSorted = false) const
    {
        auto it = PredefinedMembers.find(ClassName);
        if (it != std::end(PredefinedMembers)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(Members));
            if (bSorted) {
                std::sort(std::begin(Members), std::end(Members));
            }
            return true;
        }
        return false;
    }

    // Gets the static predefined members of the specific class.
    bool GetPredefinedClassStaticMembers(const std::string& ClassName,
                                         std::vector<GeneratorPredefinedStaticMember>& Members) const
    {
        auto it = PredefinedStaticMembers.find(ClassName);
        if (it != std::end(PredefinedStaticMembers)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(Members));
            return true;
        }
        return false;
    }

    // Gets the predefined methods of the specific class.
    bool GetPredefinedClassMethods(const std::string& ClassName,
                                   std::vector<GeneratorPredefinedMethod>& Methods) const
    {
        auto it = PredefinedMethods.find(ClassName);
        if (it != std::end(PredefinedMethods)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(Methods));
            return true;
        }
        return false;
    }

    // Gets the patterns of virtual functions of the specific class.
    // The generator loops the virtual functions of the class and adds a class method if the pattern matches.
    bool GetVirtualFunctionPatterns(const std::string& name,
                                    std::vector<GeneratorPattern>& patterns) const
    {
        auto it = VirtualFunctionPatterns.find(name);
        if (it != std::end(VirtualFunctionPatterns)) {
            std::copy(std::begin(it->second), std::end(it->second), std::back_inserter(patterns));
            return true;
        }
        return false;
    }


public:
    // Public static fields.
    static std::unordered_map<class UPackage const*, const GeneratorPackage*> PackageMap;
    static std::unordered_map<std::string, uint32_t> AlignasClasses;

    // Gets alignas size for the specific class.
    static inline size_t GetClassAlignas(const std::string& name)
    {
        auto it = AlignasClasses.find(name);
        if (it != std::end(AlignasClasses)) {
            return it->second;
        }
        return 0;
    }

private:
    void SaveSdkHeader(const std::unordered_map<const UObject*, bool>& ProcessedObjects,
                       const std::vector<std::unique_ptr<GeneratorPackage>>& Packages);

    // Private fields.
    std::experimental::filesystem::path Path;
    bool bShouldUseStrings;
    bool bShouldXorStrings;
    bool bConvertStaticMethods;
    bool bGenerateFunctionParametersFile;

    std::unordered_map<std::string, std::vector<GeneratorPredefinedMember>> PredefinedMembers;
    std::unordered_map<std::string, std::vector<GeneratorPredefinedStaticMember>> PredefinedStaticMembers;
    std::unordered_map<std::string, std::vector<GeneratorPredefinedMethod>> PredefinedMethods;
    std::unordered_map<std::string, std::vector<GeneratorPattern>> VirtualFunctionPatterns;
};

class GeneratorPackage {

    friend struct std::hash<GeneratorPackage>;
    friend bool operator==(const GeneratorPackage& lhs, const GeneratorPackage& rhs);
    friend struct GeneratorPackageDependencyComparer;

public:
    GeneratorPackage(const Generator* InGenerator, class UPackage const* PackageObj)
        : Generator(InGenerator)
        , PackageObject(PackageObj)
    {
    }

    inline GeneratorPackage& operator=(class UPackage const* InPackage) { PackageObject = InPackage; return *this; }
    inline GeneratorPackage& operator=(const GeneratorPackage& InPackage) { PackageObject = InPackage.PackageObject; return *this; }

    inline class UPackage const* GetPackageObject() const { return PackageObject; }
    std::string GetName() const;

    // Process the classes the Package contains.
    void Process(const ObjectsProxy& Objects, std::unordered_map<const UObject*, bool>& ProcessedObjects);

    // Saves the Package classes as C++ code.
    bool Save() const;

private:
    // Add object to the dependency objects list.
    inline bool AddDependency(class UPackage const* Package) const
    {
        if (Package && Package != PackageObject) {
            DependencyObjects.insert(Package);
            return true;
        }
        return false;
    }

    // Checks and generates the prerequisites of the object.
    void GeneratePrerequisites(const UObject* Obj, std::unordered_map<const UObject*, bool>& ProcessedObjects);
    // Checks and generates the prerequisites of the members.
    void GenerateMemberPrerequisites(class UProperty const* First, std::unordered_map<class UObject const*, bool>& ProcessedObjects);
    // Generates an enum.
    void GenerateEnum(class UEnum const* Enum);
    // Generates the methods of a class.
    void GenerateMethods(class UClass const* classObj, std::vector<GeneratorMethod>& methods) const;
    // Generates the members of a struct or class.
    void GenerateMembers(class UStruct const* Struct, int32_t Offset, const std::vector<UProperty*>& Props, std::vector<GeneratorMember>& Members) const;
    // Generates a class.
    void GenerateClass(class UClass const* Class);
    // Generates a script structure.
    void GenerateScriptStruct(class UScriptStruct const* ScriptStruct);


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
    class UPackage const* PackageObject;
    // Dependency objects of this package object.
    mutable std::unordered_set<class UPackage const*> DependencyObjects;

    // Prints the c++ code of the constant.
    void PrintConstant(std::ostream& os, const std::pair<std::string, std::string>& Constant) const;
    std::unordered_map<std::string, std::string> Constants;

    // Prints the c++ code of the enum.
    void PrintEnum(std::ostream& os, const GeneratorEnum& Enum) const;
    std::vector<GeneratorEnum> Enums;

    // Print the C++ code of the structure.
    void PrintStruct(std::ostream& os, const GeneratorScriptStruct& ScriptStruct) const;
    std::vector<GeneratorScriptStruct> ScriptStructs;


    std::string BuildMethodSignature(const GeneratorClass* c, const GeneratorMethod& m, bool inHeader, bool makeStaticFuncVar) const;
    // Builds the C++ method body.
    std::string BuildMethodBody(const GeneratorClass* c, const GeneratorMethod& m) const;
    // Print the C++ code of the class.
    void PrintClass(std::ostream& os, const GeneratorClass& Class) const;
    std::vector<GeneratorClass> Classes;
};

namespace std {
template<>
struct hash<GeneratorPackage> {
    size_t operator()(const GeneratorPackage& package) const {
        return std::hash<const void*>()(package.GetPackageObject());
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
        if (rhs.DependencyObjects.empty())
            return false;

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
