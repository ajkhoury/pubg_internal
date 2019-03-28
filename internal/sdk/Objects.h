#pragma once

#include <native/log.h>

#include "Names.h"

#include <unordered_map>
#include <type_traits>

class ObjectsProxy {
    friend class ObjectIterator;
public:
    ObjectsProxy();

    inline void *GetAddress() const { return ObjectArray; }

    int32_t GetNum() const;
    int64_t GetMax() const;
    class UObject *GetById(int32_t Index) const;

    class UObject* FindObject(const std::string& name) const;
    template<class T>
    T* FindObject(const std::string& Name) const { return static_cast<T*>(FindObject(Name)); }

    int32_t CountObjects(const class UClass* CmpClass, const std::string& Name) const;
    template<class T>
    inline int32_t CountObjects(const std::string& Name) const
    {
        return CountObjects(T::StaticClass(), Name);
    }

    inline class UClass* FindClass(const std::string& Name) const
    {
        return (class UClass*)FindObject(Name);
    }

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

        inline UObject* operator*() const { return Objects->GetById(Index); }
        inline UObject* operator->() const { return Objects->GetById(Index); }

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
