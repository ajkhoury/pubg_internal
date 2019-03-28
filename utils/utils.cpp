#include "utils.h"

#include <sstream>

namespace utils {

WCHAR*
UTLAPI
WcsStr(
    IN const WCHAR *String1,
    IN const WCHAR *String2,
    IN BOOLEAN CaseInsensitive
)
{
    WCHAR *p1, *p2;
    WCHAR *cp = const_cast<WCHAR *>(String1); // cast away const!

    if (CaseInsensitive) {

        while (*cp) {
            for (p1 = cp, p2 = const_cast<WCHAR *>(String2); // cast away const!
                 *p1 && *p2 && towlower(*p1) == towlower(*p2);
                 ++p1, ++p2) {
            }

            if (!*p2) {
                return cp;
            }

            ++cp;
        }

    } else {

        while (*cp) {
            p1 = cp;
            p2 = const_cast<WCHAR *>(String2); // cast away const!

            while (*p1 && *p2 && !(*p1 - *p2)) {
                p1++, p2++;
            }

            if (!*p2) {
                return (cp);
            }

            ++cp;
        }
    }

    return NULL;
}


std::string
UTLAPI
wstring_to_string(
    const std::wstring& wstr
)
{
    int size = WideCharToMultiByte(CP_UTF8,
                                   WC_ERR_INVALID_CHARS,
                                   wstr.data(),
                                   static_cast<int>(wstr.size()),
                                   NULL,
                                   0,
                                   NULL,
                                   NULL
                                   );
    if (size != 0) {

        std::string str(size, 0);
        if (WideCharToMultiByte(CP_UTF8,
                                WC_ERR_INVALID_CHARS,
                                wstr.data(),
                                static_cast<int>(wstr.size()),
                                const_cast<LPSTR>(str.data()),
                                size,
                                NULL,
                                NULL) != 0) {
            return str;
        }
    }

    return std::string();
}

std::vector<std::string> SplitString(const std::string& Str, char Delimiter)
{
    std::vector<std::string> Tokens;
    std::string Token;
    std::istringstream TokenStream(Str);
    while (std::getline(TokenStream, Token, Delimiter)) {
        Tokens.push_back(Token);
    }
    return Tokens;
}

std::string FormatBuffer(const uint8_t* Buffer, size_t Length)
{
    std::string OutString;
    char Temp[512];
    char HexChars[16];//= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    *(uint64_t*)&HexChars[0] = 0x5E5F5C5D5A5B5859 ^ 0x6969696969696969; // 0x3736353433323130
    *(uint64_t*)&HexChars[8] = 0x0441640146617B3C ^ 0x4204204204204204; // 0x4645444342413938
    for (size_t i = 0; i < Length; i++) {
        if (i == 0) {
            sprintf_s(Temp, sizeof(Temp), "%p    ", Buffer + i);
            OutString += Temp;
        } else if ((i % 16) == 0) {
            sprintf_s(Temp, sizeof(Temp), "\n%p    ", Buffer + i);
            OutString += Temp;
        }
        Temp[0] = HexChars[(Buffer[i] >> 4) & 0xF];
        Temp[1] = HexChars[(Buffer[i] >> 0) & 0xF];
        *(uint16_t*)&Temp[2] = 0x0020;
        OutString += Temp;
    }
    return OutString;
}

const UINT8*
UTLAPI
FindPattern(
    IN const void *SearchBase,
    IN SIZE_T SearchSize,
    IN const UINT8 Wildcard,
    IN const UINT8 *Pattern,
    IN SIZE_T PatternSize
)
{
    const UINT8 LastByte = Pattern[PatternSize - 1];
    const UINT8 *Search = (const UINT8 *)SearchBase;
    const UINT8 *SearchEnd = Search + SearchSize - PatternSize;

    while (Search < SearchEnd) {
        if (Search[PatternSize - 1] == LastByte &&
            MemCmp((const void*)Search, Wildcard, (const void*)Pattern, PatternSize) == 0) {
            return Search;
        }
        ++Search;
    }

    return NULL;
}

#define __IN_RANGE__(x,a,b) (x >= a && x <= b) 
#define __GET_NIBBLE__(x)   ((x == (UINT8)'\?') ? 0 : \
                                (__IN_RANGE__(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa)))
#define __GET_BYTE__(x)     (__GET_NIBBLE__(x[0]) << 4 | __GET_NIBBLE__(x[1]))

