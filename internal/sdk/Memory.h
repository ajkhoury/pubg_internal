#pragma once

#include "typetraits.h"
#include <memory.h>
#include <intrin.h>

enum {
    // Default allocator alignment. If the default is specified, the allocator applies to engine rules.
    // Blocks >= 16 bytes will be 16-byte-aligned, Blocks < 16 will be 8-byte aligned. If the allocator does
    // not support allocation alignment, the alignment will be ignored.
    DEFAULT_ALIGNMENT = 0,

    // Minimum allocator alignment
    MIN_ALIGNMENT = 8,
};

static inline uint32_t CountTrailingZeros(uint32_t Value)
{
    if (Value == 0) {
        return 32;
    }
    unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
    _BitScanForward(&BitIndex, Value);	// Scans from LSB to MSB
    return BitIndex;
}

template <typename T>
static inline void Valswap(T& A, T& B)
{
    // Usually such an implementation would use move semantics, but
    // we're only ever going to call it on fundamental types and MoveTemp
    // is not necessarily in scope here anyway, so we don't want to
    // #include it if we don't need to.
    T Tmp = A;
    A = B;
    B = Tmp;
}

struct FMemory {

    static inline void* Malloc(size_t Count, uint32_t Alignment = DEFAULT_ALIGNMENT)
    {
        return ::_aligned_malloc(Count, Alignment);
    }

    static inline void* Realloc(void* Original, size_t Count, uint32_t Alignment = DEFAULT_ALIGNMENT)
    {
        return ::_aligned_realloc(Original, Count, Alignment);
    }

    static inline void Free(void* Original)
    {
        return ::_aligned_free(Original);
    }

    static size_t GetAllocSize(void* Original)
    {
        return _msize(Original);
    }

    /**
    * For some allocators this will return the actual size that should be requested to eliminate
    * internal fragmentation. The return value will always be >= Count. This can be used to grow
    * and shrink containers to optimal sizes.
    * This call is always fast and threadsafe with no locking.
    */
    static size_t QuantizeSize(size_t Count, uint32_t Alignment = DEFAULT_ALIGNMENT)
    {
        return Count; // Default implementation has no way of determining this
    }

    static inline void* Memmove(void* Dest, const void* Src, size_t Count)
    {
        return memmove(Dest, Src, Count);
    }

    static inline int32_t Memcmp(const void* Buf1, const void* Buf2, size_t Count)
    {
        return memcmp(Buf1, Buf2, Count);
    }

    static inline void* Memset(void* Dest, uint8_t Char, size_t Count)
    {
        return memset(Dest, Char, Count);
    }

    template<class T>
    static inline void Memset(T& Src, uint8_t ValueToSet)
    {
        static_assert(!TIsPointer<T>::Value, "For pointers use the three parameters function");
        memset((void *)&Src, ValueToSet, sizeof(T));
    }

    static inline void* Memzero(void* Dest, size_t Count)
    {
        return memset(Dest, 0, Count);
    }

    template< class T >
    static inline void Memzero(T& Src)
    {
        static_assert(!TIsPointer<T>::Value, "For pointers use the two parameters function");
        Memzero(&Src, sizeof(T));
    }

    static inline void* Memcpy(void* Dest, const void* Src, size_t Count)
    {
        return memcpy(Dest, Src, Count);
    }

    template< class T >
    static inline void Memcpy(T& Dest, const T& Src)
    {
        static_assert(!TIsPointer<T>::Value, "For pointers use the three parameters function");
        Memcpy(&Dest, &Src, sizeof(T));
    }

    static inline void* BigBlockMemcpy(void* Dest, const void* Src, size_t Count)
    {
        return memcpy(Dest, Src, Count);
    }

    static inline void* StreamingMemcpy(void* Dest, const void* Src, size_t Count)
    {
        return memcpy(Dest, Src, Count);
    }

