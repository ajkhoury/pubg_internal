#include <native/native.h>
#include <native/log.h>

#include <utils/utils.h>
#include <utils/disasm.h>
#include <utils/xorstr.h>
#include <utils/vthook.h>

#include "sdk/Generator.h"
#include "sdk/OffsetDumper.h"

#include "Sdk.h"

#define DUMP_ONLY 1

static HANDLE MainThread = (HANDLE)(LONG_PTR)-1;

#if defined(ENABLE_SDK)
// Core globals.
static UTslGameViewportClient* GameViewportClient = nullptr;
static std::unique_ptr<SafeVTableHook> GameViewportClientHook = nullptr;

static UWorld* CurrentWorld = nullptr;
static bool LevelHasChanged = false;
static ULevel* PersistentLevel = nullptr;

static size_t GameViewportWorldOffset = (size_t)-1;
static std::vector<UWorld*> WorldInstances;
UWorld* FindWorld(UGameViewportClient* GameViewportClient)
{
    // Find the GameViewportWorldOffset if we haven't yet.
    if (GameViewportWorldOffset == (size_t)-1) {
        size_t PointerOffset = 0;
        bool Found = false;
    
        // Search the GameViewportClient for the matching a world, which will
        // give us the offset of the UGameViewportClient::World field.
        if (!WorldInstances.size())
            WorldInstances = UObject::GetInstancesOf<UWorld>();
        for (UWorld* WorldInstance : WorldInstances) {

            uintptr_t* Start = reinterpret_cast<uintptr_t*>(GameViewportClient);
            uintptr_t* End = Start + (sizeof(UTslGameViewportClient) / sizeof(uintptr_t));
            for (uintptr_t* P = Start; P < End; P++, PointerOffset++) {
                if (*P == reinterpret_cast<uintptr_t>(WorldInstance)) {
                    Found = true;
                    break;
                }
            }
            if (Found)
                break;
        }

        if (!Found)
            return nullptr;

        GameViewportWorldOffset = PointerOffset;
    }

    // Get the World at the offset into the UGameViewportClient object.
    return reinterpret_cast<UWorld**>(GameViewportClient)[GameViewportWorldOffset];
}

static std::vector<UFont*> FontInstances;
UFont* FindFont(const std::string& FontName)
{
    if (!FontInstances.size())
        FontInstances = UObject::GetInstancesOf<UFont>();
    for (UFont* FontInstance : FontInstances) {
        if (FontInstance->IsValid() && FontInstance->GetName() == FontName) {
            return FontInstance;
        }
    }
    return nullptr;
}

ULevel* FindPersistentLevel(UWorld* World)
{
    ObjectsProxy Objects;
    for (int32_t i = 80000; i < Objects.GetNum(); i++) {
        UObject* Object = Objects.GetById(i);
        if (utils::is_valid_ptr(Object) && Object->IsValid()) {
            if (Object->GetOuter() == World && Object->IsA<ULevel>()) {
                PersistentLevel = static_cast<ULevel*>(Object);
                return PersistentLevel;
            }
        }
    }
    return nullptr;
}

UFont* RobotoFont = nullptr;
UFont* RobotoTinyFont = nullptr;
UFont* RobotoDistanceFieldFont = nullptr;
UFont* TslFont = nullptr;

typedef
void
(__fastcall *DrawTransitionFn)(
    UGameViewportClient* GameViewportClient,
    UCanvas* Canvas
    );
DrawTransitionFn DrawTransition_orig = nullptr;

inline bool IsValidScreenPosition(const UCanvas* Canvas, const FVector& ScreenPosition)
{
    return !(ScreenPosition.X <= 0.f ||
             ScreenPosition.Y <= 0.f ||
             ScreenPosition.Z <= 0.f ||
             ScreenPosition.X > Canvas->SizeX ||
             ScreenPosition.Y > Canvas->SizeY);
}

bool IsValidLocalController(APlayerController* Controller)
{
    if (Controller->IsLocalController()) {
        APawn* Pawn = Controller->K2_GetPawn();
        if (Pawn != nullptr && utils::is_valid_ptr(Pawn) && Pawn->IsValid()) {
            return true;
        }
    }
    return false;
}