const UINT8*
UTLAPI
FindPatternIDA(
    IN const void *SearchBase,
    IN SIZE_T SearchSize,
    IN const char *InPattern
)
{
    const UINT8 *Pattern = (const UINT8 *)InPattern;
    const UINT8 *RangeStart = (const UINT8 *)SearchBase;
    const UINT8 *RangeEnd = RangeStart + SearchSize;
    const UINT8 *FirstMatch = NULL;

    for (const UINT8 *Current = RangeStart; Current < RangeEnd; ++Current) {

        if (Pattern[0] == (UINT8)'\?') {

            if (Pattern[1] == (UINT8)'\?') {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (!*Pattern)
                    return FirstMatch;

            } else if ((*Current & 0x0F) == __GET_BYTE__(Pattern)) {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (!*Pattern)
                    return FirstMatch;

            } else {

                if (FirstMatch) {
                    Current = FirstMatch;
                    Pattern = (const UINT8 *)InPattern;
                    FirstMatch = NULL;
                }
            }

        } else if (Pattern[1] == (UINT8)'\?') {

            if ((*Current & 0xF0) == __GET_BYTE__(Pattern)) {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (!*Pattern)
                    return FirstMatch;

            } else {

                if (FirstMatch) {
                    Current = FirstMatch;
                    Pattern = (const UINT8 *)InPattern;
                    FirstMatch = NULL;
                }
            }

        } else {

            if (*Current == __GET_BYTE__(Pattern)) {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (!*Pattern)
                    return FirstMatch;

            } else {

                if (FirstMatch) {
                    Current = FirstMatch;
                    Pattern = (const UINT8 *)InPattern;
                    FirstMatch = NULL;
                }
            }
        }
    }

    return NULL;
}

const UINT8*
UTLAPI
FindPatternIDA(
    IN const void *SearchBase,
    IN SIZE_T SearchSize,
    IN const std::string& InPattern
)
{
    const UINT8 *PatternStart = (const UINT8 *)InPattern.data();
    const UINT8 *PatternEnd = PatternStart + InPattern.size();
    const UINT8 *Pattern = PatternStart;
    const UINT8 *RangeStart = (const UINT8 *)SearchBase;
    const UINT8 *RangeEnd = RangeStart + SearchSize;
    const UINT8 *FirstMatch = NULL;

    for (const UINT8 *Current = RangeStart; Current < RangeEnd; ++Current) {

        if (Pattern[0] == (UINT8)'\?') {

            if (Pattern[1] == (UINT8)'\?') {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (Pattern >= PatternEnd)
                    return FirstMatch;

            } else if ((*Current & 0x0F) == __GET_BYTE__(Pattern)) {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (Pattern >= PatternEnd)
                    return FirstMatch;

            } else {

                if (FirstMatch) {
                    Current = FirstMatch;
                    Pattern = PatternStart;
                    FirstMatch = NULL;
                }
            }

        } else if (Pattern[1] == (UINT8)'\?') {

            if ((*Current & 0xF0) == __GET_BYTE__(Pattern)) {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (Pattern >= PatternEnd)
                    return FirstMatch;

            } else {

                if (FirstMatch) {
                    Current = FirstMatch;
                    Pattern = PatternStart;
                    FirstMatch = NULL;
                }
            }

        } else {

            if (*Current == __GET_BYTE__(Pattern)) {

                if (!FirstMatch)
                    FirstMatch = Current;
                Pattern += 3;
                if (Pattern >= PatternEnd)
                    return FirstMatch;

            } else {

                if (FirstMatch) {
                    Current = FirstMatch;
                    Pattern = PatternStart;
                    FirstMatch = NULL;
                }
            }
        }
    }

    return NULL;
}

int
UTLAPI
FindFunctionStartFromPtr(
    IN const void *Ptr,
    IN SIZE_T MaxSearchSize OPTIONAL,
    OUT const UINT8 **FoundAddress
)
{
    const UINT8 *P;
    const UINT8 *Found;
    SIZE_T Index;
    PUINT16 Pad16;
    PUINT64 Pad64;
    PUINT8 PadMisaligned;

    if (!FoundAddress) {
        return E_INVALIDARG;
    }

    if (!MaxSearchSize) {
        MaxSearchSize = ~((SIZE_T)0);
    }

    P = (const UINT8 *)Ptr;
    Found = NULL;

    //
    // Scan code backwards until we find the base of the function. We do this
    // by checking for padding of 0xCC or 0x00 padding.
    //
    for (Index = 0; Index < MaxSearchSize; Index++, P--) {

        Pad16 = (PUINT16)P;
        Pad64 = (PUINT64)P;

        //
        // A block of instructions with 0xCCCC or 0xCCC3 is very unlikely.
        //
        if (*Pad16 == 0xCCCC || *Pad16 == 0xCCC3) {
            Pad16++;

            PadMisaligned = (PUINT8)Pad16;
            while (*PadMisaligned == 0xCC) {
                PadMisaligned++;
            }

            Found = PadMisaligned;
            break;
        }
        //
        // A block of instructions with 0x0000000000000000 is very unlikely.
        //
        else
        if (*Pad64 == 0x0000000000000000) {
            Pad64++;

            PadMisaligned = (PUINT8)Pad64;
            while (*PadMisaligned == 0x00) {
                PadMisaligned++;
            }

            Found = PadMisaligned;
            break;
        }
    }

    if (!Found) {
        return E_NOT_SET;
    }

    *FoundAddress = (const UINT8 *)Found;
    return NOERROR;
}

