#pragma once

#include <stdint.h>
#include <memory.h>

/** Tests whether two typenames refer to the same type. */
template<typename A, typename B>
struct TAreTypesEqual;

template<typename, typename>
struct TAreTypesEqual {
    enum { Value = false };
};

template<typename A>
struct TAreTypesEqual<A, A> {
    enum { Value = true };
};

#define ARE_TYPES_EQUAL(A,B) TAreTypesEqual<A,B>::Value

/**
 * Does a boolean AND of the ::Value static members of each type, but short-circuits if any Type::Value == false.
 */
template <typename... Types>
struct TAnd;

template <bool LHSValue, typename... RHS>
struct TAndValue {
    enum { Value = TAnd<RHS...>::Value };
};

template <typename... RHS>
struct TAndValue<false, RHS...> {
    enum { Value = false };
};

template <typename LHS, typename... RHS>
struct TAnd<LHS, RHS...> : TAndValue<LHS::Value, RHS...> {};

template <>
struct TAnd<> {
    enum { Value = true };
};

/**
 * Does a boolean OR of the ::Value static members of each type, but short-circuits if any Type::Value == true.
 */
template <typename... Types>
struct TOr;

template <bool LHSValue, typename... RHS>
struct TOrValue {
    enum { Value = TOr<RHS...>::Value };
};

template <typename... RHS>
struct TOrValue<true, RHS...> {
    enum { Value = true };
};

template <typename LHS, typename... RHS>
struct TOr<LHS, RHS...> : TOrValue<LHS::Value, RHS...> {};

template <>
struct TOr<> {
    enum { Value = false };
};

/**
 * Does a boolean NOT of the ::Value static members of the type.
 */
template <typename Type>
struct TNot {
    enum { Value = !Type::Value };
};

/**
 * Includes a function in an overload set if the predicate is true.  It should be used similarly to this:
 *
 * // This function will only be instantiated if SomeTrait<T>::Value is true for a particular T
 * template <typename T>
 * typename TEnableIf<SomeTrait<T>::Value, ReturnType>::Type Function(const T& Obj)
 * {
 *     ...
 * }
 *
 * ReturnType is the real return type of the function.
 */
template <bool Predicate, typename Result = void>
class TEnableIf;

template <typename Result>
class TEnableIf<true, Result> {
public:
    typedef Result Type;
};

template <typename Result>
class TEnableIf<false, Result> {};

/**
 * This is a variant of the above that will determine the return type 'lazily', i.e. only if the function is enabled.
 * This is useful when the return type isn't necessarily legal code unless the enabling condition is true.
 *
 * // This function will only be instantiated if SomeTrait<T>::Value is true for a particular T.
 * // The function's return type is typename Transform<T>::Type.
 * template <typename T>
 * typename TLazyEnableIf<SomeTrait<T>::Value, Transform<T>>::Type Function(const T& Obj)
 * {
 *     ...
 * }
 *
 * See boost::lazy_enable_if for more details.
 */
template <bool Predicate, typename Func>
class TLazyEnableIf;

template <typename Func>
class TLazyEnableIf<true, Func> {
public:
    typedef typename Func::Type Type;
};

template <typename Func>
class TLazyEnableIf<false, Func> {};


/**
 * Traits class which tests if a type is arithmetic.
 */
template <typename T>
struct TIsArithmetic {
    enum { Value = false };
};

template <> struct TIsArithmetic<float> { enum { Value = true }; };
template <> struct TIsArithmetic<double> { enum { Value = true }; };
template <> struct TIsArithmetic<long double> { enum { Value = true }; };
template <> struct TIsArithmetic<uint8_t> { enum { Value = true }; };
template <> struct TIsArithmetic<uint16_t> { enum { Value = true }; };
template <> struct TIsArithmetic<uint32_t> { enum { Value = true }; };
template <> struct TIsArithmetic<uint64_t> { enum { Value = true }; };
template <> struct TIsArithmetic<int8_t> { enum { Value = true }; };
template <> struct TIsArithmetic<int16_t> { enum { Value = true }; };
template <> struct TIsArithmetic<int32_t> { enum { Value = true }; };
template <> struct TIsArithmetic<int64_t> { enum { Value = true }; };
template <> struct TIsArithmetic<bool> { enum { Value = true }; };
template <> struct TIsArithmetic<wchar_t> { enum { Value = true }; };
template <> struct TIsArithmetic<char> { enum { Value = true }; };

