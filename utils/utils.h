#pragma once

#include "native/ldr.h"
#include "native/rtl.h"

#include <assert.h>
#include <vector>
#include <string>

#ifndef __MAX
#define __MAX(a,b)              ((a) > (b) ? (a) : (b))
#endif
#ifndef __MIN
#define __MIN(a,b)              ((a) < (b) ? (a) : (b))
#endif

#ifndef LOWORD
#define LOWORD(l)               ((unsigned short)(((uintptr_t)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l)               ((unsigned short)((((uintptr_t)(l)) >> 16) & 0xffff))
#endif
#ifndef LODWORD
#define LODWORD(l)              ((unsigned long)(((uintptr_t)(l)) & 0xffffffff))
#endif
#ifndef HIDWORD
#define HIDWORD(l)              ((unsigned long)((((uintptr_t)(l)) >> 32) & 0xffffffff))
#endif
#ifndef LOBYTE
#define LOBYTE(w)               ((unsigned char)(((uintptr_t)(w)) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w)               ((unsigned char)((((uintptr_t)(w)) >> 8) & 0xff))
#endif

#ifndef BYTE1
#define BYTE1(a)                ((unsigned char)(((a)      ) & 0xFF))
#define BYTE2(a)                ((unsigned char)(((a) >>  8) & 0xFF))
#define BYTE3(a)                ((unsigned char)(((a) >> 16) & 0xFF))
#define BYTE4(a)                ((unsigned char)(((a) >> 24) & 0xFF))
#define BYTE5(a)                ((unsigned char)(((a) >> 32) & 0xFF))
#define BYTE6(a)                ((unsigned char)(((a) >> 40) & 0xFF))
#define BYTE7(a)                ((unsigned char)(((a) >> 48) & 0xFF))
#define BYTE8(a)                ((unsigned char)(((a) >> 56) & 0xFF))
#endif

#ifndef WORD1
#define WORD1(a)                ((unsigned short)(((unsigned int)(a)      ) & 0xFFFF))
#define WORD2(a)                ((unsigned short)(((unsigned int)(a) >> 16) & 0xFFFF))
#define WORD3(a)                ((unsigned short)(((unsigned __int64)(a) >> 32) & 0xFFFF))
#define WORD4(a)                ((unsigned short)(((unsigned __int64)(a) >> 48) & 0xFFFF))
#endif

#ifndef DWORD1
#define DWORD1(a)               ((unsigned int)((((unsigned __int64)(a))      ) & 0xFFFFFFFF))
#define DWORD2(a)               ((unsigned int)((((unsigned __int64)(a)) >> 32) & 0xFFFFFFFF))
#endif

#ifndef LONG1
#define LONG1(a)                ((unsigned int)((((unsigned __int64)(a))      ) & 0xFFFFFFFF))
#define LONG2(a)                ((unsigned int)((((unsigned __int64)(a)) >> 32) & 0xFFFFFFFF))
#endif


// Handy time macros. Thx OSR - http://www.osronline.com/article.cfm?article=261
#define INTERVAL_ABSOLUTE(wait) (wait)
#define INTERVAL_RELATIVE(wait) (-(wait))
#define NANOSECONDS(nanos)      (((LONGLONG)(nanos)) / (LONGLONG)100)
#define MICROSECONDS(micros)    (((LONGLONG)(micros)) * NANOSECONDS(1000))
#define MILLISECONDS(millis)    (((LONGLONG)(millis)) * MICROSECONDS(1000))
#define SECONDS(seconds)        (((LONGLONG)(seconds)) * MILLISECONDS(1000))


// Utils API calling convention
#define UTLAPI WINAPI

