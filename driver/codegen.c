/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file codegen.c
 * @author Aidan Khoury (ajkhoury)
 * @date 10/24/2018
 */

#include "codegen.h"
#include "mm.h"

#include "driver.h"

#include "log.h"

typedef enum _CG_PARAMETER_TYPE {
    ParameterTypeInvalid = 0,
    ParameterTypeInt8 = 1,
    ParameterTypeUInt8,
    ParameterTypeInt16,
    ParameterTypeUInt16,
    ParameterTypeInt32,
    ParameterTypeUInt32,
    ParameterTypeInt64,
    ParameterTypeUInt64,
    ParameterTypeBoolean,
    ParameterTypeFloat,
    ParameterTypeDouble,
    ParameterTypePointer,
} CG_PARAMETER_TYPE, *PCG_PARAMETER_TYPE;

typedef enum _CG_PARAMETER_INDEX {
    ParameterIndexRcxXmm0 = 0,
    ParameterIndexRdxXmm1,
    ParameterIndexR8Xmm2,
    ParameterIndexR9Xmm3,

    ParameterIndexMax
} CG_PARAMETER_INDEX, *PCG_PARAMETER_INDEX;

typedef union _CG_VALUE {
    INT8 Int8[8];
    UINT8 UInt8[8];

    INT16 Int16[4];
    UINT16 UInt16[4];

    INT32 Int32[2];
    UINT32 UInt32[2];

    PVOID Pointer;

    INT64 Int64;
    UINT64 UInt64;
} CG_VALUE;

typedef struct _CG_PARAMETER {
    CG_PARAMETER_TYPE Type;
    CG_VALUE Value;
} CG_PARAMETER, *PCG_PARAMETER;

typedef struct _CG_CODE_GENERATOR {
    UINT8 *Buffer; // Should be allocated before hand
    ULONG Offset;
    ULONG Size;
    BOOLEAN Is64Bit;
    BOOLEAN BackupAll;
    BOOLEAN LoopCall;
    ULONG ParameterStart;
    ULONG ParameterCount;
    CG_PARAMETER Parameters[ANYSIZE_ARRAY]; // Max params is 13
} CG_CODE_GENERATOR, *PCG_CODE_GENERATOR;

//
// Private implementation
//

