#pragma once

#include "rtl.h"

#if defined(__cplusplus)
extern "C" {
#endif

//
// The Executive Resource Struct
//

typedef struct _EXECUTIVE_RESOURCE {
    RTL_CRITICAL_SECTION Lock;

    BOOLEAN Initialized;
    HANDLE WaitEventHandle;

    volatile ULONG SharedCount;
    volatile ULONG StarveExclusiveCount;
    volatile ULONG ExclusiveCount;
    volatile ULONG SharedWaitersCount;
    volatile ULONG ExclusiveWaitersCount;

    RTL_AVL_TABLE LockStateMap;
} EXECUTIVE_RESOURCE, *PEXECUTIVE_RESOURCE;

//
// Executive Resource routines. 
// These routines are intended to behave exactly like the kernel mode routines behave,
// except that they operate at IRQL PASSIVE LEVEL.
//
// So, recursive acquisition is allowed, as long as the acquisition rules are
// followed, i.e. if you have it shared, you can't acquire it exclusive, but if you 
// have it exclusive and try to get it shared, your shared acquisition becomes a 
// exclusive acquisition.
//
//
// Note:  If you do something illegal with the locks, they will throw an exception.
// An exception indicates an illegal lock operation has taken place and cannot
// be recovered from.   The exception code 0xC000000X indicates where the exception
// took place.
//
// Exception codes:
//
//  0xE0000001 - IsResourceAcquiredShared - invalid lock state, corruption?
//  0xE0000002 - ReleaseResource - resource not owned by caller
//  0xE0000003 - ConvertExclusiveToShared - resource not owned exclusive
//  0xE0000004 - [NOT USED] ConvertExclusiveToShared - resource owned more than once
//  0xE0000005 - ConvertExclusiveToShared - resource not owned by caller.
//  0xE0000006 - ReleaseResourceForThread - resource not owned by input thread
//  0xE0000007 - DeleteResource - resource still in use.
//  0xE0000008 - AcquireResourceExclusive - already have the lock shared
//  0xE0000009 - Generic Error - Resource not initialized.
//  0xE000000A - OSRInitializeResourceLite - Resource already initialized.
//

BOOLEAN
WINAPI
AcquireResourceExclusiveLite (
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN BOOLEAN Wait
    );

BOOLEAN
WINAPI
AcquireResourceSharedLite (
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN BOOLEAN Wait
    );

BOOLEAN
WINAPI
AcquireSharedStarveExclusive(
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN BOOLEAN Wait
    );

BOOLEAN
WINAPI
IsResourceAcquiredExclusiveLite (
    IN PEXECUTIVE_RESOURCE Resource
    );

BOOLEAN
WINAPI
IsResourceAcquiredSharedLite (
    IN PEXECUTIVE_RESOURCE Resource
    );

VOID
WINAPI
ReleaseResourceLite(
    IN OUT PEXECUTIVE_RESOURCE Resource
    );

VOID
WINAPI
ConvertExclusiveToSharedLite(
    IN OUT PEXECUTIVE_RESOURCE Resource
    );

VOID
WINAPI
ReleaseResourceForThreadLite(
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN ULONG ResourceThreadId
    );

ULONG
WINAPI
InitializeResourceLite (
    OUT PEXECUTIVE_RESOURCE Resource
    );

ULONG
WINAPI
DeleteResourceLite (
    IN OUT PEXECUTIVE_RESOURCE Resource
    );

ULONG
WINAPI
GetCurrentResourceThreadId(
    VOID
    );

ULONG
WINAPI
GetSharedWaiterCount(
    IN OUT PEXECUTIVE_RESOURCE Resource
    );

ULONG
WINAPI
GetExclusiveWaiterCount(
    IN OUT PEXECUTIVE_RESOURCE Resource
    );

#if defined(__cplusplus)
} // extern "C"
#endif