    static inline void Memswap(void* Ptr1, void* Ptr2, size_t Size)
    {
        MemswapInternal(Ptr1, Ptr2, Size);
    }



public:
    static void MemswapGreaterThan8(void* __restrict Ptr1, void* __restrict Ptr2, size_t Size)
    {
        union PtrUnion {
            void*   PtrVoid;
            uint8_t*  Ptr8;
            uint16_t* Ptr16;
            uint32_t* Ptr32;
            uint64_t* Ptr64;
            uintptr_t PtrUint;
        };

        PtrUnion Union1 = { Ptr1 };
        PtrUnion Union2 = { Ptr2 };

        if (Union1.PtrUint & 1) {
            Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
            Size -= 1;
        }
        if (Union1.PtrUint & 2) {
            Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
            Size -= 2;
        }
        if (Union1.PtrUint & 4) {
            Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
            Size -= 4;
        }

        uint32_t CommonAlignment;
        uint32_t CommonAlignmentValue = CountTrailingZeros((uint32_t)(Union1.PtrUint - Union2.PtrUint));
        if (CommonAlignmentValue < 3) {
            CommonAlignment = CommonAlignmentValue;
        } else {
            CommonAlignment = 3u;
        }

        switch (CommonAlignment) {
        default:
            for (; Size >= 8; Size -= 8) {
                Valswap(*Union1.Ptr64++, *Union2.Ptr64++);
            }

        case 2:
            for (; Size >= 4; Size -= 4) {
                Valswap(*Union1.Ptr32++, *Union2.Ptr32++);
            }

        case 1:
            for (; Size >= 2; Size -= 2) {
                Valswap(*Union1.Ptr16++, *Union2.Ptr16++);
            }

        case 0:
            for (; Size >= 1; Size -= 1) {
                Valswap(*Union1.Ptr8++, *Union2.Ptr8++);
            }
        }
    }

    static inline void MemswapInternal(void* Ptr1, void* Ptr2, size_t Size)
    {
        switch (Size) {
        case 0:
            break;

        case 1:
            Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
            break;

        case 2:
            Valswap(*(uint16_t*)Ptr1, *(uint16_t*)Ptr2);
            break;

        case 3:
            Valswap(*((uint16_t*&)Ptr1)++, *((uint16_t*&)Ptr2)++);
            Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
            break;

        case 4:
            Valswap(*(uint32_t*)Ptr1, *(uint32_t*)Ptr2);
            break;

        case 5:
            Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
            Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
            break;

        case 6:
            Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
            Valswap(*(uint16_t*)Ptr1, *(uint16_t*)Ptr2);
            break;

        case 7:
            Valswap(*((uint32_t*&)Ptr1)++, *((uint32_t*&)Ptr2)++);
            Valswap(*((uint16_t*&)Ptr1)++, *((uint16_t*&)Ptr2)++);
            Valswap(*(uint8_t*)Ptr1, *(uint8_t*)Ptr2);
            break;

        case 8:
            Valswap(*(uint64_t*)Ptr1, *(uint64_t*)Ptr2);
            break;

        case 16:
            Valswap(((uint64_t*)Ptr1)[0], ((uint64_t*)Ptr2)[0]);
            Valswap(((uint64_t*)Ptr1)[1], ((uint64_t*)Ptr2)[1]);
            break;

        default:
            MemswapGreaterThan8(Ptr1, Ptr2, Size);
            break;
        }
    }
};

namespace UE4MemoryOps_Private {
template <typename DestinationElementType, typename SourceElementType>
struct TCanBitwiseRelocate {
    enum {
        Value =
        TOr<
        TAreTypesEqual<DestinationElementType, SourceElementType>,
        TAnd<
        TIsBitwiseConstructible<DestinationElementType, SourceElementType>,
        TIsTriviallyDestructible<SourceElementType>
        >
        >::Value
    };
};
}

/**
 * Default constructs a range of items in memory.
 *
 * @param   Elements    The address of the first memory location to construct at.
 * @param   Count       The number of elements to destruct.
 */
template <typename ElementType>
inline typename TEnableIf<!TIsZeroConstructType<ElementType>::Value>::Type DefaultConstructItems(void* Address, int32_t Count)
{
    ElementType* Element = (ElementType*)Address;
    while (Count) {
        new (Element) ElementType;
        ++Element;
        --Count;
    }
}


template <typename ElementType>
inline typename TEnableIf<TIsZeroConstructType<ElementType>::Value>::Type DefaultConstructItems(void* Elements, int32_t Count)
{
    FMemory::Memset(Elements, 0, sizeof(ElementType) * Count);
}

namespace UE4IsTriviallyDestructible_Private {
    // We have this specialization for enums to avoid the need to have a full definition of
    // the type.
template <typename T, bool bIsTriviallyTriviallyDestructible = __is_enum(T)>
struct TImpl {
    enum { Value = true };
};

template <typename T>
struct TImpl<T, false> {
    enum { Value = __has_trivial_destructor(T) };
};
}

/**
 * Traits class which tests if a type has a trivial destructor.
 */
template <typename T>
struct TIsTriviallyDestructible {
    enum { Value = UE4IsTriviallyDestructible_Private::TImpl<T>::Value };
};



