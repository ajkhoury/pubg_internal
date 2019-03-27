#include <native/native.h>
#include <native/log.h>

#include <utils/utils.h>
#include <utils/disasm.h>
#include <utils/xorstr.h>

#include "sdk/Generator.h"
#include "sdk/OffsetDumper.h"
#include "Sdk.h"

#define DUMP_ONLY 0

static HANDLE MainThread = INVALID_HANDLE_VALUE;

static
NTSTATUS
NTAPI
TestThreadRoutine(
    IN HMODULE hModule
)
{
    int rc = NOERROR;
    LARGE_INTEGER WaitInterval;
    //PVOID Found;

    // Output basic image information.
    PVOID ImageBase = utils::GetModuleHandleWIDE(NULL /*0xC4D8736D TslGame.exe */);
    ULONG ImageSize = utils::GetModuleSize((HMODULE)ImageBase);
    LOG_DEBUG(_XOR_("ImageBase = 0x%016llx  ImageSize = 0x%08x"), ImageBase, ImageSize);

    // Wait for VMProtect to completely unpack.
    WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(8000));
    NtDelayExecution(FALSE, &WaitInterval);

#if defined(DUMP_ONLY)
    // Dump heuristic search results.
    rc |= unreal::DumpObjects();
    rc |= unreal::DumpNames();
    rc |= unreal::DumpWorld();
    rc |= unreal::DumpStructs();
    if (rc != NOERROR) {
        return STATUS_UNSUCCESSFUL;
    }
#endif // DUMP_ONLY