static
ULONG
CgpAddParameter(
    IN PCG_CODE_GENERATOR Generator,
    IN ULONG ParameterIndex,
    IN PCG_PARAMETER Parameter
)
{
    ULONG Size = 0;

    if (Generator->Is64Bit) {

        //
        // Add parameters following the Microsoft x64 ABI.
        //
        switch (ParameterIndex) {

            //
            // First parameter is passed into the RCX or XMM0 register.
            //
        case ParameterIndexRcxXmm0:
            switch (Parameter->Type) {
            case ParameterTypeInt64:            //
            case ParameterTypeUInt64:           // all the same 8 bytes
            case ParameterTypePointer:          //
                // movabs rcx, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB948);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                break;
            case ParameterTypeInt32:            //
            case ParameterTypeUInt32:           // 4 byte integers
                // mov    ecx, ValueUInt32
                Size += CgAddUInt8(Generator, 0xB9);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                break;
            case ParameterTypeInt16:            //
            case ParameterTypeUInt16:           // 2 byte integers
                // mov    ecx, ValueUInt16
                Size += CgAddUInt8(Generator, 0xB9);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt16[0]);
                break;
            case ParameterTypeInt8:             //
            case ParameterTypeUInt8:            // 1 byte integers
            case ParameterTypeBoolean:          //
                // mov    ecx, ValueUInt8
                Size += CgAddUInt8(Generator, 0xB9);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt8[0]);
                break;
            case ParameterTypeFloat:            // 4 byte floating point
                // mov    rax, ValueUInt32
                Size += CgAddUInt8(Generator, 0x48);
                Size += CgAddUInt16(Generator, 0xC0C7);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                // movq   xmm0, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xC06E0F48);
                break;
            case ParameterTypeDouble:           // 8 byte floating point
                // movabs rax, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB848);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                // movq   xmm0, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xC06E0F48);
                break;
            }
            break;

            //
            // Second parameter is passed into the RDX or XMM1 register.
            //
        case ParameterIndexRdxXmm1:
            switch (Parameter->Type) {
            case ParameterTypeInt64:            //
            case ParameterTypeUInt64:           // all the same 8 bytes
            case ParameterTypePointer:          //
                // movabs rdx, ValueUInt64
                Size += CgAddUInt16(Generator, 0xBA48);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                break;
            case ParameterTypeInt32:            //
            case ParameterTypeUInt32:           // 4 byte integers
                // mov    edx, ValueUInt32
                Size += CgAddUInt8(Generator, 0xBA);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                break;
            case ParameterTypeInt16:            //
            case ParameterTypeUInt16:           // 2 byte integers
                // mov    edx, ValueUInt16
                Size += CgAddUInt8(Generator, 0xBA);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt16[0]);
                break;
            case ParameterTypeInt8:             //
            case ParameterTypeUInt8:            // 1 byte integers
            case ParameterTypeBoolean:          //
                // mov    edx, ValueUInt8
                Size += CgAddUInt8(Generator, 0xBA);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt8[0]);
                break;
            case ParameterTypeFloat:            // 4 byte floating point
                // mov    rax, ValueUInt32
                Size += CgAddUInt8(Generator, 0x48);
                Size += CgAddUInt16(Generator, 0xC0C7);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                // movq   xmm1, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xC86E0F48);
                break;
            case ParameterTypeDouble:           // 8 byte floating point
                // movabs rax, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB848);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                // movq   xmm1, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xC86E0F48);
                break;
            }
            break;

            //
            // Third parameter is passed into the R8 or XMM2 register.
            //
        case ParameterIndexR8Xmm2:
            switch (Parameter->Type) {
            case ParameterTypeInt64:            //
            case ParameterTypeUInt64:           // all the same 8 bytes
            case ParameterTypePointer:          //
                // movabs r8, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB849);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                break;
            case ParameterTypeInt32:            //
            case ParameterTypeUInt32:           // 4 byte integers
                // mov    r8d, ValueUInt32
                Size += CgAddUInt16(Generator, 0xB841);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                break;
            case ParameterTypeInt16:            //
            case ParameterTypeUInt16:           // 2 byte integers
                // mov    r8d, ValueUInt16
                Size += CgAddUInt16(Generator, 0xB841);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt16[0]);
                break;
            case ParameterTypeInt8:             //
            case ParameterTypeUInt8:            // 1 byte integers
            case ParameterTypeBoolean:          //
                // mov    r8d, ValueUInt8
                Size += CgAddUInt16(Generator, 0xB841);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt8[0]);
                break;
            case ParameterTypeFloat:            // 4 byte floating point
                // mov    rax, ValueUInt32
                Size += CgAddUInt8(Generator, 0x48);
                Size += CgAddUInt16(Generator, 0xC0C7);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                // movq   xmm2, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xD06E0F48);
                break;
            case ParameterTypeDouble:           // 8 byte floating point
                // movabs rax, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB848);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                // movq   xmm2, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xD06E0F48);
                break;
            }
            break;

            //
            // Fourth parameter is passed into the R9 or XMM3 register.
            //
        case ParameterIndexR9Xmm3:
            switch (Parameter->Type) {
            case ParameterTypeInt64:            //
            case ParameterTypeUInt64:           // all the same 8 bytes
            case ParameterTypePointer:          //
                // movabs r9, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB949);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                break;
            case ParameterTypeInt32:            //
            case ParameterTypeUInt32:           // 4 byte integers
                // mov    r9d, ValueUInt32
                Size += CgAddUInt16(Generator, 0xB941);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                break;
            case ParameterTypeInt16:            //
            case ParameterTypeUInt16:           // 2 byte integers
                // mov    r9d, ValueUInt16
                Size += CgAddUInt16(Generator, 0xB941);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt16[0]);
                break;
            case ParameterTypeInt8:             //
            case ParameterTypeUInt8:            // 1 byte integers
            case ParameterTypeBoolean:          //
                // mov    r9d, ValueUInt8
                Size += CgAddUInt16(Generator, 0xB941);
                Size += CgAddUInt32(Generator, (UINT32)Parameter->Value.UInt8[0]);
                break;
            case ParameterTypeFloat:            // 4 byte floating point
                // mov    rax, ValueUInt32
                Size += CgAddUInt8(Generator, 0x48);
                Size += CgAddUInt16(Generator, 0xC0C7);
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                // movq   xmm3, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xD86E0F48);
                break;
            case ParameterTypeDouble:           // 8 byte floating point
                // movabs rax, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB848);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                // movq   xmm3, rax
                Size += CgAddUInt8(Generator, 0x66);
                Size += CgAddUInt32(Generator, 0xD86E0F48);
                break;
            }
            break;

            //
            // Remaining parameters are put on the stack AFTER shadow space.
            //
        default:
            switch (Parameter->Type) {
            case ParameterTypeInt64:            //
            case ParameterTypeUInt64:           // all the same 8 bytes
            case ParameterTypePointer:          //
            case ParameterTypeDouble:           //
            case ParameterTypeFloat:            // cause XMM registers are actually 128 bits
                // movabs rax, ValueUInt64
                Size += CgAddUInt16(Generator, 0xB848);
                Size += CgAddUInt64(Generator, Parameter->Value.UInt64);
                // mov    qword ptr[rsp+32], rax
                if (ParameterIndex <= 31) {
                    Size += CgAddUInt32(Generator, 0x24448948);
                    Size += CgAddUInt8(Generator, (UINT8)(0x20 + ParameterIndex * sizeof(UINT64)));
                } else {
                    Size += CgAddUInt32(Generator, 0x24848948);
                    Size += CgAddUInt32(Generator, (UINT32)(0x20 + ParameterIndex * sizeof(UINT64)));
                }
                break;
            case ParameterTypeInt32:            //
            case ParameterTypeUInt32:           // 4 byte integers
                // mov    dword ptr[rsp+32], ValueUInt32
                Size += CgAddUInt8(Generator, 0xC7);
                if (ParameterIndex <= 31) {
                    Size += CgAddUInt16(Generator, 0x2444);
                    Size += CgAddUInt8(Generator, (UINT8)(0x20 + ParameterIndex * sizeof(UINT64)));
                } else {
                    Size += CgAddUInt16(Generator, 0x2484);
                    Size += CgAddUInt32(Generator, (UINT32)(0x20 + ParameterIndex * sizeof(UINT64)));
                }
                Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
                break;
            case ParameterTypeInt16:            //
            case ParameterTypeUInt16:           // 2 byte integers
                // mov    word ptr[rsp+32], ValueUInt16
                Size += CgAddUInt16(Generator, 0xC766);
                if (ParameterIndex <= 31) {
                    Size += CgAddUInt16(Generator, 0x2444);
                    Size += CgAddUInt8(Generator, (UINT8)(0x20 + ParameterIndex * sizeof(UINT64)));
                } else {
                    Size += CgAddUInt16(Generator, 0x2484);
                    Size += CgAddUInt32(Generator, (UINT32)(0x20 + ParameterIndex * sizeof(UINT64)));
                }
                Size += CgAddUInt16(Generator, Parameter->Value.UInt16[0]);
                break;
            case ParameterTypeInt8:             //
            case ParameterTypeUInt8:            // 1 byte integers
            case ParameterTypeBoolean:          //
                // mov    byte ptr[rsp+32], ValueUInt8
                Size += CgAddUInt8(Generator, 0xC6);
                if (ParameterIndex <= 31) {
                    Size += CgAddUInt16(Generator, 0x2444);
                    Size += CgAddUInt8(Generator, (UINT8)(0x20 + ParameterIndex * sizeof(UINT64)));
                } else {
                    Size += CgAddUInt16(Generator, 0x2484);
                    Size += CgAddUInt32(Generator, (UINT32)(0x20 + ParameterIndex * sizeof(UINT64)));
                }
                Size += CgAddUInt8(Generator, Parameter->Value.UInt8[0]);
                break;
            }
            break;
        }

    } else {
    
        //
        // Push parameters onto stack.
        //
        switch (Parameter->Type) {
        case ParameterTypeInt64:    //
        case ParameterTypeUInt64:   // all the same 8 bytes
        case ParameterTypeDouble:   //
            // ill do this later
            // push ulParam
            Size += CgAddUInt8(Generator, 0x68);
            Size += CgAddUInt64(Generator, Parameter->Value.UInt32[0]);
            break;
        
        case ParameterTypePointer:          // 4 bytes on 32-bit architecture
            // push ptr
            Size += CgAddUInt8(Generator, 0x68);
            Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
            break;
        
        case ParameterTypeInt16:    //
        case ParameterTypeUInt16:   // short is interpreted as 4 bytes in this case
        case ParameterTypeInt32:    // all the same shit 4 bytes 
        case ParameterTypeUInt32:   // 
        case ParameterTypeFloat:    //
            // push ulParam
            Size += CgAddUInt8(Generator, 0x68);
            Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
            break;
        
        case ParameterTypeInt8:
        case ParameterTypeUInt8:
            // push ucParam
            Size += CgAddUInt8(Generator, 0x6A); // 0x6A is for pushing bytes
            Size += CgAddUInt8(Generator, Parameter->Value.UInt8[0]);
            break;
        
        case ParameterTypeBoolean:
            // push ucParam
            Size += CgAddUInt8(Generator, 0x6A);
            Size += CgAddUInt8(Generator, Parameter->Value.UInt8[0] ? TRUE : FALSE);
            break;
        
        default:
            // Default to 32 bit long value.
            // push ulParam
            Size += CgAddUInt8(Generator, 0x68);
            Size += CgAddUInt32(Generator, Parameter->Value.UInt32[0]);
            break;
        }
    }

    return Size;
}