/**
 * Destructs a single item in memory.
 *
 * @param   Elements    A pointer to the item to destruct.
 *
 * @note: This function is optimized for values of T, and so will not dynamically dispatch destructor calls if T's destructor is virtual.
 */
template <typename ElementType>
inline typename TEnableIf<!TIsTriviallyDestructible<ElementType>::Value>::Type DestructItem(ElementType* Element)
{
    // We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
    typedef ElementType DestructItemsElementTypeTypedef;

    Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
}


template <typename ElementType>
inline typename TEnableIf<TIsTriviallyDestructible<ElementType>::Value>::Type DestructItem(ElementType* Element)
{
}


/**
 * Destructs a range of items in memory.
 *
 * @param   Elements    A pointer to the first item to destruct.
 * @param   Count       The number of elements to destruct.
 *
 * @note: This function is optimized for values of T, and so will not dynamically dispatch destructor calls if T's destructor is virtual.
 */
template <typename ElementType>
inline typename TEnableIf<!TIsTriviallyDestructible<ElementType>::Value>::Type DestructItems(ElementType* Element, int32_t Count)
{
    while (Count) {
        // We need a typedef here because VC won't compile the destructor call below if ElementType itself has a member called ElementType
        typedef ElementType DestructItemsElementTypeTypedef;

        Element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
        ++Element;
        --Count;
    }
}


template <typename ElementType>
inline typename TEnableIf<TIsTriviallyDestructible<ElementType>::Value>::Type DestructItems(ElementType* Elements, int32_t Count)
{
}


/**
 * Constructs a range of items into memory from a set of arguments.  The arguments come from an another array.
 *
 * @param   Dest        The memory location to start copying into.
 * @param   Source      A pointer to the first argument to pass to the constructor.
 * @param   Count       The number of elements to copy.
 */
template <typename DestinationElementType, typename SourceElementType>
inline typename TEnableIf<!TIsBitwiseConstructible<DestinationElementType, SourceElementType>::Value>::Type ConstructItems(void* Dest, const SourceElementType* Source, int32_t Count)
{
    while (Count) {
        new (Dest) DestinationElementType(*Source);
        ++(DestinationElementType*&)Dest;
        ++Source;
        --Count;
    }
}


template <typename DestinationElementType, typename SourceElementType>
inline typename TEnableIf<TIsBitwiseConstructible<DestinationElementType, SourceElementType>::Value>::Type ConstructItems(void* Dest, const SourceElementType* Source, int32_t Count)
{
    FMemory::Memcpy(Dest, Source, sizeof(SourceElementType) * Count);
}

/**
 * Copy assigns a range of items.
 *
 * @param   Dest        The memory location to start assigning to.
 * @param   Source      A pointer to the first item to assign.
 * @param   Count       The number of elements to assign.
 */
template <typename ElementType>
inline typename TEnableIf<!TIsTriviallyCopyAssignable<ElementType>::Value>::Type CopyAssignItems(ElementType* Dest, const ElementType* Source, int32_t Count)
{
    while (Count) {
        *Dest = *Source;
        ++Dest;
        ++Source;
        --Count;
    }
}


template <typename ElementType>
inline typename TEnableIf<TIsTriviallyCopyAssignable<ElementType>::Value>::Type CopyAssignItems(ElementType* Dest, const ElementType* Source, int32_t Count)
{
    FMemory::Memcpy(Dest, Source, sizeof(ElementType) * Count);
}


/**
 * Relocates a range of items to a new memory location as a new type. This is a so-called 'destructive move' for which
 * there is no single operation in C++ but which can be implemented very efficiently in general.
 *
 * @param   Dest        The memory location to relocate to.
 * @param   Source      A pointer to the first item to relocate.
 * @param   Count       The number of elements to relocate.
 */