#if !DUMP_ONLY
    NamesProxy Names;
    if (!Names.GetAddress()) {
        LOG_ERROR(_XOR_("Failed to initialize Names!"));
        return STATUS_UNSUCCESSFUL;
    }
    
    ObjectsProxy Objects;
    if (!Objects.GetAddress()) {
        LOG_ERROR(_XOR_("Failed to initialize Objects!"));
        return STATUS_UNSUCCESSFUL;
    }
    
    WorldProxy World;
    if (!World.GetEncryptedPointerAddress()) {
        LOG_ERROR(_XOR_("Failed to initialize World!"));
        return STATUS_UNSUCCESSFUL;
    }
    
    int32_t LoopCount = 0;
    while(TRUE) {
    
        WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(1000));
        NtDelayExecution(FALSE, &WaitInterval);
    
        LOG_DEBUG(_XOR_("Names address = 0x%016llx"), Names.GetAddress());
        LOG_DEBUG(_XOR_("Names count = %d"), Names.GetNum());
    
        LOG_DEBUG(_XOR_("Objects address = 0x%016llx"), Objects.GetAddress());
        LOG_DEBUG(_XOR_("Objects count = %d"), Objects.GetNum());
    
        LOG_DEBUG(_XOR_("World address = 0x%016llx"), World.GetAddress());
        if (World.GetAddress()) {
            if (++LoopCount > 5)
                break;
        }
    
        //++NameId;
        //const char *NameData = Names.GetById(NameId);
        //if (NameData) {
        //    LOG_DEBUG(_XOR_("Names[%d] = \"%s\""), NameId, Names.GetById(NameId));
        //}
        //void **NamesData = static_cast<void **>(Names.GetAddress());
        //LOG_INFO(_XOR_("NamesData0 = 0x%016llx"), NamesData[0]);
        //LOG_INFO(_XOR_("NamesData1 = 0x%016llx"), NamesData[1]);
    }

    //ClassProxy UEnumStatic = Objects.FindClass("Class CoreUObject.Enum");
    //if (UEnumStatic.IsValid()) {
    //    LOG_DEBUG(_XOR_("UEnumStatic = 0x%p"), UEnumStatic.GetAddress());
    //}

    //for (int32_t ObjectIdx = 0; ObjectIdx < Objects.GetNum(); ObjectIdx++) {
    //    ObjectProxy Object = Objects.GetById(ObjectIdx);
    //    if (Object.IsValid()) {
    //        auto FullName = Object.GetFullName();
    //        if (!FullName.empty()) {
    //            uint32_t uniqueId = Object.GetUniqueId();
    //            LOG_INFO(_XOR_("Object[%d] = %s"), uniqueId, FullName.c_str());
    //        }
    //    }
    //}

    unreal::InitializeUnrealObjectSizeMap();

    Generator Gen(_XOR_("C:\\Users\\Owner\\Desktop\\sdk"), true, true);

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Object"), {
        { "void**",     "VTable",                   sizeof(void**),     0x00 },
        { "int32_t",    "ObjectFlagsEncrypted",     sizeof(int32_t),    unreal::ObjectFlagsEncryptedOffset },
        { "uint64_t",   "OuterEncrypted",           sizeof(uint64_t),   unreal::ObjectOuterEncryptedOffset },
        { "int32_t",    "InternalIndexEncrypted",   sizeof(int32_t),    unreal::ObjectInternalIndexEncryptedOffset },
        { "uint64_t",   "ClassEncrypted",           sizeof(uint64_t),   unreal::ObjectClassEncryptedOffset },
        { "int32_t",    "NameIndexEncrypted",       sizeof(int32_t),    unreal::ObjectNameIndexEncryptedOffset },
        { "int32_t",    "NameNumberEncrypted",      sizeof(int32_t),    unreal::ObjectNameNumberEncryptedOffset }
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Field"), {
        { "class UField*",  "Next",                 sizeof(UField*),    unreal::GetObjectSize(_XORSTR_(L"UObject")) }
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Struct"), {
        { "int32_t",        "PropertiesSize",       sizeof(int32_t),    unreal::StructPropertiesSizeOffset },
        { "int32_t",        "MinAlignment",         sizeof(int32_t),    unreal::StructMinAlignmentOffset },
        { "UField*",        "Children",             sizeof(UField*),    unreal::StructChildrenOffset },
        { "class UStruct*", "SuperStruct",          sizeof(UStruct*),   unreal::StructSuperStructOffset },
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Function"), {
        { "uint32_t",       "FunctionFlags",        sizeof(uint32_t),   unreal::FunctionFlagsOffset },
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Property"), {
        { "int32_t",        "ElementSize",          sizeof(int32_t),   unreal::PropertyElementSizeOffset },
        { "int32_t",        "ArrayDim",             sizeof(int32_t),   unreal::PropertyArrayDimOffset },
        { "int32_t",        "Offset_Internal",      sizeof(int32_t),   unreal::PropertyOffsetInternalOffset },
        { "uint64_t",       "PropertyFlags",        sizeof(uint64_t),  unreal::PropertyFlagsOffset },
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Enum"), {
        { "int32_t",        "CppForm",              sizeof(int32_t),    unreal::EnumCppFormOffset },
        { "TArray<TPair<FName, int64_t>>", "Names", sizeof(TArray<TPair<FName, int64_t>>), unreal::EnumNamesOffset },
    });

    Gen.AddPredefinedMethods(_XORSTR_("Class CoreUObject.Object"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    static T* FindObject(const std::string& Name)\n"
    "    {\n"
    "        ObjectsProxy Objects;\n"
    "        for (int i = 0; i < Objects.Num(); ++i) {\n"
    "            UObject* Object = Objects.GetById(i);\n"
    "            if (!Object)\n"
    "                continue;\n"
    "            if (Object->GetFullName() == Name) {\n"
    "                return static_cast<T*>(Object);\n"
    "            }\n"
    "        }\n"
    "        return nullptr;\n"
    "    }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    static UClass* FindClass(const std::string& Name)\n"
    "    {\n"
    "        return FindObject<UClass>(Name);\n"
    "    }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    static T* GetObjectCasted(int32_t Id)\n"
    "    {\n"
    "        return static_cast<T*>(ObjectsProxy().GetById(Id));\n"
    "    }"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("int32_t GetFlags() const"), _XORSTR_(
    "int32_t UObject::GetFlags() const\n"
    "{\n"
    "    return (int32_t)DecryptObjectFlagsAsm(ObjectFlagsEncrypted);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("uint32_t GetUniqueId() const"), _XORSTR_(
    "uint32_t UObject::GetUniqueId() const\n"
    "{\n"
    "    return (uint32_t)DecryptObjectIndexAsm(InternalIndexEncrypted);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("class UClass* GetClass() const"), _XORSTR_(
    "UClass* UObject::GetClass() const\n"
    "{\n"
    "    return (UClass*)DecryptObjectClassAsm(ClassEncrypted);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("UObject *GetOuter() const"), _XORSTR_(
    "UObject *UObject::GetOuter() const\n"
    "{\n"
    "    return (UObject*)DecryptObjectOuterAsm(OuterEncrypted);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("FName GetFName() const"), _XORSTR_(
    "FName UObject::GetFName() const\n"
    "{\n"
    "    int32_t Index, Number;\n"
    "    DecryptObjectFNameAsm(NameIndexEncrypted, NameNumberEncrypted, &Index, &Number);\n"
    "    return FName(Index, Number);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("const UPackage* GetOutermost() const"), _XORSTR_(
    "const UPackage* UObject::GetOutermost() const\n"
    "{\n"
    "    UObject* Top = NULL;\n"
    "    for (UObject* Outer = GetOuter(); Outer; Outer = Outer->GetOuter()) {\n"
    "        Top = Outer;\n"
    "    }\n"
    "    return static_cast<const UPackage*>(Top);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("std::string GetName() const"), _XORSTR_(
    "std::string UObject::GetName() const\n"
    "{\n"
    "    FName Name = GetFName();\n"
    "    std::string NameString = NamesProxy().GetById(Name.Index);\n"
    "    if (Name.Number > 0) {\n"
    "        NameString += '_' + std::to_string(Name.Number);\n"
    "    }\n\n"
    "    size_t pos = NameString.rfind('/');\n"
    "    if (pos != std::string::npos) {\n"
    "        NameString = NameString.substr(pos + 1);\n"
    "    }\n\n"
    "    return NameString;\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("std::string GetFullName() const"), _XORSTR_(
    "std::string UObject::GetFullName() const\n"
    "{\n"
    "    std::string NameString;\n"
    "    const UClass* Class = GetClass();\n"
    "    if (Class) {\n"
    "        std::string Temp;\n\n"
    "        UObject* Outer = GetOuter();\n"
    "        while(Outer) {\n"
    "            Temp = Outer->GetName() + '.' + Temp;\n"
    "            Outer = Outer->GetOuter();\n"
    "        }\n\n"
    "        NameString = Class->GetName();\n"
    "        NameString += ' ';\n"
    "        NameString += Temp;\n"
    "        NameString += GetName();\n"
    "    }\n\n"
    "    return NameString;\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("bool IsA(UClass* CmpClass) const"), _XORSTR_(
    "bool UObject::IsA(UClass* CmpClass) const\n"
    "{\n"
    "    UClass* SuperClass = GetClass();\n"
    "    while (SuperClass) {\n"
    "        if (SuperClass == CmpClass) {\n"
    "            return true;\n"
    "        }\n"
    "        SuperClass = static_cast<UClass*>(SuperClass->GetSuper());\n"
    "    }\n"
    "    return false;\n"
    "}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    inline bool IsA() const { return IsA(T::StaticClass()); }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    inline T* Cast() { return static_cast<T*>(this); }\n"
    "    template<typename T>\n"
    "    inline const T* Cast() const { return static_cast<const T*>(this); }"
        ))
    });

    Gen.AddPredefinedMethod(_XORSTR_("Class CoreUObject.Field"), 
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline UField *GetNext() { return Next; }\n"
    "    inline const UField *GetNext() const { return Next; }"
        ))
    );

    Gen.AddPredefinedMethod(_XORSTR_("Class CoreUObject.Enum"), 
        GeneratorPredefinedMethod::Default(_XORSTR_("std::vector<std::string> GetNames() const"), _XORSTR_(
    "std::vector<std::string> UEnum::GetNames() const\n"
    "{\n"
    "    NamesProxy GlobalNames;\n"
    "    std::vector<std::string> StringArray;\n"
    "    const TArray<TPair<FName, int64_t>>& NamesArray = this->Names;\n\n"
    "    for (auto Name : NamesArray) {\n"
    "        int32_t Index = Name.Key.GetIndex();\n"
    "        StringArray.push_back(GlobalNames.GetById(Index));\n"
    "    }\n\n"
    "    return StringArray;\n"
    "}"
        ))
    );

    Gen.AddPredefinedMethod(_XORSTR_("Class CoreUObject.Struct"), 
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline UStruct* GetSuper() const { return SuperStruct; }\n"
    "    inline UField* GetChildren() const { return Children; }\n"
    "    inline int32_t GetPropertiesSize() const { return PropertiesSize; }\n"
    "    inline int32_t GetMinAlignment() const { return MinAlignment; }"
        ))
    );

    Gen.AddPredefinedMethod(_XORSTR_("Class CoreUObject.Function"), 
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline uint32_t GetFunctionFlags() const { return FunctionFlags; }\n"
        ))
    );

    Gen.AddPredefinedMethod(_XORSTR_("Class CoreUObject.Class"), 
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    inline T* CreateDefaultObject() { return static_cast<T*>(CreateDefaultObject()); }\n"
    "    template<typename T>\n"
    "    inline const T* CreateDefaultObject() const { return static_cast<const T*>(CreateDefaultObject()); }"
        ))
    );

    Gen.AddPredefinedMethod(_XORSTR_("Class CoreUObject.Property"), 
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline int32_t GetArrayDim() const { return ArrayDim; }\n"
    "    inline int32_t GetElementSize() const { return ElementSize; }\n"
    "    inline uint64_t GetPropertyFlags() const { return PropertyFlags; }\n"
    "    inline int32_t GetOffset() const { return Offset_Internal; }"
        ))
    );

    Gen.AddPredefinedMethods(_XORSTR_("ScriptStruct CoreUObject.Vector2D"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector2D() : X(0), Y(0) { }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector2D(float x, float y) : X(x), Y(y) { }"
        ))
    });

    Gen.AddPredefinedMethods(_XORSTR_("ScriptStruct CoreUObject.LinearColor"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FLinearColor() : R(0), G(0), B(0), A(0) { }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FLinearColor(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) { }"
        ))
    });


    Gen.AddVirtualFunctionPattern(_XORSTR_("Class CoreUObject.Object"),
                                  0x200,
                                  _XORSTR_("45 33 F6 BF ?? ?? ?? ?? 39 3D"),
        _XORSTR_(
    "    inline void ProcessEvent(class UFunction* fn, void* parms)\n"
    "    {\n"
    "        return utils::GetVFunction<void(*)(UObject*, class UFunction*, void*)>(this, %d)(this, fn, parms);\n"
    "    }"));

    Gen.AddVirtualFunctionPattern(_XORSTR_("Class CoreUObject.Class"),
                                  0x200,
                                  _XORSTR_("45 33 E4 4C 89 64 24 ?? 48 85 DB"),
        _XORSTR_(
    "    inline UObject* CreateDefaultObject()\n"
    "    {\n"
    "        return utils::GetVFunction<UObject*(*)(UClass*)>(this, %d)(this);\n"
    "    }"));

    Gen.Generate();
