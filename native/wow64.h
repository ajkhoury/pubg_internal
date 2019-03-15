#pragma once

#include "Native.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

ULONG64 
WINAPIV 
Wow64CallX64( 
    IN ULONG64 FuncAddress, 
    IN ULONG ArgCount,
    IN ... 
    );

ULONG64 
Wow64TebX64( 
    VOID 
    );

BOOL
Wow64CopyMemoryX64(
    OUT PVOID Dest,
    IN ULONG64 Source,
    IN SIZE_T Size
    );

BOOL
Wow64CompareMemoryX64(
    IN PVOID DestAddress,
    IN ULONG64 SourceAddress,
    IN SIZE_T Size
    );

ULONG64
WINAPI
Wow64GetModuleHandleX64(
    IN LPCWSTR lpModuleName
    );

ULONG64
WINAPI
Wow64GetNtdllX64(
    VOID
    );

ULONG64
WINAPI
Wow64GetExportAddressX64(
    IN ULONG64 BaseAddress,
    IN LPCSTR pszName
    );

ULONG64
WINAPI
Wow64VirtualAllocExX64(
    IN HANDLE hProcess,
    IN OUT ULONG64 lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect
    );

NTSTATUS
NTAPI
Wow64NtQuerySystemInformationX64(
    IN ULONG SystemInformationClass,
    IN OUT PVOID SystemInformation OPTIONAL,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
NTAPI
Wow64NtQueryObjectX64(
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    IN OUT PVOID ObjectInformation OPTIONAL,
    IN ULONG ObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus