#include <native/native.h>
#include <native/log.h>

#include <utils/utils.h>
#include <utils/disasm.h>
#include <utils/xorstr.h>
#include <utils/vthook.h>

#include "sdk/Generator.h"
#include "sdk/OffsetDumper.h"

#include "Sdk.h"

#define DUMP_ONLY 0

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
    //LOG_INFO(_XOR_("GameViewportClient->World = 0x%p"), World);

    ULevel* PersistentLevel = World->GetPersistentLevel();
    if (!PersistentLevel) {
        return;
    }
    //LOG_INFO(_XOR_("World->PersistentLevel = 0x%p"), PersistentLevel);

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

            FVector CharacterPos = Character->K2_GetActorLocation();
            FVector ScreenPos = Canvas->K2_Project(CharacterPos);

            //LOG_DBG_PRINT(_XOR_("Tsl character 0x%p at {%f, %f, %f} -> {%f, %f}\n"),
            //              Character,
            //              CharacterPos.X, CharacterPos.Y, CharacterPos.Z,
            //              ScreenPos.X, ScreenPos.Y);

            if (IsValidScreenPosition(Canvas, ScreenPos)) {

                Canvas->K2_DrawText(RobotoFont,
                                    FString(_XOR_(L"BITCH")),
                                    ScreenPos,
                                    FLinearColor(255, 255, 255),
                                    2.0f,
                                    FLinearColor(0, 0, 0),
                                    FVector2D(),
                                    false,
                                    true,
                                    true,
                                    FLinearColor(0, 0, 0));
            
                Canvas->K2_DrawLine(ScreenPos,
                                    FVector2D(10.f, 10.f),
                                    1.0f,
                                    FLinearColor(255, 0, 0));
            }

        }
    }

    //// Iterate the objects.
    //ObjectsProxy Objects;
    //for (int32_t i = 0; i < Objects.GetNum(); i++) {
    //    UObject* Object = Objects.GetById(i);
    //    if (Object && Object->IsValid()) {
    //
    //        // Local player controller.
    //        if (Object->IsA<APlayerController>()) {
    //
    //            APlayerController* PlayerController = Object->Cast<APlayerController>();
    //            if (IsValidLocalController(PlayerController)) {
    //                LOG_DBG_PRINT(_XOR_("PlayerController 0x%p -> HUD = 0x%p\n"),
    //                              PlayerController, PlayerController->GetHUD());
    //            }
    //        }
    //
    //        // Player ESP.
    //        if (Object->IsA<ATslCharacter>()) {
    //            const ATslCharacter* Character = Object->Cast<ATslCharacter>();
    //            
    //            FVector CharacterPos = Character->K2_GetActorLocation();
    //            FVector ScreenPos = Canvas->K2_Project(CharacterPos);
    //
    //            //LOG_DBG_PRINT(_XOR_("Found player 0x%p at {%f, %f, %f} -> {%f, %f}\n"),
    //            //              Character,
    //            //              CharacterPos.X, CharacterPos.Y, CharacterPos.Z,
    //            //              ScreenPos.X, ScreenPos.Y);
    //            //LOG_DBG_PRINT(_XOR_("Canvas size = {%f, %f}\n"), Canvas->SizeX, Canvas->SizeY);
    //
    //            if (IsValidScreenPosition(Canvas, ScreenPos)) {
    //
    //                Canvas->K2_DrawText(RobotoDistanceFieldFont,
    //                                    FString(_XOR_(L"BITCH")),
    //                                    ScreenPos,
    //                                    FLinearColor(255, 255, 255),
    //                                    1.0f,
    //                                    FLinearColor(0, 0, 0),
    //                                    FVector2D(),
    //                                    false,
    //                                    true,
    //                                    false,
    //                                    FLinearColor());
    //
    //                Canvas->K2_DrawLine(ScreenPos,
    //                                    FVector2D(50.f, 50.f),
    //                                    1.0f,
    //                                    FLinearColor(255, 0, 0));
    //            }
    //        }
    //    }
    //}
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

#if defined(ENABLE_SDK) && (DUMP_ONLY != 0)
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

#if (DUMP_ONLY == 0)
    unreal::InitializeUnrealObjectSizeMap();

    Generator Gen(_XOR_("F:\\Projects\\nvdid\\internal\\sdk"), true, true);

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Object"), {
        { _XORSTR_("void**"),     _XORSTR_("VTable"),                   sizeof(void**),     0x00 },
        { _XORSTR_("int32_t"),    _XORSTR_("ObjectFlagsEncrypted"),     sizeof(int32_t),    unreal::ObjectFlagsEncryptedOffset },
        { _XORSTR_("uint64_t"),   _XORSTR_("OuterEncrypted"),           sizeof(uint64_t),   unreal::ObjectOuterEncryptedOffset },
        { _XORSTR_("int32_t"),    _XORSTR_("InternalIndexEncrypted"),   sizeof(int32_t),    unreal::ObjectInternalIndexEncryptedOffset },
        { _XORSTR_("uint64_t"),   _XORSTR_("ClassEncrypted"),           sizeof(uint64_t),   unreal::ObjectClassEncryptedOffset },
        { _XORSTR_("int32_t"),    _XORSTR_("NameIndexEncrypted"),       sizeof(int32_t),    unreal::ObjectNameIndexEncryptedOffset },
        { _XORSTR_("int32_t"),    _XORSTR_("NameNumberEncrypted"),      sizeof(int32_t),    unreal::ObjectNameNumberEncryptedOffset }
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Field"), {
        { _XORSTR_("class UField*"),  _XORSTR_("Next"),                 sizeof(UField*),    unreal::GetObjectSize(_XORSTR_(L"UObject")) }
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Struct"), {
        { _XORSTR_("int32_t"),        _XORSTR_("PropertiesSize"),       sizeof(int32_t),    unreal::StructPropertiesSizeOffset },
        { _XORSTR_("int32_t"),        _XORSTR_("MinAlignment"),         sizeof(int32_t),    unreal::StructMinAlignmentOffset },
        { _XORSTR_("UField*"),        _XORSTR_("Children"),             sizeof(UField*),    unreal::StructChildrenOffset },
        { _XORSTR_("class UStruct*"), _XORSTR_("SuperStruct"),          sizeof(UStruct*),   unreal::StructSuperStructOffset },
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Function"), {
        { _XORSTR_("uint32_t"),       _XORSTR_("FunctionFlags"),        sizeof(uint32_t),   unreal::FunctionFlagsOffset },
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Property"), {
        { _XORSTR_("int32_t"),        _XORSTR_("ElementSize"),          sizeof(int32_t),   unreal::PropertyElementSizeOffset },
        { _XORSTR_("int32_t"),        _XORSTR_("ArrayDim"),             sizeof(int32_t),   unreal::PropertyArrayDimOffset },
        { _XORSTR_("int32_t"),        _XORSTR_("Offset_Internal"),      sizeof(int32_t),   unreal::PropertyOffsetInternalOffset },
        { _XORSTR_("uint64_t"),       _XORSTR_("PropertyFlags"),        sizeof(uint64_t),  unreal::PropertyFlagsOffset },
    });

    Gen.AddPredefinedClassMembers(_XORSTR_("Class CoreUObject.Enum"), {
        { _XORSTR_("int32_t"),        _XORSTR_("CppForm"),              sizeof(int32_t),    unreal::EnumCppFormOffset },
        { _XORSTR_("TArray<TPair<FName, int64_t>>"), _XORSTR_("Names"), sizeof(TArray<TPair<FName, int64_t>>), unreal::EnumNamesOffset },
    });

    Gen.AddPredefinedMethods(_XORSTR_("Class CoreUObject.Object"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
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
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
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
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    static UClass* FindClass(const std::string& Name)\n"
    "    {\n"
    "        return FindObject<class UClass>(Name);\n"
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
        GeneratorPredefinedMethod::Default(_XORSTR_("class UClass const* GetClass() const"), _XORSTR_(
    "UClass const* UObject::GetClass() const\n"
    "{\n"
    "    return (UClass const*)DecryptObjectClassAsm(ClassEncrypted);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("class UObject const* GetOuter() const"), _XORSTR_(
    "UObject const* UObject::GetOuter() const\n"
    "{\n"
    "    return (UObject const*)DecryptObjectOuterAsm(OuterEncrypted);\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("FName GetFName() const"), _XORSTR_(
    "FName UObject::GetFName() const\n"
    "{\n"
    "    FName Name;\n"
    "    DecryptObjectFNameAsm(NameIndexEncrypted, NameNumberEncrypted, &Name.Index, &Name.Number);\n"
    "    return Name;\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("class UPackage const* GetOutermost() const"), _XORSTR_(
    "UPackage const* UObject::GetOutermost() const\n"
    "{\n"
    "    UObject const* Top = NULL;\n"
    "    for (UObject const* Outer = GetOuter(); Outer; Outer = Outer->GetOuter()) {\n"
    "        Top = Outer;\n"
    "    }\n"
    "    return static_cast<UPackage const*>(Top);\n"
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
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("std::string GetNameCPP() const"), _XORSTR_(
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
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("bool IsA(class UClass const* CmpClass) const"), _XORSTR_(
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
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    inline bool IsA() const { return IsA(T::StaticClass()); }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    template<typename T>\n"
    "    inline T* Cast() { return static_cast<T*>(this); }\n"
    "    template<typename T>\n"
    "    inline T const* Cast() const { return static_cast<T const*>(this); }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline bool IsValid() const { return GetFName().Index != 0 && GetFlags() != 0 && GetUniqueId() != 0; }"
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
    "    inline class UStruct* GetSuper() const { return SuperStruct; }\n"
    "    inline class UField* GetChildren() const { return Children; }\n"
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
    "    inline T const* CreateDefaultObject() const { return static_cast<T const*>(CreateDefaultObject()); }"
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

    Gen.AddPredefinedMethods(_XORSTR_("ScriptStruct CoreUObject.Vector"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector() : X(0), Y(0), Z(0) {}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}"
        ))
    });

    Gen.AddPredefinedMethods(_XORSTR_("ScriptStruct CoreUObject.Vector2D"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector2D() : X(0), Y(0) {}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector2D(float x, float y) : X(x), Y(y) {}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FVector2D(const FVector& V) : X(V.X), Y(V.Y) {}"
        ))
    });

    Gen.AddPredefinedMethods(_XORSTR_("ScriptStruct CoreUObject.LinearColor"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FLinearColor() : R(0), G(0), B(0), A(1.f) {}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FLinearColor(float r, float g, float b, float a = 1.f) : R(r), G(g), B(b), A(a) {}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FLinearColor(const FColor& Color)\n"
    "    {\n"
    "        R = sRGBToLinearTable[Color.R];\n"
    "        G = sRGBToLinearTable[Color.G];\n"
    "        B = sRGBToLinearTable[Color.B];\n"
    "        A = float(Color.A) * (1.0f / 255.0f);\n"
    "    }"
        ))
    });

    Gen.AddPredefinedClassStaticMembers(_XORSTR_("ScriptStruct CoreUObject.LinearColor"), {
        { _XORSTR_("float"), _XORSTR_("sRGBToLinearTable[256]"), "{\n"
    "    0,\n"
    "    0.000303526983548838, 0.000607053967097675, 0.000910580950646512, 0.00121410793419535, 0.00151763491774419,\n"
    "    0.00182116190129302, 0.00212468888484186, 0.0024282158683907, 0.00273174285193954, 0.00303526983548838,\n"
    "    0.00334653564113713, 0.00367650719436314, 0.00402471688178252, 0.00439144189356217, 0.00477695332960869,\n"
    "    0.005181516543916, 0.00560539145834456, 0.00604883284946662, 0.00651209061157708, 0.00699540999852809,\n"
    "    0.00749903184667767, 0.00802319278093555, 0.0085681254056307, 0.00913405848170623, 0.00972121709156193,\n"
    "    0.0103298227927056, 0.0109600937612386, 0.0116122449260844, 0.012286488094766, 0.0129830320714536,\n"
    "    0.0137020827679224, 0.0144438433080002, 0.0152085141260192, 0.0159962930597398, 0.0168073754381669,\n"
    "    0.0176419541646397, 0.0185002197955389, 0.0193823606149269, 0.0202885627054049, 0.0212190100154473,\n"
    "    0.0221738844234532, 0.02315336579873, 0.0241576320596103, 0.0251868592288862, 0.0262412214867272,\n"
    "    0.0273208912212394, 0.0284260390768075, 0.0295568340003534, 0.0307134432856324, 0.0318960326156814,\n"
    "    0.0331047661035236, 0.0343398063312275, 0.0356013143874111, 0.0368894499032755, 0.0382043710872463,\n"
    "    0.0395462347582974, 0.0409151963780232, 0.0423114100815264, 0.0437350287071788, 0.0451862038253117,\n"
    "    0.0466650857658898, 0.0481718236452158, 0.049706565391714, 0.0512694577708345, 0.0528606464091205,\n"
    "    0.0544802758174765, 0.0561284894136735, 0.0578054295441256, 0.0595112375049707, 0.0612460535624849,\n"
    "    0.0630100169728596, 0.0648032660013696, 0.0666259379409563, 0.0684781691302512, 0.070360094971063,\n"
    "    0.0722718499453493, 0.0742135676316953, 0.0761853807213167, 0.0781874210336082, 0.0802198195312533,\n"
    "    0.0822827063349132, 0.0843762107375113, 0.0865004612181274, 0.0886555854555171, 0.0908417103412699,\n"
    "    0.0930589619926197, 0.0953074657649191, 0.0975873462637915, 0.0998987273569704, 0.102241732185838,\n"
    "    0.104616483176675, 0.107023102051626, 0.109461709839399, 0.1119324268857, 0.114435372863418,\n"
    "    0.116970666782559, 0.119538426999953, 0.122138771228724, 0.124771816547542, 0.127437679409664,\n"
    "    0.130136475651761, 0.132868320502552, 0.135633328591233, 0.138431613955729, 0.141263290050755,\n"
    "    0.144128469755705, 0.147027265382362, 0.149959788682454, 0.152926150855031, 0.155926462553701,\n"
    "    0.158960833893705, 0.162029374458845, 0.16513219330827, 0.168269398983119, 0.171441099513036,\n"
    "    0.174647402422543, 0.17788841473729, 0.181164242990184, 0.184474993227387, 0.187820771014205,\n"
    "    0.191201681440861, 0.194617829128147, 0.198069318232982, 0.201556252453853, 0.205078735036156,\n"
    "    0.208636868777438, 0.212230756032542, 0.215860498718652, 0.219526198320249, 0.223227955893977,\n"
    "    0.226965872073417, 0.23074004707378, 0.23455058069651, 0.238397572333811, 0.242281120973093,\n"
    "    0.246201325201334, 0.250158283209375, 0.254152092796134, 0.258182851372752, 0.262250655966664,\n"
    "    0.266355603225604, 0.270497789421545, 0.274677310454565, 0.278894261856656, 0.283148738795466,\n"
    "    0.287440836077983, 0.291770648154158, 0.296138269120463, 0.300543792723403, 0.304987312362961,\n"
    "    0.309468921095997, 0.313988711639584, 0.3185467763743, 0.323143207347467, 0.32777809627633,\n"
    "    0.332451534551205, 0.337163613238559, 0.341914423084057, 0.346704054515559, 0.351532597646068,\n"
    "    0.356400142276637, 0.361306777899234, 0.36625259369956, 0.371237678559833, 0.376262121061519,\n"
    "    0.381326009488037, 0.386429431827418, 0.39157247577492, 0.396755228735618, 0.401977777826949,\n"
    "    0.407240209881218, 0.41254261144808, 0.417885068796976, 0.423267667919539, 0.428690494531971,\n"
    "    0.434153634077377, 0.439657171728079, 0.445201192387887, 0.450785780694349, 0.456411021020965,\n"
    "    0.462076997479369, 0.467783793921492, 0.473531493941681, 0.479320180878805, 0.485149937818323,\n"
    "    0.491020847594331, 0.496932992791578, 0.502886455747457, 0.50888131855397, 0.514917663059676,\n"
    "    0.520995570871595, 0.527115123357109, 0.533276401645826, 0.539479486631421, 0.545724458973463,\n"
    "    0.552011399099209, 0.558340387205378, 0.56471150325991, 0.571124827003694, 0.577580437952282,\n"
    "    0.584078415397575, 0.590618838409497, 0.597201785837643, 0.603827336312907, 0.610495568249093,\n"
    "    0.617206559844509, 0.623960389083534, 0.630757133738175, 0.637596871369601, 0.644479679329661,\n"
    "    0.651405634762384, 0.658374814605461, 0.665387295591707, 0.672443154250516, 0.679542466909286,\n"
    "    0.686685309694841, 0.693871758534824, 0.701101889159085, 0.708375777101046, 0.71569349769906,\n"
    "    0.723055126097739, 0.730460737249286, 0.737910405914797, 0.745404206665559, 0.752942213884326,\n"
    "    0.760524501766589, 0.768151144321824, 0.775822215374732, 0.783537788566466, 0.791297937355839,\n"
    "    0.799102735020525, 0.806952254658248, 0.81484656918795, 0.822785751350956, 0.830769873712124,\n"
    "    0.838799008660978, 0.846873228412837, 0.854992605009927, 0.863157210322481, 0.871367116049835,\n"
    "    0.879622393721502, 0.887923114698241, 0.896269350173118, 0.904661171172551, 0.913098648557343,\n"
    "    0.921581853023715, 0.930110855104312, 0.938685725169219, 0.947306533426946, 0.955973349925421,\n"
    "    0.964686244552961, 0.973445287039244, 0.982250546956257, 0.991102093719252, 1.0,\n"
    "}" },
        { _XORSTR_("const FLinearColor"), _XORSTR_("White"), _XORSTR_("FLinearColor(1.f,1.f,1.f)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Gray"), _XORSTR_("FLinearColor(0.5f,0.5f,0.5f)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Black"), _XORSTR_("FLinearColor(0,0,0)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Transparent"), _XORSTR_("FLinearColor(0,0,0,0)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Red"), _XORSTR_("FLinearColor(1.f,0,0)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Green"), _XORSTR_("FLinearColor(0,1.f,0)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Blue"), _XORSTR_("FLinearColor(0,0,1.f)") },
        { _XORSTR_("const FLinearColor"), _XORSTR_("Yellow"), _XORSTR_("FLinearColor(1.f,1.f,0)") },
    });

    Gen.AddPredefinedMethods(_XORSTR_("ScriptStruct CoreUObject.Color"), {
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FColor() { R = G = B = 0; A = 255; }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FColor(uint8_t InR, uint8_t InG, uint8_t InB, uint8_t InA = 255) { R = InR; G = InG; B = InB; A = InA; }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline explicit FColor(uint32_t InColor) { DWColor() = InColor; }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    uint32& DWColor(void) {return *((uint32*)this);}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    const uint32& DWColor(void) const {return *((uint32*)this);}"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline bool operator==(const FColor &C) const { return DWColor() == C.DWColor(); }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline bool operator==(const FColor &C) const { return DWColor() != C.DWColor(); }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FColor WithAlpha(uint8_t Alpha) const { return FColor(R, G, B, Alpha); }"
        )),
        GeneratorPredefinedMethod::Inline(_XORSTR_(
    "    inline FLinearColor ReinterpretAsLinear() const { return FLinearColor(R / 255.f, G / 255.f, B / 255.f, A / 255.f); }"
        )),
    });

    Gen.AddPredefinedClassStaticMembers(_XORSTR_("ScriptStruct CoreUObject.LinearColor"), {
        { _XORSTR_("const FColor"), _XORSTR_("White"), _XORSTR_("FColor(255,255,255)") },
        { _XORSTR_("const FColor"), _XORSTR_("Gray"), _XORSTR_("FColor(127,127,127)") },
        { _XORSTR_("const FColor"), _XORSTR_("Black"), _XORSTR_("FColor(0,0,0)") },
        { _XORSTR_("const FColor"), _XORSTR_("Transparent"), _XORSTR_("FColor(0,0,0,0)") },
        { _XORSTR_("const FColor"), _XORSTR_("Red"), _XORSTR_("FColor(255,0,0)") },
        { _XORSTR_("const FColor"), _XORSTR_("Green"), _XORSTR_("FColor(0,255,0)") },
        { _XORSTR_("const FColor"), _XORSTR_("Blue"), _XORSTR_("FColor(0,0,255)") },
        { _XORSTR_("const FColor"), _XORSTR_("Yellow"), _XORSTR_("FColor(255,255,0)") },
        { _XORSTR_("const FColor"), _XORSTR_("Cyan"), _XORSTR_("FColor(0,255,255)") },
        { _XORSTR_("const FColor"), _XORSTR_("Magenta"), _XORSTR_("FColor(255,0,255)") },
        { _XORSTR_("const FColor"), _XORSTR_("Orange"), _XORSTR_("FColor(243, 156, 18)") },
        { _XORSTR_("const FColor"), _XORSTR_("Purple"), _XORSTR_("FColor(169, 7, 228)") },
        { _XORSTR_("const FColor"), _XORSTR_("Turquoise"), _XORSTR_("FColor(26, 188, 156)") },
        { _XORSTR_("const FColor"), _XORSTR_("Silver"), _XORSTR_("FColor(189, 195, 199)") },
        { _XORSTR_("const FColor"), _XORSTR_("Emerald"), _XORSTR_("FColor(46, 204, 113)") },
    });

    Gen.AddPredefinedMethods(_XORSTR_("Class Engine.World"), {
        GeneratorPredefinedMethod::Inline(tfm::format(_XOR_(
    "    inline uint64_t GetPersistentLevelEncrypted() const\n"
    "    {\n"
    "        return *(uint64_t*)(reinterpret_cast<const uint8_t*>(this) + 0x%X);\n"
    "    }"), unreal::WorldPersistentLevelEncryptedOffset
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("class ULevel* GetPersistentLevel() const"), _XORSTR_(
    "ULevel* UWorld::GetPersistentLevel() const\n"
    "{\n"
    "    return (ULevel*)DecryptPersistentLevelAsm(GetPersistentLevelEncrypted());\n"
    "}"
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("class ULevel* GetCurrentLevel() const"), _XORSTR_(
    "ULevel* UWorld::GetCurrentLevel() const\n"
    "{\n"
    "    return (ULevel*)DecryptCurrentLevelAsm((uint64_t)CurrentLevel);\n"
    "}"
        ))
    });

    Gen.AddPredefinedMethods(_XORSTR_("Class Engine.Level"), {
        GeneratorPredefinedMethod::Inline(tfm::format(_XOR_(
    "    inline uint64_t GetActorsEncrypted() const\n"
    "    {\n"
    "        return *(uint64_t*)(reinterpret_cast<const uint8_t*>(this) + 0x%X);\n"
    "    }"), unreal::LevelActorsEncryptedOffset
        )),
        GeneratorPredefinedMethod::Default(_XORSTR_("TArray<class AActor*>* GetActors() const"), _XORSTR_(
    "TArray<AActor*>* ULevel::GetActors() const\n"
    "{\n"
    "    return (TArray<AActor*>*)DecryptActorsAsm(GetActorsEncrypted());\n"
    "}"
        ))
    });

    Gen.AddVirtualFunctionPattern(_XORSTR_("Class CoreUObject.Object"),
                                  0x200,
                                  _XORSTR_("45 33 F6 BF ?? ?? ?? ?? 39 3D"),
        _XORSTR_(
    "    inline void ProcessEvent(class UFunction* fn, void* parms) const\n"
    "    {\n"
    "        return utils::GetVFunction<void(*)(UObject const*, class UFunction*, void*)>(this, %d)(this, fn, parms);\n"
    "    }"));

    Gen.AddVirtualFunctionPattern(_XORSTR_("Class CoreUObject.Class"),
                                  0x200,
                                  _XORSTR_("45 33 E4 4C 89 64 24 ?? 48 85 DB"),
        _XORSTR_(
    "    inline class UObject* CreateDefaultObject() const\n"
    "    {\n"
    "        return utils::GetVFunction<class UObject*(*)(class UClass const*)>(this, %d)(this);\n"
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