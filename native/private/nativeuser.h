#pragma once

#include "nativecommon.h"
#include "nativertl.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Definitions & Macros
 */

#define IS_PTR(p)       ((((ULONG_PTR)(p)) & ~USHRT_MAX) != 0)

/* Unicode->Unicode macros */
#define COPYLPWSTR(pinstr, psz)                             \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                 \
    RtlInitUnicodeString(&(pinstr)->strCapture, (psz));

#define COPYLPWSTRID(pinstr, psz)                           \
    (pinstr)->fAllocated = FALSE;                           \
    (pinstr)->pstr = &(pinstr)->strCapture;                 \
    if (IS_PTR(psz)) {                                      \
        RtlInitUnicodeString(&(pinstr)->strCapture, (psz)); \
    } else {                                                \
        (pinstr)->strCapture.Length =                       \
                (pinstr)->strCapture.MaximumLength = 0;     \
        (pinstr)->strCapture.Buffer = (LPWSTR)(psz);        \
    }

#define COPYLPWSTRIDOPT                 COPYLPWSTRID
#define COPYLPWSTROPT                   COPYLPWSTR

#define FIRSTCOPYLPWSTR                 COPYLPWSTR
#define FIRSTCOPYLPWSTRID               COPYLPWSTRID
#define FIRSTCOPYLPWSTRIDOPT            COPYLPWSTRIDOPT
#define FIRSTCOPYLPWSTROPT              COPYLPWSTROPT

#define CLEANUPLPWSTR(instr)

/* Type-neutral macros */
#if 1 //UNICODE
#define COPYLPTSTR                  COPYLPWSTR
#define COPYLPTSTRID                COPYLPWSTRID
#define COPYLPTSTRIDOPT             COPYLPWSTRIDOPT
#define COPYLPTSTROPT               COPYLPWSTROPT
#define FIRSTCOPYLPTSTR             COPYLPWSTR
#define FIRSTCOPYLPTSTRID           COPYLPWSTRID
#define FIRSTCOPYLPTSTRIDOPT        COPYLPWSTRIDOPT
#define CLEANUPLPTSTR               CLEANUPLPWSTR
#else
#define COPYLPTSTR                  COPYLPSTRW
#define COPYLPTSTRID                COPYLPSTRIDW
#define COPYLPTSTRIDOPT             COPYLPSTRIDOPTW
#define COPYLPTSTROPT               COPYLPSTROPTW
#define FIRSTCOPYLPTSTR             COPYLPSTRW
#define FIRSTCOPYLPTSTRID           COPYLPSTRIDW
#define FIRSTCOPYLPTSTRIDOPT        COPYLPSTRIDOPTW
#define CLEANUPLPTSTR               CLEANUPLPSTRW
#endif

/* Use this macro if you don't need to access shared memory. */
#define BEGINCALL()             \
    {                           \
    ULONG_PTR retval;           \
    {

#define BEGINCALLVOID()         \
    {

#define ERRORTRAP(error)        \
       goto cleanup;            \
    }                           \
    goto errorexit;             \
errorexit:                      \
    retval = (ULONG_PTR)error;  \
cleanup:

#define ERRORTRAPVOID()         \
    goto errorexit;             \
errorexit:

#define ENDCALL(type)           \
    return (type)retval;        \
    }

#define ENDCALLVOID()           \
    return;                     \
    } 



/// 
/// < Win32k/User32 Structs & Typedefs >
/// 

/* Typedefs used for capturing string arguments to be passed to the kernel. */
typedef struct _IN_STRING {
    UNICODE_STRING strCapture;
    PUNICODE_STRING pstr;
    BOOL fAllocated;
} IN_STRING, *PIN_STRING;

typedef LRESULT(CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);


//
// Win32k Routines
// 

typedef HHOOK( NTAPI *tNtUserSetWindowsHookEx )(
    IN HINSTANCE Mod,
    IN PUNICODE_STRING UnsafeModuleName,
    IN DWORD ThreadId,
    IN int HookId,
    IN HOOKPROC HookProc,
    IN BOOL Ansi
    );

typedef BOOL( NTAPI *tNtUserUnhookWindowsHookEx )(
    IN HHOOK Hook
    );

typedef LRESULT( NTAPI *tNtUserCallNextHookEx )(
    IN int Code,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN BOOL Ansi
    );

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus