/**
 * Blackout Driver
 * Copyright (c) 2018 Archetype Entertainment Private Limited. All rights reserved.
 *
 * @file crypto.h
 * @author Aidan Khoury (dude719)
 * @date 12/12/2018
 */

#ifndef _BLACKOUT_DRIVER_CRYPTO_H_
#define _BLACKOUT_DRIVER_CRYPTO_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

// Crypto API calling convention
#define CRYPTAPI  NTAPI

// Magical XOR value
#define CRYPT_MAGICAL_MAGIC 0xA55A5AA5


SIZE_T
CRYPTAPI
CryptGetEncryptedXTEABufferSize(
    IN SIZE_T UnencryptedSize,
    IN BOOLEAN WithXorSeed
    );

VOID
CRYPTAPI
CryptEncryptXTEABuffer(
    IN PUCHAR UnencryptedBuffer,
    IN SIZE_T UnencryptedSize,
    IN CONST PULONG Key,
    IN ULONG XorSeed OPTIONAL,
    OUT PUCHAR EncryptedBuffer,
    OUT PSIZE_T EncryptedSize
    );

VOID
CRYPTAPI
CryptDecryptXTEABuffer(
    IN PUCHAR EncryptedBuffer,
    IN SIZE_T EncryptedSize,
    IN CONST PULONG Key,
    IN ULONG XorSeed OPTIONAL,
    OUT PUCHAR UnencryptedBuffer,
    OUT PSIZE_T UnencryptedSize
    );

#endif // _BLACKOUT_DRIVER_CRYPTO_H_