template <typename T> struct TIsArithmetic<const          T> { enum { Value = TIsArithmetic<T>::Value }; };
template <typename T> struct TIsArithmetic<      volatile T> { enum { Value = TIsArithmetic<T>::Value }; };
template <typename T> struct TIsArithmetic<const volatile T> { enum { Value = TIsArithmetic<T>::Value }; };

/**
 * Traits class which tests if a type is a pointer.
 */
template <typename T>
struct TIsPointer {
    enum { Value = false };
};

template <typename T> struct TIsPointer<               T*> { enum { Value = true }; };
template <typename T> struct TIsPointer<const          T*> { enum { Value = true }; };
template <typename T> struct TIsPointer<      volatile T*> { enum { Value = true }; };
template <typename T> struct TIsPointer<const volatile T*> { enum { Value = true }; };

template <typename T> struct TIsPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
template <typename T> struct TIsPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };


/**
 * TIsReferenceType
 */
template<typename T> struct TIsReferenceType { enum { Value = false }; };
template<typename T> struct TIsReferenceType<T&> { enum { Value = true }; };
template<typename T> struct TIsReferenceType<T&&> { enum { Value = true }; };

/**
 * TIsLValueReferenceType
 */
template<typename T> struct TIsLValueReferenceType { enum { Value = false }; };
template<typename T> struct TIsLValueReferenceType<T&> { enum { Value = true }; };

/**
 * TIsRValueReferenceType
 */
template<typename T> struct TIsRValueReferenceType { enum { Value = false }; };
template<typename T> struct TIsRValueReferenceType<T&&> { enum { Value = true }; };

/**
 * TIsVoidType
 */
template<typename T> struct TIsVoidType { enum { Value = false }; };
template<> struct TIsVoidType<void> { enum { Value = true }; };
template<> struct TIsVoidType<void const> { enum { Value = true }; };
template<> struct TIsVoidType<void volatile> { enum { Value = true }; };
template<> struct TIsVoidType<void const volatile> { enum { Value = true }; };

/**
 * TIsFundamentalType
 */
template<typename T>
struct TIsFundamentalType {
    enum { Value = TOr<TIsArithmetic<T>, TIsVoidType<T>>::Value };
};

/**
 * TIsFunction
 *
 * Tests is a type is a function.
 */
template <typename T>
struct TIsFunction {
    enum { Value = false };
};

template <typename RetType, typename... Params>
struct TIsFunction<RetType(Params...)> {
    enum { Value = true };
};

/**
 * TIsZeroConstructType
 */
template<typename T>
struct TIsZeroConstructType {
    enum { Value = TOr<TIsEnum<T>, TIsArithmetic<T>, TIsPointer<T>>::Value };
};

/**
 * TIsWeakPointerType
 */
template<typename T>
struct TIsWeakPointerType {
    enum { Value = false };
};

/**
 * Traits class which tests if a type is POD.
 */

#if defined(_MSC_VER) && _MSC_VER >= 1900
// __is_pod changed in VS2015, however the results are still correct for all usages I've been able to locate.
#pragma warning(push)
#pragma warning(disable:4647)
#endif // _MSC_VER == 1900
template <typename T>
struct TIsPODType {
    enum { Value = TOrValue<__is_pod(T) || __is_enum(T), TIsArithmetic<T>, TIsPointer<T>>::Value };
};
#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning(pop)
#endif // _MSC_VER >= 1900


/** Chooses between two different classes based on a boolean. */
template<bool Predicate, typename TrueClass, typename FalseClass>
class TChooseClass;

template<typename TrueClass, typename FalseClass>
class TChooseClass<true, TrueClass, FalseClass> {
public:
    typedef TrueClass Result;
};

template<typename TrueClass, typename FalseClass>
class TChooseClass<false, TrueClass, FalseClass> {
public:
    typedef FalseClass Result;
};

