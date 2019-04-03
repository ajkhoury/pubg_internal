/**
 * User-mode Logger
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * Copyright (c) 2015-2016, tandasat. All rights reserved.
 * Use of this source code is governed by a MIT-style license:
 *
 * The MIT License ( MIT )
 *
 * Copyright (c) 2016 Satoshi Tanda
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files ( the "Software" ), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file log.c
 * @authors Aidan Khoury (dude719)
 *          Satoshi Tanda (tandasat)
 * @date 8/30/2018
 */

#if defined(ENABLE_LOG)

#include "log.h"
#include "eresource.h"

#include <stdio.h>
#include <stdlib.h>

#include <processenv.h>
#include <processthreadsapi.h>
#include <consoleapi.h>
#include <debugapi.h>
#include <sysinfoapi.h>

// A size for log buffer in NonPagedPool. Two buffers are allocated with this
// size. Exceeded logs are ignored silently. Make it bigger if a buffered log
// size often reach this size.
#define LOG_BUFFER_SIZE_IN_PAGES    (64UL)
// An actual log buffer size in bytes.
#define LOG_BUFFER_SIZE             (LOG_BUFFER_SIZE_IN_PAGES << 12)
// A size that is usable for logging. Minus one because the last byte is kept for \0.
#define LOG_BUFFER_USABLE_SIZE      (LOG_BUFFER_SIZE - 1)
// An interval in milliseconds to flush buffered log entries into a log file.
#define LOG_FLUSH_INTERVAL          (50)
#define LOG_TICKS_PER_NS            ((LONGLONG)1 * 10)
#define LOG_TICKS_PER_MS            (LOG_TICKS_PER_NS * 1000)

typedef struct _LOG_BUFFER_INFO {
    // A pointer to buffer currently used.
    // It is either LogBuffer1 or LogBuffer2.
    volatile char *LogBufferHead;
    // A pointer to where the next log should be written.
    volatile char *LogBufferTail;
    char *LogBuffer1;
    char *LogBuffer2;
    // Holds the biggest buffer usage to determine a necessary buffer size.
    SIZE_T LogMaxUsage;
    HANDLE LogFileHandle;
    RTL_CRITICAL_SECTION Lock;
    EXECUTIVE_RESOURCE Resource;
    BOOLEAN ResourceInitialized;
    HANDLE Stdout, StdoutOld;
    HANDLE Stderr, StderrOld;
    HANDLE Stdin, StdinOld;
    volatile BOOLEAN BufferFlushThreadShouldBeAlive;
    volatile BOOLEAN BufferFlushThreadStarted;
    HANDLE BufferFlushThreadHandle;
    wchar_t LogFilePath[MAX_PATH + 4];
} LOG_BUFFER_INFO, *PLOG_BUFFER_INFO;

static DWORD g_LogFlags = LogPutLevelDisable;
static LOG_BUFFER_INFO g_LogBufferInfo = { 0 };
static char const *s_LogLevelStrings[4] = { "DBG ", "INF ", "WRN ", "ERR " };


// Determines if a specified file path exists.
static
BOOLEAN
LogpFileExists(
    IN PUNICODE_STRING FilePath
)
{
    NTSTATUS Status;
    HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes, FilePath, 0, NULL, NULL);

    Status = NtCreateFile(&FileHandle,
                          FILE_READ_DATA | SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatus,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                          );
    if (NT_SUCCESS(Status)) {
        NtClose(FileHandle);
        return TRUE;
    }

    return FALSE;
}

// Returns true when a log file is enabled.
inline
BOOLEAN
LogpIsLogFileEnabled(
    IN const LOG_BUFFER_INFO *Info
)
{
    if (Info->LogBuffer1 != NULL) {
        //assert(Info->LogBuffer2 != NULL);
        //assert(Info->LogBufferHead != NULL);
        //assert(Info->LogBufferTail != NULL);
        return TRUE;
    }
    //assert(!Info->LogBuffer2);
    //assert(!Info->LogBufferHead);
    //assert(!Info->LogBufferTail);
    return FALSE;
}

// Returns true when a log file is opened.
static
BOOLEAN
LogpIsLogFileActivated(
    IN const LOG_BUFFER_INFO *Info
)
{
    if (Info->BufferFlushThreadShouldBeAlive) {
        //assert(Info->BufferFlushThreadHandle != NULL);
        //assert(Info->LogFileHandle != NULL);
        return TRUE;
    }
    //assert(!Info->BufferFlushThreadHandle);
    //assert(!Info->LogFileHandle);
    return FALSE;
}

