/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file crypto.c
 * @author Aidan Khoury (ajkhoury)
 * @date 12/12/2018
 */

#include "crypto.h"
#include "driver.h"

#define XTEA_NUM_ROUNDS 32
#define XTEA_DELTA      0x9E3779B9
#define XTEA_INIT_SUM   0xC6EF3720 // (ULONG)(XTEA_NUM_ROUNDS * XTEA_DELTA);

#define ENDIAN_LITTLE   1
#define ENDIAN_BIG      0

//
// Read 32bit ULONG value from buffer.
//
FORCEINLINE
ULONG
CRYPT_READ_ULONG(
    IN PUCHAR Data,
    IN BOOLEAN LittleEndian
)
{
    if (LittleEndian == ENDIAN_BIG) {
        return _byteswap_ulong(*(ULONG*)Data);
    } else {
        return *(ULONG*)Data;
    }
}

//
// Write 32bit ULONG value to buffer.
//
FORCEINLINE
PUCHAR
CRYPT_WRITE_ULONG(
    IN ULONG Value,
    OUT PUCHAR Data,
    IN BOOLEAN LittleEndian
)
{
    if (LittleEndian == ENDIAN_BIG) {
        *(ULONG*)Data = _byteswap_ulong(Value);
    } else {
        *(ULONG*)Data = Value;
    }

    return Data;
}

static
VOID
XTEA_Encrypt(
    IN OUT PULONG V,
    IN PULONG XorSeed,
    IN CONST PULONG Key
)
{
    ULONG Sum = 0;
    ULONG V0 = V[0] ^ XorSeed[0];
    ULONG V1 = V[1] ^ XorSeed[1];
    ULONG Round;

    for (Round = 0; Round < XTEA_NUM_ROUNDS; Round++) {
        V0 += (Key[(Sum >> 0) & 3] + Sum) ^ (V1 + ((V1 >> 5) ^ (V1 << 4)));
        Sum += XTEA_DELTA;
        V1 += (Key[(Sum >> 11) & 3] + Sum) ^ (V0 + ((V0 >> 5) ^ (V0 << 4)));
    }

    V[0] = V0;
    V[1] = V1;
}

static
VOID
XTEA_Decrypt(
    IN OUT PULONG V,
    IN PULONG XorSeed,
    IN CONST PULONG Key
)
{
    ULONG Sum = XTEA_INIT_SUM;
    ULONG V0 = V[0];
    ULONG V1 = V[1];
    ULONG Round;

    for (Round = 0; Round < XTEA_NUM_ROUNDS; Round++) {
        V1 -= (Key[(Sum >> 11) & 3] + Sum) ^ (V0 + ((V0 >> 5) ^ (V0 << 4)));
        Sum -= XTEA_DELTA;
        V0 -= (Key[(Sum >> 0) & 3] + Sum) ^ (V1 + ((V1 >> 5) ^ (V1 << 4)));
    }

    V[0] = XorSeed[0] ^ V0;
    V[1] = XorSeed[1] ^ V1;
}

SIZE_T
CRYPTAPI
CryptGetEncryptedXTEABufferSize(
    IN SIZE_T UnencryptedSize,
    IN BOOLEAN WithXorSeed
)
{
    SIZE_T EncryptedBufferSize;
    UCHAR Count;

    if (!UnencryptedSize) {
        return 0;
    }

    EncryptedBufferSize = UnencryptedSize;

    Count = 8 - UnencryptedSize % 8;            // align to an 8-byte (64-bit) boundary
    if (Count == 8)
        Count = 0;
    EncryptedBufferSize += Count;

    EncryptedBufferSize += sizeof(UCHAR);       // for the appended alignment Count
    if (WithXorSeed)
        EncryptedBufferSize += sizeof(ULONG);   // for the appended XorSeed

    return EncryptedBufferSize;
}

