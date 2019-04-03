#pragma once

#include "native.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Log calling convention
#define LOGAPI __stdcall

//
// Log Macro Utilities
//

/**
 * Logs a message as respective severity
 *  @param fmt - A format string
 *  @return  STATUS_SUCCESS on success
 *
 *  Debug prints or buffers a log message with information about current
 *  execution context such as time, PID and TID as respective severity.
 *  Here are some guide lines to decide which level is appropriate:
 *   @li DEBUG: info for only developers.
 *   @li INFO: info for all users.
 *   @li WARN: info may require some attention but does not prevent the program working properly.
 *   @li ERROR: info about issues may stop the program working properly.
 *
 *  A message should not exceede 512 bytes after all string construction is
 *  done; otherwise this macro fails to log and returns non STATUS_SUCCESS.
 */
#if defined(ENABLE_LOG)

#if defined(__GNUC__) || defined(__clang__)
#define LOG_DEBUG(fmt,...)  LogPrint(LogLevelDebug, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt,...)   LogPrint(LogLevelInfo,  __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt,...)   LogPrint(LogLevelWarn,  __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt,...)  LogPrint(LogLevelError, __FUNCTION__, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt,...)  LogPrint(LogLevelDebug, __FUNCTION__, fmt,   __VA_ARGS__)
#define LOG_INFO(fmt,...)   LogPrint(LogLevelInfo,  __FUNCTION__, fmt,   __VA_ARGS__)
#define LOG_WARN(fmt,...)   LogPrint(LogLevelWarn,  __FUNCTION__, fmt,   __VA_ARGS__)
#define LOG_ERROR(fmt,...)  LogPrint(LogLevelError, __FUNCTION__, fmt,   __VA_ARGS__)
#endif

#else
#define LOG_DEBUG(fmt,...)  ((void)fmt)
#define LOG_INFO(fmt,...)   ((void)fmt)
#define LOG_WARN(fmt,...)   ((void)fmt)
#define LOG_ERROR(fmt,...)  ((void)fmt)
#endif // ENABLE_LOG

/**
 * Buffers a message as respective severity
 *
 *  @param[in] fmt  A format string
 *
 *  @return 0 on success.
 *
 *  Buffers the log to buffer and neither calls DbgPrint() nor writes to a file.
 *  It is strongly recommended to use it when a status of a system is not
 *  expectable in order to avoid system instability.
 *  @see LOG_DEBUG
 */
#if defined(ENABLE_LOG)

#if defined(__GNUC__) || defined(__clang__)
#define LOG_DEBUG_SAFE(fmt, ...)  LogPrint(LogLevelDebug | LogLevelOptSafe, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_INFO_SAFE(fmt, ...)   LogPrint(LogLevelInfo  | LogLevelOptSafe, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_WARN_SAFE(fmt, ...)   LogPrint(LogLevelWarn  | LogLevelOptSafe, __FUNCTION__, fmt, ##__VA_ARGS__)
#define LOG_ERROR_SAFE(fmt, ...)  LogPrint(LogLevelError | LogLevelOptSafe, __FUNCTION__, fmt, ##__VA_ARGS__)
#else                             
#define LOG_DEBUG_SAFE(fmt, ...)  LogPrint(LogLevelDebug | LogLevelOptSafe, __FUNCTION__, fmt,   __VA_ARGS__)
#define LOG_INFO_SAFE(fmt, ...)   LogPrint(LogLevelInfo  | LogLevelOptSafe, __FUNCTION__, fmt,   __VA_ARGS__)
#define LOG_WARN_SAFE(fmt, ...)   LogPrint(LogLevelWarn  | LogLevelOptSafe, __FUNCTION__, fmt,   __VA_ARGS__)
#define LOG_ERROR_SAFE(fmt, ...)  LogPrint(LogLevelError | LogLevelOptSafe, __FUNCTION__, fmt,   __VA_ARGS__)
#endif

#else
#define LOG_DEBUG_SAFE(fmt,...)     ((void)fmt)
#define LOG_INFO_SAFE(fmt,...)      ((void)fmt)
#define LOG_WARN_SAFE(fmt,...)      ((void)fmt)
#define LOG_ERROR_SAFE(fmt,...)     ((void)fmt)
#endif // ENABLE_LOG


typedef enum _LOG_LEVEL_OPTIONS {
    // Save this log to buffer and not try to write to a log file.
    LogLevelOptSafe = 0x1ul,

    LogLevelDebug = 0x10ul,
    LogLevelInfo = 0x20ul,
    LogLevelWarn = 0x40ul,
    LogLevelError = 0x80ul,

    // For LogInitialize(). Enables all levels of logs
    LogPutLevelDebug = LogLevelError | LogLevelWarn | LogLevelInfo | LogLevelDebug,

    // For LogInitialize(). Enables ERROR, WARN and INFO levels of logs
    LogPutLevelInfo = LogLevelError | LogLevelWarn | LogLevelInfo,

    // For LogInitialize(). Enables ERROR and WARN levels of logs
    LogPutLevelWarn = LogLevelError | LogLevelWarn,

    // For LogInitialize(). Enables an ERROR level of logs
    LogPutLevelError = LogLevelError,

    // For LogInitialize(). Disables all levels of logs
    LogPutLevelDisable = 0x00ul,

    // For LogInitialize(). Do not log a current time.
    LogOptDisableTime = 0x100ul,

    // For LogInitialize(). Do not log a current function name.
    LogOptDisableFunctionName = 0x200ul,

    // For LogInitialize(). Do not log a current processor number.
    LogOptDisableProcessorNumber = 0x400ul,

    // For LogInitialize(). Do not append to log file.
    LogOptDisableAppend = 0x800ul
} LOG_LEVEL_OPTIONS;


#if defined(ENABLE_LOG)

/**
 * Initializes the log system.
 *
 * Allocates internal log buffers, initializes related resources, starts a
 * log flush thread and creates a log file if requested. This function returns
 * STATUS_REINITIALIZATION_NEEDED if a file-syetem is not initialized yet. In
 * that case, a driver must call LogRegisterReinitialization() for completing
 * initialization.
 *
 * @param[in] flags    A OR-ed flag to control a log level and options
 * @param[in] filePath A log file path
 *
 * @return 0 on success, or non-0 on failure.
 */
int
LOGAPI
LogInitialize(
    unsigned int flags,
    const wchar_t* filePath
    );

/**
 * Destroys the log system. Should be called from OnUnload routine.
 */
void
LOGAPI
LogDestroy(
    void
    );

/**
 * Logs a message; use LOG_*() macros instead.
 *
 * @param LogLevel      Severity of a message.
 * @param FunctionName  A name of a function called this function.
 * @param Fmt           A format string.
 *
 * @return STATUS_SUCCESS on success.
 *
 * @see LOG_DEBUG
 * @see LOG_DEBUG_SAFE
 */
int
LOGAPI
LogPrint(
    IN unsigned int LogLevel,
    IN const char *FunctionName,
    IN const char *Fmt,
    ...
    );

VOID
LogDoDbgPrint(
    IN const char *Format,
    ...
    );
#if defined(__GNUC__) || defined(__clang__)
#define LOG_DBG_PRINT(fmt, ...)  LogDoDbgPrint(fmt, ##__VA_ARGS__)
#else                             
#define LOG_DBG_PRINT(fmt, ...)  LogDoDbgPrint(fmt, __VA_ARGS__)
#endif

#else

#define LogInitialize(Flag,LogFilePath) \
    STATUS_SUCCESS; \
    (void)(Flag); \
    (void)(LogFilePath)

#define LogDestroy() ((void)0)

#define LOG_DBG_PRINT(fmt, ...)     ((void)fmt)

#endif // ENABLE_LOG

#if defined(__cplusplus)
} // extern "C"
#endif