void __fastcall DrawTransition_hook(UGameViewportClient* GameViewportClient, UCanvas* Canvas)
{
    if (!GameViewportClient || !Canvas) {
        return;
    }
    
    UWorld* World = FindWorld(GameViewportClient);
    if (!World) {
        return;
    }

    ULevel* PersistentLevel = World->GetPersistentLevel();
    if (!PersistentLevel) {
        return;
    }

    // Draw menu.
    Canvas->K2_DrawText(RobotoFont,
                        FString(_XOR_(L"96")),
                        FVector2D(5.0f, 5.0f),
                        FLinearColor::Gray,
                        1.0f,
                        FLinearColor::Transparent,
                        FVector2D(),
                        false,
                        false,
                        true,
                        FLinearColor::Black);

    // Iterate the actors.
    TArray<AActor*>* Actors = PersistentLevel->GetActors();
    if (!Actors) {
        return;
    }
    //LOG_INFO(_XOR_("PersistentLevel->Actors = 0x%p"), Actors);

    for (int32_t i = 0; i < Actors->Num(); i++) {
        AActor* Actor = Actors->Get(i);
        if (!Actor)
            continue;
        USceneComponent* RootComponent = Actor->K2_GetRootComponent();
        if (!RootComponent)
            continue;

        // Player ESP.
        if (Actor->IsA<ATslCharacter>()) {
            ATslCharacter* Character = Actor->Cast<ATslCharacter>();

            FVector CharacterPos = RootComponent->RelativeLocation;
            FVector ScreenPos = Canvas->K2_Project(CharacterPos);

            //LOG_DBG_PRINT(_XOR_("Tsl character 0x%p at {%f, %f, %f} -> {%f, %f}\n"),
            //              Character,
            //              CharacterPos.X, CharacterPos.Y, CharacterPos.Z,
            //              ScreenPos.X, ScreenPos.Y);

            if (IsValidScreenPosition(Canvas, ScreenPos)) {

                Canvas->K2_DrawText(RobotoFont,
                                    FString(_XOR_(L"Actor")),
                                    ScreenPos,
                                    FLinearColor::White,
                                    1.0f,
                                    FLinearColor::Transparent,
                                    FVector2D(),
                                    false,
                                    true,
                                    true,
                                    FLinearColor::Black);
            
                Canvas->K2_DrawLine(ScreenPos,
                                    FVector2D(10.f, 10.f),
                                    1.0f,
                                    FLinearColor::Red);
            }
        }
    }
}
#endif // ENABLE_SDK

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

    int32_t LoopCount = 0;
    while(TRUE) {
    
        WaitInterval.QuadPart = INTERVAL_RELATIVE(MILLISECONDS(1000));
        NtDelayExecution(FALSE, &WaitInterval);
    
        LOG_DEBUG(_XOR_("Names address = 0x%p"), Names.GetAddress());
        LOG_DEBUG(_XOR_("Names count = %d"), Names.GetNum());
    
        LOG_DEBUG(_XOR_("Objects address = 0x%p"), Objects.GetAddress());
        LOG_DEBUG(_XOR_("Objects count = %d"), Objects.GetNum());
    
        if (++LoopCount >= 8)
            break;
    
        //++NameId;
        //const char *NameData = Names.GetById(NameId);
        //if (NameData) {
        //    LOG_DEBUG(_XOR_("Names[%d] = \"%s\""), NameId, Names.GetById(NameId));
        //}
        //void **NamesData = static_cast<void **>(Names.GetAddress());
        //LOG_INFO(_XOR_("NamesData0 = 0x%016llx"), NamesData[0]);
        //LOG_INFO(_XOR_("NamesData1 = 0x%016llx"), NamesData[1]);
    }