VOID
CRYPTAPI
CryptEncryptXTEABuffer(
    IN PUCHAR UnencryptedBuffer,
    IN SIZE_T UnencryptedSize,
    IN CONST PULONG Key,
    IN ULONG XorSeed OPTIONAL,
    OUT PUCHAR EncryptedBuffer,
    OUT PSIZE_T EncryptedSize
)
{
    SIZE_T BufferSize;
    UCHAR Count;
    SIZE_T Offset;
    PUCHAR Current;
    ULONG V[2];
    LARGE_INTEGER TickCount;
    ULONG XorSeeds[2];

    BO_ASSERT(UnencryptedBuffer);
    BO_ASSERT(UnencryptedSize);
    BO_ASSERT(Key);
    BO_ASSERT(EncryptedBuffer);
    BO_ASSERT(EncryptedSize);

    BufferSize = UnencryptedSize;
    RtlCopyMemory(EncryptedBuffer, UnencryptedBuffer, BufferSize);

    Count = 8 - BufferSize % 8;
    if (Count == 8) {
        Count = 0;
    } else {
        RtlZeroMemory(EncryptedBuffer + BufferSize, Count);
        BufferSize += Count;
    }

    Current = EncryptedBuffer;

    if (!XorSeed) {
        KeQueryTickCount(&TickCount);
        XorSeed = (ULONG)TickCount.LowPart;
    }

    XorSeeds[0] = XorSeed;
    XorSeeds[1] = XorSeed ^ (~CRYPT_MAGICAL_MAGIC);

    Offset = BufferSize;
    while (Offset > (sizeof(ULONG) * 2) - 1) {

        V[0] = CRYPT_READ_ULONG(Current, ENDIAN_LITTLE);
        V[1] = CRYPT_READ_ULONG(Current + sizeof(ULONG), ENDIAN_LITTLE);

        XTEA_Encrypt(V, XorSeeds, Key);

        CRYPT_WRITE_ULONG(V[0], Current, ENDIAN_LITTLE);
        Current += sizeof(ULONG);
        CRYPT_WRITE_ULONG(V[1], Current, ENDIAN_LITTLE);
        Current += sizeof(ULONG);

        XorSeeds[0] = V[0];
        XorSeeds[1] = V[1];

        Offset -= sizeof(ULONG) * 2;
    }

    *(EncryptedBuffer + BufferSize) = Count;
    BufferSize += sizeof(UCHAR);

    CRYPT_WRITE_ULONG(XorSeed, EncryptedBuffer + BufferSize, ENDIAN_BIG);
    BufferSize += sizeof(ULONG);

    *EncryptedSize = BufferSize;
}

VOID
CRYPTAPI
CryptDecryptXTEABuffer(
    IN PUCHAR EncryptedBuffer,
    IN SIZE_T EncryptedSize,
    IN CONST PULONG Key,
    IN ULONG XorSeed OPTIONAL,
    OUT PUCHAR UnencryptedBuffer,
    OUT PSIZE_T UnencryptedSize
)
{
    SIZE_T BufferSize;
    UCHAR Count;
    SIZE_T Offset;
    PUCHAR Current;
    ULONG V[2];
    ULONG OriginalV[2];
    ULONG XorSeeds[2];

    BO_ASSERT(EncryptedBuffer);
    BO_ASSERT(EncryptedSize);
    BO_ASSERT(Key);
    BO_ASSERT(UnencryptedBuffer);
    BO_ASSERT(UnencryptedSize);

    BufferSize = EncryptedSize;
    RtlCopyMemory(UnencryptedBuffer, EncryptedBuffer, BufferSize);

    if (!XorSeed) {
        XorSeed = CRYPT_READ_ULONG(EncryptedBuffer + BufferSize - sizeof(ULONG), ENDIAN_BIG);
        BufferSize -= sizeof(ULONG);
    }

    Count = *(EncryptedBuffer + BufferSize - sizeof(UCHAR));
    BufferSize -= sizeof(UCHAR);

    Current = EncryptedBuffer;

    XorSeeds[0] = XorSeed;
    XorSeeds[1] = XorSeed ^ (~CRYPT_MAGICAL_MAGIC);

    Offset = BufferSize;
    while (Offset > (sizeof(ULONG) * 2) - 1) {

        V[0] = CRYPT_READ_ULONG(Current, ENDIAN_LITTLE);
        V[1] = CRYPT_READ_ULONG(Current + sizeof(ULONG), ENDIAN_LITTLE);
        OriginalV[0] = V[0];
        OriginalV[1] = V[1];

        XTEA_Decrypt(V, XorSeeds, Key);

        CRYPT_WRITE_ULONG(V[0], Current, ENDIAN_LITTLE);
        Current += sizeof(ULONG);
        CRYPT_WRITE_ULONG(V[1], Current, ENDIAN_LITTLE);
        Current += sizeof(ULONG);

        XorSeeds[0] = OriginalV[0];
        XorSeeds[1] = OriginalV[1];

        Offset -= sizeof(ULONG) * 2;
    }

    BufferSize -= Count;
    *UnencryptedSize = BufferSize;
}


