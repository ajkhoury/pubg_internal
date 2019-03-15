#ifndef _UTILS_DISASM_H_
#define _UTILS_DISASM_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include <contrib/distorm/include/distorm.h>
#include <contrib/distorm/include/mnemonics.h>

#define DISASM_MAX_INSTRUCTION_LENGTH 15

#define DISASM_X64      1
#define DISASM_X86      0

#ifdef __cplusplus
extern "C" {
#endif

#define DIS_SUCCESS 0
#define DIS_OVERFLOW -1
#define DIS_INVALID_PARAM -2
#define DIS_INVALID -3

typedef int32_t DisStatus;
typedef _DInst DisData;
typedef _Operand DisOp;

/**
 * Disassemble a single instruction.
 *
 * @param[in] Instruction Pointer to the code for disassemble
 * @param[out] Result     Pointer to disassembly data structure
 * @param[in] LongMode    Set this flag for 64-bit code, and clear for 32-bit
 *
 * @return length of the instruction. 0 if invalid instruction.
 */
uint32_t
DisInstruction(
    /* In */ const uint8_t *Instruction,
    /* Out */ DisData *Result,
    /* In */ uint8_t LongMode
    );

/**
 * Decomposes a series of instructions at the address given by the Code
 * parameter.
 *
 * @param[in] Code              The code to decompose.
 * @param[in] CodeLength        The length of code to decompose.
 * @param[in] VirtualAddress    The optional virtual address of the code.
 * @param[out] ResultInsts      The resulting array of instructions.
 * @param[in] MaxInstCount      The maximum number of instructions in the array.
 * @param[out] ResultInstCount  The number of resulting decomposed instructions.
 * @param[out] NextOffset       The offset of the next instructions to decompose.
 * @param[in] LongMode          Set this flag for 64-bit code, and clear for 32-bit.
 *
 * @return  An appropriate status/rc value depending on if an error occurred
 *          or not.
 */
DisStatus
DisDecompose(
    /* In */ const uint8_t *Code,
    /* In */ uint32_t CodeLength,
    /* In */ uint64_t VirtualAddress /*optional*/,
    /* Out */ DisData ResultInsts[],
    /* In */ uint32_t MaxInstCount,
    /* Out */ uint32_t *ResultInstCount,
    /* Out */ size_t *NextOffset /*optional*/,
    /* In */ uint8_t LongMode
    );

#ifdef __cplusplus
}
#endif

#endif // _UTILS_DISASM_H_