#if defined(ENABLE_SDK)
    // Find TslGameViewportClient object instance by name.
    do {
        GameViewportClient = UObject::FindObject<UTslGameViewportClient>(
            _XORSTR_("TslGameViewportClient Transient.TslEngine_1.TslGameViewportClient_1"));
    } while (!GameViewportClient);
    LOG_INFO(_XOR_("TslGameViewportClient object = 0x%p"), GameViewportClient);

    // Get all UWorld instances.
    WorldInstances = UObject::GetInstancesOf<UWorld>();
    for (auto&& WorldInstance : WorldInstances) {
        LOG_INFO(_XOR_("UWorld instance = \"%s\" -> 0x%p"),
                 WorldInstance->GetFullName().c_str(), WorldInstance);
    }

    // Get all UFont instances.
    FontInstances = UObject::GetInstancesOf<UFont>();
    for (auto&& FontInstance : FontInstances) {
        LOG_INFO(_XOR_("UFont instance = \"%s\" -> 0x%p"),
                 FontInstance->GetFullName().c_str(), FontInstance);
    }

    // Find the DrawTransition method VTable index.
    size_t DrawTransitionIndex = utils::FindVFunctionIndex(
                                    GameViewportClient, 64,
                                    _XORSTR_("48 8B F2 48 8B F9 80 B9 ?? ?? ?? ?? 00 0F 85"));
    if (DrawTransitionIndex == (size_t)-1) {
        return STATUS_NOT_FOUND;
    }
    LOG_INFO(_XOR_("UTslGameViewportClient::DrawTransition vtable index = %d"), DrawTransitionIndex);

    // Hook the UTslGameViewportClient::DrawTransition function.
    GameViewportClientHook = std::make_unique<SafeVTableHook>(GameViewportClient);
    DrawTransition_orig = GameViewportClientHook->Install<DrawTransitionFn>(
                                    DrawTransitionIndex, DrawTransition_hook);
    if (!DrawTransition_orig) {
        LOG_ERROR(_XOR_("Failed to hook the UGameViewportClient::DrawTransistion function!"));
        return STATUS_UNSUCCESSFUL;
    }
    LOG_INFO(_XOR_("UTslGameViewportClient::DrawTransition original = 0x%p"), DrawTransition_orig);  

    // Find font objects by name.
    if (!RobotoFont)
        RobotoFont = FindFont(_XORSTR_("Roboto"));
    if (!RobotoTinyFont)
        RobotoTinyFont = FindFont(_XORSTR_("RobotoTiny"));
    if (!RobotoDistanceFieldFont)
        RobotoDistanceFieldFont = FindFont(_XORSTR_("RobotoDistanceField"));
    if (!TslFont)
        TslFont = FindFont(_XORSTR_("TSLFont"));

    LOG_INFO(_XOR_("Done, exiting thread..."));

#endif // ENABLE_SDK

    //UClass* UEnumStaticClass = Objects.FindClass("Class CoreUObject.Enum");
    //if (UEnumStaticClass) {
    //    LOG_DEBUG(_XOR_("UEnumStaticClass = 0x%p"), UEnumStaticClass);
    //}
    //for (int32_t ObjectIdx = 0; ObjectIdx < Objects.GetNum(); ObjectIdx++) {
    //    UObject const* Object = Objects.GetById(ObjectIdx);
    //    if (Object) {
    //        std::string FullName = Object->GetFullName();
    //        if (!FullName.empty()) {
    //            uint32_t uniqueId = Object->GetUniqueId();
    //            LOG_INFO(_XOR_("Object[%d] = %s"), uniqueId, FullName.c_str());
    //        }
    //    }
    //}

