#pragma once

#include "Types.h"

//class WorldProxy {
//public:
//    WorldProxy();
//
//    inline uint64_t* GetEncryptedPtr() const { return WorldEncryptedPtr; }
//    const void* GetAddress() const;
//
//    class UWorld const* GetConstPtr() const { return (class UWorld const*)GetAddress(); }
//    class UWorld* GetPtr() const { return (class UWorld*)GetAddress(); }
//
//    class UWorld const& GetConstRef() const { return *GetConstPtr(); }
//    class UWorld& GetRef() const { return *GetPtr(); }
//
//#if defined(ENABLE_SDK)
//    TArray<class ULevel*> GetLevels() const;
//    class ULevel* GetPersistentLevel() const;
//    class ULevel* GetCurrentLevel() const;
//#endif // ENABLE_SDK
//
//private:
//    uint64_t *WorldEncryptedPtr;
//};
//
//class LevelProxy {
//public:
//    LevelProxy(class ULevel* InLevel)
//        : Level(InLevel)
//    {
//    }
//
//    inline LevelProxy& operator=(class ULevel* InLevel) { Level = InLevel; *this; }
//
//    inline const void* GetAddress() const { return static_cast<void const*>(Level); }
//
//    class ULevel const* GetConstPtr() const { return Level; }
//    class ULevel* GetPtr() const { return Level; }
//
//    class ULevel const& GetConstRef() const { return *GetConstPtr(); }
//    class ULevel& GetRef() const { return *GetPtr(); }
//
//private:
//    class ULevel* Level;
//};
