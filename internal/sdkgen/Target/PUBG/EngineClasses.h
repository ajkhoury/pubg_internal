#pragma once

#include <set>
#include <string>
#include <windows.h>

union CryptValue {

/* Byte Values */

    unsigned char Byte;
    struct {
        unsigned char LoByte;
        unsigned char HiByte;
    };
    struct {
        unsigned char Byte1;
        unsigned char Byte2;
        unsigned char Byte3;
        unsigned char Byte4;
        unsigned char Byte5;
        unsigned char Byte6;
        unsigned char Byte7;
        unsigned char Byte8;
    };

/* Word Values */

    unsigned short Word;
    struct {
        unsigned short LoWord;
        unsigned short HiWord;
    };
    struct {
        unsigned short Word1;
        unsigned short Word2;
        unsigned short Word3;
        unsigned short Word4;
    };

/* Double Word Values */

    unsigned long Dword;
    struct {
        unsigned long LoDword;
        unsigned long HiDword;
    };
    struct {
        unsigned long Dword1;
        unsigned long Dword2;
    };

/* Quad Word Value */

    unsigned __int64 Qword;
};

namespace Windows {
struct CRITICAL_SECTION { void* Opaque1[1]; long Opaque2[2]; void* Opaque3[3]; };
}

struct FCriticalSection {
    Windows::CRITICAL_SECTION CriticalSection;
};

struct FThreadSafeCounter {
    volatile int32_t Counter;
};

struct FPointer {
    uintptr_t Dummy;
};

struct FQWord {
    int32_t A;
    int32_t B;
};

struct FName {
    int32_t ComparisonIndex;
    int32_t Number;
};

template<class T>
struct TArray {
    friend struct FString;

public:
    TArray()
        : Data(nullptr)
        , ArrayNum(0)
        , ArrayMax(0)
    {
    }

    int32_t Num() const { return ArrayNum; }
    int32_t Max() const { return ArrayMax; }

    FORCEINLINE bool IsValidIndex(int32_t Index) const { return Index >= 0 && Index < ArrayNum; }

    T& operator[](int32_t Index) { return Data[Index]; }
    const T& operator[](int32_t Index) const { return Data[Index]; }

private:
    T* Data;
    int32_t ArrayNum;
    int32_t ArrayMax;
};

template<typename KeyType, typename ValueType>
class TPair {
public:
    KeyType   Key;
    ValueType Value;
};

struct FString : public TArray<wchar_t> {
    std::string ToString() const
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, Data, ArrayNum, nullptr, 0, nullptr, nullptr);
        std::string str(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, Data, ArrayNum, &str[0], size, nullptr, nullptr);
        return str;
    }
};