/**
 * TRemoveCV<type> will remove any const/volatile qualifiers from a type.
 * (based on std::remove_cv<>
 * note: won't remove the const from "const int*", as the pointer is not const
 */
template <typename T> struct TRemoveCV { typedef T Type; };
template <typename T> struct TRemoveCV<const T> { typedef T Type; };
template <typename T> struct TRemoveCV<volatile T> { typedef T Type; };
template <typename T> struct TRemoveCV<const volatile T> { typedef T Type; };


/**
 * Traits class which tests if a type has a trivial copy constructor.
 */
template <typename T>
struct TIsTriviallyCopyConstructible {
    enum { Value = TOrValue<__has_trivial_copy(T), TIsPODType<T>>::Value };
};

/**
 * Traits class which tests if a type has a trivial copy assignment operator.
 */
template <typename T>
struct TIsTriviallyCopyAssignable {
    enum { Value = TOrValue<__has_trivial_assign(T), TIsPODType<T>>::Value };
};

/**
 * Call traits helpers
 */
template <typename T, bool TypeIsSmall>
struct TCallTraitsParamTypeHelper {
    typedef const T& ParamType;
    typedef const T& ConstParamType;
};
template <typename T>
struct TCallTraitsParamTypeHelper<T, true> {
    typedef const T ParamType;
    typedef const T ConstParamType;
};
template <typename T>
struct TCallTraitsParamTypeHelper<T*, true> {
    typedef T* ParamType;
    typedef const T* ConstParamType;
};


/*-----------------------------------------------------------------------------
    Helper templates for dealing with 'const' in template code
-----------------------------------------------------------------------------*/

/**
 * TRemoveConst<> is modeled after boost's implementation.  It allows you to take a templatized type
 * such as 'const Foo*' and use const_cast to convert that type to 'Foo*' without knowing about Foo.
 *
 *		MutablePtr = const_cast< RemoveConst< ConstPtrType >::Type >( ConstPtr );
 */
template< class T >
struct TRemoveConst {
    typedef T Type;
};
template< class T >
struct TRemoveConst<const T> {
    typedef T Type;
};


/*-----------------------------------------------------------------------------
 * TCallTraits
 *
 * Same call traits as boost, though not with as complete a solution.
 *
 * The main member to note is ParamType, which specifies the optimal
 * form to pass the type as a parameter to a function.
 *
 * Has a small-value optimization when a type is a POD type and as small as a pointer.
-----------------------------------------------------------------------------*/

/**
 * base class for call traits. Used to more easily refine portions when specializing
 */
template <typename T>
struct TCallTraitsBase {
private:
    enum { PassByValue = TOr<TAndValue<(sizeof(T) <= sizeof(void*)), TIsPODType<T>>, TIsArithmetic<T>, TIsPointer<T>>::Value };
public:
    typedef T ValueType;
    typedef T& Reference;
    typedef const T& ConstReference;
    typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::ParamType ParamType;
    typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::ConstParamType ConstPointerType;
};

/**
 * TCallTraits
 */
template <typename T>
struct TCallTraits : public TCallTraitsBase<T> {};

// Fix reference-to-reference problems.
template <typename T>
struct TCallTraits<T&> {
    typedef T& ValueType;
    typedef T& Reference;
    typedef const T& ConstReference;
    typedef T& ParamType;
    typedef T& ConstPointerType;
};

// Array types
template <typename T, size_t N>
struct TCallTraits<T[N]> {
private:
    typedef T ArrayType[N];
public:
    typedef const T* ValueType;
    typedef ArrayType& Reference;
    typedef const ArrayType& ConstReference;
    typedef const T* const ParamType;
    typedef const T* const ConstPointerType;
};

// const array types
template <typename T, size_t N>
struct TCallTraits<const T[N]> {
private:
    typedef const T ArrayType[N];
public:
    typedef const T* ValueType;
    typedef ArrayType& Reference;
    typedef const ArrayType& ConstReference;
    typedef const T* const ParamType;
    typedef const T* const ConstPointerType;
};


/*-----------------------------------------------------------------------------
    Traits for our particular container classes
-----------------------------------------------------------------------------*/

/**
 * Helper for array traits. Provides a common base to more easily refine a portion of the traits
 * when specializing. Mainly used by MemoryOps.h which is used by the contiguous storage containers like TArray.
 */
template<typename T>
struct TTypeTraitsBase {
    typedef typename TCallTraits<T>::ParamType ConstInitType;
    typedef typename TCallTraits<T>::ConstPointerType ConstPointerType;

    // There's no good way of detecting this so we'll just assume it to be true for certain known types and expect
    // users to customize it for their custom types.
    enum { IsBytewiseComparable = TOr<TIsEnum<T>, TIsArithmetic<T>, TIsPointer<T>>::Value };
};

/**
 * Traits for types.
 */
template<typename T> struct TTypeTraits : public TTypeTraitsBase<T> {};


/**
 * Traits for containers.
 */
template<typename T> struct TContainerTraitsBase {
    // This should be overridden by every container that supports emptying its contents via a move operation.
    enum { MoveWillEmptyContainer = false };
};

template<typename T> struct TContainerTraits : public TContainerTraitsBase<T> {};

struct FVirtualDestructor {
    virtual ~FVirtualDestructor() {}
};

template <typename T, typename U>
struct TMoveSupportTraitsBase {
    // Param type is not an const lvalue reference, which means it's pass-by-value, so we should just provide a single type for copying.
    // Move overloads will be ignored due to SFINAE.
    typedef U Copy;
};

template <typename T>
struct TMoveSupportTraitsBase<T, const T&> {
    // Param type is a const lvalue reference, so we can provide an overload for moving.
    typedef const T& Copy;
    typedef T&&      Move;
};

/**
 * This traits class is intended to be used in pairs to allow efficient and correct move-aware overloads for generic types.
 * For example:
 *
 * template <typename T>
 * void Func(typename TMoveSupportTraits<T>::Copy Obj)
 * {
 *     // Copy Obj here
 * }
 *
 * template <typename T>
 * void Func(typename TMoveSupportTraits<T>::Move Obj)
 * {
 *     // Move from Obj here as if it was passed as T&&
 * }
 *
 * Structuring things in this way will handle T being a pass-by-value type (e.g. ints, floats, other 'small' types) which
 * should never have a reference overload.
 */
template <typename T>
struct TMoveSupportTraits : TMoveSupportTraitsBase<T, typename TCallTraits<T>::ParamType> {};

/**
 * Tests if a type T is bitwise-constructible from a given argument type U.  That is, whether or not
 * the U can be memcpy'd in order to produce an instance of T, rather than having to go
 * via a constructor.
 *
 * Examples:
 * TIsBitwiseConstructible<PODType,    PODType   >::Value == true  // PODs can be trivially copied
 * TIsBitwiseConstructible<const int*, int*      >::Value == true  // a non-const Derived pointer is trivially copyable as a const Base pointer
 * TIsBitwiseConstructible<int*,       const int*>::Value == false // not legal the other way because it would be a const-correctness violation
 * TIsBitwiseConstructible<int32,      uint32    >::Value == true  // signed integers can be memcpy'd as unsigned integers
 * TIsBitwiseConstructible<uint32,     int32     >::Value == true  // and vice versa
 */

template <typename T, typename Arg>
struct TIsBitwiseConstructible {
    static_assert(
        !TIsReferenceType<T  >::Value &&
        !TIsReferenceType<Arg>::Value,
        "TIsBitwiseConstructible is not designed to accept reference types");

    static_assert(
        TAreTypesEqual<T, typename TRemoveCV<T  >::Type>::Value &&
        TAreTypesEqual<Arg, typename TRemoveCV<Arg>::Type>::Value,
        "TIsBitwiseConstructible is not designed to accept qualified types");

    // Assume no bitwise construction in general
    enum { Value = false };
};

template <typename T>
struct TIsBitwiseConstructible<T, T> {
    // Ts can always be bitwise constructed from itself if it is trivially copyable.
    enum { Value = TIsTriviallyCopyConstructible<T>::Value };
};

template <typename T, typename U>
struct TIsBitwiseConstructible<const T, U> : TIsBitwiseConstructible<T, U> {
    // Constructing a const T is the same as constructing a T
};