static
ULONG
CgpPushAllParameters(
    IN PCG_CODE_GENERATOR Generator,
    IN BOOLEAN RightToLeft
)
{
    ULONG Index;
    ULONG Size = 0;
    ULONG ParameterIndex = 0;

    if (RightToLeft) {

        //
        // Right to left.
        //
        for (Index = Generator->ParameterCount;
             Index > Generator->ParameterStart;
             Index--) {

            Size += CgpAddParameter(Generator,
                                    ParameterIndex++,
                                    &Generator->Parameters[Index - 1]
                                    );
        }

    } else {

        //
        // Left to right.
        //
        for (Index = Generator->ParameterStart;
             Index < Generator->ParameterCount;
             Index++) {

            Size += CgpAddParameter(Generator,
                                    ParameterIndex++,
                                    &Generator->Parameters[Index]
                                    );
        }
    }

    return Size;
}

FORCEINLINE
VOID
CgpPushParameter(
    IN PCG_CODE_GENERATOR Generator,
    IN PCG_PARAMETER Parameter
)
{
    RtlCopyMemory(&Generator->Parameters[Generator->ParameterCount],
                  Parameter,
                  sizeof(CG_PARAMETER));

    ++Generator->ParameterCount;
}

static
ULONG
CgpAddJumpShort(
    IN PCG_CODE_GENERATOR Generator,
    IN LONG Jump
)
{
    ULONG Size;

    Size = CgAddUInt8(Generator, 0xEB);
    if (Jump < 2) {
        Size += CgAddUInt8(Generator, (UINT8)(0xFE + Jump));
    } else {
        Size += CgAddUInt8(Generator, (UINT8)(Jump - 0x02));
    }

    return Size;
}

static
ULONG
CgpAddCall(
    IN PCG_CODE_GENERATOR Generator,
    IN PVOID CallAddress
)
{
    ULONG Size;

    if (Generator->Is64Bit) {

        // movabs rax, CallAddress
        Size  = CgAddUInt16(Generator, 0xB848); // 14
        Size += CgAddUInt64(Generator, (UINT64)CallAddress);
        // call rax
        Size += CgAddUInt16(Generator, 0xD0FF);

    } else {

        // movabs eax, CallAddress
        Size  = CgAddUInt8(Generator, 0xB8);
        Size += CgAddUInt32(Generator, (UINT32)(ULONG_PTR)CallAddress);
        // call eax
        Size += CgAddUInt16(Generator, 0xD0FF);
    }

    return Size;
}

#define IS_ALIGNED(v,a) (ALIGN_DOWN_BY((v), (a)) == (v))

