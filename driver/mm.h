/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file mm.h
 * @author Aidan Khoury (ajkhoury)
 * @date 8/27/2018
 */

#ifndef _BLACKOUT_DRIVER_MM_H_
#define _BLACKOUT_DRIVER_MM_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

#define MMAPI       NTAPI

#define MM_POOL_TAG '  mM'


PVOID
MMAPI
MmAllocateNonPaged(
    IN SIZE_T NumberOfBytes
    );

PVOID
MMAPI
MmAllocateNonPagedNx(
    IN SIZE_T NumberOfBytes
    );

PVOID
MMAPI
MmAllocatePaged(
    IN SIZE_T NumberOfBytes
    );

VOID
MMAPI
MmFreeNonPaged(
    IN PVOID P
    );

#define MmFreeNonPagedNx MmFreeNonPaged

VOID
MMAPI
MmFreePaged(
    IN PVOID P
    );


#endif // _BLACKOUT_DRIVER_MM_H_