#if !defined(ENABLE_SDK)
    unreal::InitializeUnrealObjectSizeMap();

    Generator Gen(_XOR_("F:\\Projects\\nvdid\\internal\\sdk"), true, true);

    Gen.AddPredefinedMembers("Class CoreUObject.Object", {
        { "void**",     "VTable",                   sizeof(void**),     0x00 },
        { "int32_t",    "ObjectFlagsEncrypted",     sizeof(int32_t),    unreal::ObjectFlagsEncryptedOffset },
        { "uint64_t",   "OuterEncrypted",           sizeof(uint64_t),   unreal::ObjectOuterEncryptedOffset },
        { "int32_t",    "InternalIndexEncrypted",   sizeof(int32_t),    unreal::ObjectInternalIndexEncryptedOffset },
        { "uint64_t",   "ClassEncrypted",           sizeof(uint64_t),   unreal::ObjectClassEncryptedOffset },
        { "int32_t",    "NameIndexEncrypted",       sizeof(int32_t),    unreal::ObjectNameIndexEncryptedOffset },
        { "int32_t",    "NameNumberEncrypted",      sizeof(int32_t),    unreal::ObjectNameNumberEncryptedOffset }
    });

    Gen.AddPredefinedMembers("Class CoreUObject.Field", {
        { "class UField*",  "Next",                 sizeof(UField*),    unreal::GetObjectSize(_XORSTR_(L"UObject")) }
    });

    Gen.AddPredefinedMembers("Class CoreUObject.Struct", {
        { "int32_t",        "PropertiesSize",       sizeof(int32_t),    unreal::StructPropertiesSizeOffset },
        { "int32_t",        "MinAlignment",         sizeof(int32_t),    unreal::StructMinAlignmentOffset },
        { "UField*",        "Children",             sizeof(UField*),    unreal::StructChildrenOffset },
        { "class UStruct*", "SuperStruct",          sizeof(UStruct*),   unreal::StructSuperStructOffset },
    });

    Gen.AddPredefinedMembers("Class CoreUObject.Function", {
        { "uint32_t",       "FunctionFlags",        sizeof(uint32_t),   unreal::FunctionFlagsOffset },
    });

    Gen.AddPredefinedMembers("Class CoreUObject.Property", {
        { "int32_t",        "ElementSize",          sizeof(int32_t),   unreal::PropertyElementSizeOffset },
        { "int32_t",        "ArrayDim",             sizeof(int32_t),   unreal::PropertyArrayDimOffset },
        { "int32_t",        "Offset_Internal",      sizeof(int32_t),   unreal::PropertyOffsetInternalOffset },
        { "uint64_t",       "PropertyFlags",        sizeof(uint64_t),  unreal::PropertyFlagsOffset },
    });

    Gen.AddPredefinedMembers("Class CoreUObject.Enum", {
        { "int32_t",        "CppForm",              sizeof(int32_t),    unreal::EnumCppFormOffset },
        { "TArray<TPair<FName, int64_t>>", "Names", sizeof(TArray<TPair<FName, int64_t>>), unreal::EnumNamesOffset },
    });

    Gen.AddPredefinedMethods("Class CoreUObject.Object", {
        GeneratorPredefinedMethod::Inline(
    "    template<class T>\n"
    "    static std::vector<T*> GetInstancesOf()\n"
    "    {\n"
    "        std::vector<T*> ObjectInstances;\n"
    "        UClass* Class = T::StaticClass();\n"
    "        ObjectsProxy Objects;\n"
    "        for (int32_t i = 0; i < Objects.GetNum(); ++i) {\n"
    "            UObject* Object = Objects.GetById(i);\n"
    "            if (Object && Object->IsA(Class)) {\n"
    "                if (Object->GetFullName().find(\"Default\") == std::string::npos) {\n"
    "                    ObjectInstances.push_back(static_cast<T*>(Object));\n"
    "                }\n"
    "            }\n"
    "        }\n"
    "        return ObjectInstances;\n"
    "    }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    template<typename T>\n"
    "    static T* FindObject(const std::string& Name)\n"
    "    {\n"
    "        ObjectsProxy Objects;\n"
    "        for (int i = 0; i < Objects.GetNum(); ++i) {\n"
    "            UObject* Object = Objects.GetById(i);\n"
    "            if (Object && Object->GetFullName() == Name) {\n"
    "                return static_cast<T*>(Object);\n"
    "            }\n"
    "        }\n"
    "        return nullptr;\n"
    "    }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    static UClass* FindClass(const std::string& Name)\n"
    "    {\n"
    "        return FindObject<class UClass>(Name);\n"
    "    }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    template<typename T>\n"
    "    static T* GetObjectCasted(int32_t Id)\n"
    "    {\n"
    "        return static_cast<T*>(ObjectsProxy().GetById(Id));\n"
    "    }"
        ),
        GeneratorPredefinedMethod::Default("int32_t GetFlags() const",
    "int32_t UObject::GetFlags() const\n"
    "{\n"
    "    return (int32_t)DecryptObjectFlagsAsm(ObjectFlagsEncrypted);\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("uint32_t GetUniqueId() const",
    "uint32_t UObject::GetUniqueId() const\n"
    "{\n"
    "    return (uint32_t)DecryptObjectIndexAsm(InternalIndexEncrypted);\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("class UClass const* GetClass() const",
    "UClass const* UObject::GetClass() const\n"
    "{\n"
    "    return (UClass const*)DecryptObjectClassAsm(ClassEncrypted);\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("class UObject const* GetOuter() const",
    "UObject const* UObject::GetOuter() const\n"
    "{\n"
    "    return (UObject const*)DecryptObjectOuterAsm(OuterEncrypted);\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("FName GetFName() const",
    "FName UObject::GetFName() const\n"
    "{\n"
    "    FName Name;\n"
    "    DecryptObjectFNameAsm(NameIndexEncrypted, NameNumberEncrypted, &Name.Index, &Name.Number);\n"
    "    return Name;\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("class UPackage const* GetOutermost() const",
    "UPackage const* UObject::GetOutermost() const\n"
    "{\n"
    "    UObject const* Top = NULL;\n"
    "    for (UObject const* Outer = GetOuter(); Outer; Outer = Outer->GetOuter()) {\n"
    "        Top = Outer;\n"
    "    }\n"
    "    return static_cast<UPackage const*>(Top);\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("std::string GetName() const",
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
        ),
        GeneratorPredefinedMethod::Default("std::string GetFullName() const",
    "std::string UObject::GetFullName() const\n"
    "{\n"
    "    std::string NameString;\n"
    "    const UClass* Class = GetClass();\n"
    "    if (Class) {\n"
    "        std::string Temp;\n\n"
    "        UObject const* Outer = GetOuter();\n"
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
        ),
        GeneratorPredefinedMethod::Default("std::string GetNameCPP() const",
    "std::string UObject::GetNameCPP() const\n"
    "{\n"
    "    std::string NameString;\n"
    "    if (IsA<UClass>()) {\n\n"
    "        const UClass* Class = static_cast<const UClass*>(this);\n"
    "        while (Class) {\n"
    "            const std::string ClassName = Class->GetName();\n"
    "            if (ClassName == \"Actor\") {\n"
    "                NameString += 'A';\n"
    "                break;\n"
    "            }\n"
    "            if (ClassName == \"Object\") {\n"
    "                NameString += 'U';\n"
    "                break;\n"
    "            }\n"
    "            Class = static_cast<const UClass*>(Class->GetSuper());\n"
    "        }\n\n"
    "    } else {\n"
    "        NameString += 'F';\n"
    "    }\n\n"
    "    NameString += GetName();\n"
    "    return NameString;\n"
    "}"
        ),
        GeneratorPredefinedMethod::Default("bool IsA(class UClass const* CmpClass) const",
    "bool UObject::IsA(UClass const* CmpClass) const\n"
    "{\n"
    "    UClass const* SuperClass = GetClass();\n"
    "    while (SuperClass) {\n"
    "        if (SuperClass == CmpClass) {\n"
    "            return true;\n"
    "        }\n"
    "        SuperClass = static_cast<UClass const*>(SuperClass->GetSuper());\n"
    "    }\n"
    "    return false;\n"
    "}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    template<typename T>\n"
    "    inline bool IsA() const { return IsA(T::StaticClass()); }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    template<typename T>\n"
    "    inline T* Cast() { return static_cast<T*>(this); }\n"
    "    template<typename T>\n"
    "    inline T const* Cast() const { return static_cast<T const*>(this); }"
        ),
        GeneratorPredefinedMethod::Inline("    inline bool IsValid() const { return GetFName().Index != 0 && GetFlags() != 0 && GetUniqueId() != 0; }")
    });

    Gen.AddPredefinedMethod("Class CoreUObject.Field", 
        GeneratorPredefinedMethod::Inline(
    "    inline UField *GetNext() { return Next; }\n"
    "    inline const UField *GetNext() const { return Next; }"
        )
    );

    Gen.AddPredefinedMethod("Class CoreUObject.Enum", 
        GeneratorPredefinedMethod::Default("std::vector<std::string> GetNames() const",
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
        )
    );

    Gen.AddPredefinedMethod("Class CoreUObject.Struct", 
        GeneratorPredefinedMethod::Inline(
    "    inline class UStruct* GetSuper() const { return SuperStruct; }\n"
    "    inline class UField* GetChildren() const { return Children; }\n"
    "    inline int32_t GetPropertiesSize() const { return PropertiesSize; }\n"
    "    inline int32_t GetMinAlignment() const { return MinAlignment; }"
        )
    );

    Gen.AddPredefinedMethod("Class CoreUObject.Function", 
        GeneratorPredefinedMethod::Inline(
    "    inline uint32_t GetFunctionFlags() const { return FunctionFlags; }\n"
        )
    );

    Gen.AddPredefinedMethod("Class CoreUObject.Class", 
        GeneratorPredefinedMethod::Inline(
    "    template<typename T>\n"
    "    inline T* CreateDefaultObject() { return static_cast<T*>(CreateDefaultObject()); }\n"
    "    template<typename T>\n"
    "    inline T const* CreateDefaultObject() const { return static_cast<T const*>(CreateDefaultObject()); }"
        )
    );

    Gen.AddPredefinedMethod("Class CoreUObject.Property", 
        GeneratorPredefinedMethod::Inline(
    "    inline int32_t GetArrayDim() const { return ArrayDim; }\n"
    "    inline int32_t GetElementSize() const { return ElementSize; }\n"
    "    inline uint64_t GetPropertyFlags() const { return PropertyFlags; }\n"
    "    inline int32_t GetOffset() const { return Offset_Internal; }"
        )
    );

    Gen.AddPredefinedMethods("ScriptStruct CoreUObject.Vector", {
        GeneratorPredefinedMethod::Inline(
    "    inline FVector() : X(0), Y(0), Z(0) {}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}"
        )
    });

    Gen.AddPredefinedMethods("ScriptStruct CoreUObject.Vector2D", {
        GeneratorPredefinedMethod::Inline(
    "    inline FVector2D() : X(0), Y(0) {}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FVector2D(float x, float y) : X(x), Y(y) {}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FVector2D(const FVector& V) : X(V.X), Y(V.Y) {}"
        )
    });

    Gen.AddPredefinedStaticMembers("ScriptStruct CoreUObject.LinearColor", {
        { "float", "sRGBToLinearTable[256]", "{\n"
    "    0.f,\n"
    "    0.000303526983548838f, 0.000607053967097675f, 0.000910580950646512f, 0.00121410793419535f, 0.00151763491774419f,\n"
    "    0.00182116190129302f, 0.00212468888484186f, 0.0024282158683907f, 0.00273174285193954f, 0.00303526983548838f,\n"
    "    0.00334653564113713f, 0.00367650719436314f, 0.00402471688178252f, 0.00439144189356217f, 0.00477695332960869f,\n"
    "    0.005181516543916f, 0.00560539145834456f, 0.00604883284946662f, 0.00651209061157708f, 0.00699540999852809f,\n"
    "    0.00749903184667767f, 0.00802319278093555f, 0.0085681254056307f, 0.00913405848170623f, 0.00972121709156193f,\n"
    "    0.0103298227927056f, 0.0109600937612386f, 0.0116122449260844f, 0.012286488094766f, 0.0129830320714536f,\n"
    "    0.0137020827679224f, 0.0144438433080002f, 0.0152085141260192f, 0.0159962930597398f, 0.0168073754381669f,\n"
    "    0.0176419541646397f, 0.0185002197955389f, 0.0193823606149269f, 0.0202885627054049f, 0.0212190100154473f,\n"
    "    0.0221738844234532f, 0.02315336579873f, 0.0241576320596103f, 0.0251868592288862f, 0.0262412214867272f,\n"
    "    0.0273208912212394f, 0.0284260390768075f, 0.0295568340003534f, 0.0307134432856324f, 0.0318960326156814f,\n"
    "    0.0331047661035236f, 0.0343398063312275f, 0.0356013143874111f, 0.0368894499032755f, 0.0382043710872463f,\n"
    "    0.0395462347582974f, 0.0409151963780232f, 0.0423114100815264f, 0.0437350287071788f, 0.0451862038253117f,\n"
    "    0.0466650857658898f, 0.0481718236452158f, 0.049706565391714f, 0.0512694577708345f, 0.0528606464091205f,\n"
    "    0.0544802758174765f, 0.0561284894136735f, 0.0578054295441256f, 0.0595112375049707f, 0.0612460535624849f,\n"
    "    0.0630100169728596f, 0.0648032660013696f, 0.0666259379409563f, 0.0684781691302512f, 0.070360094971063f,\n"
    "    0.0722718499453493f, 0.0742135676316953f, 0.0761853807213167f, 0.0781874210336082f, 0.0802198195312533f,\n"
    "    0.0822827063349132f, 0.0843762107375113f, 0.0865004612181274f, 0.0886555854555171f, 0.0908417103412699f,\n"
    "    0.0930589619926197f, 0.0953074657649191f, 0.0975873462637915f, 0.0998987273569704f, 0.102241732185838f,\n"
    "    0.104616483176675f, 0.107023102051626f, 0.109461709839399f, 0.1119324268857f, 0.114435372863418f,\n"
    "    0.116970666782559f, 0.119538426999953f, 0.122138771228724f, 0.124771816547542f, 0.127437679409664f,\n"
    "    0.130136475651761f, 0.132868320502552f, 0.135633328591233f, 0.138431613955729f, 0.141263290050755f,\n"
    "    0.144128469755705f, 0.147027265382362f, 0.149959788682454f, 0.152926150855031f, 0.155926462553701f,\n"
    "    0.158960833893705f, 0.162029374458845f, 0.16513219330827f, 0.168269398983119f, 0.171441099513036f,\n"
    "    0.174647402422543f, 0.17788841473729f, 0.181164242990184f, 0.184474993227387f, 0.187820771014205f,\n"
    "    0.191201681440861f, 0.194617829128147f, 0.198069318232982f, 0.201556252453853f, 0.205078735036156f,\n"
    "    0.208636868777438f, 0.212230756032542f, 0.215860498718652f, 0.219526198320249f, 0.223227955893977f,\n"
    "    0.226965872073417f, 0.23074004707378f, 0.23455058069651f, 0.238397572333811f, 0.242281120973093f,\n"
    "    0.246201325201334f, 0.250158283209375f, 0.254152092796134f, 0.258182851372752f, 0.262250655966664f,\n"
    "    0.266355603225604f, 0.270497789421545f, 0.274677310454565f, 0.278894261856656f, 0.283148738795466f,\n"
    "    0.287440836077983f, 0.291770648154158f, 0.296138269120463f, 0.300543792723403f, 0.304987312362961f,\n"
    "    0.309468921095997f, 0.313988711639584f, 0.3185467763743f, 0.323143207347467f, 0.32777809627633f,\n"
    "    0.332451534551205f, 0.337163613238559f, 0.341914423084057f, 0.346704054515559f, 0.351532597646068f,\n"
    "    0.356400142276637f, 0.361306777899234f, 0.36625259369956f, 0.371237678559833f, 0.376262121061519f,\n"
    "    0.381326009488037f, 0.386429431827418f, 0.39157247577492f, 0.396755228735618f, 0.401977777826949f,\n"
    "    0.407240209881218f, 0.41254261144808f, 0.417885068796976f, 0.423267667919539f, 0.428690494531971f,\n"
    "    0.434153634077377f, 0.439657171728079f, 0.445201192387887f, 0.450785780694349f, 0.456411021020965f,\n"
    "    0.462076997479369f, 0.467783793921492f, 0.473531493941681f, 0.479320180878805f, 0.485149937818323f,\n"
    "    0.491020847594331f, 0.496932992791578f, 0.502886455747457f, 0.50888131855397f, 0.514917663059676f,\n"
    "    0.520995570871595f, 0.527115123357109f, 0.533276401645826f, 0.539479486631421f, 0.545724458973463f,\n"
    "    0.552011399099209f, 0.558340387205378f, 0.56471150325991f, 0.571124827003694f, 0.577580437952282f,\n"
    "    0.584078415397575f, 0.590618838409497f, 0.597201785837643f, 0.603827336312907f, 0.610495568249093f,\n"
    "    0.617206559844509f, 0.623960389083534f, 0.630757133738175f, 0.637596871369601f, 0.644479679329661f,\n"
    "    0.651405634762384f, 0.658374814605461f, 0.665387295591707f, 0.672443154250516f, 0.679542466909286f,\n"
    "    0.686685309694841f, 0.693871758534824f, 0.701101889159085f, 0.708375777101046f, 0.71569349769906f,\n"
    "    0.723055126097739f, 0.730460737249286f, 0.737910405914797f, 0.745404206665559f, 0.752942213884326f,\n"
    "    0.760524501766589f, 0.768151144321824f, 0.775822215374732f, 0.783537788566466f, 0.791297937355839f,\n"
    "    0.799102735020525f, 0.806952254658248f, 0.81484656918795f, 0.822785751350956f, 0.830769873712124f,\n"
    "    0.838799008660978f, 0.846873228412837f, 0.854992605009927f, 0.863157210322481f, 0.871367116049835f,\n"
    "    0.879622393721502f, 0.887923114698241f, 0.896269350173118f, 0.904661171172551f, 0.913098648557343f,\n"
    "    0.921581853023715f, 0.930110855104312f, 0.938685725169219f, 0.947306533426946f, 0.955973349925421f,\n"
    "    0.964686244552961f, 0.973445287039244f, 0.982250546956257f, 0.991102093719252f, 1.0f,\n"
    "}" },
        { "const FLinearColor", "White", "FLinearColor(1.f,1.f,1.f)" },
        { "const FLinearColor", "Gray", "FLinearColor(0.5f,0.5f,0.5f)" },
        { "const FLinearColor", "Black", "FLinearColor(0,0,0)" },
        { "const FLinearColor", "Transparent", "FLinearColor(0,0,0,0)" },
        { "const FLinearColor", "Red", "FLinearColor(1.f,0,0)" },
        { "const FLinearColor", "Green", "FLinearColor(0,1.f,0)" },
        { "const FLinearColor", "Blue", "FLinearColor(0,0,1.f)" },
        { "const FLinearColor", "Yellow", "FLinearColor(1.f,1.f,0)" },
    });

    Gen.AddPredefinedMethods("ScriptStruct CoreUObject.LinearColor", {
        GeneratorPredefinedMethod::Inline(
    "    inline FLinearColor() : R(0), G(0), B(0), A(1.f) {}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FLinearColor(float r, float g, float b, float a = 1.f) : R(r), G(g), B(b), A(a) {}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FLinearColor(const FColor& Color)\n"
    "    {\n"
    "        R = sRGBToLinearTable[Color.R];\n"
    "        G = sRGBToLinearTable[Color.G];\n"
    "        B = sRGBToLinearTable[Color.B];\n"
    "        A = float(Color.A) * (1.0f / 255.0f);\n"
    "    }"
        )
    });

    Gen.AddPredefinedStaticMembers("ScriptStruct CoreUObject.Color", {
        { "const FColor", "White", "FColor(255,255,255)" },
        { "const FColor", "Gray", "FColor(127,127,127)" },
        { "const FColor", "Black", "FColor(0,0,0)" },
        { "const FColor", "Transparent", "FColor(0,0,0,0)" },
        { "const FColor", "Red", "FColor(255,0,0)" },
        { "const FColor", "Green", "FColor(0,255,0)" },
        { "const FColor", "Blue", "FColor(0,0,255)" },
        { "const FColor", "Yellow", "FColor(255,255,0)" },
        { "const FColor", "Cyan", "FColor(0,255,255)" },
        { "const FColor", "Magenta", "FColor(255,0,255)" },
        { "const FColor", "Orange", "FColor(243, 156, 18)" },
        { "const FColor", "Purple", "FColor(169, 7, 228)" },
        { "const FColor", "Turquoise", "FColor(26, 188, 156)" },
        { "const FColor", "Silver", "FColor(189, 195, 199)" },
        { "const FColor", "Emerald", "FColor(46, 204, 113)" },
    });

    Gen.AddPredefinedMethods("ScriptStruct CoreUObject.Color", {
        GeneratorPredefinedMethod::Inline(
    "    inline FColor() { R = G = B = 0; A = 255; }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FColor(uint8_t InR, uint8_t InG, uint8_t InB, uint8_t InA = 255) { R = InR; G = InG; B = InB; A = InA; }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline explicit FColor(uint32_t InColor) { DWColor() = InColor; }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    uint32_t& DWColor(void) {return *((uint32_t*)this);}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    const uint32_t& DWColor(void) const {return *((uint32_t*)this);}"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline bool operator==(const FColor &C) const { return DWColor() == C.DWColor(); }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline bool operator!=(const FColor &C) const { return DWColor() != C.DWColor(); }"
        ),
        GeneratorPredefinedMethod::Inline(
    "    inline FColor WithAlpha(uint8_t Alpha) const { return FColor(R, G, B, Alpha); }"
        ),
    //    GeneratorPredefinedMethod::Inline(
    //"    inline FLinearColor ReinterpretAsLinear() const { return FLinearColor(R / 255.f, G / 255.f, B / 255.f, A / 255.f); }"
    //    ),
    });

    Gen.AddPredefinedMethods("Class Engine.World", {
        GeneratorPredefinedMethod::Inline(tfm::format(
    "    inline uint64_t GetPersistentLevelEncrypted() const\n"
    "    {\n"
    "        return *(uint64_t*)(reinterpret_cast<const uint8_t*>(this) + 0x%X);\n"
    "    }", unreal::WorldPersistentLevelEncryptedOffset
        ).c_str()),
        GeneratorPredefinedMethod::Default("class ULevel* GetPersistentLevel() const",
    "ULevel* UWorld::GetPersistentLevel() const\n"
    "{\n"
    "    return (ULevel*)DecryptPersistentLevelAsm(GetPersistentLevelEncrypted());\n"
    "}"
        )
    });

    Gen.AddPredefinedMethods("Class Engine.Level", {
        GeneratorPredefinedMethod::Inline(tfm::format(
    "    inline uint64_t GetActorsEncrypted() const\n"
    "    {\n"
    "        return *(uint64_t*)(reinterpret_cast<const uint8_t*>(this) + 0x%X);\n"
    "    }", unreal::LevelActorsEncryptedOffset
        ).c_str()),
        GeneratorPredefinedMethod::Default("TArray<class AActor*>* GetActors() const",
    "TArray<AActor*>* ULevel::GetActors() const\n"
    "{\n"
    "    return (TArray<AActor*>*)DecryptActorsAsm(GetActorsEncrypted());\n"
    "}"
        )
    });

    Gen.AddVirtualFunctionPattern("Class CoreUObject.Object",
                                  0x200,
                                  _XOR_("45 33 F6 BF ?? ?? ?? ?? 39 3D"),
    "    inline void ProcessEvent(class UFunction* fn, void* parms) const\n"
    "    {\n"
    "        return utils::GetVFunction<void(*)(UObject const*, class UFunction*, void*)>(this, %d)(this, fn, parms);\n"
    "    }");

    Gen.AddVirtualFunctionPattern("Class CoreUObject.Class",
                                  0x200,
                                  _XOR_("45 33 E4 4C 89 64 24 ?? 48 85 DB"),
    "    inline class UObject* CreateDefaultObject() const\n"
    "    {\n"
    "        return utils::GetVFunction<class UObject*(*)(class UClass const*)>(this, %d)(this);\n"
    "    }");

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

        // Initialize logger subsystem.
#if defined(ENABLE_LOG_FILE)
        Status = LogInitialize(LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend, _XOR_(L"C:\\Users\\Owner\\Log.txt"));
#else
        Status = LogInitialize(LogPutLevelDebug | LogOptDisableFunctionName | LogOptDisableAppend, NULL);
#endif
        if (Status != NOERROR) {
            return FALSE;
        }
        LOG_DEBUG(_XOR_("hModule = 0x%016llx"), hModule);

        // Create main thread.
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