static
ULONG
CgpCallBegin(
    IN PCG_CODE_GENERATOR Generator,
    IN ULONG CallingConvention
)
{
    ULONG Size = 0;
    ULONG ParameterCount = Generator->ParameterCount;
    ULONG StackOffset;

    // May use this later.
    UNREFERENCED_PARAMETER(CallingConvention);

    if (Generator->Is64Bit) {

        StackOffset = 32;
        if (ParameterCount > 4) {
            StackOffset += ParameterCount * sizeof(UINT64);
        }
        if (IS_ALIGNED(StackOffset, 16)) {
            StackOffset += 8;   // Adjust for return address.
        }

        //
        // Allocate needed stack space.
        //
        if (StackOffset <= 0x7F) {

            // sub rsp, (byte)StackOffset
            Size += CgAddUInt8(Generator, 0x48);
            Size += CgAddUInt16(Generator, 0xEC83);
            Size += CgAddUInt8(Generator, (UINT8)StackOffset);

        } else {

            // sub rsp, StackOffset
            Size += CgAddUInt8(Generator, 0x48);
            Size += CgAddUInt16(Generator, 0xEC81);
            Size += CgAddUInt32(Generator, (UINT32)StackOffset);
        }

    } else {
        // TODO: create stack frame here?
    }

    return Size;
}

static
ULONG
CgpCallEnd(
    IN PCG_CODE_GENERATOR Generator,
    IN ULONG CallingConvention
)
{
    ULONG Size = 0;
    ULONG ParameterCount = Generator->ParameterCount;
    ULONG StackOffset;

    if (Generator->Is64Bit) {

        StackOffset = 32;
        if (ParameterCount > 4) {
            StackOffset += ParameterCount * sizeof(UINT64);
        }
        if (IS_ALIGNED(StackOffset, 16)) {
            StackOffset += 8;   // Adjust for return address.
        }

        //
        // Cleanup the stack space.
        //
        if (StackOffset <= 0x7F) {

            // add rsp, (byte)StackOffset
            Size += CgAddUInt8(Generator, 0x48);
            Size += CgAddUInt16(Generator, 0xC483);
            Size += CgAddUInt8(Generator, (UINT8)StackOffset);

        } else {

            // add rsp, StackOffset
            Size += CgAddUInt8(Generator, 0x48);
            Size += CgAddUInt16(Generator, 0xC481);
            Size += CgAddUInt32(Generator, (UINT32)StackOffset);
        }

    } else {

        //
        // Caller stack clean up with cdecl. 
        //
        if (CallingConvention == CC_CDECL) {

            //
            // Determine size to clean.
            //
            StackOffset = ParameterCount * sizeof(UINT32);
            if (StackOffset != 0) {
                if (StackOffset <= 0x7F) {

                    // add esp, (byte)StackOffset
                    Size += CgAddUInt16(Generator, 0xC483); // 83 is for adding a byte value
                    Size += CgAddUInt8(Generator, (UINT8)StackOffset);

                } else {

                    // add esp, StackOffset
                    Size += CgAddUInt16(Generator, 0xC481);
                    Size += CgAddUInt32(Generator, (UINT32)StackOffset);
                }
            }
        }
    }

    return Size;
}


//
// Public Implementation
//

//
// Add code data api.
//

