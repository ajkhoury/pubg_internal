#include "eresource.h"
#include "rtl.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4005)
#endif

#include <assert.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#define LOCK_WAIT_IN_MILLISECONDS 200

typedef enum _ACQUIRED_STATE {
    ACQUIRED_NONE,
    ACQUIRED_SHARED,
    ACQUIRED_EXCLUSIVE
} ACQUIRED_STATE;

//
// This structure keeps track of the state of the resource. We need
// to know how the resource was acquired and how many times it was 
// recursively acquired.
//
typedef struct _RESOURCE_STATE {
    ULONG ThreadId;     // ID of thread that owns resource.
    ULONG State;        // Acquisition State
    ULONG AcquireCount; // Count of times acquired
} RESOURCE_STATE, *PRESOURCE_STATE;

static
RTL_GENERIC_COMPARE_RESULTS
NTAPI
LockStateMapCompare(
    IN struct _RTL_AVL_TABLE *Table,
    IN PRESOURCE_STATE FirstStruct,
    IN PRESOURCE_STATE SecondStruct
)
{
    UNREFERENCED_PARAMETER(Table);

    if (FirstStruct->ThreadId < SecondStruct->ThreadId) {
        return GenericLessThan;
    } else if (FirstStruct->ThreadId > SecondStruct->ThreadId) {
        return GenericGreaterThan;
    } else {
        return GenericEqual;
    }
}

static
PVOID
NTAPI
LockStateMapAllocate(
    IN struct _RTL_AVL_TABLE *Table,
    IN CLONG ByteSize
)
{
    UNREFERENCED_PARAMETER(Table);
    return malloc(ByteSize);
}

static
VOID
NTAPI
LockStateMapFree(
    IN struct _RTL_AVL_TABLE *Table,
    IN PVOID Buffer
)
{
    UNREFERENCED_PARAMETER(Table);
    free(Buffer);
}



//
// C Interface routines. All C++ constructs are hidden to the caller.
//

