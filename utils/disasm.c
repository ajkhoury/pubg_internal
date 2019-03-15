#include "disasm.h"

uint32_t
DisInstruction(
    /* In */ const uint8_t *Instruction,
    /* Out */ DisData *Result,
    /* In */ uint8_t LongMode
)
{
    _CodeInfo ci;
    unsigned int instCount;
    _DecodeResult decodeResult;

    ci.codeOffset = (_OffsetType)Instruction;
    ci.code = Instruction;
    ci.codeLen = DISASM_MAX_INSTRUCTION_LENGTH;
    ci.dt = LongMode ? Decode64Bits : Decode32Bits;
    ci.features = DF_NONE;

    decodeResult = distorm_decompose64(&ci, Result, 1, &instCount);
    if (!(decodeResult == DECRES_SUCCESS || (decodeResult == DECRES_MEMORYERR && instCount))) {
        return 0;
    }

    return Result->size;
}

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
)
{
    _CodeInfo ci;
    _DecodeResult decodeResult;

    ci.codeOffset = (_OffsetType)VirtualAddress;
    ci.code = Code;
    ci.codeLen = CodeLength;
    ci.dt = LongMode ? Decode64Bits : Decode32Bits;
    ci.features = DF_NONE;

    decodeResult = distorm_decompose64(&ci, ResultInsts, MaxInstCount, ResultInstCount);

    if (NextOffset) {
        *NextOffset = ci.nextOffset;
    }

    switch (decodeResult) {
    case DECRES_INPUTERR:
        return DIS_INVALID_PARAM;
    case DECRES_MEMORYERR:
        return DIS_OVERFLOW;
    case DECRES_SUCCESS:
        return DIS_SUCCESS;
    default:
        return DIS_INVALID;
    }
}
