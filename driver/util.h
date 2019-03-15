/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file util.h
 * @author Aidan Khoury (ajkhoury)
 * @date 8/30/2018
 */

#ifndef _BLACKOUT_DRIVER_UTIL_H_
#define _BLACKOUT_DRIVER_UTIL_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

// Util API calling convention
#define UTLAPI  NTAPI

// Define macros to build data structure signatures from characters.
#define SIGNATURE_16(A,B)               (((B) << 8) | (A))
#define SIGNATURE_32(A,B,C,D)           ((SIGNATURE_16(C,D) << 16) | SIGNATURE_16(A,B))
#define SIGNATURE_64(A,B,C,D,E,F,G,H)   (((ULONGLONG)(SIGNATURE_32(E,F,G,H)) << 32) | SIGNATURE_32(A,B,C,D))

// Handy section name macros.
#define SECTION_TEXT            SIGNATURE_64('.','t','e','x','t',0,0,0)         // ".text"      0x000000747865742E
#define SECTION_PAGE            SIGNATURE_64('P','A','G','E',0,0,0,0)           // "PAGE"       0x0000000045474150
#define SECTION_INIT            SIGNATURE_64('I','N','I','T',0,0,0,0)           // "INIT"       0x0000000054494E49
#define SECTION_PAGEBGFX        SIGNATURE_64('P','A','G','E','B','G','F','X')   // "PAGEBGFX"   0x5846474245474150
#define SECTION_PAGELK          SIGNATURE_64('P','A','G','E','L','K',0,0)       // "PAGELK"     0x0000454741504C4B

// Handy time macros. Thx OSR - http://www.osronline.com/article.cfm?article=261
#define ABSOLUTE(wait)          (wait)
#define RELATIVE(wait)          (-(wait))
#define NANOSECONDS(nanos)      (((LONGLONG)(nanos)) / 100L)
#define MICROSECONDS(micros)    (((LONGLONG)(micros)) * NANOSECONDS(1000L))
#define MILLISECONDS(millis)    (((LONGLONG)(millis)) * MICROSECONDS(1000L))
#define SECONDS(seconds)        (((LONGLONG)(seconds)) * MILLISECONDS(1000L))


WCHAR*
UTLAPI
UtlWcsStr(
    IN CONST WCHAR* String1,
    IN CONST WCHAR* String2,
    IN BOOLEAN CaseInsensitive
    );

BOOLEAN
UTLAPI
UtlSuffixUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

int
UTLAPI
UtlAsciiStrToInteger(
    IN CONST CHAR *str
    );

int
UTLAPI
UtlWideStrToInteger(
    IN CONST WCHAR *str
    );

NTSTATUS
UTLAPI
UtlExtractFileName(
    IN PCUNICODE_STRING FilePath,
    OUT PUNICODE_STRING FileName
    );

NTSTATUS
UTLAPI
UtlExtractDirectory(
    IN PCUNICODE_STRING FilePath,
    OUT PUNICODE_STRING FileDirectory
    );

BOOLEAN
UTLAPI
UtlIsAddressValid(
    IN ULONG_PTR Address
    );


USHORT
UTLAPI
UtlGetNtKernelBuild(
    OUT PUSHORT *BuildNumberPtr OPTIONAL
    );

CONST CHAR*
UTLAPI
UtlGetNtKernelBuildLab(
    VOID
    );

PVOID
UTLAPI
UtlGetNtKernelBase(
    OUT PULONG Size OPTIONAL
    );

PKSERVICE_TABLE_DESCRIPTOR
UTLAPI
UtlGetServiceDescriptorTable(
    IN ULONG ServiceTableIndex OPTIONAL
    );

PVOID
UTLAPI
UtlGetServiceDescriptorTableEntry(
    IN PKSERVICE_TABLE_DESCRIPTOR ServiceTable OPTIONAL,
    IN ULONG ServiceNumber
    );

/**
 * Gets the service descriptor entry index from a given function pointer.
 *
 * @param[in] pfn  The service descriptor entry function pointer.
 * @return  The service descriptor entry index.
 */
#define UtlGetServiceDescriptorTableEntryIndex(pfn) \
    *(PULONG)((ULONG_PTR)pfn + 21)

NTSTATUS
UTLAPI
UtlGetFileCertificate(
    IN WCHAR* Filename,
    OUT LPWIN_CERTIFICATE *Certificate,
    OUT ULONG *CertificateLength
    );

VOID
UTLAPI
UtlFreeCertificate(
    IN LPWIN_CERTIFICATE Certificate
    );

/**
 * Calculates a relative offset to the target from the instruction address.
 *
 * @param[in] InstructionAddress  The instruction address.
 * @param[in] TargetAddress       The target address.
 * @param[in] Offset              The offset to the instruction relative offset.
 * @return  The relative offset (MUST BE SIGNED!) between the instruction address
 *          and the target address.
 */
#define UtlGetRelativeOffset(InstructionAddress, TargetAddress, Offset) \
    ((INT32)((UINT8 *)(TargetAddress) - \
                ((UINT8 *)(InstructionAddress) + (Offset) + sizeof(INT32))))

#define UtlSetRelativeOffset(InstructionAddress, RelativeOffset, Offset) \
    *((INT32 *)((UINT8 *)(InstructionAddress) + (Offset))) = (RelativeOffset)

/**
 * Calculates a relative offset to the target from the jmp address.
 *
 * @param[in] JmpAddress    The jmp address.
 * @param[in] TargetAddress The target address.
 * @return  The relative offset (MUST BE SIGNED!) between the jmp address
 *          and the target address.
 */
FORCEINLINE
INT32
UtlGetRelativeJmpOffset(
    IN PVOID JmpAddress,
    IN PVOID TargetAddress
)
{
    if (*(UINT8 *)JmpAddress == 0xE9 || *(UINT8 *)JmpAddress == 0xEB) {
        return UtlGetRelativeOffset(JmpAddress, TargetAddress, 1);
    } else if (*(UINT8 *)JmpAddress == 0x0F && (*((UINT8 *)JmpAddress + 1) & 0x80) == 0x80) {
        return UtlGetRelativeOffset(JmpAddress, TargetAddress, 2);
    }
    return 0;
}

FORCEINLINE
VOID
UtlSetRelativeJmpOffset(
    IN PVOID JmpAddress,
    IN INT32 RelativeJmpOffset
)
{
    if (*(UINT8 *)JmpAddress == 0xE9 || *(UINT8 *)JmpAddress == 0xEB) {
        UtlSetRelativeOffset(JmpAddress, RelativeJmpOffset, 1);
    } else if (*(UINT8 *)JmpAddress == 0x0F && (*((UINT8 *)JmpAddress + 1) & 0x80) == 0x80) {
        UtlSetRelativeOffset(JmpAddress, RelativeJmpOffset, 2);
    }
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
INT32
UtlGetRelativeCallOffset(
    IN PVOID CallAddress,
    IN PVOID TargetAddress
)
{
    if (*(UINT8 *)CallAddress == 0xFF && *((UINT8 *)CallAddress + 1) == 0x15) {
        return UtlGetRelativeOffset(CallAddress, TargetAddress, 2);
    } else if (*(UINT8 *)CallAddress == 0xE8) {
        return UtlGetRelativeOffset(CallAddress, TargetAddress, 1);
    }
    return 0;
}

FORCEINLINE
VOID
UtlSetRelativeCallOffset(
    IN PVOID CallAddress,
    IN INT32 RelativeCallOffset
)
{
    if (*(UINT8 *)CallAddress == 0xE8) {
        UtlSetRelativeOffset(CallAddress, RelativeCallOffset, 1);
    } else if (*(UINT8 *)CallAddress == 0xFF && *((UINT8 *)CallAddress + 1) == 0x15) {
        UtlSetRelativeOffset(CallAddress, RelativeCallOffset, 2);
    }
}

/**
 * Calculates the target address from an instruction.
 *
 * @param[in] InstructionAddress  The instruction to get the target address from.
 * @param[in] Offset              The offset to the instruction relative offset.
 * @return The address of the target.
 */
#define UtlGetTargetAddress(InstructionAddress, Offset) \
    ((PVOID)((ULONG_PTR)(InstructionAddress) + \
                ((*(INT32 *)((UINT8 *)(InstructionAddress) + (Offset))) + \
                                (Offset) + sizeof(INT32))))

PVOID
UTLAPI
UtlCallTargetAddress(
    IN PVOID CallInstruction
    );

PVOID
UTLAPI
UtlJmpTargetAddress(
    IN PVOID JmpInstruction
    );

PVOID
UTLAPI
UtlMemOperandTargetAddress(
    IN PVOID MemInstruction,
    IN INT32 AdditionalBias
    );

PVOID
UTLAPI
UtlMemMem(
    IN const VOID *SearchBase,
    IN SIZE_T SearchSize,
    IN const VOID *Pattern,
    IN SIZE_T PatternSize
    );

int
UTLAPI
UtlMemCmp(
    IN const UCHAR *Buf1,
    IN const UCHAR Wildcard,
    IN const UCHAR *Buf2,
    IN SIZE_T Size
    );

PVOID
UTLAPI
UtlFindPattern(
    IN CONST VOID *SearchBase,
    IN SIZE_T SearchSize,
    IN UCHAR Wildcard,
    IN CONST UCHAR *Pattern,
    IN SIZE_T PatternSize
    );

NTSTATUS
UTLAPI
UtlFindPatternInSection(
    IN PVOID Base,
    IN ULONGLONG SectionName,
    IN UCHAR Wildcard,
    IN CONST UCHAR* Pattern,
    IN SIZE_T PatternSize,
    OUT PVOID *FoundAddress
    );

NTSTATUS
UTLAPI
UtlFindFunctionStartFromPtr(
    IN PVOID Ptr,
    IN SIZE_T MaxSearchSize OPTIONAL,
    OUT PVOID *FoundAddress
    );

NTSTATUS
UTLAPI
UtlGetSymbolicLink(
    IN PCUNICODE_STRING SymbolicLinkName,
    IN OUT PUNICODE_STRING SymbolicLink,
    OUT PHANDLE LinkHandle
    );

NTSTATUS
UTLAPI
UtlCloseSymbolicLink(
    IN HANDLE LinkHandle
    );

NTSTATUS
UTLAPI
UtlGetSystemRootPath(
    IN OUT PUNICODE_STRING SystemRootPath
    );

#endif // _BLACKOUT_DRIVER_UTIL_H_
