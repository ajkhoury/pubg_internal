#pragma once

#include "nativecommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


///
/// < Exception Routines >
///

typedef
NTSTATUS
(NTAPI *PNT_RAISE_EXCEPTION)(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT Context,
    IN BOOLEAN FirstChance
    );
NTSYSAPI
NTSTATUS
NTAPI
NtRaiseException(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT Context,
    IN BOOLEAN FirstChance
    );


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus