#pragma once

#include "Types.h"

class WorldProxy {
public:
    WorldProxy();

    inline uint64_t *GetEncryptedPointerAddress() const { return WorldEncryptedPtr; }
    void *GetAddress() const;

private:
    uint64_t *WorldEncryptedPtr;
};