UINT32
UTLAPI
CRC32Checksum(
    IN const void *Data,
    IN SIZE_T Size
)
{
    UINT32 i, b;
    UINT32 Crc = 0xFFFFFFFF;

    while (Size--) {
        b = (UINT32)(*(UINT8 *)Data);
        Data = (UINT8 *)Data + 1;

        for (i = 0; i < 8; i++) {
            if ((Crc ^ b) & 1) {
                Crc = (Crc >> 1) ^ 0xEDB88320;
            } else {
                Crc = (Crc >> 1);
            }
            b >>= 1;
        }
    }

    return (UINT32)Crc;
}

SIZE_T
UTLAPI
GetModuleSizeWIDE(
    IN LPCWSTR lpModuleName
)
{
    NTSTATUS Status;
    HMODULE ModuleHandle;
    UNICODE_STRING DestinationString;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeaders;

    if (lpModuleName != NULL) {

        RtlInitUnicodeString(&DestinationString, lpModuleName);
        Status = LdrFindModuleHandle(NULL, NULL, &DestinationString, (PVOID *)&ModuleHandle);

        if (!NT_SUCCESS(Status)) {
            RtlSetLastNTError(Status);
            return 0;
        }

    } else {

        ModuleHandle = (HMODULE)NtCurrentPeb()->ImageBaseAddress;
    }

    DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(ModuleHandle);
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    NtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>((PUINT8)ModuleHandle + DosHeader->e_lfanew);
    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    return NtHeaders->OptionalHeader.SizeOfImage;
}

HMODULE
UTLAPI
GetModuleHandleWIDE(
    IN LPCWSTR lpModuleName
)
{
    NTSTATUS Status;
    HMODULE Result;
    UNICODE_STRING DestinationString;
    HMODULE ModuleHandle;

    if (lpModuleName != NULL) {

        RtlInitUnicodeString(&DestinationString, lpModuleName);
        Status = LdrFindModuleHandle(NULL, NULL, &DestinationString, (PVOID *)&ModuleHandle);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastNTError(Status);
            return NULL;
        }

        Result = ModuleHandle;

    } else {

        Result = (HMODULE)NtCurrentPeb()->ImageBaseAddress;
    }

    return Result;
}

HMODULE
UTLAPI
GetModuleHandleCRC32(
    IN UINT32 CRC32ModuleName
)
{
    NTSTATUS Status;
    HMODULE Result;
    HMODULE ModuleHandle;

    if (CRC32ModuleName != 0) {

        Status = LdrFindModuleHandleEx(LDR_FIND_MODULE_HANDLE_EX_UNCHANGED_REFCOUNT,
                                       CRC32ModuleName,
                                       NULL,
                                       NULL,
                                       NULL,
                                       (PVOID *)&ModuleHandle
                                       );
        if (!NT_SUCCESS(Status)) {
            RtlSetLastNTError(Status);
            return NULL;
        }

        Result = ModuleHandle;

    } else {

        Result = (HMODULE)NtCurrentPeb()->ImageBaseAddress;
    }

    return Result;
}


PVOID
UTLAPI
GetProcAddressANSI(
    IN HMODULE hModule,
    IN LPCSTR lpProcName
)
{
    NTSTATUS Status;
    CHAR *ProcNamePtr;
    ULONG Ordinal;
    PVOID ProcedureAddress = NULL;

    if (HIWORD(lpProcName) != 0) {

        //
        // Look up by name.
        //
        ProcNamePtr = (CHAR *)lpProcName;
        Ordinal = 0;

    } else {

        //
        // Look up by ordinal.
        //
        Ordinal = (ULONG)(ULONG_PTR)lpProcName;
        ProcNamePtr = NULL;
    }

    //
    // Find the procedure address.
    //
    Status = LdrFindExportAddressAscii((VOID*)hModule, ProcNamePtr, Ordinal, &ProcedureAddress);

    if (!NT_SUCCESS(Status)) {
        RtlSetLastNTError(Status);
        return NULL;
    }

    if (!ProcedureAddress) {
        RtlSetLastWin32Error(ERROR_NOT_FOUND);
    }

    //
    // Return the found procedure pointer.
    //
    return ProcedureAddress;
}

