#pragma once

#include <stdint.h>
#include <string.h>
#include <native/native.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

#include "TypeTraits.h"
#include "Memory.h"
#include "CryptValue.h"

#include <initializer_list>
#include <string>
#include <vector>

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
    inline friend ElementType* begin(TArray& Array) { return Array.GetData(); }
    inline friend const ElementType* begin(const TArray& Array) { return Array.GetData(); }
    inline friend ElementType* end(TArray& Array) { return Array.GetData() + Array.Num(); }
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
    class UObject* GetObjectPtr() const { return ObjectPointer; }
    class UObject*& GetObjectRef() { return ObjectPointer; }
    void* GetInterface() const { return ObjectPointer ? InterfacePointer : nullptr; }

private:
    class UObject* ObjectPointer;
    void* InterfacePointer;
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface {
public:
    InterfaceType* operator->() const { return (InterfaceType*)GetInterface(); }
    InterfaceType& operator*() const { return *(InterfaceType*)GetInterface(); }

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

class FUObjectItem {
public:
    UObject *Object; // 0x00
    int32_t Flags; // 0x08
    int32_t ClusterIndex; // 0x0C
    int32_t SerialNumber; // 0x10
}; // size=0x18