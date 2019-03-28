#pragma once

#include <native/log.h>

#include "Names.h"

#include <unordered_map>
#include <type_traits>

/** Objects flags for internal use (GC, low level UObject code) */
enum class EInternalObjectFlags : int32_t {
    None = 0,
    //~ All the other bits are reserved, DO NOT ADD NEW FLAGS HERE!
    ReachableInCluster = 1 << 23, ///< External reference to object in cluster exists
    ClusterRoot = 1 << 24, ///< Root of a cluster
    Native = 1 << 25, ///< Native (UClass only).
    Async = 1 << 26, ///< Object exists only on a different thread than the game thread.
    AsyncLoading = 1 << 27, ///< Object is being asynchronously loaded.
    Unreachable = 1 << 28, ///< Object is not reachable on the object graph.
    PendingKill = 1 << 29, ///< Objects that are pending destruction (invalid for gameplay but valid objects)
    RootSet = 1 << 30, ///< Object will not be garbage collected, even if unreferenced.

    GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
    //~ Make sure this is up to date!
    AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet
};
ENUM_CLASS_FLAGS(EInternalObjectFlags);

struct FUObjectItem {
    // Pointer to the allocated object
    class UObject *Object;      // 0x00
    // Internal flags
    int32_t Flags;              // 0x08
    // UObject Owner Cluster Index
    int32_t ClusterRootIndex;   // 0x0C
    // Weak Object Pointer Serial number associated with the object
    int32_t SerialNumber;       // 0x10

    inline int32_t GetClusterIndex() const { return -ClusterRootIndex - 1; }
    inline int32_t GetSerialNumber() const { return SerialNumber; }
    inline int32_t GetFlags() const { return Flags; }
    inline bool HasAnyFlags(EInternalObjectFlags InFlags) const { return !!(Flags & int32_t(InFlags)); }
    inline bool IsUnreachable() const { return !!(Flags & int32_t(EInternalObjectFlags::Unreachable)); }
    inline bool IsPendingKill() const { return !!(Flags & int32_t(EInternalObjectFlags::PendingKill)); }
    inline bool IsRootSet() const { return !!(Flags & int32_t(EInternalObjectFlags::RootSet)); }
}; // size=0x18
C_ASSERT(sizeof(FUObjectItem) == 0x18);


class ObjectsProxy {
    friend class ObjectIterator;
public:
    ObjectsProxy();

    inline void *GetAddress() const { return ObjectArray; }

    int32_t GetNum() const;
    int64_t GetMax() const;

    class UObject* GetById(int32_t Index);
    class UObject const* GetById(int32_t Index) const;

    class UObject* FindObject(const std::string& name);
    class UObject const* FindObject(const std::string& name) const;

    class UClass* FindClass(const std::string& Name);
    class UClass const* FindClass(const std::string& Name) const;

    int32_t CountObjects(const class UClass* CmpClass, const std::string& Name) const;
    template<class T>
    inline int32_t CountObjects(const std::string& Name) const { return CountObjects(T::StaticClass(), Name); }

    class ObjectIterator {
    public:
        ObjectIterator(const class ObjectsProxy* InObjects, const int32_t InObjectIndex) : Index(InObjectIndex), Objects(InObjects) {}
        ObjectIterator(const ObjectIterator& Other) : Index(Other.Index), Objects(Other.Objects) {}
        ObjectIterator(ObjectIterator&& Other) noexcept : Index(Other.Index), Objects(Other.Objects) {}

        inline ObjectIterator& operator=(const ObjectIterator& rhs) { Index = rhs.Index; return *this; }
        inline void swap(ObjectIterator& Other) noexcept { std::swap(Index, Other.Index); }

        inline bool operator==(const ObjectIterator& rhs) const { return Index == rhs.Index; }
        inline bool operator!=(const ObjectIterator& rhs) const { return Index != rhs.Index; }

        inline ObjectIterator& operator++() { ++Index; return *this; }
        inline ObjectIterator operator++(int) { auto tmp(*this); ++(*this); return tmp; }

        inline UObject const* operator*() const { return Objects->GetById(Index); }
        inline UObject const* operator->() const { return Objects->GetById(Index); }

    private:
        int32_t Index;
        const class ObjectsProxy* Objects;
    };

    inline ObjectIterator begin() { return ObjectIterator(this, 0); }
    inline const ObjectIterator begin() const { return ObjectIterator(this, 0); }
    inline ObjectIterator end() { return ObjectIterator(this, GetNum()); }
    inline const ObjectIterator end() const { return ObjectIterator(this, GetNum()); }

private:
    FUObjectItem *GetObjectsPrivate() const;

    void *ObjectArray;

    static std::unordered_map<std::string, int32_t> ObjectsCacheMap;
};