class FScriptInterface {
public:
    UObject* GetObject() const { return ObjectPointer; }
    UObject*& GetObjectRef() { return ObjectPointer; }
    void* GetInterface() const { return ObjectPointer ? InterfacePointer : nullptr; }

private:
    UObject* ObjectPointer;
    void* InterfacePointer;
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface {
public:
    InterfaceType* operator->() const { return (InterfaceType*)GetInterface(); }

    InterfaceType& operator*() const { return *((InterfaceType*)GetInterface()); }

    operator bool() const { return GetInterface() != nullptr; }
};

struct FText {
    char UnknownData[0x18];
};

struct FWeakObjectPtr {
    int32_t ObjectIndex;
    int32_t ObjectSerialNumber;
};

struct FStringAssetReference {
    FString AssetLongPathname;
};

template<typename TObjectID>
class TPersistentObjectPtr {
public:
    FWeakObjectPtr WeakPtr;
    int32_t TagAtLastTest;
    TObjectID ObjectID;
};

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference> {
};

struct FGuid {
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
};

struct FUniqueObjectGuid {
    FGuid Guid;
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid> {
};

struct FScriptDelegate {
    unsigned char UnknownData[20];
};

struct FScriptMulticastDelegate {
    unsigned char UnknownData[16];
};

class UClass;

class UObject {
public:
    FPointer VTableObject;
    int32_t ObjectFlags;
    UClass *ClassPrivate;
    FName NamePrivate;
    UObject *OuterPrivate;
    union CryptValue InternalIndexEncrypted;
};

class UField : public UObject {
public:
    UField* Next;
};

class UEnum : public UField {
public:
    FString CppType; //0x0030 
    TArray<TPair<FName, uint64_t>> Names; //0x0040 
    __int64 CppForm; //0x0050 
};

class UStruct : public UField {
public:
    UStruct* SuperField;
    UField* Children;
    int32_t PropertySize;
    int32_t MinAlignment;
    char pad_0x0048[0x40];
};

class UScriptStruct : public UStruct {
public:
    char pad_0x0088[0x10]; //0x0088
};

class UFunction : public UStruct {
public:
    __int32 FunctionFlags; //0x0088
    __int16 RepOffset; //0x008C
    __int8 NumParms; //0x008E
    char pad_0x008F[0x1]; //0x008F
    __int16 ParmsSize; //0x0090
    __int16 ReturnValueOffset; //0x0092
    __int16 RPCId; //0x0094
    __int16 RPCResponseId; //0x0096
    class UProperty* FirstPropertyToInit; //0x0098
    UFunction* EventGraphFunction; //0x00A0
    __int32 EventGraphCallOffset; //0x00A8
    char pad_0x00AC[0x4]; //0x00AC
    void* Func; //0x00B0
};

class UClass : public UStruct {
public:
    char pad_0x0088[0x198]; //0x0088
};

class UProperty : public UField {
public:
    __int32 ArrayDim; //0x0030 
    __int32 ElementSize; //0x0034 
    FQWord PropertyFlags; //0x0038
    __int32 PropertySize; //0x0040 
    char pad_0x0044[0xC]; //0x0044
    __int32 Offset; //0x0050 
    char pad_0x0054[0x24]; //0x0054
};

class UNumericProperty : public UProperty {
public:
};

class UByteProperty : public UNumericProperty {
public:
    UEnum* Enum;  // 0x0088 (0x04)
};

class UUInt16Property : public UNumericProperty {
public:
};

class UUInt32Property : public UNumericProperty {
public:
};

class UUInt64Property : public UNumericProperty {
public:
};

class UInt8Property : public UNumericProperty {
public:
};

class UInt16Property : public UNumericProperty {
public:
};

class UIntProperty : public UNumericProperty {
public:
};

class UInt64Property : public UNumericProperty {
public:
};

class UFloatProperty : public UNumericProperty {
public:
};

class UDoubleProperty : public UNumericProperty {
public:
};

class UBoolProperty : public UProperty {
public:
    uint8_t FieldSize;
    uint8_t ByteOffset;
    uint8_t ByteMask;
    uint8_t FieldMask;
};

class UObjectPropertyBase : public UProperty {
public:
    UClass* PropertyClass;
};

class UObjectProperty : public UObjectPropertyBase {
public:
};

class UClassProperty : public UObjectProperty {
public:
    UClass* MetaClass;
};

class UInterfaceProperty : public UProperty {
public:
    UClass* InterfaceClass;
};

class UWeakObjectProperty : public UObjectPropertyBase {
public:
};

class ULazyObjectProperty : public UObjectPropertyBase {
public:
};

class UAssetObjectProperty : public UObjectPropertyBase {
public:
};

class UAssetClassProperty : public UAssetObjectProperty {
public:
    UClass* MetaClass;
};

class UNameProperty : public UProperty {
public:
};

class UStructProperty : public UProperty {
public:
    UScriptStruct* Struct;
};

class UStrProperty : public UProperty {
public:
};

class UTextProperty : public UProperty {
public:
};

class UArrayProperty : public UProperty {
public:
    UProperty* Inner;
};

class UMapProperty : public UProperty {
public:
    UProperty* KeyProp;
    UProperty* ValueProp;
};

class UDelegateProperty : public UProperty {
public:
    UFunction* SignatureFunction;
};

class UMulticastDelegateProperty : public UProperty {
public:
    UFunction* SignatureFunction;
};

class UEnumProperty : public UProperty {
public:
    class UNumericProperty* UnderlyingProp; //0x0070
    class UEnum* Enum; //0x0078
}; //Size: 0x0080