#endif // !DUMP_ONLY

    return STATUS_SUCCESS;
}

BOOL
WINAPI 
DllMain(
    IN HMODULE hModule,
    IN ULONG dwReason,
    IN OUT PVOID lpReserved
)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ThreadAttributes;

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        LdrDisableThreadCalloutsForDll(hModule);

        //
        // Initialize logger subsystem.
        //
        Status = LogInitialize(LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend,
                               _XOR_(L"C:\\Users\\Owner\\Log.txt"));
        if (Status != NOERROR) {
            return FALSE;
        }

        LOG_DEBUG(_XOR_("hModule = 0x%016llx"), hModule);

        //
        // Create main thread.
        //
        InitializeObjectAttributes(&ThreadAttributes, NULL, 0, NULL, NULL);
        Status = NtCreateThreadEx(&MainThread,
                                  THREAD_ALL_ACCESS,
                                  &ThreadAttributes,
                                  NtCurrentProcess(),
                                  TestThreadRoutine,
                                  hModule,
                                  0,
                                  0,
                                  0x1000,
                                  0x100000,
                                  NULL
                                  );

        if (!NT_SUCCESS(Status)) {
            LOG_DEBUG(_XOR_("Failed to create thread with status %08x"), Status);
            LogDestroy();
            return FALSE;
        }

        LOG_DEBUG(_XOR_("Main thread created %d"), (ULONG)(ULONG_PTR)MainThread);
        return TRUE;

    case DLL_PROCESS_DETACH:
        LogDestroy();
        return TRUE;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return FALSE;
}