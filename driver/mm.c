/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file mm.c
 * @author Aidan Khoury (ajkhoury)
 * @date 8/27/2018
 */

#include "mm.h"

PVOID
MMAPI
MmAllocateNonPaged(
    IN SIZE_T NumberOfBytes
)
{
    return ExAllocatePoolWithTag(NonPagedPool, NumberOfBytes, MM_POOL_TAG);
}

PVOID
MMAPI
MmAllocateNonPagedNx(
    IN SIZE_T NumberOfBytes
)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, NumberOfBytes, MM_POOL_TAG);
}

PVOID
MMAPI
MmAllocatePaged(
    IN SIZE_T NumberOfBytes
)
{
    return ExAllocatePoolWithTag(PagedPool, NumberOfBytes, MM_POOL_TAG);
}

VOID
MMAPI
MmFreeNonPaged(
    IN PVOID P
)
{
    ExFreePoolWithTag(P, MM_POOL_TAG);
}

VOID
MMAPI
MmFreePaged(
    IN PVOID P
)
{
    ExFreePoolWithTag(P, MM_POOL_TAG);
}