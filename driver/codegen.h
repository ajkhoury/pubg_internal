/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file codegen.h
 * @author Aidan Khoury (ajkhoury)
 * @date 10/24/2018
 */

#ifndef _BLACKOUT_DRIVER_CODEGEN_H_
#define _BLACKOUT_DRIVER_CODEGEN_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

// Codegen API calling convention.
#define CGAPI  NTAPI

// HCODE is a pointer to the opaque CG_CODE_GENERATOR structure.
typedef PVOID  HCODEGEN;
typedef HCODEGEN *PHCODEGEN;

// Various supported calling conventions.
#define CC_CDECL    0
#define CC_STDCALL  1
#define CC_THISCALL 2
#define CC_FASTCALL 3
#define CC_WIN64    4

// General Purpose Registers
typedef enum _CG_REGISTER {
    RAX = 0,
    RCX,
    RDX,
    RBX,

    EAX = 0,
    ECX,
    EDX,
    EBX,

    AX = 0,
    CX,
    DX,
    BX,

    AL = 0,
    CL,
    DL,
    BL,

    RSP = 4,
    RBP,
    RSI,
    RDI,

    ESP = 4,
    EBP,
    ESI,
    EDI,

    SP = 4,
    BP,
    SI,
    DI,

    AH = 4,
    CH,
    DH,
    BH,

    R8 = 8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
} CG_REGISTER;

// Segment Registers
typedef enum _CG_SEGMENT {
    ES = 0,
    CS,
    SS,
    DS,
    FS,
    GS,
} CG_SEGMENT;

// Various MOV instruction types
typedef enum _CG_MOV_TYPE {
    MOV_r8_r8 = 1,  // Move r8 to r8
    MOV_r16_r16,    // Move r16 to r16
    MOV_r32_r32,    // Move r32 to r32
    MOV_r64_r64,    // Move r64 to r64

    MOV_m8_r8,      // Move r8 to m8
    MOV_m16_r16,    // Move r16 to m16
    MOV_m32_r32,    // Move r32 to m32
    MOV_m64_r64,    // Move r64 to m64

    MOV_r8_m8,      // Move m8 to r8
    MOV_r16_m16,    // Move m16 to r16
    MOV_r32_m32,    // Move m32 to r32
    MOV_r64_m64,    // Move m64 to r64

    MOV_r8_imm8,    // Move imm8 to r8
    MOV_r16_imm16,  // Move imm16 to r16
    MOV_r32_imm32,  // Move imm32 to r32
    MOV_r64_imm64,  // Move imm64 to r64

    MOV_m8_imm8,    // Move imm8 to m8
    MOV_m16_imm16,  // Move imm16 to m16
    MOV_m32_imm32,  // Move imm32 to m32
    MOV_m64_imm64,  // Move imm64 to m64

    MOV_r16_Sreg,   // Move segment register to r16
    MOV_m16_Sreg,   // Move segment register to m16

    MOV_Sreg_r16,   // Move r16 to segment register
    MOV_Sreg_m16,   // Move m16 to segment register

} CG_MOV_TYPE;

// Various push instruction types
typedef enum _CG_PUSH_TYPE {
    PUSH_r16 = 1,   // Push r16
    PUSH_r32,       // Push r32
    PUSH_r64,       // Push r64

    PUSH_rm16,      // Push r/m16
    PUSH_rm32,      // Push r/m32
    PUSH_rm64,      // Push r/m64

    PUSH_m16,       // Push m16
    PUSH_m32,       // Push m32
    PUSH_m64,       // Push m64

    PUSH_imm8,      // Push imm8
    PUSH_imm16,     // Push imm16
    PUSH_imm32,     // Push imm32
    PUSH_imm64,     // Push imm64

    PUSH_Sreg,      // Push segment register

} CG_PUSH_TYPE;

// Various pop instruction types
typedef enum _CG_POP_TYPE {
    POP_r16 = 1,    // Pop r16
    POP_r32,        // Pop r32
    POP_r64,        // Pop r64
                    
    POP_rm16,       // Pop r/m16
    POP_rm32,       // Pop r/m32
    POP_rm64,       // Pop r/m64

    POP_m16,        // Pop m16
    POP_m32,        // Pop m32
    POP_m64,        // Pop m64

    POP_Sreg,       // Pop segment register

} CG_POP_TYPE;