// Tests if the printed bit is on
inline
BOOLEAN
LogpIsPrinted(
    IN CHAR *Message // NOLINT
)
{
    return (BOOLEAN)((Message[0] & 0x80) != 0);
}

// Marks the message as it is already printed out, or clears the printed bit and
// restores it to the original
inline
VOID
LogpSetPrintedBit(
    IN CHAR *Message,
    IN BOOLEAN On
)
{
    if (On)
        Message[0] |= 0x80;
    else
        Message[0] &= 0x7F;
}

// Calls WriteConsoleA while converting \r\n to \n\0
static
VOID
LogpDoDbgPrint(
    IN HANDLE ConsoleOutputHandle,
    IN CHAR *Message
)
{
    size_t MessageLen = strlen(Message);
    size_t LocationOfCr = MessageLen - 2;
    Message[LocationOfCr] = '\n';
    Message[LocationOfCr + 1] = '\0';

    WriteConsoleA(ConsoleOutputHandle, Message, (DWORD)MessageLen - 1, NULL, NULL);
    //fputs(Message, stderr);
}

VOID
LogDoDbgPrint(
    IN const char *Format,
    ...
)
{
    va_list Args;
    char Message[512];
    va_start(Args, Format);
    DWORD MessageLen = vsnprintf(Message, RTL_NUMBER_OF(Message), Format, Args);
    va_end(Args);
    WriteConsoleA(g_LogBufferInfo.Stdout, Message, MessageLen, NULL, NULL);
}

// Switches the current log buffer, saves the contents of old buffer to the log
// file, and prints them out as necessary. This function does not flush the log
// file, so code should call LogpWriteMessageToFile() or ZwFlushBuffersFile() later.
static
NTSTATUS
LogpFlushLogBuffer(
    IN OUT PLOG_BUFFER_INFO Info
)
{
    NTSTATUS Status = 0;
    IO_STATUS_BLOCK IoStatus;
    CHAR *OldLogBuffer;
    CHAR *CurrentLogEntry;
    ULONG CurrentLogEntryLength;
    BOOLEAN PrintedOut;

    //
    // Enter a critical section and acquire a reader lock for info in order to
    // write a log file safely.
    //
    AcquireResourceExclusiveLite(&Info->Resource, TRUE);

    //
    // Acquire a spin lock for Info.LogBuffer(s) in order
    // to switch its head safely.
    //
    RtlEnterCriticalSection(&Info->Lock);

    OldLogBuffer = (CHAR *)Info->LogBufferHead;
    Info->LogBufferHead = (OldLogBuffer == Info->LogBuffer1)
        ? Info->LogBuffer2
        : Info->LogBuffer1;
    Info->LogBufferHead[0] = '\0';
    Info->LogBufferTail = Info->LogBufferHead;

    RtlLeaveCriticalSection(&Info->Lock);

    //
    // Write all log entries in old log buffer.
    //
    for (CurrentLogEntry = OldLogBuffer; *CurrentLogEntry; /**/) {

        //
        // Check the printed bit and clear it
        //
        PrintedOut = LogpIsPrinted(CurrentLogEntry);
        LogpSetPrintedBit(CurrentLogEntry, FALSE);

        CurrentLogEntryLength = (ULONG)strlen(CurrentLogEntry);

        Status = NtWriteFile(Info->LogFileHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatus,
                             CurrentLogEntry,
                             CurrentLogEntryLength,
                             NULL,
                             NULL
                             );

        if (!NT_SUCCESS(Status)) {

            //
            // It could happen when you did not register IRP_SHUTDOWN and call
            // LogIrpShutdownHandler and the system tried to log to a file after
            // a file system was unmounted.
            //
            __debugbreak();
        }

        //
        // Print it out if requested, and the message is not already printed out.
        //
        if (!PrintedOut) {
            LogpDoDbgPrint(Info->Stdout, CurrentLogEntry);
        }

        CurrentLogEntry += CurrentLogEntryLength + 1;
    }

    OldLogBuffer[0] = '\0';
    ReleaseResourceLite(&Info->Resource);

    return Status;
}