namespace utils {

/**
 * Calculates a relative offset to the target from the supplied instruction.
 *
 * @param[in] Instruction  The instruction address.
 * @param[in] Target       The target address.
 * @param[in] Offset       The offset to the instruction relative offset.
 * @return  The relative offset (MUST BE SIGNED!) between the instruction address
 *          and the target address.
 */
FORCEINLINE
const int
CalculateRelativeOffset(
    IN const void *Instruction,
    IN const void *Target,
    IN const unsigned char Offset
)
{
    return (const int)((const unsigned char *)Target -
                        ((const unsigned char *)Instruction + Offset + sizeof(int)));
}

/**
 * Gets a relative offset of an instruction at the supplied offset from the
 * instruction.
 *
 * @param[in] Instruction  The instruction address.
 * @param[in] Offset       The offset in the instruction to the relative offset.
 * @return  The relative offset (WILL BE SIGNED!) between the instruction
 *          address and the target address.
 */
FORCEINLINE
const int
GetRelativeOffset(
    IN const void *Instruction,
    IN const unsigned char Offset
)
{
    return *((const int *)((const unsigned char *)Instruction + Offset));
}

FORCEINLINE
const char
GetInstructionImm8(
    IN const void *Instruction,
    IN const unsigned char Offset
)
{
    return *((const char *)((const unsigned char *)Instruction + Offset));
}

FORCEINLINE
const short
GetInstructionImm16(
    IN const void *Instruction,
    IN const unsigned char Offset
)
{
    return *((const short *)((const unsigned char *)Instruction + Offset));
}

FORCEINLINE
const int
GetInstructionImm32(
    IN const void *Instruction,
    IN const unsigned char Offset
)
{
    return *((const int *)((const unsigned char *)Instruction + Offset));
}

FORCEINLINE
const __int64
GetInstructionImm64(
    IN const void *Instruction,
    IN const unsigned char Offset
)
{
    return *((const __int64 *)((const unsigned char *)Instruction + Offset));
}

/**
 * Sets a RelativeOffset in the supplied Instruction at the given Offset.
 *
 * @param[in] Instruction  The instruction address.
 * @param[in] Target       The target address.
 * @param[in] Offset       The offset to the instruction relative offset.
 */
FORCEINLINE
void
SetRelativeOffset(
    IN OUT void *Instruction,
    IN const int RelativeOffset,
    IN const unsigned char Offset
)
{
    *((int *)((unsigned char *)Instruction + Offset)) = RelativeOffset;
}

/**
 * Calculates the instruction target using the relative offset at the supplied
 * offset into the instruction.
 *
 * @param[in] Instruction  The instruction to get the target for.
 * @return  The address of the target.
 */
FORCEINLINE
void*
GetInstructionTarget(
    IN const void *Instruction,
    IN const unsigned char Offset
)
{
    return (void *)((const unsigned char *)Instruction +
                        (Offset + GetRelativeOffset(Instruction, Offset) + sizeof(int)));
}

/**
 * Calculates a relative offset to the target from the supplied jump
 * instruction address.
 *
 * @param[in] JmpAddress    The jmp address.
 * @param[in] TargetAddress The target address.
 * @return  The relative offset (MUST BE SIGNED!) between the jump
 *          instruction address and the target address.
 */
FORCEINLINE
const int
CalculateJmpRelativeOffset(
    IN const void *JmpInstruction,
    IN const void *Target
)
{
    int RelativeOffset;

    if (*(unsigned char *)JmpInstruction == 0xE9) {

        RelativeOffset = CalculateRelativeOffset(JmpInstruction, Target, 1);

    } else if (*(unsigned char *)JmpInstruction == 0xEB) {

        RelativeOffset = (const int)((const unsigned char *)Target -
                                    ((const unsigned char *)JmpInstruction + 1 + sizeof(char)));
        assert(abs(RelativeOffset) <= 127);

    } else if (*(unsigned char *)JmpInstruction == 0x0F &&
                (*((unsigned char *)JmpInstruction + 1) & 0x80) == 0x80) {

        RelativeOffset = CalculateRelativeOffset(JmpInstruction, Target, 2);

    } else {

        RelativeOffset = 0;
    }

    return RelativeOffset;
}

/**
 * Gets a relative offset of a jump instruction.
 *
 * @param[in] Instruction  The jump instruction address.
 * @return  The relative offset (MUST BE SIGNED!) between the jump instruction
 *          address and the target address.
 */
FORCEINLINE
const int
GetJmpRelativeOffset(
    IN const void *JmpInstruction
)
{
    if (*(unsigned char *)JmpInstruction == 0xE9) {

        return GetRelativeOffset(JmpInstruction, 1);

    } else if (*(unsigned char *)JmpInstruction == 0xEB) {

        return (const int)*((const char *)JmpInstruction + 1);

    } else if (*(unsigned char *)JmpInstruction == 0x0F &&
                (*((unsigned char *)JmpInstruction + 1) & 0x80) == 0x80) {

        return GetRelativeOffset(JmpInstruction, 2);
    }

    return 0;
}

/**
 * Sets a RelativeOffset in the supplied JmpInstruction at the given Offset.
 *
 * @param[in] JmpInstruction  The jump instruction address.
 * @param[in] RelativeOffset  The relative offset to the target.
 */
FORCEINLINE
void
SetJmpRelativeOffset(
    IN void *JmpInstruction,
    IN const int RelativeOffset
)
{
    if (*(unsigned char *)JmpInstruction == 0xE9) {

        SetRelativeOffset(JmpInstruction, RelativeOffset, 1);

    } else if (*(unsigned char *)JmpInstruction == 0xEB) {

        assert(abs(RelativeOffset) <= 127);
        *((char *)JmpInstruction + 1) = (char)RelativeOffset;

    } else if (*(unsigned char *)JmpInstruction == 0x0F &&
                (*((unsigned char *)JmpInstruction + 1) & 0x80) == 0x80) {

        SetRelativeOffset(JmpInstruction, RelativeOffset, 2);
    }
}

/**
 * Calculates the jump target address from a jump instruction (E9 ? ? ? ?)
 *
 * @param[in] JmpInstruction  The jump instruction to get the address of the
 *                            target for.
 * @return  The address of the jump instruction target.
 */
FORCEINLINE
void*
GetJmpTargetAddress(
    IN const void *JmpInstruction
)
{
    if (*(unsigned char *)JmpInstruction == 0xE9) {

        return GetInstructionTarget(JmpInstruction, 1);

    } else if (*(unsigned char *)JmpInstruction == 0xEB) {

        return (void *)((const unsigned char *)JmpInstruction + 
                                (1 + *((const char *)JmpInstruction + 1) + sizeof(char)));

    } else if (*(unsigned char *)JmpInstruction == 0x0F &&
                (*((unsigned char *)JmpInstruction + 1) & 0x80) == 0x80) {

        return GetInstructionTarget(JmpInstruction, 2);
    }

    return NULL;
}

/**
 * Calculates a relative offset to the target from the call address.
 *
 * @param[in] CallAddress   The call address.
 * @param[in] TargetAddress The target address.
 * @return  The relative offset (MUST BE SIGNED!) between the call address
 *          and the target address.
 */
FORCEINLINE
const int
CalculateCallRelativeOffset(
    IN const void *CallInstruction,
    IN const void *Target
)
{
    if (*(unsigned char *)CallInstruction == 0xE8) {

        return CalculateRelativeOffset(CallInstruction, Target, 1);

    } else if (*(unsigned char *)CallInstruction == 0xFF &&
                *((unsigned char *)CallInstruction + 1) == 0x15) {

        return CalculateRelativeOffset(CallInstruction, Target, 2);
    }

    return 0;
}

/**
 * Gets a relative offset of a call instruction.
 *
 * @param[in] Instruction  The call instruction address.
 * @return  The relative offset (MUST BE SIGNED!) between the call instruction
 *          address and the target address.
 */
FORCEINLINE
const int
GetCallRelativeOffset(
    IN const void *CallInstruction
)
{
    if (*(unsigned char *)CallInstruction == 0xE8) {

        return GetRelativeOffset(CallInstruction, 1);

    } else if (*(unsigned char *)CallInstruction == 0xFF &&
               *((unsigned char *)CallInstruction + 1) == 0x15) {

        return GetRelativeOffset(CallInstruction, 2);
    }

    return 0;
}

/**
 * Sets a RelativeOffset in the supplied CallInstruction at the given Offset.
 *
 * @param[in] CallInstruction The call instruction address.
 * @param[in] RelativeOffset  The relative offset to the target.
 */
FORCEINLINE
void
SetCallRelativeOffset(
    IN void *CallInstruction,
    IN const int RelativeOffset
)
{
    if (*(unsigned char *)CallInstruction == 0xE8) {

        SetRelativeOffset(CallInstruction, RelativeOffset, 1);

    } else if (*(unsigned char *)CallInstruction == 0xFF &&
                *((unsigned char *)CallInstruction + 1) == 0x15) {

        SetRelativeOffset(CallInstruction, RelativeOffset, 2);
    }
}

/**
 * Calculates the call target address from a call instruction (E8 ? ? ? ?).
 *
 * @param[in] CallInstruction   The call instruction to get the address of the
 *                              called function from.
 * @return  The address of the called function.
 *          NULL if invalid.
 */
FORCEINLINE
void*
GetCallTargetAddress(
    IN const void *CallInstruction
)
{
    if (*(unsigned char *)CallInstruction == 0xE8) {

        return GetInstructionTarget(CallInstruction, 1);

    } else if (*(unsigned char *)CallInstruction == 0xFF &&
               *((unsigned char *)CallInstruction + 1) == 0x15) {

        return GetInstructionTarget(CallInstruction, 2);
    }

    return NULL;
}

/**
 * Execute a memory comparison operation with an additional wildcard byte check.
 *
 * @param[in] Buf1      The first buffer to compare against the buffer in Buf2.
 * @param[in] Wildcard  The wildcard byte.
 * @param[in] Buf2      The second comparison buffer.
 * @param[in] Size      The size to compare (should be the size of Buf2).
 *
 * @return  Returns an integral value indicating the relationship between the
 *          content of the memory blocks:
 *
 *          <0 The first byte that does not match in both memory blocks has a
 *             lower value in Buf1 than in Buf2.
 *          0  The contents of both memory blocks are equal.
 *          >0 The first byte that does not match in both memory blocks has a
 *             greater value in Buf1 than in Buf2.
 */
FORCEINLINE
int
MemCmp(
    IN const void *Buf1,
    IN const UINT8 Wildcard,
    IN const void *Buf2,
    IN SIZE_T Size
)
{
    if (!Size) {
        return 0;
    }

    //
    // This is intentionally written like this for optimizations. DO NOT TOUCH!
    //
    while (--Size &&
            ((*(char *)Buf1 == *(char *)Buf2) || (*(unsigned char *)Buf2 == Wildcard))) {
        Buf1 = (char *)Buf1 + 1;
        Buf2 = (char *)Buf2 + 1;
    }

    return (*(unsigned char *)Buf1 - *(unsigned char *)Buf2);
}

/**
 * Search for substring String2 in String1.
 *
 * @param[in] String1          The string to search for a substring in.
 * @param[in] String2          The substring to search for.
 * @param[in] CaseInsensitive  Whether the search should be case insensitive
 *                             or not.
 * @return  A pointer to the found substring String2 inside String1.
 *          NULL if not found.
 */
WCHAR*
UTLAPI
WcsStr(
    IN const WCHAR *String1,
    IN const WCHAR *String2,
    IN BOOLEAN CaseInsensitive
    );

std::string
UTLAPI
wstring_to_string(
    const std::wstring& wstr
    );

std::vector<std::string>
UTLAPI
SplitString(
    const std::string& Str,
    char Delimiter
    );

std::string
UTLAPI
FormatBuffer(
    const uint8_t *Buffer,
    size_t Length
    );

/**
 * Find a byte pattern in a specified range.
 *
 * @param[in] Pattern     The signature pattern to search for.
 * @param[in] PatternSize The byte signature pattern length.
 * @param[in] Wildcard    The wildcard byte to check for.
 * @param[in] SearchBase  The start address to search for the signature pattern.
 * @param[in] SearchSize  The size of the range to search for the signature
 *                        pattern.
 * @return  A pointer to the found byte signature in the serach range.
 *          NULL if not found.
 */
const UINT8*
UTLAPI
FindPattern(
    IN const void *SearchBase,
    IN SIZE_T SearchSize,
    IN const UINT8 Wildcard,
    IN const UINT8 *Pattern,
    IN SIZE_T PatternSize
    );

/**
 * Find an IDA-style byte pattern with nibble wildcards in a specified range.
 *
 * @param[in] SearchBase  The start address to search for the signature pattern.
 * @param[in] SearchSize  The size of the range to search for the signature pattern.
 * @param[in] Pattern     The signature pattern to search for.
 *
 * @return  A pointer to the found byte signature in the serach range.
 *          NULL if not found.
 */
const UINT8*
UTLAPI
FindPatternIDA(
    IN const void *SearchBase,
    IN SIZE_T SearchSize,
    IN const char *Pattern
    );

const UINT8*
UTLAPI
FindPatternIDA(
    IN const void *SearchBase,
    IN SIZE_T SearchSize,
    IN const std::string& Pattern
    );

/**
 * Find a the start or base of a function by a given pointer inside the function.
 *
 * @param[in] Ptr           The pointer inside of the function to find the start of.
 * @param[in] MaxSearchSize The maximum size to traverse backwards until the start.
 * @param[out] FoundAddress The returned found address of the start of the function.
 *                          NULL if not found.
 *
 * @return  NOERROR if the function start was found.
 *          E_NOT_SET if the function start was not found.
 *          Or the appropriate error status code if a failure occured.
 */
int
UTLAPI
FindFunctionStartFromPtr(
    IN const void *Ptr,
    IN SIZE_T MaxSearchSize OPTIONAL,
    OUT const UINT8 **FoundAddress
    );

/**
 * Computes a CRC32 checksum for the supplied data.
 *
 * @param[in] Data  The data to cacluate the checksum for.
 * @param[in] Size  The size of the data.
 * @return  The calculated CRC32 checksum value.
 */
UINT32
UTLAPI
CRC32Checksum(
    IN const void *Data,
    IN SIZE_T Size
    );

HMODULE
UTLAPI
GetModuleHandleWIDE(
    IN LPCWSTR lpModuleName
    );

HMODULE
UTLAPI
GetModuleHandleCRC32(
    IN UINT32 CRC32ModuleName
    );

PVOID
UTLAPI
GetProcAddressANSI(
    IN HMODULE hModule,
    IN LPCSTR lpProcName
    );

PVOID
UTLAPI
GetProcAddressWIDE(
    IN HMODULE hModule,
    IN LPCWSTR lpProcName
    );

PVOID
UTLAPI
GetProcAddressCRC32(
    IN HMODULE hModule,
    IN UINT32 ProcNameCRC32
    );

ULONG
UTLAPI
GetModuleFileNameWIDE(
    IN HMODULE hModule,
    OUT LPWSTR lpFileName,
    IN ULONG FileNameSize
    );

ULONG
UTLAPI
GetModuleFileNameANSI(
    IN HMODULE hModule,
    OUT LPSTR lpFileName,
    IN ULONG FileNameSize
    );

ULONG
UTLAPI
GetModuleFileNameCRC32(
    IN HMODULE hModule,
    OUT PULONG lpNameCRC32,
    IN ULONG FileNameSize
    );

ULONG
UTLAPI
GetModuleSize(
    IN HMODULE hModule
    );




// Read bits.
template <typename T> inline T read_bits(const void *address);

template<> inline unsigned __int64 read_bits(const void *address) {
    return *static_cast<const unsigned __int64 *>(address);
}

template<> inline unsigned long read_bits(const void *address) {
    return *static_cast<const unsigned long *>(address);
}

template<> inline unsigned int read_bits(const void *address) {
    return *static_cast<const unsigned int *>(address);
}

template<> inline unsigned short read_bits(const void *address) {
    return *static_cast<const unsigned short *>(address);
}

template<> inline unsigned char read_bits(const void *address) {
    return *static_cast<const unsigned char *>(address);
}

template<> inline __int64 read_bits(const void *address) {
    return *static_cast<const __int64 *>(address);
}

template<> inline long read_bits(const void *address) {
    return *static_cast<const long *>(address);
}

template<> inline int read_bits(const void *address) {
    return *static_cast<const int *>(address);
}

template<> inline short read_bits(const void *address) {
    return *static_cast<const short *>(address);
}

template<> inline char read_bits(const void *address) {
    return *static_cast<const char *>(address);
}

constexpr unsigned __int64 read_bits(const void *address, int width) {
    switch(width) {
    case 8:     return read_bits<unsigned char>(address);
    case 16:    return read_bits<unsigned short>(address);
    case 32:    return read_bits<unsigned int>(address);
    case 64:    return read_bits<unsigned __int64>(address);
    default:    return 0;
    }
}

constexpr unsigned __int64 read_bytes(const void *address, int width) {
    switch(width) {
    case sizeof(char):      return read_bits<unsigned char>(address);
    case sizeof(short):     return read_bits<unsigned short>(address);
    case sizeof(int):       return read_bits<unsigned int>(address);
    case sizeof(__int64):   return read_bits<unsigned __int64>(address);
    default:                return 0;
    }
}

// Write bits.
template<typename T> inline void write_bits(void *address, T value);

template<> inline void write_bits(void *address, unsigned __int64 value) {
    *static_cast<unsigned __int64 *>(address) = value;
}

template<> inline void write_bits(void *address, unsigned long value) {
    *static_cast<unsigned long *>(address) = value;
}

template<> inline void write_bits(void *address, unsigned int value) {
    *static_cast<unsigned int *>(address) = value;
}

template<> inline void write_bits(void *address, unsigned short value) {
    *static_cast<unsigned short *>(address) = value;
}

template<> inline void write_bits(void *address, unsigned char value) {
    *static_cast<unsigned char *>(address) = value;
}

template<> inline void write_bits(void *address, __int64 value) {
    *static_cast<__int64 *>(address) = value;
}

template<> inline void write_bits(void *address, long value) {
    *static_cast<long *>(address) = value;
}

template<> inline void write_bits(void *address, int value) {
    *reinterpret_cast<int *>(address) = value;
}

template<> inline void write_bits(void *address, short value) {
    *static_cast<short *>(address) = value;
}

template<> inline void write_bits(void *address, char value) {
    *static_cast<char *>(address) = value;
}

constexpr void write_bits(void *address, int width, unsigned __int64 value) {
    switch(width) {
    case 8:     write_bits<unsigned char>(address, (unsigned char)value); break;
    case 16:    write_bits<unsigned short>(address, (unsigned short)value); break;
    case 32:    write_bits<unsigned int>(address, (unsigned int)value); break;
    case 64:    write_bits<unsigned __int64>(address, (unsigned __int64)value); break;
    default:    break;
    }
}

constexpr void write_bytes(void *address, int width, unsigned __int64 value) {
    switch(width) {
    case sizeof(char):      write_bits<unsigned char>(address, (unsigned char)value); break;
    case sizeof(short):     write_bits<unsigned short>(address, (unsigned short)value); break;
    case sizeof(int):       write_bits<unsigned int>(address, (unsigned int)value); break;
    case sizeof(__int64):   write_bits<unsigned __int64>(address, (unsigned __int64)value); break;
    default:                break;
    }
}

template<typename T> inline void write_value(void *buffer, T value) {
    *reinterpret_cast<T *>(buffer) = value;
}

template<typename TFn> inline TFn GetVFunction(const void *instance, std::size_t index)
{
    const void** VTable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
    return reinterpret_cast<TFn>(VTable[index]);
}

} // utils