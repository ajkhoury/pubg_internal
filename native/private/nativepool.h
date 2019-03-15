#pragma once

#include "nativecommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < Pool Definitions >
///

#define POOL_COLD_ALLOCATION 256     // Note this cannot encode into the header.
#define POOL_NX_ALLOCATION   512     // Note this cannot encode into the header.

#define __IS_ASCII(c) \
    (((c) >= 'A' && (c) <= 'z') ||                          \
    ((c) >= '0' && (c) <= '9') ||                           \
    (c) == ' ' || (c) == '@' || (c) == '_' || (c) == '?')

#define IsPoolTagValid(PoolTag) \
    (__IS_ASCII( ((PoolTag) >> 24) & 0xFF ) && __IS_ASCII( ((PoolTag) >> 16) & 0xFF ) &&  \
     __IS_ASCII( ((PoolTag) >> 8) & 0xFF ) && __IS_ASCII( ((PoolTag) >> 0) & 0xFF ))


///
/// < Pool Enums >
///

//
// Pool Allocation routines (in pool.c)
//
typedef enum _POOL_TYPE {
    NonPagedPool,
    NonPagedPoolExecute = NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed = NonPagedPool + 2,
    DontUseThisType,
    NonPagedPoolCacheAligned = NonPagedPool + 4,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS = NonPagedPool + 6,
    MaxPoolType,

    //
    // Define base types for NonPaged (versus Paged) pool, for use in cracking
    // the underlying pool type.
    //
    NonPagedPoolBase = 0,
    NonPagedPoolBaseMustSucceed = NonPagedPoolBase + 2,
    NonPagedPoolBaseCacheAligned = NonPagedPoolBase + 4,
    NonPagedPoolBaseCacheAlignedMustS = NonPagedPoolBase + 6,

    //
    // Note these per session types are carefully chosen so that the appropriate
    // masking still applies as well as MaxPoolType above.
    //
    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

    NonPagedPoolNx = 512,
    NonPagedPoolNxCacheAligned = NonPagedPoolNx + 4,
    NonPagedPoolSessionNx = NonPagedPoolNx + 32,
} POOL_TYPE;



///
/// < Pool Structures & Types >
///

typedef struct _POOL_HEADER {
    union {
        struct {
#if defined(_WIN64)
            ULONG PreviousSize : 8;
            ULONG PoolIndex : 8;
            ULONG BlockSize : 8;
            ULONG PoolType : 8;
#else
            USHORT PreviousSize : 9;
            USHORT PoolIndex : 7;
            USHORT BlockSize : 9;
            USHORT PoolType : 7;
#endif
        };
        ULONG Ulong1;
    };
#if defined (_WIN64)
    ULONG PoolTag;
#endif
    union {
#if defined (_WIN64)
        struct _EPROCESS *ProcessBilled;
#else
        ULONG PoolTag;
#endif
        struct {
            USHORT AllocatorBackTraceIndex;
            USHORT PoolTagHash;
        };
    };
} POOL_HEADER, *PPOOL_HEADER;

typedef struct _POOL_HEADER64 {
    union {
        struct {
            ULONG PreviousSize : 8;
            ULONG PoolIndex : 8;
            ULONG BlockSize : 8;
            ULONG PoolType : 8;
        };
        ULONG Ulong1;
    };
    union {
        ULONG PoolTag;
        CHAR PoolTagChars[4];
    };
    union {
        ULONGLONG ProcessBilled; // PEPROCESS
        struct {
            USHORT AllocatorBackTraceIndex;
            USHORT PoolTagHash;
        };
    };
} POOL_HEADER64, *PPOOL_HEADER64;


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus