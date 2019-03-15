#pragma once

#include "nativecommon.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define WOW64_BEGIN_X64(cs) \
{   \
    __asm __emit 0x6A __asm __emit (cs)                                                                                                             /* push cs                  */  \
    __asm __emit 0xE8 __asm __emit 0x00 __asm __emit 0x00 __asm __emit 0x00  __asm __emit 0x00                                                      /* call $+5                 */  \
    __asm __emit 0x83 __asm __emit 0x04 __asm __emit 0x24 __asm __emit 0x05                                                                         /* add dword [esp], 0x05    */  \
    __asm __emit 0xCB                                                                                                                               /* retf                     */  \
}

#define WOW64_END_X64(cs)   \
{   \
    __asm __emit 0xE8 __asm __emit 0x00 __asm __emit 0x00 __asm __emit 0x00 __asm __emit 0x00                                                       /* call $+5                 */  \
    __asm __emit 0xC7 __asm __emit 0x44 __asm __emit 0x24 __asm __emit 0x04 __asm __emit (cs) __asm __emit 0x00 __asm __emit 0x00 __asm __emit 0x00 /* mov dword [rsp + 4], cs  */  \
    __asm __emit 0x83 __asm __emit 0x04 __asm __emit 0x24 __asm __emit 0x0D                                                                         /* add dword [rsp], 0x0D    */  \
    __asm __emit 0xCB                                                                                                                               /* retf                     */  \
}

#define WOW64_PUSH_X64(r)   __asm __emit (0x48 | ((r) >> 3)) __asm __emit (0x50 | ((r) & 7))

#define WOW64_POP_X64(r)    __asm __emit (0x48 | ((r) >> 3)) __asm __emit (0x58 | ((r) & 7))

#define WOW64_REX_W         __asm __emit 0x48 __asm


#define WOW64_RAX   0
#define WOW64_RCX   1
#define WOW64_RDX   2
#define WOW64_RBX   3
#define WOW64_RSP   4
#define WOW64_RBP   5
#define WOW64_RSI   6
#define WOW64_RDI   7
#define WOW64_R8    8
#define WOW64_R9    9
#define WOW64_R10   10
#define WOW64_R11   11
#define WOW64_R12   12
#define WOW64_R13   13
#define WOW64_R14   14
#define WOW64_R15   15

typedef union _WOW64_REGISTER64 {
    ULONG64 Dword64;
    ULONG Dword[2];
    USHORT Word[4];
    UCHAR Byte[8];
} WOW64_REGISTER64, *PWOW64_REGISTER64;

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//
#define WOW64_CONTAINING_RECORD_X64(address, type, field) \
        ((ULONG64)((ULONG64)(address) - (ULONG64)(&((type *)0)->field)))

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
