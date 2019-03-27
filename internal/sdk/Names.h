#pragma once

#include "Types.h"

class NamesProxy {
public:
    NamesProxy();

    inline void *GetAddress() const { return Names; }

    int32_t GetNum() const;
    bool IsValid(int32_t id) const;
    std::string GetById(int32_t id)  const;

private:
    void *Names;
};