template <typename DestinationElementType, typename SourceElementType>
inline typename TEnableIf<!UE4MemoryOps_Private::TCanBitwiseRelocate<DestinationElementType, SourceElementType>::Value>::Type RelocateConstructItems(void* Dest, const SourceElementType* Source, int32_t Count)
{
    while (Count) {
        // We need a typedef here because VC won't compile the destructor call below if SourceElementType itself has a member called SourceElementType
        typedef SourceElementType RelocateConstructItemsElementTypeTypedef;

        new (Dest) DestinationElementType(*Source);
        ++(DestinationElementType*&)Dest;
        (Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
        --Count;
    }
}

template <typename DestinationElementType, typename SourceElementType>
inline typename TEnableIf<UE4MemoryOps_Private::TCanBitwiseRelocate<DestinationElementType, SourceElementType>::Value>::Type RelocateConstructItems(void* Dest, const SourceElementType* Source, int32_t Count)
{
    /* All existing UE containers seem to assume trivial relocatability (i.e. memcpy'able) of their members,
     * so we're going to assume that this is safe here.  However, it's not generally possible to assume this
     * in general as objects which contain pointers/references to themselves are not safe to be trivially
     * relocated.
     *
     * However, it is not yet possible to automatically infer this at compile time, so we can't enable
     * different (i.e. safer) implementations anyway.
     */

    FMemory::Memmove(Dest, Source, sizeof(SourceElementType) * Count);
}

/**
 * Move constructs a range of items into memory.
 *
 * @param   Dest        The memory location to start moving into.
 * @param   Source      A pointer to the first item to move from.
 * @param   Count       The number of elements to move.
 */
template <typename ElementType>
inline typename TEnableIf<!TIsTriviallyCopyConstructible<ElementType>::Value>::Type MoveConstructItems(void* Dest, const ElementType* Source, int32_t Count)
{
    while (Count) {
        new (Dest) ElementType((ElementType&&)*Source);
        ++(ElementType*&)Dest;
        ++Source;
        --Count;
    }
}

template <typename ElementType>
inline typename TEnableIf<TIsTriviallyCopyConstructible<ElementType>::Value>::Type MoveConstructItems(void* Dest, const ElementType* Source, int32_t Count)
{
    FMemory::Memmove(Dest, Source, sizeof(ElementType) * Count);
}

/**
 * Move assigns a range of items.
 *
 * @param	Dest		The memory location to start move assigning to.
 * @param	Source		A pointer to the first item to move assign.
 * @param	Count		The number of elements to move assign.
 */
template <typename ElementType>
inline typename TEnableIf<!TIsTriviallyCopyAssignable<ElementType>::Value>::Type MoveAssignItems(ElementType* Dest, const ElementType* Source, int32_t Count)
{
    while (Count) {
        *Dest = (ElementType&&)*Source;
        ++Dest;
        ++Source;
        --Count;
    }
}

template <typename ElementType>
inline typename TEnableIf<TIsTriviallyCopyAssignable<ElementType>::Value>::Type MoveAssignItems(ElementType* Dest, const ElementType* Source, int32_t Count)
{
    FMemory::Memmove(Dest, Source, sizeof(ElementType) * Count);
}

template <typename ElementType>
inline typename TEnableIf<TTypeTraits<ElementType>::IsBytewiseComparable, bool>::Type CompareItems(const ElementType* A, const ElementType* B, int32_t Count)
{
    return !FMemory::Memcmp(A, B, sizeof(ElementType) * Count);
}


template <typename ElementType>
inline typename TEnableIf<!TTypeTraits<ElementType>::IsBytewiseComparable, bool>::Type CompareItems(const ElementType* A, const ElementType* B, int32_t Count)
{
    while (Count) {
        if (!(*A == *B)) {
            return false;
        }

        ++A;
        ++B;
        --Count;
    }

    return true;
}

inline int32_t DefaultCalculateSlackShrink(int32_t NumElements, int32_t NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, uint32_t Alignment = DEFAULT_ALIGNMENT)
{
    int32_t Retval;

    // If the container has too much slack, shrink it to exactly fit the number of elements.
    const uint32_t CurrentSlackElements = NumAllocatedElements - NumElements;
    const size_t CurrentSlackBytes = (NumAllocatedElements - NumElements)*BytesPerElement;
    const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
    const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;
    if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements)) //  hard coded 64 :-(
    {
        Retval = NumElements;
        if (Retval > 0) {
            if (bAllowQuantize) {
                Retval = static_cast<int32_t>(FMemory::QuantizeSize(Retval * BytesPerElement, Alignment) / BytesPerElement);
            }
        }
    } else {
        Retval = NumAllocatedElements;
    }

    return Retval;
}

inline int32_t DefaultCalculateSlackGrow(int32_t NumElements, int32_t NumAllocatedElements, size_t BytesPerElement, bool bAllowQuantize, uint32_t Alignment = DEFAULT_ALIGNMENT)
{
    int32_t Retval;

    size_t Grow = 4; // this is the amount for the first alloc
    if (NumAllocatedElements || size_t(NumElements) > Grow) {
        // Allocate slack for the array proportional to its size.
        Grow = size_t(NumElements) + 3 * size_t(NumElements) / 8 + 16;
    }
    if (bAllowQuantize) {
        Retval = static_cast<int32_t>(FMemory::QuantizeSize(Grow * BytesPerElement, Alignment) / BytesPerElement);
    } else {
        Retval = static_cast<int32_t>(Grow);
    }
    // NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
    if (NumElements > Retval) {
        Retval = INT32_MAX;
    }

    return Retval;
}

