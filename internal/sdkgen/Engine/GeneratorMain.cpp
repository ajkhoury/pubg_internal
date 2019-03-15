// Unreal Engine SDK Generator
// by KN4CK3R
// https://www.oldschoolhack.me

#include <windows.h>

#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <bitset>
#include "cpplinq.h"

#include "Logger.h"

#include "IGenerator.h"
#include "ObjectsStore.h"
#include "NamesStore.h"
#include "Package.h"
#include "NameValidator.h"
#include "PrintHelper.h"

#include "../../utils.h"

extern IGenerator* generator;

/// <summary>
/// Dumps the objects and names to files.
/// </summary>
/// <param name="path">The path where to create the dumps.</param>
void Dump(const std::experimental::filesystem::path& path)
{
    {
        std::ofstream o(path / "ObjectsDump.txt");
        tfm::format(o, "Address: 0x%P\n\n", ObjectsStore::GetAddress());

        for (auto obj : ObjectsStore()) {
            tfm::format(o, "[%06i] %-100s 0x%P\n", obj.GetIndex(), obj.GetFullName(), obj.GetAddress());
        }
    }

    {
        std::ofstream o(path / "NamesDump.txt");
        tfm::format(o, "Address: 0x%P\n\n", NamesStore::GetAddress());

        for (auto name : NamesStore()) {
            tfm::format(o, "[%06i] %s\n", name.Index, name.Name);
        }
    }
}

/// <summary>
/// Generates the sdk header.
/// </summary>
/// <param name="path">The path where to create the sdk header.</param>
/// <param name="processedObjects">The list of processed objects.</param>
/// <param name="packageOrder">The package order info.</param>
void SaveSDKHeader(
    const std::experimental::filesystem::path& path,
    const std::unordered_map<UEObject, bool>& processedObjects,
    const std::vector<Package>& packages
)
{
    std::ofstream os(path / "SDK.hpp");

    os << "#pragma once\n\n"
        << tfm::format("// %s (%s) SDK\n\n", generator->GetGameName(), generator->GetGameVersion());

    //include the basics
    {
        {
            std::ofstream os2(path / "SDK" / tfm::format("%s_Basic.hpp", generator->GetGameNameShort()));

            std::vector<std::string> includes{ { "<unordered_set>" }, { "<string>" } };

            auto&& generatorIncludes = generator->GetIncludes();
            includes.insert(includes.end(), std::begin(generatorIncludes), std::end(generatorIncludes));

            PrintFileHeader(os2, includes, true);

            os2 << generator->GetBasicDeclarations() << "\n";

            PrintFileFooter(os2);

            os << "\n#include \"SDK/" << tfm::format("%s_Basic.hpp", generator->GetGameNameShort()) << "\"\n";
        }
        {
            std::ofstream os2(path / "SDK" / tfm::format("%s_Basic.cpp", generator->GetGameNameShort()));

            PrintFileHeader(os2, { "../SDK.hpp" }, false);

            os2 << generator->GetBasicDefinitions() << "\n";

            PrintFileFooter(os2);
        }
    }

    using namespace cpplinq;

    //check for missing structs
    const auto missing = from(processedObjects) >> where([](auto&& kv) { return kv.second == false; });
    if (missing >> any()) {
        std::ofstream os2(path / "SDK" / tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()));

        PrintFileHeader(os2, true);

        for (auto&& s : missing >> select([](auto&& kv) { return kv.first.Cast<UEStruct>(); }) >> experimental::container()) {
            os2 << "// " << s.GetFullName() << "\n// ";
            os2 << tfm::format("0x%04X\n", s.GetPropertySize());

            os2 << "struct " << MakeValidName(s.GetNameCPP()) << "\n{\n";
            os2 << "\tunsigned char UnknownData[0x" << tfm::format("%X", s.GetPropertySize()) << "];\n};\n\n";
        }

        PrintFileFooter(os2);

        os << "\n#include \"SDK/" << tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()) << "\"\n";
    }

    os << "\n";

    for (auto&& package : packages) {
        os << R"(#include "SDK/)" << GenerateFileName(FileContentType::Classes, package) << "\"\n";
    }
}

/// <summary>
/// Process the packages.
/// </summary>
/// <param name="path">The path where to create the package files.</param>
void ProcessPackages(const std::experimental::filesystem::path& path)
{
    using namespace cpplinq;

    const auto sdkPath = path / "SDK";
    std::experimental::filesystem::create_directories(sdkPath);

    std::vector<Package> packages;

    std::unordered_map<UEObject, bool> processedObjects;

    auto packageObjects = from(ObjectsStore())
        >> select([](auto&& o) { return o.GetPackageObject(); })
        >> where([](auto&& o) { return o.IsValid(); })
        >> distinct()
        >> to_vector();

    for (auto obj : packageObjects) {
        Package package(obj);

        package.Process(processedObjects);
        if (package.Save(sdkPath)) {
            packages.emplace_back(std::move(package));
        }
    }

    SaveSDKHeader(path, processedObjects, packages);
}

DWORD WINAPI SdkGeneratorOnAttach(LPVOID lpParameter)
{
    if (!ObjectsStore::Initialize()) {
        fprintf(stderr, "ObjectsStore::Initialize failed\n");
        return -1;
    }
    if (!NamesStore::Initialize()) {
        fprintf(stderr, "NamesStore::Initialize failed\n");
        return -1;
    }

    if (!generator->Initialize(lpParameter)) {
        fprintf(stderr, "Initialize failed\n");
        return -1;
    }

    std::experimental::filesystem::path outputDirectory(generator->GetOutputDirectory());
    if (!outputDirectory.is_absolute()) {

        char buffer[2048];
        if (GetModuleFileNameA(static_cast<HMODULE>(lpParameter), buffer, sizeof(buffer)) == 0) {
            fprintf(stderr, "GetModuleFileName failed\n");
            return -1;
        }

        outputDirectory = std::experimental::filesystem::path(buffer).remove_filename() / outputDirectory;
    }

    outputDirectory /= generator->GetGameNameShort();
    std::experimental::filesystem::create_directories(outputDirectory);

    std::ofstream log(outputDirectory / "Generator.log");
    Logger::SetStream(&log);

    if (generator->ShouldDumpArrays()) {
        Dump(outputDirectory);
    }

    std::experimental::filesystem::create_directories(outputDirectory);

    const auto begin = std::chrono::system_clock::now();

    ProcessPackages(outputDirectory);

    Logger::Log("Finished, took %d seconds.", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - begin).count());

    Logger::SetStream(nullptr);

    fprintf(stdout, "Finished!\n");

    return 0;
}