// A thread runs as long as Info.BufferFlushThreadShouldBeAlive is true and
// flushes a log buffer to a log file every kLogpLogFlushIntervalMsec msec.
static
NTSTATUS
NTAPI
LogpBufferFlushThreadRoutine(
    IN PVOID Parameter
)
{
    NTSTATUS Status = 0;
    PLOG_BUFFER_INFO Info = (PLOG_BUFFER_INFO)Parameter;

    Info->BufferFlushThreadStarted = TRUE;

    while (Info->BufferFlushThreadShouldBeAlive) {
        //assert(LogpIsLogFileActivated(Info));

        if (Info->LogBufferHead[0]) {
            Status = LogpFlushLogBuffer(Info);

            //
            // Do not flush the file for overall performance. Even a case of bug check,
            // we should be able to recover logs by looking at both log buffers!
            //
        }

        //
        // Delay the current thread's execution for LOG_FLUSH_INTERVAL milliseconds.
        //
        LARGE_INTEGER WaitInterval;
        WaitInterval.QuadPart = -LOG_FLUSH_INTERVAL * LOG_TICKS_PER_MS;
        NtDelayExecution(FALSE, &WaitInterval);
    }

    return Status;
}

// Initializes a log file and startes a log buffer thread.
static
int
LogpInitializeLogFile(
    IN OUT PLOG_BUFFER_INFO Info
)
{
    NTSTATUS Status;
    UNICODE_STRING LogFilePath;
    OBJECT_ATTRIBUTES Attributes;
    IO_STATUS_BLOCK IoStatus;

    //
    // Check if the log file has already been initialized.
    //
    if (Info->LogFileHandle != NULL) {
        return ERROR_SUCCESS;
    }

    //
    // Initialize a log file path.
    //
    LogFilePath.Length = (USHORT)(wcslen(Info->LogFilePath) * sizeof(WCHAR));
    LogFilePath.MaximumLength = LogFilePath.Length;
    LogFilePath.Buffer = (PWSTR)Info->LogFilePath;
    InitializeObjectAttributes(&Attributes, &LogFilePath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    //
    // Check if the file already exists.
    //
    if (g_LogFlags & LogOptDisableAppend) {
        if (LogpFileExists(&LogFilePath)) {
            Status = NtDeleteFile(&Attributes);
            if (!NT_SUCCESS(Status)) {
                goto Exit;
            }
        }
    }

    //
    // Create the file handle.
    //
    Status = NtCreateFile(&Info->LogFileHandle,
                          (ULONG)((g_LogFlags & LogOptDisableAppend) ?
                                  FILE_WRITE_DATA : FILE_APPEND_DATA) | SYNCHRONIZE,
                          &Attributes,
                          &IoStatus,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
                          NULL,
                          0
                          );

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    //
    // Initialize a log buffer flush thread.
    //
    Info->BufferFlushThreadShouldBeAlive = TRUE;
    Status = NtCreateThreadEx(&Info->BufferFlushThreadHandle,
                              THREAD_ALL_ACCESS,
                              NULL,
                              NtCurrentProcess(),
                              LogpBufferFlushThreadRoutine,
                              Info,
                              0,
                              0,
                              0x1000,
                              0x100000,
                              NULL
                              );

    if (!NT_SUCCESS(Status)) {
        NtClose(Info->LogFileHandle);
        Info->LogFileHandle = NULL;
        Info->BufferFlushThreadShouldBeAlive = FALSE;
        goto Exit;
    }

    //
    // Wait until the thead has started
    //
    // ACTUALLY DONT!!
    //
    // BECAUSE WHEN WE QUEUE AN APC SUPER EARLY THIS WAIT WILL NEVER
    // BE SATISFIED - SO WE JUST GOTTA HAVE  FAITH THAT THIS STUPID
    // FUCKING THREAD WILL START AFTER THE APC HAS COMPLETED!
    //

    //LARGE_INTEGER WaitInterval;
    //WaitInterval.QuadPart = -LOG_FLUSH_INTERVAL * LOG_TICKS_PER_MS;
    //NtWaitForSingleObject(Info->BufferFlushThreadHandle, TRUE, &WaitInterval);
    //while (!Info->BufferFlushThreadStarted) {
    //    NtDelayExecution(TRUE, &WaitInterval);
    //}
    //WriteConsoleA(Info->Stdout, "Fucker fuck", sizeof("Fucker fuck"), NULL, NULL);

Exit:
    return RtlNtStatusToDosError(Status);
}

// Terminates a log file related code.
static
VOID
LogpFinalizeBufferInfo(
    IN PLOG_BUFFER_INFO Info
)
{
    // Closing the log buffer flush thread.
    if (Info->BufferFlushThreadHandle) {
        Info->BufferFlushThreadShouldBeAlive = FALSE;
        NtWaitForSingleObject(Info->BufferFlushThreadHandle, FALSE, NULL);

        NtClose(Info->BufferFlushThreadHandle);
        Info->BufferFlushThreadHandle = NULL;
    }

    // Clean up other things.
    if (Info->LogFileHandle) {
        NtClose(Info->LogFileHandle);
        Info->LogFileHandle = NULL;
    }

    // Detach and free attached console.
    if (Info->Stdout) {

        FreeConsole();

        if (Info->StdoutOld) {
            SetStdHandle(STD_OUTPUT_HANDLE, Info->StdoutOld);
        }
        if (Info->StderrOld) {
            SetStdHandle(STD_ERROR_HANDLE, Info->StderrOld);
        }
        if (Info->StdinOld) {
            SetStdHandle(STD_INPUT_HANDLE, Info->StdinOld);
        }
    }

    if (Info->LogBuffer1) {
        free(Info->LogBuffer1);
        Info->LogBuffer1 = NULL;
    }

    if (Info->ResourceInitialized) {
        DeleteResourceLite(&Info->Resource);
        Info->ResourceInitialized = FALSE;
    }
}

// Initialize a log file related code such as a flushing thread.
static
int
LogpInitializeBufferInfo(
    IN CONST WCHAR *LogFilePath,
    IN OUT PLOG_BUFFER_INFO Info
)
{
    int rc;

    if (!LogFilePath || !Info) {
        return ERROR_INVALID_PARAMETER;
    }

    RtlInitializeCriticalSection(&Info->Lock);

    rc = wcscpy_s(Info->LogFilePath, RTL_NUMBER_OF_FIELD(LOG_BUFFER_INFO, LogFilePath), L"\\??\\");
    if (rc) {
        return rc;
    }

    rc = wcscat_s(Info->LogFilePath, RTL_NUMBER_OF_FIELD(LOG_BUFFER_INFO, LogFilePath), LogFilePath);
    if (rc) {
        return rc;
    }

    //
    // Initialize the resource
    //
    rc = InitializeResourceLite(&Info->Resource);
    if (rc) {
        return rc;
    }

    //
    // Allocate two log buffers as NonPagedPools.
    //
    Info->LogBuffer1 = (CHAR *)malloc(LOG_BUFFER_SIZE * 2);
    if (!Info->LogBuffer1) {
        LogpFinalizeBufferInfo(Info);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    Info->LogBuffer2 = Info->LogBuffer1 + LOG_BUFFER_SIZE;

    //
    // Initialize these buffers
    //
    RtlFillMemory(Info->LogBuffer1, LOG_BUFFER_SIZE, 0xFF);  // for diagnostic
    Info->LogBuffer1[0] = '\0';
    Info->LogBuffer1[LOG_BUFFER_SIZE - 1] = '\0';  // at the end

    RtlFillMemory(Info->LogBuffer2, LOG_BUFFER_SIZE, 0xFF);  // for diagnostic
    Info->LogBuffer2[0] = '\0';
    Info->LogBuffer2[LOG_BUFFER_SIZE - 1] = '\0';  // at the end

    //
    // Buffer should be used is LogBuffer1, and location should be written
    // logs is the head of the buffer.
    //
    Info->LogBufferHead = Info->LogBuffer1;
    Info->LogBufferTail = Info->LogBuffer1;

    //
    // Set the stdout, stderr, and stdin modes.
    //
    SetConsoleMode(Info->Stdout, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    SetConsoleMode(Info->Stdin, ENABLE_INSERT_MODE |
                   ENABLE_EXTENDED_FLAGS |
                   ENABLE_PROCESSED_INPUT |
                   ENABLE_QUICK_EDIT_MODE);

    //
    // Initialize the log file.
    //
    rc = LogpInitializeLogFile(Info);
    if (rc) {
        LogpFinalizeBufferInfo(Info);
    }

    return rc;
}


int
LOGAPI
LogInitialize(
    unsigned int flags,
    const wchar_t* filePath
)
{
    int rc = NOERROR;

    g_LogFlags = flags;
    RtlZeroMemory(&g_LogBufferInfo, sizeof(LOG_BUFFER_INFO));

    //
    // Create console instance.
    //
    AllocConsole();

    //AttachConsole((DWORD)(ULONG_PTR)NtCurrentProcessId());

    //
    // Get the stdout, stderr, and stdin handles.
    //
    g_LogBufferInfo.Stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    g_LogBufferInfo.Stderr = GetStdHandle(STD_ERROR_HANDLE);
    g_LogBufferInfo.Stdin = GetStdHandle(STD_INPUT_HANDLE);
    g_LogBufferInfo.StdoutOld = g_LogBufferInfo.Stdout;
    g_LogBufferInfo.StderrOld = g_LogBufferInfo.Stderr;
    g_LogBufferInfo.StdinOld = g_LogBufferInfo.Stdin;

    //
    // Initialize a log file if a log file path is specified.
    //
    if (filePath != NULL) {
        rc = LogpInitializeBufferInfo(filePath, &g_LogBufferInfo);
    }

#if defined(_DEBUG)
    if (rc == NOERROR) {
        // Test the log.
        rc = LOG_INFO("Log has been initialized.");
        if (rc) {
            if (filePath) {
                LogpFinalizeBufferInfo(&g_LogBufferInfo);
            }
        }
    }
#endif // _DEBUG

    return rc;
}

// Destroys the log functions.
void
LOGAPI
LogDestroy(
    void
)
{
#if defined(DBG)
    LOG_DEBUG("Finalizing... (Max log usage = %08x bytes)", g_LogBufferInfo.LogMaxUsage);
#endif

    g_LogFlags = LogPutLevelDisable;
    LogpFinalizeBufferInfo(&g_LogBufferInfo);
}

// Returns true when logging is necessary according to the log's severity and
// a set log level.
static
BOOLEAN
LogpIsLogNeeded(
    IN ULONG Level
)
{
    return (BOOLEAN)((g_LogFlags & Level) != 0);
}

// Returns the function's base name, for example,
// NamespaceName::ClassName::MethodName will be returned as MethodName.
static
const char*
LogpFindBaseFunctionName(
    IN const char *FunctionName
)
{
    const char* p;
    const char* BaseFunctionName;

    if (!FunctionName)
        return NULL;

    p = FunctionName;
    BaseFunctionName = FunctionName;

    while (*(p++)) {
        if (*p == ':')
            BaseFunctionName = p + 1;
    }

    return BaseFunctionName;
}

// Concatenates meta information such as the current time and a process ID to
// user given log message.
static
int
LogpMakePrefix(
    IN ULONG Level,
    IN const char *FunctionName,
    IN const char *LogMessage,
    OUT char *LogBuffer,
    IN size_t LogBufferLength
)
{
    int rc;
    ULONG LogLevelIndex;
    SYSTEMTIME LocalTime;
    ULONG CurrentProcessorNumber;
    char TimeBuffer[20];
    char FunctionNameBuffer[50];
    char ProcessorNumber[10];
    const char *BaseFunctionName;

    //
    // Get the log level prefix index.
    //
    if (!_BitScanForward((unsigned long*)&LogLevelIndex, (Level >> 4))) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Want the current time.
    //
    if (!(g_LogFlags & LogOptDisableTime)) {
        GetLocalTime(&LocalTime);
        rc = snprintf(TimeBuffer,
                      RTL_NUMBER_OF(TimeBuffer),
                      "%02u:%02u:%02u.%03u  ",
                      LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, LocalTime.wMilliseconds);
        if (rc < 0)
            return rc;
    } else {
        TimeBuffer[0] = '\0';
    }

    //
    // Want the function name.
    //
    if (!(g_LogFlags & LogOptDisableFunctionName)) {
        BaseFunctionName = LogpFindBaseFunctionName(FunctionName);
        if (!BaseFunctionName)
            goto EmptyFunctionName;

        rc = snprintf(FunctionNameBuffer, RTL_NUMBER_OF(FunctionNameBuffer), "%-40s  ", BaseFunctionName);
        if (rc < 0)
            return rc;
    } else {
    EmptyFunctionName:;
        FunctionNameBuffer[0] = '\0';
    }

    //
    // Want the processor number.
    //
    if (!(g_LogFlags & LogOptDisableProcessorNumber)) {
        CurrentProcessorNumber = GetCurrentProcessorNumber();
        rc = snprintf(ProcessorNumber,
                      RTL_NUMBER_OF(ProcessorNumber),
                      (CurrentProcessorNumber >= 10) ? "#%lu" : "#%lu ",
                      CurrentProcessorNumber);
        if (rc < 0)
            return rc;
    } else {
        *((USHORT*)ProcessorNumber) = 0x2020; // "  "
        ProcessorNumber[2] = '\0';
    }

    //
    // Format the prefix.
    //
    rc = snprintf(LogBuffer,
                  LogBufferLength,
                  "%s%s%s%6Iu  %s%s\r\n",
                  TimeBuffer, s_LogLevelStrings[LogLevelIndex],
                  ProcessorNumber, (ULONG_PTR)GetCurrentThreadId(),
                  FunctionNameBuffer, LogMessage);

    return rc;
}

// Logs the current log entry to and flush the log file.
static
NTSTATUS
LogpWriteMessageToFile(
    IN const char *Message,
    IN const LOG_BUFFER_INFO *Info
)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    Status = NtWriteFile(Info->LogFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         (PVOID)Message,
                         (ULONG)strlen(Message),
                         NULL,
                         NULL
                         );

    if (!NT_SUCCESS(Status)) {

        //
        // It could happen when you did not register IRP_SHUTDOWN and call
        // LogIrpShutdownHandler and the system tried to log to a file
        // after a file system was unmounted.
        //
        DebugBreak();
    }

    Status = NtFlushBuffersFile(Info->LogFileHandle, &IoStatusBlock);

    return Status;
}

// Buffer the log entry to the log buffer.
static
NTSTATUS
LogpBufferMessage(
    IN const char *Message,
    IN OUT PLOG_BUFFER_INFO Info
)
{
    int rc;
    SIZE_T UsedBufferSize;
    SIZE_T MessageLength;

    //
    // Acquire a spin lock to add the log safely.
    //
    RtlEnterCriticalSection(&Info->Lock);

    //
    // Copy the current log to the buffer.
    //
    UsedBufferSize = (SIZE_T)(Info->LogBufferTail - Info->LogBufferHead);
    rc = snprintf((char*)Info->LogBufferTail, LOG_BUFFER_USABLE_SIZE - UsedBufferSize, Message);
    if (rc < 0) {
        goto Exit;
    }

    //
    // Update Info->LogMaxUsage if necessary.
    //
    if ((SIZE_T)rc > LOG_BUFFER_USABLE_SIZE - UsedBufferSize - 1) {

        Info->LogMaxUsage = LOG_BUFFER_SIZE;  // Indicates overflow

    } else {

        if (rc > 0) {
            MessageLength = (SIZE_T)(strlen(Message) + 1);

            Info->LogBufferTail += MessageLength;
            UsedBufferSize += MessageLength;

            if (UsedBufferSize > Info->LogMaxUsage) {
                Info->LogMaxUsage = UsedBufferSize;  // Update
            }
        }
    }

    *Info->LogBufferTail = '\0';

Exit:
    //
    // Release the spin lock.
    //
    RtlLeaveCriticalSection(&Info->Lock);
    return rc;
}


// Logs the entry according to attribute and the thread condition.
static
int
LogpPut(
    IN char *Message,
    IN ULONG Attribute
)
{
    int rc = 0;
    BOOLEAN ShouldDbgPrint = (BOOLEAN)((Attribute & LogLevelOptSafe) == 0);

    //
    // Log the entry to a file or buffer.
    //
    if (LogpIsLogFileEnabled(&g_LogBufferInfo)) {

        //
        // Can it log it to a file now?
        //
        if ((Attribute & LogLevelOptSafe) == 0 && LogpIsLogFileActivated(&g_LogBufferInfo)) {

            //
            // Yes, it can! So lets do it.
            //
            LogpFlushLogBuffer(&g_LogBufferInfo);
            rc = LogpWriteMessageToFile(Message, &g_LogBufferInfo);

        } else {

            //
            // No, it cannot. Set the printed bit if needed, and then buffer it.
            //
            if (ShouldDbgPrint) {
                LogpSetPrintedBit(Message, TRUE);
            }

            rc = LogpBufferMessage(Message, &g_LogBufferInfo);
            LogpSetPrintedBit(Message, FALSE);
        }
    }

    //
    // Can it safely be printed?
    //
    if (ShouldDbgPrint) {
        LogpDoDbgPrint(g_LogBufferInfo.Stdout, Message);
    }

    return rc;
}


// Actual implementation of logging API.
int
LOGAPI
LogPrint(
    IN unsigned int Level,
    IN const char *FunctionName,
    IN const char *Format,
    ...
)
{
    int rc;
    va_list Args;

    size_t cchLength;
    size_t cchPrinted;
    size_t cchLengthToCopy;

    char *LogMessage;
    char LogMessageBuffer[411 + 1];
    char Message[512];

    // A single entry of a log should not exceed 512 bytes. See
    // Reading and Filtering Debugging Messages in MSDN for details.
    C_ASSERT(RTL_NUMBER_OF(Message) <= 512);

    if (!LogpIsLogNeeded(Level)) {
        return 0;
    }

    LogMessage = LogMessageBuffer;

    va_start(Args, Format);
    rc = vsnprintf(LogMessage, RTL_NUMBER_OF(LogMessageBuffer), Format, Args);
    va_end(Args);

    if (rc < 0) {
        #if defined(_DEBUG)
        DebugBreak();
        #endif
        return rc;
    }

    //
    // Handle buffer overflow.
    //
    if (rc > RTL_NUMBER_OF(LogMessageBuffer) - 1) {
        cchLength = rc;

        LogMessage = (char*)malloc(4096);
        if (!LogMessage) {
            #if defined(_DEBUG)
            DebugBreak();
            #endif
            return STATUS_NO_MEMORY;
        }

        va_start(Args, Format);
        rc = vsnprintf(LogMessage, 4096, Format, Args);
        va_end(Args);

        if (rc < 0) {
            #if defined(_DEBUG)
            DebugBreak();
            #endif
            return rc;
        }

        cchLength = rc;

    } else {

        if (LogMessage[0] == '\0') {
            #if defined(_DEBUG)
            DebugBreak();
            #endif
            return STATUS_INVALID_PARAMETER;
        }

        cchLength = 0;
    }

    if (cchLength) {

        cchPrinted = 0;
        do {
            cchLengthToCopy = min(cchLength - cchPrinted, sizeof(LogMessageBuffer) - sizeof(LogMessageBuffer[0]));
            RtlZeroMemory(&LogMessageBuffer, sizeof(LogMessageBuffer) - sizeof(LogMessageBuffer[0]));
            RtlCopyMemory(LogMessageBuffer, LogMessage + cchPrinted, cchLengthToCopy);

            if (cchPrinted == 0) {
                rc = LogpMakePrefix(Level & 0xF0, FunctionName, LogMessageBuffer, Message, RTL_NUMBER_OF(Message));
            } else {
                rc = snprintf(Message, RTL_NUMBER_OF(Message), "> %s\r\n", LogMessageBuffer);
            }

            if (rc < 0) {
                #if defined(_DEBUG)
                DebugBreak();
                #endif
                break;
            }

            cchPrinted += cchLengthToCopy;

            rc = LogpPut(Message, Level & 0x0F);
            if (rc < 0) {
                #if defined(_DEBUG)
                DebugBreak();
                #endif
                break;
            }

        } while (cchPrinted < cchLength);

        //
        // Free the previously allocated log message pool.
        //
        free(LogMessage);

    } else {

        //
        // No overflow occurred, we should be safe to print.
        //
        rc = LogpMakePrefix(Level & 0xF0, FunctionName, LogMessage, Message, RTL_NUMBER_OF(Message));
        if (rc < 0) {
            #if defined(_DEBUG)
            DebugBreak();
            #endif
            return rc;
        }

        rc = LogpPut(Message, Level & 0x0F);

        #if defined(_DEBUG)
        if (rc < 0) {
            DebugBreak();
        }
        #endif
    }

    return rc;
}

HANDLE
LogGetStdout(
    VOID
)
{
    return g_LogBufferInfo.Stdout;
}

HANDLE
LogGetStderr(
    VOID
)
{
    return g_LogBufferInfo.Stderr;
}

HANDLE
LogGetStdin(
    VOID
)
{
    return g_LogBufferInfo.Stdin;
}

#endif // ENABLE_LOG