inline int32_t DefaultCalculateSlackReserve(int32_t NumElements, size_t BytesPerElement, bool bAllowQuantize, uint32_t Alignment = DEFAULT_ALIGNMENT)
{
    int32_t Retval = NumElements;

    if (bAllowQuantize) {
        Retval = static_cast<int32_t>(FMemory::QuantizeSize(size_t(Retval) * size_t(BytesPerElement), Alignment) / BytesPerElement);
        // NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
        if (NumElements > Retval) {
            Retval = INT32_MAX;
        }
    }

    return Retval;
}

/** A type which is used to represent a script type that is unknown at compile time. */
struct FScriptContainerElement {};

template <typename AllocatorType>
struct TAllocatorTraitsBase {
    enum { SupportsMove = false };
    enum { IsZeroConstruct = false };
};

template <typename AllocatorType>
struct TAllocatorTraits : TAllocatorTraitsBase<AllocatorType> {};


/** The indirect allocation policy always allocates the elements indirectly. */
class FHeapAllocator {
public:

    enum { NeedsElementType = false };
    enum { RequireRangeCheck = true };

    class ForAnyElementType {
    public:
        /** Default constructor. */
        ForAnyElementType()
            : Data(nullptr)
        {}

        /**
         * Moves the state of another allocator into this one.
         * Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements have already been destructed (if necessary).
         * @param Other - The allocator to move the state from.  This allocator should be left in a valid empty state.
         */
        inline void MoveToEmpty(ForAnyElementType& Other)
        {
            if (Data) {
                FMemory::Free(Data);
            }

            Data = Other.Data;
            Other.Data = nullptr;
        }

        /** Destructor. */
        inline ~ForAnyElementType()
        {
            if (Data) {
                FMemory::Free(Data);
            }
        }

        // FContainerAllocatorInterface
        inline FScriptContainerElement* GetAllocation() const
        {
            return Data;
        }
        inline void ResizeAllocation(int32_t PreviousNumElements, int32_t NumElements, size_t NumBytesPerElement)
        {
            // Avoid calling FMemory::Realloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
            if (Data || NumElements) {
                //checkSlow(((uint64)NumElements*(uint64)ElementTypeInfo.GetSize() < (uint64)INT_MAX));
                Data = (FScriptContainerElement*)FMemory::Realloc(Data, NumElements*NumBytesPerElement);
            }
        }
        inline int32_t CalculateSlackReserve(int32_t NumElements, int32_t NumBytesPerElement) const
        {
            return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true);
        }
        inline int32_t CalculateSlackShrink(int32_t NumElements, int32_t NumAllocatedElements, int32_t NumBytesPerElement) const
        {
            return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true);
        }
        inline int32_t CalculateSlackGrow(int32_t NumElements, int32_t NumAllocatedElements, int32_t NumBytesPerElement) const
        {
            return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true);
        }

        size_t GetAllocatedSize(int32_t NumAllocatedElements, size_t NumBytesPerElement) const
        {
            return NumAllocatedElements * NumBytesPerElement;
        }

        bool HasAllocation()
        {
            return !!Data;
        }

    private:
        ForAnyElementType(const ForAnyElementType&);
        ForAnyElementType& operator=(const ForAnyElementType&);

        /** A pointer to the container's elements. */
        FScriptContainerElement* Data;
    };

    template<typename ElementType>
    class ForElementType : public ForAnyElementType {
    public:

        /** Default constructor. */
        ForElementType()
        {}

        inline ElementType* GetAllocation() const
        {
            return (ElementType*)ForAnyElementType::GetAllocation();
        }
    };
};

template <>
struct TAllocatorTraits<FHeapAllocator> : TAllocatorTraitsBase<FHeapAllocator> {
    enum { SupportsMove = true };
    enum { IsZeroConstruct = true };
};

class FDefaultAllocator;

/**
 * 'typedefs' for various allocator defaults.
 *
 * These should be replaced with actual typedefs when Core.h include order is sorted out, as then we won't need to
 * 'forward' these TAllocatorTraits specializations below.
 */

class FDefaultAllocator : public FHeapAllocator { public: typedef FHeapAllocator          Typedef; };