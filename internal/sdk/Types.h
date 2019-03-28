#pragma once

#include <stdint.h>
#include <string.h>
#include <native/native.h>
#include <utils/utils.h>
#include <utils/xorstr.h>

#include "TypeTraits.h"
#include "EnumClassFlags.h"
#include "Encryption.h"
#include "Memory.h"

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

template<class ElementType>
class TArray {
    friend struct FString;
public:
    inline TArray() : Data(nullptr), ArrayNum(0), ArrayMax(0) {}

    inline ElementType* GetData() { return (ElementType*)Data; }
    inline const ElementType* GetData() const { return (const ElementType*)Data; }

    inline uint32_t GetTypeSize() const { return sizeof(ElementType); }

    inline bool IsValidIndex(int32_t Index) const { return Index >= 0 && Index < ArrayNum; }

    inline int32_t Num() const { return ArrayNum; }
    inline int32_t Max() const { return ArrayMax; }

    ElementType& operator[](int32_t Index) { return GetData()[Index]; }
    const ElementType& operator[](int32_t Index) const { return GetData()[Index]; }

protected:
    ElementType* Data;
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

struct FString {
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

    ElementType *Get() const { return const_cast<ElementType *>(Data.GetData()); }
};

struct FName {
    inline FName() : Index(0), Number(0) {}
    inline FName(int32_t InIndex, int32_t InNumber) : Index(InIndex), Number(InNumber) {}

    inline int32_t GetIndex() const { return Index; }
    inline int32_t GetNumber() const { return Number; }

    std::string ToString() const;

    inline bool operator==(const FName &other) const { return Index == other.Index; }

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

template<class TEnum>
class TEnumAsByte {
public:
    inline TEnumAsByte() { }
    inline TEnumAsByte(TEnum _value) : Value(static_cast<uint8_t>(_value)) { }
    explicit inline TEnumAsByte(int32_t _value) : Value(static_cast<uint8_t>(_value)) { }
    explicit inline TEnumAsByte(uint8_t _value) : Value(_value) { }

    inline operator TEnum() const { return (TEnum)Value; }
    inline TEnum GetValue() const { return (TEnum)Value; }

private:
    uint8_t Value;
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface {
public:
    InterfaceType* operator->() const { return (InterfaceType*)GetInterface(); }
    InterfaceType& operator*() const { return *(InterfaceType*)GetInterface(); }

    operator bool() const { return GetInterface() != nullptr; }
};

/**
 * ESPMode is used select between either 'fast' or 'thread safe' shared pointer types.
 * This is only used by templates at compile time to generate one code path or another.
 */
enum class ESPMode {
    NotThreadSafe = 0,
    Fast = 0,
    ThreadSafe = 1
};

template<class ObjectType, ESPMode Mode>
class TSharedRef {
public:
    ObjectType* Object;
    uint64_t SharedReferenceCount;
};

class ITextData {
public:
    void **VTable;
};  

struct FText {
public:
    TSharedRef<ITextData, ESPMode::ThreadSafe> TextData;
    uint32_t Flags;
};

struct FWeakObjectPtr {
    int32_t ObjectIndex;
    int32_t ObjectSerialNumber;
};

template<typename TObjectID>
class TPersistentObjectPtr {
public:
    FWeakObjectPtr WeakPtr;
    int32_t TagAtLastTest;
    TObjectID ObjectID;
};

struct UGuid {
    uint32_t A;     // 0x00
    uint32_t B;     // 0x04
    uint32_t C;     // 0x08
    uint32_t D;     // 0x0C
};

struct FUniqueObjectGuid {
    UGuid Guid;     // 0x00
};

struct FSoftObjectPath {
    FName AssetPathName;
    FString SubPathString;
};

struct FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath> {
};
typedef FSoftObjectPtr FAssetPtr;

class FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid> {
};

template <typename TWeakPtr = FWeakObjectPtr>
class TScriptDelegate {
public:
    /** The object bound to this delegate, or nullptr if no object is bound */
    TWeakPtr Object;
    /** Name of the function to call on the bound object */
    FName FunctionName;
};

template <typename TWeakPtr = FWeakObjectPtr>
class TMulticastScriptDelegate {
public:
    /** Ordered list functions to invoke when the Broadcast function is called */
    typedef TArray< TScriptDelegate<TWeakPtr> > FInvocationList;
    mutable FInvocationList InvocationList;     // Mutable so that we can housekeep list even with 'const' broadcasts
};

// Typedef script delegates for convenience.
typedef TScriptDelegate<> FScriptDelegate;
typedef TMulticastScriptDelegate<> FMulticastScriptDelegate;

struct FScriptMulticastDelegate {
    FMulticastScriptDelegate Delegate;
};