ULONG
CGAPI
CgAddInt8(
    IN HCODEGEN Generator,
    IN INT8 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(INT8*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(INT8);

    return sizeof(INT8);
}

ULONG
CGAPI
CgAddUInt8(
    IN HCODEGEN Generator,
    IN UINT8 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(UINT8*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(UINT8);

    return sizeof(UINT8);
}

ULONG
CGAPI
CgAddInt16(
    IN HCODEGEN Generator,
    IN INT16 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(INT16*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(INT16);

    return sizeof(INT16);
}

ULONG
CGAPI
CgAddUInt16(
    IN HCODEGEN Generator,
    IN UINT16 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(UINT16*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(UINT16);

    return sizeof(UINT16);
}

ULONG
CGAPI
CgAddInt32(
    IN HCODEGEN Generator,
    IN INT32 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(INT32*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(INT32);

    return sizeof(INT32);
}

ULONG
CGAPI
CgAddUInt32(
    IN HCODEGEN Generator,
    IN UINT32 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(UINT32*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(UINT32);

    return sizeof(UINT32);
}

ULONG
CGAPI
CgAddInt64(
    IN HCODEGEN Generator,
    IN INT64 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(INT64*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(INT64);

    return sizeof(INT64);
}

ULONG
CGAPI
CgAddUInt64(
    IN HCODEGEN Generator,
    IN UINT64 Value
)
{
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;

    *(UINT64*)((ULONG_PTR)CodeGenerator->Buffer + CodeGenerator->Offset) = Value;
    CodeGenerator->Offset += sizeof(UINT64);

    return sizeof(UINT64);
}

ULONG
CGAPI
CgAddPointer(
    IN HCODEGEN Generator,
    IN PVOID Value
)
{
    if (((PCG_CODE_GENERATOR)Generator)->Is64Bit) {
        return CgAddUInt64(Generator, (UINT64)(ULONG_PTR)Value);
    } else {
        return CgAddUInt32(Generator, (UINT32)(ULONG_PTR)Value);
    }
}

//
// Push parameter data.
//

VOID
CGAPI
CgPushPointer(
    IN HCODEGEN Generator,
    IN PVOID Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypePointer;
    if (((PCG_CODE_GENERATOR)Generator)->Is64Bit) {
        Parameter.Value.UInt64 = (UINT64)Value;
    } else {
        Parameter.Value.UInt32[0] = (UINT32)(ULONG_PTR)Value;
        Parameter.Value.UInt32[1] = 0;
    }

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushInt64(
    IN HCODEGEN Generator,
    IN INT64 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeInt64;
    Parameter.Value.Int64 = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushUInt64(
    IN HCODEGEN Generator,
    IN UINT64 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeUInt64;
    Parameter.Value.UInt64 = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushInt32(
    IN HCODEGEN Generator,
    IN INT32 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeInt32;
    Parameter.Value.Int32[0] = Value;
    Parameter.Value.Int32[1] = 0;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushUInt32(
    IN HCODEGEN Generator,
    IN UINT32 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeUInt32;
    Parameter.Value.UInt32[0] = Value;
    Parameter.Value.UInt32[1] = 0;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushInt16(
    IN HCODEGEN Generator,
    IN INT16 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeInt16;
    Parameter.Value.UInt64 = 0;
    Parameter.Value.Int16[0] = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushUInt16(
    IN HCODEGEN Generator,
    IN UINT16 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeUInt16;
    Parameter.Value.UInt64 = 0;
    Parameter.Value.UInt16[0] = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushInt8(
    IN HCODEGEN Generator,
    IN INT8 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeInt8;
    Parameter.Value.UInt64 = 0;
    Parameter.Value.Int8[0] = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushUInt8(
    IN HCODEGEN Generator,
    IN UINT8 Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeUInt8;
    Parameter.Value.UInt64 = 0;
    Parameter.Value.UInt8[0] = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}

VOID
CGAPI
CgPushBoolean(
    IN HCODEGEN Generator,
    IN BOOLEAN Value
)
{
    CG_PARAMETER Parameter;

    Parameter.Type = ParameterTypeBoolean;
    Parameter.Value.UInt64 = 0;
    Parameter.Value.UInt8[0] = Value;

    CgpPushParameter((PCG_CODE_GENERATOR)Generator, &Parameter);
}


//
// The core of the code generator.
//

ULONG
CGAPI
CgBeginCode(
    IN HCODEGEN Generator,
    IN BOOLEAN BackupAll
)
{
    //ULONG RegisterIndex;
    ULONG Size = 0;

    if (BackupAll) {

        //
        // Mark that this code backs up all general purpose registers.
        //
        ((PCG_CODE_GENERATOR)Generator)->BackupAll = TRUE;
    }

    if (((PCG_CODE_GENERATOR)Generator)->Is64Bit) {

        //
        // Backup just non-volatile registers unless BackupAll is set.
        //
        if (BackupAll) {
            Size += CgAddUInt8(Generator, 0x50);    // push rax
            Size += CgAddUInt8(Generator, 0x51);    // push rcx
            Size += CgAddUInt8(Generator, 0x52);    // push rdx
        }
        Size += CgAddUInt8(Generator, 0x53);        // push rbx
        Size += CgAddUInt8(Generator, 0x55);        // push rbp
        Size += CgAddUInt8(Generator, 0x56);        // push rsi
        Size += CgAddUInt8(Generator, 0x57);        // push rdi
        if (BackupAll) {
            Size += CgAddUInt16(Generator, 0x5041); // push r8 
            Size += CgAddUInt16(Generator, 0x5141); // push r9
            Size += CgAddUInt16(Generator, 0x5241); // push r10
            Size += CgAddUInt16(Generator, 0x5341); // push r11
        }
        Size += CgAddUInt16(Generator, 0x5441);     // push r12
        Size += CgAddUInt16(Generator, 0x5541);     // push r13
        Size += CgAddUInt16(Generator, 0x5641);     // push r14
        Size += CgAddUInt16(Generator, 0x5741);     // push r15

    } else {

        //
        // Create stack frame
        //
        Size += CgAddUInt8(Generator, 0x55);        // push ebp
        Size += CgAddUInt16(Generator, 0xEC8B);     // mov ebp, esp

        //
        // Backup non-volatile registers.
        //
        Size += CgAddUInt8(Generator, 0x53);        // push ebx
        Size += CgAddUInt8(Generator, 0x55);        // push ebp
        Size += CgAddUInt8(Generator, 0x56);        // push esi
        Size += CgAddUInt8(Generator, 0x57);        // push edi
    }

    return Size;
}

ULONG
CGAPI
CgEndCode(
    IN HCODEGEN Generator
)
{
    //ULONG RegisterIndex;
    ULONG Size = 0;
    BOOLEAN RestoreAll = ((PCG_CODE_GENERATOR)Generator)->BackupAll;
    ((PCG_CODE_GENERATOR)Generator)->BackupAll = FALSE;

    if (((PCG_CODE_GENERATOR)Generator)->Is64Bit) {

        //
        // Restore general purpose registers.
        //
        Size += CgAddUInt16(Generator, 0x5F41);     // pop r15
        Size += CgAddUInt16(Generator, 0x5E41);     // pop r14
        Size += CgAddUInt16(Generator, 0x5D41);     // pop r13
        Size += CgAddUInt16(Generator, 0x5C41);     // pop r12
        if (RestoreAll) {
            Size += CgAddUInt16(Generator, 0x5B41); // pop r11
            Size += CgAddUInt16(Generator, 0x5A41); // pop r10
            Size += CgAddUInt16(Generator, 0x5941); // pop r9
            Size += CgAddUInt16(Generator, 0x5841); // pop r8
        }
        Size += CgAddUInt8(Generator, 0x5F);        // pop rdi
        Size += CgAddUInt8(Generator, 0x5E);        // pop rsi
        Size += CgAddUInt8(Generator, 0x5D);        // pop rbp
        Size += CgAddUInt8(Generator, 0x5B);        // pop rbx
        if (RestoreAll) {
            Size += CgAddUInt8(Generator, 0x5A);    // pop rdx
            Size += CgAddUInt8(Generator, 0x59);    // pop rcx
            Size += CgAddUInt8(Generator, 0x58);    // pop rax
        }

    } else {

        //
        // Restore non-volatile registers.
        //
        Size += CgAddUInt8(Generator, 0x5F);        // pop edi
        Size += CgAddUInt8(Generator, 0x5E);        // pop esi
        Size += CgAddUInt8(Generator, 0x5D);        // pop ebp
        Size += CgAddUInt8(Generator, 0x5B);        // pop ebx

        //
        // Cleanup stack frame
        //
        Size += CgAddUInt16(Generator, 0xEC89);     // mov esp, ebp
        Size += CgAddUInt8(Generator, 0x5D);        // pop ebp
    }

    return Size;
}

ULONG
CGAPI
CgCall(
    IN HCODEGEN Generator,
    IN ULONG CallingConvention,
    IN PVOID CallAddress
)
{
    ULONG Size;
    PCG_PARAMETER Parameters;
    PCG_CODE_GENERATOR CodeGenerator = (PCG_CODE_GENERATOR)Generator;
    ULONG ParameterCount = CodeGenerator->ParameterCount;

    //
    // Backup registers and create a new stack frame.
    //
    Size = CgpCallBegin(CodeGenerator, CallingConvention);

    //
    // Microsoft x64 calling convention.
    //
    if (CodeGenerator->Is64Bit) {

        //
        // Push all parameters (or store in RCX, RDX, R8, R9 in 64 bit).
        //
        Size += CgpPushAllParameters(CodeGenerator, FALSE);

        // movabs rax, CallAddress
        // call rax
        Size += CgpAddCall(CodeGenerator, CallAddress);
    
    } else {

        //
        // Any of the follwoing x86 calling conventions:
        //  fastcall, stdcall, cdecl, or thiscall.
        //
        Parameters = CodeGenerator->Parameters;

        if (CallingConvention == CC_FASTCALL) {

            //
            // A fastcall with one parameter is actually a stdcall with the single
            // parameter in ECX.
            //
            if (ParameterCount >= 1) {

                // mov ecx, EcxParam
                Size += CgAddUInt8(Generator, 0xB9);
                Size += CgAddUInt32(Generator, Parameters[0].Value.UInt32[0]);

                //
                // The first two parameters are stored in ECX and EDX.
                //
                if (ParameterCount >= 2) {

                    // mov edx, EdxParam
                    Size += CgAddUInt8(Generator, 0xBA);
                    Size += CgAddUInt32(Generator, Parameters[1].Value.UInt32[0]);

                    //
                    // The remaining parameters are pushed onto the stack.
                    //
                    if (ParameterCount >= 3) {

                        //
                        // We need to remove the first two parameters, so when we execute the
                        // parameter iteration function they are not re-included.
                        //
                        CodeGenerator->ParameterStart += 2;

                        //
                        // Push the rest of the parameters onto the stack in right to left order.
                        //
                        Size += CgpPushAllParameters(CodeGenerator, TRUE);
                    }
                }
            }

            // mov eax, CallAddress
            // call eax
            Size += CgpAddCall(CodeGenerator, CallAddress);

        } else if (CallingConvention == CC_STDCALL || CallingConvention == CC_CDECL) {

            //
            // Push parameters onto stack in right to left order.
            //
            Size += CgpPushAllParameters(CodeGenerator, TRUE);

            // mov eax, CallAddress
            // call eax
            Size += CgpAddCall(CodeGenerator, CallAddress);

        } else if (CallingConvention == CC_THISCALL) {

            //
            // A thiscall should have at least one parameter in ECX!
            //
            if (!ParameterCount) {

                //
                // No parameters is not valid for a thiscall!
                //
                LOG_ERROR("No parameters passed for thiscall which requires at least one parameter");
                return 0;
            }

            //
            // The first parameter of a thiscall is always ECX. The parameter type 
            // should also be a the pointer type.
            //
            if (Parameters[0].Type != ParameterTypePointer) {
                LOG_WARN("'this' parameter type invalid, should be pointer type");
            }

            //
            // 'this' pointer should never be NULL!
            //
            if (!Parameters[0].Value.UInt32[0]) {
                LOG_WARN("'this' parameter NULL for thiscall function");
            }

            // mov ecx, ptr
            Size += CgAddUInt16(Generator, 0x0D8B);
            Size += CgAddPointer(Generator, Parameters[0].Value.Pointer);

            //
            // We need to remove the first parameter, so when we execute the parameter
            // iteration function it is not included.
            //
            CodeGenerator->ParameterStart++;

            //
            // Push remainder of parameters onto stack in right to left order.
            //
            Size += CgpPushAllParameters(CodeGenerator, TRUE);

            // mov eax, CallAddress
            // call eax
            Size += CgpAddCall(CodeGenerator, CallAddress);
        }
    }

    //
    // Restore registers and cleanup stack.
    //
    Size += CgpCallEnd(CodeGenerator, CallingConvention);

    return Size;
}

ULONG
CGAPI
CgReturn(
    IN HCODEGEN Generator,
    IN USHORT ReturnSize
)
{
    ULONG Size;

    if (ReturnSize && !((PCG_CODE_GENERATOR)Generator)->Is64Bit) {
        Size = CgAddUInt8(Generator, 0xC2);
        Size += CgAddUInt16(Generator, ReturnSize); // ret 04h
    } else {
        Size = CgAddUInt8(Generator, 0xC3);         // ret
    }

    return Size;
}

#define VALIDATE_REGISTER(R) \
    (((R) & ~0x7) == 0)

#define VALIDATE_REGISTER_X64(R) \
    (((R) & ~0xF) == 0)

#define IS_REGISTER_X64(R) \
    (((R) & 8) != 0)

ULONG
CGAPI
CgPush(
    IN HCODEGEN Generator,
    IN CG_PUSH_TYPE PushType,
    IN ULONGLONG Operand
)
{
    ULONG Size = 0;
    BOOLEAN Is64Bit = ((PCG_CODE_GENERATOR)Generator)->Is64Bit;

    switch (PushType) {
        //
        // push reg instructions.
        //
    case PUSH_r16:
        Size += CgAddUInt8(Generator, 0x66);
        /* Fall-through */
    case PUSH_r32:
    case PUSH_r64:

        BO_ASSERT(VALIDATE_REGISTER(Operand));

        if (Is64Bit && IS_REGISTER_X64(Operand)) {
            Size += CgAddUInt16(Generator, (UINT16)(((0x50 | (Operand & 7)) << 8) | 0x41));
        } else {
            Size += CgAddUInt8(Generator, (UINT8)(0x50 | (Operand & 7)));
        }
        break;


        //
        // push [reg] instructions.
        //
    case PUSH_rm16:
        Size += CgAddUInt8(Generator, 0x66);
        /* Fall-through */
    case PUSH_rm32:
    case PUSH_rm64:

        BO_ASSERT(VALIDATE_REGISTER(Operand));

        if (Is64Bit && IS_REGISTER_X64(Operand)) {
            Size += CgAddUInt8(Generator, 0x41);
        }

        Size += CgAddUInt16(Generator, (UINT16)(((0x30 | (Operand & 7)) << 8) | 0xFF));
        break;


        //
        // push [addr] instructions.
        //
    case PUSH_m16:
    case PUSH_m32:
    case PUSH_m64: // RIP-relative in x64
        Size += CgAddUInt8(Generator, 0xFF);
        Size += CgAddUInt16(Generator, 0x2534);
        Size += CgAddUInt32(Generator, (UINT32)Operand);
        break;


        //
        // push imm instructions.
        //
    case PUSH_imm8:
        Size += CgAddUInt16(Generator, (UINT16)(((Operand & 0xFF) << 8) | 0x6A));
        break;
    case PUSH_imm16:
        Size += CgAddUInt16(Generator, 0x6866);
        Size += CgAddUInt16(Generator, (UINT16)Operand);
        break;
    case PUSH_imm32:
        Size += CgAddUInt8(Generator, 0x68);
        Size += CgAddUInt32(Generator, (UINT32)Operand);
        break;
    case PUSH_imm64:
        // push low32
        Size += CgAddUInt8(Generator, 0x68);
        Size += CgAddUInt32(Generator, (UINT32)Operand);
        // mov dword [rsp+4], high32
        Size += CgAddUInt32(Generator, 0x042444C7);
        Size += CgAddUInt32(Generator, (UINT32)(Operand >> 32));
        break;


        //
        // push Sreg instructions.
        //
    case PUSH_Sreg:

        BO_ASSERT(Operand <= GS);

        if (Is64Bit) {
            BO_ASSERT(Operand == FS || Operand == GS);
        }

        switch (Operand) {
        case ES:
        case CS:
        case SS:
        case DS:
            Size += CgAddUInt8(Generator, 0x06 | (Operand & 0x1F));
            break;
        case FS:
            Size += CgAddUInt16(Generator, 0xA00F);
            break;
        case GS:
            Size += CgAddUInt16(Generator, 0xA80F);
            break;
        }

        break;

    default:
        break;
    }

    return Size;
}

ULONG
CGAPI
CgPop(
    IN HCODEGEN Generator,
    IN CG_POP_TYPE PopType,
    IN ULONGLONG Operand
)
{
    ULONG Size = 0;
    BOOLEAN Is64Bit = ((PCG_CODE_GENERATOR)Generator)->Is64Bit;

    switch (PopType) {
        //
        // pop reg instructions.
        //
    case POP_r16:
        Size += CgAddUInt8(Generator, 0x66);
        /* Fall-through */
    case POP_r32:
    case POP_r64:

        BO_ASSERT(VALIDATE_REGISTER(Operand));

        if (Is64Bit && IS_REGISTER_X64(Operand)) {
            Size += CgAddUInt16(Generator, (UINT16)(((0x58 | (Operand & 7)) << 8) | 0x41));
        } else {
            Size += CgAddUInt8(Generator, (UINT8)(0x58 | (Operand & 7)));
        }
        break;


        //
        // pop [reg] instructions.
        //
    case POP_rm16:
        Size += CgAddUInt8(Generator, 0x66);
        /* Fall-through */
    case POP_rm32:
    case POP_rm64:

        BO_ASSERT(VALIDATE_REGISTER(Operand));

        if (Is64Bit && IS_REGISTER_X64(Operand)) {
            Size += CgAddUInt8(Generator, 0x41);
        }

        Size += CgAddUInt16(Generator, (UINT16)(((Operand & 7) << 8) | 0x8F));
        break;


        //
        // pop [addr] instructions.
        //
    case POP_m16:
    case POP_m32:
    case POP_m64: // RIP-relative in x64
        Size += CgAddUInt8(Generator, 0x8F);
        Size += CgAddUInt16(Generator, 0x2504);
        Size += CgAddUInt32(Generator, (UINT32)Operand);
        break;


        //
        // push Sreg instructions.
        //
    case POP_Sreg:

        BO_ASSERT(Operand <= GS && Operand != CS);

        if (Is64Bit) {
            BO_ASSERT(Operand == FS || Operand == GS);
        }

        switch (Operand) {
        case ES:
        case SS:
        case DS:
            Size += CgAddUInt8(Generator, 0x07 | (Operand & 0x1F));
            break;
        case FS:
            Size += CgAddUInt16(Generator, 0xA10F);
            break;
        case GS:
            Size += CgAddUInt16(Generator, 0xA90F);
            break;
        }

        break;

    default:
        break;
    }

    return Size;
}

//ULONG
//CGAPI
//CgMov(
//    IN HCODEGEN Generator,
//    IN CG_MOV_TYPE MovType,
//    IN ULONGLONG Operand1,
//    IN ULONGLONG Operand2
//)
//{
//    ULONG Size = 0;
//    BOOLEAN Is64Bit = ((PCG_CODE_GENERATOR)Generator)->Is64Bit;
//
//    switch (MovType) {
//    case MOV_r8_r8:
//
//        ASSERT(VALIDATE_REGISTER_X86(Operand1) && VALIDATE_REGISTER_X86(Operand2));
//
//        if (Is64Bit && (IS_REGISTER_X64(Operand1) || IS_REGISTER_X64(Operand2))) {
//            Size = CgAddUInt8(Generator, (UINT8)(0x40 | IS_REGISTER_X64(Operand2) << 2 | IS_REGISTER_X64(Operand1)));
//        }
//
//        Size += CgAddUInt16(Generator, 0x88 | ((UINT16)(0xC0 | (Operand2 << 3) | Operand1) << 8));
//        break;
//
//    case MOV_r16_r16:
//
//        ASSERT(VALIDATE_REGISTER_X86(Operand1) && VALIDATE_REGISTER_X86(Operand2));
//
//        Size += CgAddUInt8(Generator, 0x66);
//
//        if (!Is64Bit || !(IS_REGISTER_X64(Operand1) || IS_REGISTER_X64(Operand2))) {
//            goto case_MOV_r32_r32;
//        }
//        /* Fall-through */
//
//    case MOV_r64_r64:
//
//        ASSERT(VALIDATE_REGISTER_X64(Operand1) && VALIDATE_REGISTER_X64(Operand2));
//
//        Size += CgAddUInt8(Generator, (UINT8)(0x40 | IS_REGISTER_X64(Operand2) << 2 | IS_REGISTER_X64(Operand1)));
//        /* Fall-through */
//
//    case MOV_r32_r32:
//        ASSERT(VALIDATE_REGISTER_X86(Operand1) && VALIDATE_REGISTER_X86(Operand2));
//
//    case_MOV_r32_r32:
//        Size += CgAddUInt16(Generator, 0x89 | ((UINT16)(0xC0 | (Operand2 << 3) | Operand1) << 8));
//        break;
//
//    case MOV_m8_r8:
//
//        ASSERT(VALIDATE_REGISTER_X86(Operand2));
//
//        if (VALIDATE_REGISTER_X64(Operand1)) {
//            if (VALIDATE_REGISTER_X86(Operand1)) {
//                Size = CgAddUInt16(Generator, 0x88 | ((UINT16)((Operand2 << 3) | Operand1) << 8));
//            } else {
//
//            }
//        } else {
//            Size = CgAddUInt16(Generator, 0x88 | ((UINT16)((Operand2 << 3) | 0x05) << 8));
//        }
//        break;
//
//
//    }
//}

NTSTATUS
CGAPI
CgInitializeGenerator(
    OUT HCODEGEN *Generator,
    IN PVOID CodeBaseAddress,
    IN SIZE_T CodeSize,
    IN BOOLEAN Generate64BitCode
)
{
    PCG_CODE_GENERATOR CodeGenerator;
    SIZE_T CodeGeneratorSize;

    if (!CodeBaseAddress || !CodeSize || !ARGUMENT_PRESENT(Generator)) {
        return STATUS_INVALID_PARAMETER;
    }

    CodeGeneratorSize = FIELD_OFFSET(CG_CODE_GENERATOR, Parameters) + (sizeof(CG_PARAMETER) * 13);
    CodeGenerator = (PCG_CODE_GENERATOR)MmAllocateNonPagedNx(CodeGeneratorSize);
    if (!CodeGenerator) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(CodeGenerator, CodeGeneratorSize);

    CodeGenerator->Buffer = (UINT8 *)CodeBaseAddress;
    CodeGenerator->Size = (ULONG)CodeSize;
    CodeGenerator->Is64Bit = Generate64BitCode;
    CodeGenerator->BackupAll = FALSE;

    *Generator = (HCODEGEN)CodeGenerator;

    return STATUS_SUCCESS;
}

VOID
CGAPI
CgResetGenerator(
    IN OUT HCODEGEN Generator
)
{
    PCG_CODE_GENERATOR CodeGenerator;
    SIZE_T CodeGeneratorSize;
    UINT8 *Buffer;
    ULONG Size;
    BOOLEAN Is64Bit;

    CodeGenerator = (PCG_CODE_GENERATOR)Generator;
    Buffer = CodeGenerator->Buffer;
    Size = CodeGenerator->Size;
    Is64Bit = CodeGenerator->Is64Bit;
    RtlZeroMemory(Buffer, Size);

    CodeGeneratorSize = FIELD_OFFSET(CG_CODE_GENERATOR, Parameters) + (sizeof(CG_PARAMETER) * 13);
    RtlZeroMemory(CodeGenerator, CodeGeneratorSize);

    CodeGenerator->Buffer = Buffer;
    CodeGenerator->Size = Size;
    CodeGenerator->Is64Bit = Is64Bit;
}

VOID
CGAPI
CgDestroyGenerator(
    IN OUT HCODEGEN Generator
)
{
    MmFreeNonPagedNx((PVOID)Generator);
}