BOOLEAN
WINAPI
AcquireResourceExclusiveLite(
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN BOOLEAN Wait
)
{
    RESOURCE_STATE State;
    PRESOURCE_STATE LockState;
    LARGE_INTEGER WaitInterval;
    ULONG CurrentThreadId = GetCurrentResourceThreadId();
    ULONG loopCount = 0;
    

    RtlEnterCriticalSection(&Resource->Lock);

    Resource->ExclusiveWaitersCount++;

    while (TRUE) {

        //
        // See if we already own the resource.
        //
        State.ThreadId = CurrentThreadId;
        LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &State
                                                );
        if (LockState) {

            //
            // We are in the list.   See what type of ownership we have.
            //
            if (LockState->State & ACQUIRED_SHARED) {

                //
                // We have it shared. This is bad, we can't acquire it.
                //
                RaiseException(0xE0000008, EXCEPTION_NONCONTINUABLE, 0, NULL);

            } else if (LockState->State & ACQUIRED_EXCLUSIVE) {

                //
                // We already own it exclusively, so just increment
                // our ownership count.
                //
                LockState->AcquireCount++;
                Resource->ExclusiveCount++;
                Resource->ExclusiveWaitersCount--;

                RtlLeaveCriticalSection(&Resource->Lock);
                return TRUE;
            }

        } else if (Resource->SharedCount ||             // Ignore shared waiters
                   Resource->ExclusiveCount ||          //
                   Resource->StarveExclusiveCount) {    //
            //
            // The lock is either already owned by another thread shared, 
            // owned by another thread exclusively, or there is a sharedstarveExclusive
            // waiter.   Therefore we have to wait for the lock.   When the Event gets set
            // we will reaccess our access to the lock.
            //
            if (!Wait) {

                //
                // The caller does not want to wait, so get out of here.
                //
                Resource->ExclusiveWaitersCount--;
                RtlLeaveCriticalSection(&Resource->Lock);

                break;
            }

            RtlLeaveCriticalSection(&Resource->Lock);
            NtWaitForSingleObject(Resource->WaitEventHandle, FALSE, NULL);

            //
            // Add a wait loop to prevent this thread from hogging
            // the CPU. The event gets set when the lock holder
            // releases the lock and we have to ensure that
            // everyone gets a chance to run before continuing.
            //
            loopCount++;
            if (loopCount == 5) {
                WaitInterval.QuadPart = LOCK_WAIT_IN_MILLISECONDS * -10000LL;
                NtDelayExecution(TRUE, &WaitInterval);
                loopCount = 0;
            }

            RtlEnterCriticalSection(&Resource->Lock);
            continue;

        } else {

            //
            // We are free to acquire the lock.
            //
            State.State = ACQUIRED_EXCLUSIVE;
            State.AcquireCount = 1;
            State.ThreadId = CurrentThreadId;

            Resource->ExclusiveCount++;
            Resource->ExclusiveWaitersCount--;

            //
            // Insert new resource state element.
            //
            RtlInsertElementGenericTableAvl(&Resource->LockStateMap, &State, sizeof(State), NULL);

            NtClearEvent(Resource->WaitEventHandle);

            RtlLeaveCriticalSection(&Resource->Lock);
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN
AcquireResourceSharedLite(
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN BOOLEAN Wait
)
{
    RESOURCE_STATE State;
    PRESOURCE_STATE LockState;
    LARGE_INTEGER WaitInterval;
    ULONG CurrentThreadId = GetCurrentResourceThreadId();
    ULONG loopCount = 0;

    RtlEnterCriticalSection(&Resource->Lock);

    Resource->SharedWaitersCount++;

    while (TRUE) {

        //
        // See if we already own the lock.
        //
        State.ThreadId = CurrentThreadId;
        LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &State
                                                );
        if (LockState) {

            //
            // We already own it, let's see what access we already have.
            //
            LockState->AcquireCount++;
            if (LockState->State & ACQUIRED_SHARED) {

                //
                // We already have it shared, so just bump the count.
                //
                Resource->SharedCount++;

            } else if (LockState->State & ACQUIRED_EXCLUSIVE) {

                //
                // We have it exclusive, so just give it to the
                // caller exclusive, 
                //
                Resource->ExclusiveCount++;
            }

            Resource->SharedWaitersCount--;
            RtlLeaveCriticalSection(&Resource->Lock);

            return TRUE;

        } else if (Resource->ExclusiveCount || Resource->ExclusiveWaitersCount) {

            //
            // Someone else owns it exclusive or there are exclusive
            // waiters, so we have to wait.
            //
            if (!Wait) {

                //
                // The caller does not want to wait, so get out of here.
                //
                Resource->SharedWaitersCount--;
                RtlLeaveCriticalSection(&Resource->Lock);

                break;
            }

            RtlLeaveCriticalSection(&Resource->Lock);
            NtWaitForSingleObject(Resource->WaitEventHandle, FALSE, NULL);

            //
            // Add a wait loop to prevent this thread from hogging
            // the CPU.   The event gets set when the lock holder
            // releases the lock and we have to ensure that
            // everyone gets a chance to run before continuing.
            //
            loopCount++;
            if (loopCount == 5) {
                WaitInterval.QuadPart = LOCK_WAIT_IN_MILLISECONDS * -10000LL;
                NtDelayExecution(TRUE, &WaitInterval);
                loopCount = 0;
            }

            RtlEnterCriticalSection(&Resource->Lock);
            continue;

        } else {

            //
            // Nobody has it or wants it, so we will take it.
            //
            State.State = ACQUIRED_SHARED;
            State.AcquireCount = 1;
            State.ThreadId = CurrentThreadId;

            Resource->SharedCount++;
            Resource->SharedWaitersCount--;

            //
            // Insert new resource state element.
            //
            RtlInsertElementGenericTableAvl(&Resource->LockStateMap, &State, sizeof(State), NULL);

            NtClearEvent(Resource->WaitEventHandle);

            RtlLeaveCriticalSection(&Resource->Lock);
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN
AcquireSharedStarveExclusive(
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN BOOLEAN Wait
)
{
    RESOURCE_STATE State;
    PRESOURCE_STATE LockState;
    LARGE_INTEGER WaitInterval;
    ULONG CurrentThreadId = GetCurrentResourceThreadId();
    ULONG loopCount = 0;

    RtlEnterCriticalSection(&Resource->Lock);

    //
    // Indicate that there is a thread that wants to acquire the lock
    // shared and start any exclusive waiters.
    //
    Resource->StarveExclusiveCount++;
    Resource->SharedWaitersCount++;

    while (TRUE) {

        //
        // See if we already own the lock. We had better!
        //
        State.ThreadId = CurrentThreadId;
        LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &State
                                                );
        if (LockState) {

            //
            // We own it, let's check our access.
            //
            LockState->AcquireCount++;
            if (LockState->State & ACQUIRED_SHARED) {

                //
                // We already have it shared, so we'll just
                // bump the shared count.
                //
                Resource->SharedCount++;

            } else if (LockState->State & ACQUIRED_EXCLUSIVE) {

                //
                // We already have it exclusive, so we'll just
                // give the caller exclusive access.
                //
                Resource->ExclusiveCount++;
            }

            Resource->StarveExclusiveCount--;

            RtlLeaveCriticalSection(&Resource->Lock);
            return TRUE;

        } else if (Resource->ExclusiveCount) {

            //
            //  Somebody owns it exclusive, so we have to wait.
            //
            if (!Wait) {

                //
                // The caller does not want to wait, so get out of here.
                //
                Resource->StarveExclusiveCount--;
                Resource->SharedWaitersCount--;
                RtlLeaveCriticalSection(&Resource->Lock);

                break;
            }

            RtlLeaveCriticalSection(&Resource->Lock);
            NtWaitForSingleObject(Resource->WaitEventHandle, FALSE, NULL);

            //
            // Add a wait loop to prevent this thread from hogging
            // the CPU.   The event gets set when the lock holder
            // releases the lock and we have to ensure that
            // everyone gets a chance to run before continuing.
            //
            loopCount++;
            if (loopCount == 5) {
                WaitInterval.QuadPart = LOCK_WAIT_IN_MILLISECONDS * -10000LL;
                NtDelayExecution(TRUE, &WaitInterval);
                loopCount = 0;
            }

            RtlEnterCriticalSection(&Resource->Lock);
            continue;

        } else {

            //
            // No one owns the lock and we've ignored checking exclusive
            // waiters....
            //
            State.State = ACQUIRED_SHARED;
            State.AcquireCount = 1;
            State.ThreadId = CurrentThreadId;

            Resource->SharedCount++;
            Resource->StarveExclusiveCount--;
            Resource->SharedWaitersCount--;

            //
            // Insert new resource state element.
            //
            RtlInsertElementGenericTableAvl(&Resource->LockStateMap, &State, sizeof(State), NULL);

            NtClearEvent(Resource->WaitEventHandle);

            RtlLeaveCriticalSection(&Resource->Lock);
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN
IsResourceAcquiredExclusiveLite(
    IN PEXECUTIVE_RESOURCE Resource
)
{
    PRESOURCE_STATE LockState;

    ULONG CurrentThreadId = GetCurrentResourceThreadId();
    BOOLEAN result = FALSE;

    RtlEnterCriticalSection(&Resource->Lock);

    //
    // See if we own the lock.
    //
    LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &CurrentThreadId
                                                );
    if (LockState) {

        //
        // We own the lock, check our access.
        //
        if (LockState->State & ACQUIRED_SHARED) {

            //
            // We own it shared, not exclusive.
            //
            RtlLeaveCriticalSection(&Resource->Lock);
            return FALSE;
        }

        //
        // We own it exclusive.
        //
        result = TRUE;
    }

    //
    // If we don't own the lock, result will be false by 
    // default, otherwise the if above will have set 
    // result.
    //
    RtlLeaveCriticalSection(&Resource->Lock);
    return result;
}

BOOLEAN
IsResourceAcquiredSharedLite(
    IN PEXECUTIVE_RESOURCE Resource
)
{
    PRESOURCE_STATE LockState;

    ULONG CurrentThreadId = GetCurrentResourceThreadId();
    BOOLEAN result = FALSE;

    RtlEnterCriticalSection(&Resource->Lock);

    //
    // See if we own the lock.
    //
    LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &CurrentThreadId
                                                );
    if (LockState) {

        //
        // We own the lock, check our access.
        //
        if (LockState->State & (ACQUIRED_SHARED | ACQUIRED_EXCLUSIVE)) {

            //
            // We own the lock, so return TRUE.
            //
            RtlLeaveCriticalSection(&Resource->Lock);
            return TRUE;
        }

        //
        // Something very bad has happened...
        //
        RaiseException(0xE0000001, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

    //
    // If we get here, we don't own the lock and will return FALSE.
    //
    RtlLeaveCriticalSection(&Resource->Lock);
    return result;
}

VOID
ReleaseResourceLite(
    IN OUT PEXECUTIVE_RESOURCE Resource
)
{
    PRESOURCE_STATE LockState;
    ULONG CurrentThreadId = GetCurrentResourceThreadId();

    RtlEnterCriticalSection(&Resource->Lock);

    //
    // See if we own the lock.  We'd better.....
    //
    LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &CurrentThreadId
                                                );
    if (LockState) {

        //
        // We own it. Decrement the ownership count based upon our access.
        // 
        if (LockState->State & ACQUIRED_SHARED) {
            Resource->SharedCount--;
        } else {
            Resource->ExclusiveCount--;
        }

        //
        // Look at our ownership count in the lowpart of the
        // large integer.  If it is 1, then this is our
        // last reference to the lock, so we will delete
        // ourselves from the ownership map.
        //
        if (LockState->AcquireCount == 1) {

            //
            // erase us, we no longer own the lock.
            //
            assert(RtlDeleteElementGenericTableAvl(&Resource->LockStateMap, LockState));

            //
            // Wake up anyone waiting on access to the lock.
            //
            NtSetEvent(Resource->WaitEventHandle, NULL);

        } else {

            //
            // We still have outstanding references.
            //
            LockState->AcquireCount--;
        }

        RtlLeaveCriticalSection(&Resource->Lock);

    } else {

        RaiseException(0xE0000002, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
}

VOID
ConvertExclusiveToSharedLite(
    IN OUT PEXECUTIVE_RESOURCE Resource
)
{
    PRESOURCE_STATE LockState;
    ULONG CurrentThreadId = GetCurrentResourceThreadId();

    RtlEnterCriticalSection(&Resource->Lock);

    //
    // See if we own the lock. We had better!
    //
    LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &CurrentThreadId
                                                );
    if (LockState) {

        //
        // We own the lock, check our access.
        //
        if (LockState->State != ACQUIRED_EXCLUSIVE) {

            //
            // We don't own it exclusive, this is bad.
            //
            RaiseException(0xE0000003, EXCEPTION_NONCONTINUABLE, 0, NULL);

        } else {

            ////
            //// Make sure that we only have one reference on the lock
            //// I don't know what to do if we one it more than once...
            ////
            //if (iter->second.AcquireCount != 1)
            //{
            //    //
            //    // We have it owned more than once.... Bad.
            //    //
            //    RaiseException( 0xE0000004, EXCEPTION_NONCONTINUABLE, 0, NULL );
            //}
            //else
            //{

            //
            // We own it once exclusively, convert it to
            // shared and update the counts.
            //
            LockState->State = ACQUIRED_SHARED;
            Resource->SharedCount += LockState->AcquireCount;
            Resource->ExclusiveCount -= LockState->AcquireCount;

            //
            // Wake up anyone waiting for the lock. If there are
            // other shared waiters they will be able to get the
            // lock now.
            //
            NtSetEvent(Resource->WaitEventHandle, NULL);

            RtlLeaveCriticalSection(&Resource->Lock);
            return;

            //}
        }
    }

    RaiseException(0xE0000005, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

VOID
ReleaseResourceForThreadLite(
    IN OUT PEXECUTIVE_RESOURCE Resource,
    IN ULONG ResourceThreadId
)
{
    PRESOURCE_STATE LockState;

    RtlEnterCriticalSection(&Resource->Lock);

    //
    // See if the input thread owns the lock.
    //
    LockState = (PRESOURCE_STATE)RtlLookupElementGenericTableAvl(
                                                &Resource->LockStateMap,
                                                &ResourceThreadId
                                                );
    if (LockState) {

        //
        // We found the input thread, update the counts based upon
        // the callers access.
        //
        if (LockState->State & ACQUIRED_SHARED) {
            Resource->SharedCount--;
        } else {
            Resource->ExclusiveCount--;
        }

        //
        // Look at the reference count in the lock.  If there is only one
        // reference left, then we can delete this owner from the map.
        //
        if (LockState->AcquireCount == 1) {

            //
            // This is the callers last reference to the lock. Erase them
            // and then signal all waiters to reaccess their lock ownership.
            //
            assert(RtlDeleteElementGenericTableAvl(&Resource->LockStateMap, LockState));

            //
            // Wake up all waiters.
            //
            NtSetEvent(Resource->WaitEventHandle, NULL);

        } else {

            LockState->AcquireCount--;
        }

        RtlLeaveCriticalSection(&Resource->Lock);

    } else {

        RaiseException(0xE0000006, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
}

ULONG
InitializeResourceLite(
    OUT PEXECUTIVE_RESOURCE Resource
)
{
    NTSTATUS Status;

    if (!Resource->Initialized) {

        RtlZeroMemory(Resource, sizeof(EXECUTIVE_RESOURCE));

        //
        // Initialize critical section object.
        //
        RtlInitializeCriticalSection(&Resource->Lock);

        //
        // Create a notification event.  All threads waiting on this event will be awaken
        // when this event is set.  These threads will then try to acquire the resource.
        //
        Status = NtCreateEvent(&Resource->WaitEventHandle,
                               GENERIC_ALL,
                               NULL,
                               NotificationEvent,
                               FALSE
                               );

        if (Status != STATUS_SUCCESS) {
            RtlDeleteCriticalSection(&Resource->Lock);
            return RtlNtStatusToDosError(Status);
        }

        if (!Resource->WaitEventHandle) {
            RtlDeleteCriticalSection(&Resource->Lock);
            return RtlNtStatusToDosError(STATUS_UNSUCCESSFUL);
        }

        //
        // Initialize the lock state AVL table.
        //
        RtlInitializeGenericTableAvl(&Resource->LockStateMap,
                                     (PRTL_AVL_COMPARE_ROUTINE)LockStateMapCompare,
                                     LockStateMapAllocate,
                                     LockStateMapFree,
                                     NULL
                                     );
        //
        // Indicate as initialized.
        //
        Resource->Initialized = TRUE;
    }

    return ERROR_SUCCESS;
}

ULONG
DeleteResourceLite(
    IN OUT PEXECUTIVE_RESOURCE Resource
)
{
    PVOID P;
    ULONG CurrentThreadId = GetCurrentResourceThreadId();

    RtlEnterCriticalSection(&Resource->Lock);

    if (Resource->SharedCount |
        Resource->ExclusiveCount |
        Resource->SharedWaitersCount |
        Resource->ExclusiveWaitersCount |
        (!Resource->Initialized)) {
        RaiseException(0xE0000007, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

    RtlDeleteCriticalSection(&Resource->Lock);
    NtClose(Resource->WaitEventHandle);

    //
    // Clear our all elements in the AVL table.
    //
    for (P = RtlEnumerateGenericTableAvl(&Resource->LockStateMap, TRUE);
         P != NULL;
         P = RtlEnumerateGenericTableAvl(&Resource->LockStateMap, FALSE)) {
        RtlDeleteElementGenericTableAvl(&Resource->LockStateMap, P);
    }

    Resource->Initialized = FALSE;

    return ERROR_SUCCESS;
}

ULONG
GetCurrentResourceThreadId(
    VOID
)
{
    return (ULONG)(ULONG_PTR)NtCurrentThreadId();
}

ULONG
GetSharedWaiterCount(
    IN OUT PEXECUTIVE_RESOURCE Resource
)
{
    return Resource->SharedWaitersCount;
}

ULONG
GetExclusiveWaiterCount(
    IN OUT PEXECUTIVE_RESOURCE Resource
)
{
    return Resource->ExclusiveWaitersCount;
}