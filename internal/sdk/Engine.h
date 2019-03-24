#pragma once

#include "types.h"
#include "memory.h"
#include "cryptvalue.h"

#include <initializer_list>
#include <string>

// Forwarded types
class UObject;
class UClass;
class UProperty;
class UFunction;

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

template<typename KeyType, typename ValueType>
class TPair {
public:
    KeyType Key;
    ValueType Value;
};

template<typename Key, typename Value>
class TMap {
public:
    uint8_t UnknownData[0x50];
};

/**
 * Templated dynamic array
 *
 * A dynamically sized array of typed elements.  Makes the assumption that your elements are relocate-able;
 * i.e. that they can be transparently moved to new memory without a copy constructor.  The main implication
 * is that pointers to elements in the TArray may be invalidated by adding or removing other elements to the array.
 * Removal of elements is O(N) and invalidates the indices of subsequent elements.
 *
 * Caution: as noted below some methods are not safe for element types that require constructors.
 *
 **/
namespace UE4Array_Private {
template <typename FromArrayType, typename ToArrayType>
struct TCanMoveTArrayPointersBetweenArrayTypes {
    typedef typename FromArrayType::Allocator   FromAllocatorType;
    typedef typename ToArrayType::Allocator   ToAllocatorType;
    typedef typename FromArrayType::ElementType FromElementType;
    typedef typename ToArrayType::ElementType ToElementType;

    enum {
        Value =
        TAreTypesEqual<FromAllocatorType, ToAllocatorType>::Value && // Allocators must be equal
        TContainerTraits<FromArrayType>::MoveWillEmptyContainer &&   // A move must be allowed to leave the source array empty
        (TAreTypesEqual<ToElementType, FromElementType>::Value ||    // The element type of the container must be the same, or...
         TIsBitwiseConstructible<ToElementType, FromElementType>::Value    // ... the element type of the source container must be bitwise constructible from the element type in the destination container
         )
    };
};
}

template<class InElementType, typename InAllocator = FDefaultAllocator>
class TArray {

    template <typename OtherInElementType, typename OtherAllocator>
    friend class TArray;

public:
    typedef InElementType ElementType;
    typedef InAllocator Allocator;

    inline TArray() : ArrayNum(0), ArrayMax(0) {}

    inline ElementType* GetData() { return (ElementType*)AllocatorInstance.GetAllocation(); }
    inline const ElementType* GetData() const { return (const ElementType*)AllocatorInstance.GetAllocation(); }

    inline uint32_t GetTypeSize() const { return sizeof(ElementType); }

    inline bool IsValidIndex(int32_t Index) const { return Index >= 0 && Index < ArrayNum; }

    inline int32_t Num() const { return ArrayNum; }
    inline int32_t Max() const { return ArrayMax; }

    ElementType& operator[](int32_t Index) { return GetData()[Index]; }
    const ElementType& operator[](int32_t Index) const { return GetData()[Index]; }

protected:
    typedef typename TChooseClass<
        Allocator::NeedsElementType,
        typename Allocator::template ForElementType<ElementType>,
        typename Allocator::ForAnyElementType
    >::Result ElementAllocatorType;

    ElementAllocatorType AllocatorInstance;
    int32_t ArrayNum;
    int32_t ArrayMax;

private:
    /**
     * DO NOT USE DIRECTLY
     * STL-like iterators to enable range-based for loop support.
     */
    inline friend ElementType*       begin(TArray& Array) { return Array.GetData(); }
    inline friend const ElementType* begin(const TArray& Array) { return Array.GetData(); }
    inline friend ElementType*       end(TArray& Array) { return Array.GetData() + Array.Num(); }
    inline friend const ElementType* end(const TArray& Array) { return Array.GetData() + Array.Num(); }
};

class FString {
private:
    /** Array holding the character data */
    typedef TArray<wchar_t> DataType;
    DataType Data;

public:
    using ElementType = wchar_t;

    std::string ToString() const
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, Data.GetData(), Data.Num(), NULL, 0, NULL, NULL);
        std::string str(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, Data.GetData(), Data.Num(), &str[0], size, NULL, NULL);
        return str;
    }

    wchar_t *Get() const { return const_cast<wchar_t *>(Data.GetData()); }
};

struct FName {
    //FName() : Index(0), Number(0) {}
    FName(int32_t InIndex, int32_t InNumber) : Index(InIndex), Number(InNumber) {}

    inline int32_t GetIndex() const { return Index; }
    inline int32_t GetNumber() const { return Number; }

    std::string ToString() const;

    int32_t Index;  // 0x00
    int32_t Number; // 0x04
};

class FScriptInterface {
public:
    UObject* GetObjectPtr() const { return ObjectPointer; }
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

class FAssetPtr : public TPersistentObjectPtr<FStringAssetReference> {};

struct FGuid {
    uint32_t A;     // 0x00
    uint32_t B;     // 0x04
    uint32_t C;     // 0x08
    uint32_t D;     // 0x0C
};

struct FUniqueObjectGuid {
    FGuid Guid;     // 0x00
};

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid> {};

struct FScriptDelegate {
    uint8_t UnknownData[20];
};

struct FScriptMulticastDelegate {
    uint8_t UnknownData[16];
};

//class UObject {
//public:
//    FPointer VTableObject;                  // 0x00 void**
//    FNameEncrypted NameEncrypted;           // 0x08 FName       <--- NEEDED
//    uint64_t InternalIndexEncrypted;        // 0x10 int32_t     <--- NEEDED
//    uint64_t OuterEncrypted;                // 0x18 UObject*    <--- NEEDED
//    uint32_t ObjectFlagsEncrypted;          // 0x20 int32_t     <--- NEEDED
//    uint64_t ClassEncrypted;                // 0x28 UClass*     <--- NEEDED
//}; // size=0x30

class UObject {
public:
    void** VTable; // 0x0000 (size=0x0008)
    int32_t NameIndexEncrypted; // 0x0008 (size=0x0004)
    int32_t NameNumberEncrypted; // 0x000C (size=0x0004)
    uint64_t OuterEncrypted; // 0x0010 (size=0x0008)
    int32_t ObjectFlagsEncrypted; // 0x0018 (size=0x0004)
    int32_t InternalIndexEncrypted; // 0x001C (size=0x0004)
    uint64_t ClassEncrypted; // 0x0020 (size=0x0008)
}; // size=0x0028
C_ASSERT(FIELD_OFFSET(UObject, InternalIndexEncrypted) == 0x1C);
C_ASSERT(sizeof(UObject) == 0x28);

class UPackage : public UObject {
public:
    bool bDirty;                            // 0x30
    bool bHasBeenFullyLoaded;               // 0x31
    FName FolderName;                       // 0x38
    float LoadTime;                         // 0x40
    FGuid Guid;                             // 0x48
    TArray<int32_t> ChunkIDs;               // 0x58
    FName ForcedExportBasePackageName;      // 0x68
    uint32_t *PackageFlagsPrivate;          // 0x70
    uint8_t pad_0x0078[0xE8];               // 0x78
}; // size=0x160

class UField : public UObject {
public:
    UField* Next; // 0x0028 (size=0x0008)
}; // size=0x0030
C_ASSERT(sizeof(UField) == 0x30);

class UEnum : public UField {
public:
    FString CppType; // 0x0030 (size=0x0010)
    TArray<TPair<FName, int64_t>> Names; // 0x0040 (size=0x0010)
    uint8_t UnknownData0x0050[0x10]; // 0x0050 (size=0x0010)
    int32_t CppForm; // 0x0060 (size=0x0004)
    uint8_t UnknownData0x0064[0x4]; // 0x0064 (size=0x0004)
    void* EnumDisplayNameFn; // 0x0068  (size=0x0008)
}; // size=0x0070
C_ASSERT(sizeof(UEnum) == 0x70);

class UStruct : public UField {
public:
    uint8_t UnknownData0x0030[0x10]; // 0x0030 (size=0x0010)
    int32_t MinAlignment; // 0x0040 (size=0x0004)
    uint8_t UnknownData0x0044[0x7C]; // 0x0044 (size=0x007C)
    class UStruct* SuperStruct; // 0x00C0 (size=0x0008)
    UField* Children; // 0x00C8 (size=0x0008)
    uint8_t UnknownData0x00D0[0x8]; // 0x00D0 (size=0x0008)
    int32_t PropertiesSize; // 0x00D8 (size=0x0004)
    uint8_t UnknownData0x00DC[0x4]; // 0x00DC (size=0x0004)
}; // size=0x00E0
C_ASSERT(sizeof(UStruct) == 0xE0);

class UScriptStruct : public UStruct {
public:
    uint8_t pad_0x00E0[0x10];                  // 0xE0
}; // size=0xF0
C_ASSERT(sizeof(UScriptStruct) == 0xF0);

class UFunction : public UStruct {
public:
    uint8_t UnknownData0x00E0[0x38]; // 0x00E0 (size=0x0038)
    uint32_t FunctionFlags; // 0x0118 (size=0x0004)
    uint8_t UnknownData0x011C[0x14]; // 0x011C (size=0x0014)
}; // size=0x0130
C_ASSERT(sizeof(UFunction) == 0x130);

class FClassBaseChain {
public:
    FClassBaseChain** ClassBaseChainArray;  // 0x00
    int32_t NumClassBasesInChainMinusOne;   // 0x08
};

class UClass : public UStruct, public FClassBaseChain {
public:
    uint8_t pad_0x00F8[0x1C0];              // 0xF8
}; // size=0x2B8

class UProperty : public UField {
public:
    int32_t ArrayDim; // 0x0030 (size=0x0004)
    int32_t ElementSize; // 0x0034 (size=0x0004)
    uint64_t PropertyFlags; // 0x0038 (size=0x0008)
    uint8_t UnknownData0x0040[0x10]; // 0x0040 (size=0x0010)
    int32_t Offset_Internal; // 0x0050 (size=0x0004)
    uint8_t UnknownData0x0054[0x24]; // 0x0054 (size=0x0024)
}; // size=0x0078
C_ASSERT(sizeof(UProperty) == 0x78);

class UNumericProperty : public UProperty {
public:
};

class UByteProperty : public UNumericProperty {
public:
    UEnum *Enum;
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

class UInterfaceProperty : public UProperty {
public:
    UClass* InterfaceClass;
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
    class UNumericProperty* UnderlyingProp;
    class UEnum* Enum;
};

struct FKey {
    struct FName KeyName;
    uint8_t UnknownData0x0008[16];
};