PVOID
UTLAPI
GetProcAddressWIDE(
    IN HMODULE hModule,
    IN LPCWSTR lpProcName
)
{
    NTSTATUS Status;
    WCHAR* ProcNamePtr;
    ULONG Ordinal;
    PVOID ProcedureAddress;

    if (HIWORD(lpProcName) != 0) {

        //
        // Look up by name.
        //
        ProcNamePtr = (WCHAR *)lpProcName;
        Ordinal = 0;

    } else {

        //
        // Look up by ordinal
        //
        Ordinal = (ULONG)(ULONG_PTR)lpProcName;
        ProcNamePtr = NULL;
    }

    //
    // Get the proc address.
    //
    Status = LdrFindExportAddressUnicode((PVOID)hModule, ProcNamePtr, Ordinal, &ProcedureAddress);

    if (!NT_SUCCESS(Status)) {
        RtlSetLastNTError(Status);
        return NULL;
    }

    if (!ProcedureAddress) {
        RtlSetLastWin32Error(ERROR_NOT_FOUND);
    }

    //
    // Return the found procedure pointer.
    //
    return ProcedureAddress;
}

PVOID
UTLAPI
GetProcAddressCRC32(
    IN HMODULE hModule,
    IN UINT32 ProcNameCrc32
)
{
    NTSTATUS Status;
    PVOID ProcedureAddress;

    //
    // Get the procedure address.
    //
    Status = LdrFindExportAddressCrc32(hModule, ProcNameCrc32, 0, &ProcedureAddress);
    if (!NT_SUCCESS(Status)) {
        RtlSetLastNTError(Status);
        return NULL;
    }

    if (!ProcedureAddress) {
        RtlSetLastWin32Error(ERROR_NOT_FOUND);
    }

    //
    // Return the found procedure pointer.
    //
    return ProcedureAddress;
}

ULONG
UTLAPI
GetModuleFileNameWIDE(
    IN HMODULE hModule,
    OUT LPWSTR lpFileName,
    IN ULONG FileNameSize
)
{
    ULONG Length = 0;
    PLDR_DATA_TABLE_ENTRY ModuleEntry;
    NTSTATUS Status;

    if (!hModule) {
        hModule = (HMODULE)NtCurrentPeb()->ImageBaseAddress;
    }

    Status = LdrFindEntryForAddress(hModule, &ModuleEntry);
    if (NT_SUCCESS(Status)) {

        Length = __MIN(FileNameSize, ModuleEntry->FullDllName.Length / sizeof(WCHAR));
        RtlCopyMemory(lpFileName, ModuleEntry->FullDllName.Buffer, Length * sizeof(WCHAR));

        if (Length < FileNameSize) {

            lpFileName[Length] = '\0';
            RtlSetLastWin32Error(ERROR_SUCCESS);

        } else {

            RtlSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
        }

    } else {

        RtlSetLastNTError(Status);
    }

    return Length;
}

ULONG
UTLAPI
GetModuleFileNameANSI(
    IN HMODULE hModule,
    OUT LPSTR lpFileName,
    IN ULONG FileNameSize
)
{
    WCHAR FileName[MAX_PATH];
    SIZE_T Length;
    int Errno;

    Length = GetModuleFileNameWIDE(hModule, FileName, MAX_PATH);
    if (Length) {

        Errno = (int)wcstombs_s(&Length, lpFileName, FileNameSize, FileName, MAX_PATH);
        if (!Errno) {

            if (Length < FileNameSize) {
                lpFileName[Length] = '\0';
            } else {
                RtlSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
            }

        } else {

            Length = 0;
            RtlSetLastWin32Error(Errno);
        }
    }

    return (ULONG)Length;
}

ULONG
UTLAPI
GetModuleFileNameCRC32(
    IN HMODULE hModule,
    OUT PULONG lpNameCRC32,
    IN ULONG FileNameSize
)
{
    CHAR FilePath[MAX_PATH];
    CHAR* FileName;
    size_t FilePathLength, FileNameLength;

    *lpNameCRC32 = 0;

    FilePathLength = GetModuleFileNameANSI(hModule, FilePath, MAX_PATH);

    FileName = strrchr(FilePath, '\\');
    if (!FileName) {
        FileName = strrchr(FilePath, '/');
    }

    if (!FileName) {
        FileName = FilePath;
    }

    FileNameLength = strlen(FileName);
    if (FileNameLength) {
        *lpNameCRC32 = CRC32Checksum(FileName, FileNameLength);
    }

    return (ULONG)FilePathLength;
}

ULONG
UTLAPI
GetModuleSize(
    IN HMODULE hModule
)
{
    NTSTATUS Status;
    ULONG Size;
    PLDR_DATA_TABLE_ENTRY ModuleEntry;

    Status = LdrFindEntryForAddress(hModule, &ModuleEntry);
    if (NT_SUCCESS(Status)) {

        Size = ModuleEntry->SizeOfImage;

    } else {

        RtlSetLastNTError(Status);
        Size = 0;
    }

    return Size;
}


} // utils