NTSTATUS
CGAPI
CgInitializeGenerator(
    OUT HCODEGEN *Generator,
    IN PVOID CodeBaseAddress,
    IN SIZE_T CodeSize,
    IN BOOLEAN Generate64BitCode
    );

VOID
CGAPI
CgResetGenerator(
    IN OUT HCODEGEN Generator
    );

VOID
CGAPI
CgDestroyGenerator(
    IN OUT HCODEGEN Generator
    );

ULONG
CGAPI
CgAddInt8(
    IN HCODEGEN Generator,
    IN INT8 Value
    );

#define CgAddChar(code, value) \
    CgAddInt8(code,value)

ULONG
CGAPI
CgAddUInt8(
    IN HCODEGEN Generator,
    IN UINT8 Value
    );

#define CgAddUChar(code, value) \
    CgAddUInt8(code,value)

ULONG
CGAPI
CgAddInt16(
    IN HCODEGEN Generator,
    IN INT16 Value
    );

ULONG
CGAPI
CgAddUInt16(
    IN HCODEGEN Generator,
    IN UINT16 Value
    );

ULONG
CGAPI
CgAddInt32(
    IN HCODEGEN Generator,
    IN INT32 Value
    );

ULONG
CGAPI
CgAddUInt32(
    IN HCODEGEN Generator,
    IN UINT32 Value
    );

ULONG
CGAPI
CgAddInt64(
    IN HCODEGEN Generator,
    IN INT64 Value
    );

ULONG
CGAPI
CgAddUInt64(
    IN HCODEGEN Generator,
    IN UINT64 Value
    );

ULONG
CGAPI
CgAddPointer(
    IN HCODEGEN Generator,
    IN PVOID Value
    );

VOID
CGAPI
CgPushPointer(
    IN HCODEGEN Generator,
    IN PVOID Value
    );

VOID
CGAPI
CgPushInt64(
    IN HCODEGEN Generator,
    IN INT64 Value
    );

VOID
CGAPI
CgPushUInt64(
    IN HCODEGEN Generator,
    IN UINT64 Value
    );

VOID
CGAPI
CgPushInt32(
    IN HCODEGEN Generator,
    IN INT32 Value
    );

VOID
CGAPI
CgPushUInt32(
    IN HCODEGEN Generator,
    IN UINT32 Value
    );

VOID
CGAPI
CgPushInt16(
    IN HCODEGEN Generator,
    IN INT16 Value
    );

VOID
CGAPI
CgPushUInt16(
    IN HCODEGEN Generator,
    IN UINT16 Value
    );

VOID
CGAPI
CgPushInt8(
    IN HCODEGEN Generator,
    IN INT8 Value
    );

VOID
CGAPI
CgPushUInt8(
    IN HCODEGEN Generator,
    IN UINT8 Value
    );

VOID
CGAPI
CgPushBoolean(
    IN HCODEGEN Generator,
    IN BOOLEAN Value
    );

ULONG
CGAPI
CgBeginCode(
    IN HCODEGEN Generator,
    IN BOOLEAN BackupAll
    );

ULONG
CGAPI
CgEndCode(
    IN HCODEGEN Generator
    );

ULONG
CGAPI
CgCall(
    IN HCODEGEN Generator,
    IN ULONG CallingConvention,
    IN PVOID CallAddress
    );

ULONG
CGAPI
CgReturn(
    IN HCODEGEN Generator,
    IN USHORT ReturnSize
    );

ULONG
CGAPI
CgPush(
    IN HCODEGEN Generator,
    IN CG_PUSH_TYPE PushType,
    IN ULONGLONG Operand
    );

ULONG
CGAPI
CgPop(
    IN HCODEGEN Generator,
    IN CG_POP_TYPE PopType,
    IN ULONGLONG Operand
    );

//ULONG
//CGAPI
//CgMov(
//    IN HCODEGEN Generator,
//    IN CG_MOV_TYPE MovType,
//    IN ULONGLONG Operand1,
//    IN ULONGLONG Operand2
//    );

#endif // _BLACKOUT_DRIVER_CODEGEN_H_
