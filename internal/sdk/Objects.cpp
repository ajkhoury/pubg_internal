#include "Objects.h"
#include "Names.h"

#include <native/log.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

#include <sstream>
#include <iterator>

DEFINE_STATIC_CLASS(Object);
DEFINE_STATIC_CLASS(Field);
DEFINE_STATIC_CLASS(Enum);
DEFINE_STATIC_CLASS(Struct);
DEFINE_STATIC_CLASS(ScriptStruct);
DEFINE_STATIC_CLASS(Function);
DEFINE_STATIC_CLASS(Class);

//ObjectProxy ObjectIterator::operator*() const { return ObjectsProxy().GetById(Index); }
//ObjectProxy ObjectIterator::operator->() const { return ObjectsProxy().GetById(Index); }

#ifdef __cplusplus
extern "C" {
#endif
uint64_t DecryptObjectsAsm(uint64_t ObjectsEncrypted);
uint32_t DecryptObjectFlagsAsm(uint32_t ObjectFlagsEncrypted);
uint32_t DecryptObjectInternalIndexAsm(uint32_t InternalIndexEncrypted);
uint64_t DecryptObjectClassAsm(uint64_t ClassEncrypted);
uint64_t DecryptObjectOuterAsm(uint64_t OuterEncrypted);
void DecryptObjectFNameAsm(const int32_t InNameIndexEncrypted,
                           const int32_t InNameNumberEncrypted,
                           int32_t* OutIndex,
                           int32_t* OutNumber);
#ifdef __cplusplus
}
#endif

class TUObjectArray {
public:
    uint64_t ObjectsEncrypted; // 0x00 FUObjectItem *
    int64_t MaxElements; // 0x08
    int32_t NumElements; // 0x10
};

class FUObjectArray {
public:
    int32_t ObjFirstGCIndex; // 0x00
    int32_t ObjLastNonGCIndex; // 0x04
    int32_t MaxObjectsNotConsideredByGC; // 0x08
    bool OpenForDisregardForGC; // 0x0C

    TUObjectArray ObjObjects; // 0x10

    FCriticalSection ObjObjectsCritical; // 0x28
    FThreadSafeCounter ObjAvailableCount; // 0x50
    TArray<void *> UObjectCreateListeners; // 0x58
    TArray<void *> UObjectDeleteListeners; // 0x68

    // .... 
};

static FUObjectArray *GUObjectArray = NULL;

static bool ObjectsInitializeGlobal()
{
    PVOID ImageBase;
    ULONG ImageSize;

    if (GUObjectArray) {
        return true;
    }

    ImageBase = utils::GetModuleHandleWIDE(NULL);
    ImageSize = utils::GetModuleSize((HMODULE)ImageBase);

    // 48 8D 0D ? ? ? ? E8 ? ? ? ? 90 48 8B 5C 24 ? 48 83 C4 30
    //static const UINT8 ObjectsSig[] = {
    //    0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC,   /* lea     rcx, GUObjectArray */
    //    0xE8, 0xCC, 0xCC, 0xCC, 0xCC,               /* call    FUObjectArray__FreeUObjectIndex */
    //    0x90,                                       /* nop */
    //                                                /* loc_7FF785413906: */
    //    0x48, 0x8B, 0x5C, 0x24, 0xCC,               /* mov     rbx, [rsp+48h] */
    //    0x48, 0x83, 0xC4, 0x30                      /* add     rsp, 30h */
    //};
    //const uint8_t *Found;
    //do Found = utils::FindPattern(ImageBase, ImageSize, 0xCC, ObjectsSig, sizeof(ObjectsSig));
    //while (!Found);

    // Search for the GUObjectArray address inside the UObjectBase::Deconstructor routine:
    // .text:7FF7F194D035 90                    nop
    // .text:7FF7F194D036 48 8B D7              mov     rdx, rdi                        ; Object
    // .text:7FF7F194D039 48 8D 0D 88 41 92 04  lea     rcx, GUObjectArray              ; <---- GObjectsArray
    // .text:7FF7F194D040 E8 5B 3A 00 00        call    FUObjectArray__FreeUObjectIndex
    // .text:7FF7F194D045 90                    nop
    // .text:7FF7F194D046                       loc_7FF7F194D046:                       ; CODE XREF: UObjectBase__Deconstructor+28^j
    // .text:7FF7F194D046                                                               ; UObjectBase__Deconstructor+6C^j ...
    // .text:7FF7F194D046 48 8B 5C 24 48        mov     rbx, [rsp+38h+arg_8]
    // .text:7FF7F194D04B 48 83 C4 30           add     rsp, 30h
    // 48 8D 0D ? ? ? ? E8 ? ? ? ? 90 48 8B 5C 24 ? 48 83 C4 30
    const uint8_t *Found;
    do Found = utils::FindPatternIDA(ImageBase, ImageSize, _XOR_("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 90 48 8B 5C 24 ?? 48 83 C4 30"));
    while (!Found);

    GUObjectArray = static_cast<FUObjectArray *>(utils::GetInstructionTarget(Found, 3));
    LOG_INFO(_XOR_("GUObjectArray = 0x%016llx"), GUObjectArray);

    return true;
}

ObjectsProxy::ObjectsProxy()
{
    if (!GUObjectArray) {
        ObjectsInitializeGlobal();
    }

    ObjectArray = static_cast<void *>(GUObjectArray);
}

int32_t ObjectsProxy::GetNum() const
{
    return static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.NumElements;
}

int64_t ObjectsProxy::GetMax() const
{
    return static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.MaxElements;
}

FUObjectItem *ObjectsProxy::GetObjectsPrivate() const
{
    union CryptValue ObjectsDecrypted;
    uint64_t ObjectsEncrypted = static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.ObjectsEncrypted;

    ObjectsDecrypted.Qword = DecryptObjectsAsm(ObjectsEncrypted);

    return (FUObjectItem *)ObjectsDecrypted.Pointer;
}

UObject *ObjectsProxy::GetById(int32_t Index) const
{
    union CryptValue ObjectsDecrypted;
    uint64_t ObjectsEncrypted = static_cast<FUObjectArray *>(ObjectArray)->ObjObjects.ObjectsEncrypted;

    ObjectsDecrypted.Qword = DecryptObjectsAsm(ObjectsEncrypted);

    FUObjectItem *Objects = static_cast<FUObjectItem *>(ObjectsDecrypted.Pointer);

    return Objects[Index].Object;
}



int32_t ObjectProxy::GetFlags() const
{
    return static_cast<int32_t>(DecryptObjectFlagsAsm(Object->ObjectFlagsEncrypted));
}

uint32_t ObjectProxy::GetUniqueId() const
{
    return static_cast<uint32_t>(DecryptObjectInternalIndexAsm(Object->InternalIndexEncrypted));
}

UClass *ObjectProxy::GetClass() const
{
    return (UClass *)DecryptObjectClassAsm(Object->ClassEncrypted);
}

UObject *ObjectProxy::GetOuter() const
{
    return (UObject *)DecryptObjectOuterAsm(Object->OuterEncrypted);
}

FName ObjectProxy::GetFName() const
{
    int32_t Index, Number;
    DecryptObjectFNameAsm(Object->NameIndexEncrypted,
                          Object->NameNumberEncrypted,
                          &Index, &Number);
    return FName(Index, Number);
}

const UPackage* ObjectProxy::GetOutermost() const
{
    UObject* Top = NULL;

    for (UObject* Outer = GetOuter(); Outer; Outer = ObjectProxy(Outer).GetOuter()) {
        Top = Outer;
    }

    return static_cast<const UPackage*>(Top);
}

std::string ObjectProxy::GetName() const
{
    std::string NameString;

    if (IsValid()) {

        FName Name = GetFName();
        NameString = NamesProxy().GetById(Name.Index);
        if (Name.Number > 0) {
            NameString += '_' + std::to_string(Name.Number);
        }

        auto pos = NameString.rfind('/');
        if (pos != std::string::npos) {
            NameString = NameString.substr(pos + 1);
        }
    }

    return NameString;
}

std::string ObjectProxy::GetFullName() const
{
    std::string NameString;
    if (IsValid() && GetClass()) {
        std::string temp;
        for (UObject* Outer = GetOuter(); Outer; Outer = ObjectProxy(Outer).GetOuter()) {
            temp = ObjectProxy(Outer).GetName() + "." + temp;
        }
        ClassProxy ObjectClass = GetClass();
        NameString = ObjectClass.GetName();
        NameString += " ";
        NameString += temp;
        NameString += GetName();
    }
    return NameString;
}

std::string ObjectProxy::GetNameCPP() const
{
    std::string NameString;

    if (IsA<ClassProxy>()) {

        ClassProxy Class = Cast<ClassProxy>();
        while (Class.IsValid()) {
            const std::string ClassName = Class.GetName();
            if (ClassName == "Actor") {
                NameString += "A";
                break;
            }
            if (ClassName == "Object") {
                NameString += "U";
                break;
            }
            Class = ObjectProxy(Class.GetSuper()).Cast<ClassProxy>();
        }

    } else {
        NameString += "F";
    }

    NameString += GetName();
    return NameString;
}

std::string StringifyFlags(const FunctionFlags Flags)
{
    std::vector<const char*> buffer;

    if (Flags & FunctionFlags::Final) { buffer.push_back("Final"); }
    if (Flags & FunctionFlags::RequiredAPI) { buffer.push_back("RequiredAPI"); }
    if (Flags & FunctionFlags::BlueprintAuthorityOnly) { buffer.push_back("BlueprintAuthorityOnly"); }
    if (Flags & FunctionFlags::BlueprintCosmetic) { buffer.push_back("BlueprintCosmetic"); }
    if (Flags & FunctionFlags::Net) { buffer.push_back("Net"); }
    if (Flags & FunctionFlags::NetReliable) { buffer.push_back("NetReliable"); }
    if (Flags & FunctionFlags::NetRequest) { buffer.push_back("NetRequest"); }
    if (Flags & FunctionFlags::Exec) { buffer.push_back("Exec"); }
    if (Flags & FunctionFlags::Native) { buffer.push_back("Native"); }
    if (Flags & FunctionFlags::Event) { buffer.push_back("Event"); }
    if (Flags & FunctionFlags::NetResponse) { buffer.push_back("NetResponse"); }
    if (Flags & FunctionFlags::Static) { buffer.push_back("Static"); }
    if (Flags & FunctionFlags::NetMulticast) { buffer.push_back("NetMulticast"); }
    if (Flags & FunctionFlags::MulticastDelegate) { buffer.push_back("MulticastDelegate"); }
    if (Flags & FunctionFlags::Public) { buffer.push_back("Public"); }
    if (Flags & FunctionFlags::Private) { buffer.push_back("Private"); }
    if (Flags & FunctionFlags::Protected) { buffer.push_back("Protected"); }
    if (Flags & FunctionFlags::Delegate) { buffer.push_back("Delegate"); }
    if (Flags & FunctionFlags::NetServer) { buffer.push_back("NetServer"); }
    if (Flags & FunctionFlags::HasOutParms) { buffer.push_back("HasOutParms"); }
    if (Flags & FunctionFlags::HasDefaults) { buffer.push_back("HasDefaults"); }
    if (Flags & FunctionFlags::NetClient) { buffer.push_back("NetClient"); }
    if (Flags & FunctionFlags::DLLImport) { buffer.push_back("DLLImport"); }
    if (Flags & FunctionFlags::BlueprintCallable) { buffer.push_back("BlueprintCallable"); }
    if (Flags & FunctionFlags::BlueprintEvent) { buffer.push_back("BlueprintEvent"); }
    if (Flags & FunctionFlags::BlueprintPure) { buffer.push_back("BlueprintPure"); }
    if (Flags & FunctionFlags::Const) { buffer.push_back("Const"); }
    if (Flags & FunctionFlags::NetValidate) { buffer.push_back("NetValidate"); }

    switch (buffer.size()) {
    case 0:
        return std::string();
    case 1:
        return std::string(buffer[0]);
    default:
        std::ostringstream os;
        std::copy(buffer.begin(), buffer.end() - 1, std::ostream_iterator<const char*>(os, ", "));
        os << *buffer.rbegin();
        return os.str();
    }
}