// Const pointers can be bitwise constructed from non-const pointers.
// This is not true for pointer conversions in general, e.g. where an offset may need to be applied in the case
// of multiple inheritance, but there is no way of detecting that at compile-time.
template <typename T>
struct TIsBitwiseConstructible<const T*, T*> {
    // Constructing a const T is the same as constructing a T
    enum { Value = true };
};

// Unsigned types can be bitwise converted to their signed equivalents, and vice versa.
// (assuming two's-complement, which we are)
template <> struct TIsBitwiseConstructible<uint8_t, int8_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< int8_t, uint8_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<uint16_t, int16_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< int16_t, uint16_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<uint32_t, int32_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< int32_t, uint32_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible<uint64_t, int64_t> { enum { Value = true }; };
template <> struct TIsBitwiseConstructible< int64_t, uint64_t> { enum { Value = true }; };


/**
 * Used to declare an untyped array of data with compile-time alignment.
 * It needs to use template specialization as the MS_ALIGN and GCC_ALIGN macros require literal parameters.
 */
template<int32_t Size, uint32_t Alignment>
struct TAlignedBytes; // this intentionally won't compile, we don't support the requested alignment

/** Unaligned storage. */
template<int32_t Size>
struct TAlignedBytes<Size, 1> {
    uint8_t Pad[Size];
};


// C++/CLI doesn't support alignment of native types in managed code, so we enforce that the element
// size is a multiple of the desired alignment
#ifdef __cplusplus_cli
#define IMPLEMENT_ALIGNED_STORAGE(Align) \
    template<int32_t Size>        \
    struct TAlignedBytes<Size,Align> { \
        uint8_t Pad[Size]; \
        static_assert(Size % Align == 0, "CLR interop types must not be aligned."); \
    };
#else
/** A macro that implements TAlignedBytes for a specific alignment. */
#define IMPLEMENT_ALIGNED_STORAGE(Align) \
    template<int32_t Size>        \
    struct TAlignedBytes<Size,Align> { \
        struct __declspec(align(Align)) TPadding { \
            uint8_t Pad[Size]; \
        }; \
        TPadding Padding; \
    };
#endif

// Implement TAlignedBytes for these alignments.
IMPLEMENT_ALIGNED_STORAGE(16);
IMPLEMENT_ALIGNED_STORAGE(8);
IMPLEMENT_ALIGNED_STORAGE(4);
IMPLEMENT_ALIGNED_STORAGE(2);

#undef IMPLEMENT_ALIGNED_STORAGE

/** An untyped array of data with compile-time alignment and size derived from another type. */
template<typename ElementType>
struct TTypeCompatibleBytes :
    public TAlignedBytes<
    sizeof(ElementType),
    alignof(ElementType)
    > {};

/**
 * Removes one level of pointer from a type, e.g.:
 *
 * TRemovePointer<      int32  >::Type == int32
 * TRemovePointer<      int32* >::Type == int32
 * TRemovePointer<      int32**>::Type == int32*
 * TRemovePointer<const int32* >::Type == const int32
 */
template <typename T> struct TRemovePointer { typedef T Type; };
template <typename T> struct TRemovePointer<T*> { typedef T Type; };

/**
 * TRemoveReference<type> will remove any references from a type.
 */
template <typename T> struct TRemoveReference { typedef T Type; };
template <typename T> struct TRemoveReference<T& > { typedef T Type; };
template <typename T> struct TRemoveReference<T&&> { typedef T Type; };

/**
 * MoveTemp will cast a reference to an rvalue reference.
 * This is UE's equivalent of std::move except that it will not compile when passed an rvalue or
 * const object, because we would prefer to be informed when MoveTemp will have no effect.
 */
template <typename T>
inline typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
    typedef typename TRemoveReference<T>::Type CastType;

    // Validate that we're not being passed an rvalue or a const object - the former is redundant, the latter is almost certainly a mistake
    static_assert(TIsLValueReferenceType<T>::Value, "MoveTemp called on an rvalue");
    static_assert(!TAreTypesEqual<CastType&, const CastType&>::Value, "MoveTemp called on a const object");

    return (CastType&&)Obj;
}

