#pragma once

#include <stdint.h>
#include <memory>

class SafeVTableHook {
private:
    uintptr_t **Instance = nullptr;
    uintptr_t *OriginalVTable = nullptr;
    std::unique_ptr<uintptr_t[]> CurrentVTable = nullptr;

    size_t VTableSize = 0;

public:
    SafeVTableHook() = delete;
    explicit SafeVTableHook(void *InObject);
    ~SafeVTableHook() { *Instance = OriginalVTable; }

    template<typename Fn = void *>
    inline Fn Install(const size_t FunctionIdx, void *NewFn)
    {
        if (FunctionIdx > VTableSize)
            return nullptr;

        CurrentVTable[FunctionIdx] = reinterpret_cast<uintptr_t>(NewFn);
        return GetOriginalFunction<Fn>(FunctionIdx);
    }

    inline bool Uninstall(const size_t FunctionIdx)
    {
        if (FunctionIdx > VTableSize)
            return false;

        CurrentVTable[FunctionIdx] = OriginalVTable[FunctionIdx];
        return true;
    }

    template<typename Fn = void *>
    inline const Fn GetOriginalFunction(std::size_t FunctionIdx) const
    {
        return reinterpret_cast<Fn>(OriginalVTable[FunctionIdx]);
    }

    inline size_t GetSize() const { return VTableSize; }
};