/**
 * MoveTemp will cast a reference to an rvalue reference.
 * This is UE's equivalent of std::move.  It doesn't static assert like MoveTemp, because it is useful in
 * templates or macros where it's not obvious what the argument is, but you want to take advantage of move semantics
 * where you can but not stop compilation.
 */
template <typename T>
inline typename TRemoveReference<T>::Type&& MoveTempIfPossible(T&& Obj)
{
    typedef typename TRemoveReference<T>::Type CastType;
    return (CastType&&)Obj;
}

/**
 * CopyTemp will enforce the creation of an rvalue which can bind to rvalue reference parameters.
 * Unlike MoveTemp, the source object will never be modifed. (i.e. a copy will be made)
 * There is no std:: equivalent.
 */
template <typename T>
inline T CopyTemp(T& Val)
{
    return const_cast<const T&>(Val);
}

template <typename T>
inline T CopyTemp(const T& Val)
{
    return Val;
}

template <typename T>
inline T&& CopyTemp(T&& Val)
{
    // If we already have an rvalue, just return it unchanged, rather than needlessly creating yet another rvalue from it.
    return MoveTemp(Val);
}

/**
 * Forward will cast a reference to an rvalue reference.
 * This is UE's equivalent of std::forward.
 */
template <typename T>
inline T&& Forward(typename TRemoveReference<T>::Type& Obj)
{
    return (T&&)Obj;
}

template <typename T>
inline T&& Forward(typename TRemoveReference<T>::Type&& Obj)
{
    return (T&&)Obj;
}

/**
 * A traits class which specifies whether a Swap of a given type should swap the bits or use a traditional value-based swap.
 */
template <typename T>
struct TUseBitwiseSwap {
    // We don't use bitwise swapping for 'register' types because this will force them into memory and be slower.
    enum { Value = !TOrValue<__is_enum(T), TIsPointer<T>, TIsArithmetic<T>>::Value };
};

/**
 * Swap two values.  Assumes the types are trivially relocatable.
 */
template <typename T>
inline typename TEnableIf<TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
{
    if ((&A != &B)) {
        TTypeCompatibleBytes<T> Temp;
        memcpy(&Temp, &A, sizeof(T));
        memcpy(&A, &B, sizeof(T));
        memcpy(&B, &Temp, sizeof(T));
    }
}

template <typename T>
inline typename TEnableIf<!TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
{
    T Temp = MoveTemp(A);
    A = MoveTemp(B);
    B = MoveTemp(Temp);
}

template <typename T>
inline void Exchange(T& A, T& B)
{
    Swap(A, B);
}

/**
 * This exists to avoid a Visual Studio bug where using a cast to forward an rvalue reference array argument
 * to a pointer parameter will cause bad code generation.  Wrapping the cast in a function causes the correct
 * code to be generated.
 */
template <typename T, typename ArgType>
inline T StaticCast(ArgType&& Arg)
{
    return static_cast<T>(Arg);
}

/**
 * TRValueToLValueReference converts any rvalue reference type into the equivalent lvalue reference, otherwise returns the same type.
 */
template <typename T> struct TRValueToLValueReference { typedef T  Type; };
template <typename T> struct TRValueToLValueReference<T&&> { typedef T& Type; };

/**
 * Reverses the order of the bits of a value.
 * This is an TEnableIf'd template to ensure that no undesirable conversions occur.  Overloads for other types can be added in the same way.
 *
 * @param Bits - The value to bit-swap.
 * @return The bit-swapped value.
 */
template <typename T>
inline typename TEnableIf<TAreTypesEqual<T, uint32_t>::Value, T>::Type ReverseBits(T Bits)
{
    Bits = (Bits << 16) | (Bits >> 16);
    Bits = ((Bits & 0x00ff00ff) << 8) | ((Bits & 0xff00ff00) >> 8);
    Bits = ((Bits & 0x0f0f0f0f) << 4) | ((Bits & 0xf0f0f0f0) >> 4);
    Bits = ((Bits & 0x33333333) << 2) | ((Bits & 0xcccccccc) >> 2);
    Bits = ((Bits & 0x55555555) << 1) | ((Bits & 0xaaaaaaaa) >> 1);
    return Bits;
}