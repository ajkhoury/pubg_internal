#include "rtl.h"

#include <intrin.h>
#include <stdlib.h>

///
/// < RTL Heap API Implementation >
///
PVOID
NTAPI
RtlGetProcessHeap(
    VOID
)
{
    return RtlProcessHeap();
}


///
/// < RTL String API Implementation >
///

VOID
NTAPI
RtlInitUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
)
{
    ULONG Length;

    if ((DestinationString->Buffer = (PWSTR)SourceString)) {

        Length = (ULONG)(wcslen(SourceString) * sizeof(WCHAR));
        if (Length > 0xFFFC) {
            Length = 0xFFFC;
        }

        DestinationString->Length = (USHORT)Length;
        DestinationString->MaximumLength = DestinationString->Length + sizeof(WCHAR);

    } else {

        DestinationString->Length = DestinationString->MaximumLength = 0;
    }
}

NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
)
{
    ULONG Length;

    if (SourceString != NULL) {

        Length = (ULONG)(wcslen(SourceString) * sizeof(WCHAR));
        if (Length > 0xFFFC) {
            return STATUS_NAME_TOO_LONG;
        } else {
            DestinationString->Length = (USHORT)Length;
            DestinationString->MaximumLength = (USHORT)(Length + sizeof(WCHAR));
            DestinationString->Buffer = (PWSTR)SourceString;
        }

    } else {

        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;
    }
    return STATUS_SUCCESS;
}

VOID
NTAPI
RtlFreeUnicodeString(
    IN OUT PUNICODE_STRING UnicodeString
)
{
    if (UnicodeString->Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeString->Buffer);
        RtlZeroMemory(UnicodeString, sizeof(*UnicodeString));
    }
}

VOID
NTAPI
RtlCopyUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString OPTIONAL
)
{
    USHORT Length;

    if (SourceString != NULL) {

        Length = (USHORT)(SourceString->Length < DestinationString->MaximumLength) ? 
                            SourceString->Length :
                            DestinationString->MaximumLength;

        RtlCopyMemory(DestinationString->Buffer, SourceString->Buffer, Length);
        DestinationString->Length = Length;

        // Append terminating '\0' if enough space
        if (Length < DestinationString->MaximumLength) {
            DestinationString->Buffer[Length / sizeof(WCHAR)] = L'\0';
        }

    } else {

        DestinationString->Length = 0;
    }
}

NTSTATUS
NTAPI
RtlAppendUnicodeStringToString(
    IN OUT PUNICODE_STRING Destination,
    IN PCUNICODE_STRING Source
)
{
    ULONG DestinationLength;

    if (Source->Length != 0) {

        DestinationLength = Source->Length + Destination->Length;

        if (DestinationLength > Destination->MaximumLength) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        RtlCopyMemory(Destination->Buffer + (Destination->Length / sizeof(WCHAR)), Source->Buffer, Source->Length);

        Destination->Length = (USHORT)DestinationLength;

        // Append terminating '\0' if enough space
        if (DestinationLength + sizeof(WCHAR) <= Destination->MaximumLength) {
            Destination->Buffer[DestinationLength / sizeof(WCHAR)] = L'\0';
        }
    }
    return STATUS_SUCCESS;
}

LONG
NTAPI
RtlCompareUnicodeStrings(
    IN CONST PWCHAR String1,
    IN UINT32 Length1,
    IN CONST PWCHAR String2,
    IN UINT32 Length2,
    IN BOOLEAN CaseInSensitive
)
{
    LONG Result = 0;
    ULONG Length = __MIN(Length1, Length2) / sizeof(WCHAR);
    PWCHAR Str1 = String1;
    PWCHAR Str2 = String2;

    if (CaseInSensitive) {

        while (!Result && Length--) {
            Result = (RtlUpcaseUnicodeChar(*Str1) - RtlUpcaseUnicodeChar(*Str2));
            Str1++;
            Str2++;
        }

    } else {

        while (!Result && Length--) {
            Result = (*Str1 - *Str2);
            Str1++;
            Str2++;
        }
    }

    if (!Result)
        Result = (Length1 - Length2);

    return Result;
}

BOOLEAN
NTAPI
RtlEqualUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
)
{
    if (String1->Length != String2->Length)
        return FALSE;

    return (BOOLEAN)(RtlCompareUnicodeString(String1, String2, CaseInSensitive) == 0);
}

NTSTATUS
NTAPI
RtlHashUnicodeString(
    IN PCUNICODE_STRING String,
    IN BOOLEAN CaseInSensitive,
    IN UINT32 HashAlgorithm,
    OUT UINT32 *HashValue
)
{
    UINT32 i;

    if (!String || !HashValue) {
        return STATUS_INVALID_PARAMETER;
    }

    switch (HashAlgorithm) {
    case HASH_STRING_ALGORITHM_DEFAULT:
    case HASH_STRING_ALGORITHM_X65599:
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    *HashValue = 0;

    for (i = 0; i < (String->Length / sizeof(WCHAR)); i++) {
        *HashValue =
            *HashValue * 65599 + (CaseInSensitive ?
                                  RtlUpcaseUnicodeChar(String->Buffer[i]) : String->Buffer[i]);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
    OUT PULONG BytesInMultiByteString,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
)
{
    SIZE_T UnicodeStringLength;

    UnicodeStringLength = (SIZE_T)wcslen(UnicodeString);

    if (UnicodeStringLength > (BytesInUnicodeString / sizeof(WCHAR))) {

        *BytesInMultiByteString = (ULONG)(UnicodeStringLength * sizeof(CHAR));

    } else {

        if (BytesInUnicodeString & 1) {
            return STATUS_INVALID_PARAMETER;
        }

        *BytesInMultiByteString = (BytesInUnicodeString / sizeof(WCHAR)) * sizeof(CHAR);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(
    OUT UINT32 *BytesInUnicodeString,
    IN CONST CHAR *MultiByteString,
    IN UINT32 BytesInMultiByteString
)
{
    SIZE_T MbStringLength;

    MbStringLength = (SIZE_T)strnlen(MultiByteString, BytesInMultiByteString / sizeof(CHAR));
    if (MbStringLength > (BytesInMultiByteString / sizeof(CHAR))) {

        *BytesInUnicodeString = (UINT32)(MbStringLength * sizeof(WCHAR));

    } else {

        if (BytesInMultiByteString & 1) {
            return STATUS_INVALID_PARAMETER;
        }
        *BytesInUnicodeString = (BytesInMultiByteString / sizeof(CHAR)) * sizeof(WCHAR);
    }

    return STATUS_SUCCESS;
}

ULONG
NTAPI
RtlUnicodeStringToAnsiSize(
    IN PUNICODE_STRING UnicodeString
)
{
    ULONG Total;

    RtlUnicodeToMultiByteSize(&Total, UnicodeString->Buffer, UnicodeString->Length);

    return Total + sizeof(CHAR);
}

UINT32
NTAPI
RtlAnsiStringToUnicodeSize(
    IN PCANSI_STRING AnsiString
)
{
    UINT32 Total;

    RtlMultiByteToUnicodeSize(&Total, AnsiString->Buffer, AnsiString->Length);

    return Total + sizeof(WCHAR);
}

NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
    OUT WCHAR *UnicodeString,
    IN UINT32 MaxBytesInUnicodeString,
    OUT UINT32 *BytesInUnicodeString OPTIONAL,
    IN CONST CHAR *MultiByteString,
    IN UINT32 BytesInMultiByteString
)
{
    UINT32 MaxCharsInUnicodeString = MaxBytesInUnicodeString / sizeof(WCHAR);
    UINT32 Converted = 0;

    //
    // Do the conversion.
    //
    do {
        if ((*UnicodeString++ = (WCHAR)*MultiByteString++) == 0)
            break;
        Converted++;
    } while ((--MaxCharsInUnicodeString) > 0);

    //
    // Return the number of byte in the unicode string.
    //
    if (BytesInUnicodeString) {
        *BytesInUnicodeString = (UINT32)(Converted * sizeof(WCHAR));
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
    OUT PCHAR MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PCWCH UnicodeString,
    IN ULONG BytesInUnicodeString
)
{
    errno_t Errno;
    SIZE_T Converted;

    Errno = wcstombs_s(&Converted, MultiByteString, MaxBytesInMultiByteString, UnicodeString, MaxBytesInMultiByteString);
    if (Errno == ERANGE) {

        if (BytesInMultiByteString != NULL) {
            *BytesInMultiByteString = 0;
        }
        return STATUS_BUFFER_TOO_SMALL;

    } else if (Errno != 0) {

        return STATUS_INVALID_PARAMETER;
    }

    if (BytesInMultiByteString != NULL)
        *BytesInMultiByteString = (ULONG)(Converted * sizeof(CHAR));

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    IN OUT PANSI_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Total = RtlUnicodeStringToAnsiSize((PUNICODE_STRING)SourceString);

    DestinationString->Length = (USHORT)(Total - sizeof(CHAR));
    if (AllocateDestinationString) {

        DestinationString->MaximumLength = (USHORT)Total;
        DestinationString->Buffer = (PSTR)RtlAllocateHeap(RtlProcessHeap(), 0, Total);
        if (!DestinationString->Buffer) {
            return STATUS_NO_MEMORY;
        }

    } else if (DestinationString->MaximumLength < Total) {

        if (!DestinationString->MaximumLength) {
            return STATUS_BUFFER_OVERFLOW;
        }
        DestinationString->Length = (USHORT)(DestinationString->MaximumLength - 1);
        Status = STATUS_BUFFER_OVERFLOW;
    }

    RtlUnicodeToMultiByteN(DestinationString->Buffer,
                           DestinationString->MaximumLength,
                           NULL,
                           SourceString->Buffer,
                           SourceString->Length
                           );

    DestinationString->Buffer[DestinationString->Length / sizeof(CHAR)] = '\0';

    return Status;
}

NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCANSI_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
)
{
    UINT32 UnicodeLength;
    UINT32 Index;
    NTSTATUS st;

    UnicodeLength = RtlAnsiStringToUnicodeSize(SourceString);
    if (UnicodeLength > UNICODE_STRING_MAX_BYTES) {
        return STATUS_INVALID_PARAMETER_2;
    }

    DestinationString->Length = (UINT16)(UnicodeLength - sizeof(UNICODE_NULL));
    if (AllocateDestinationString) {

        DestinationString->MaximumLength = (UINT16)UnicodeLength;
        DestinationString->Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, UnicodeLength);
        if (!DestinationString->Buffer) {
            return STATUS_NO_MEMORY;
        }

    } else {

        if ((DestinationString->Length + 1) >= DestinationString->MaximumLength) {
            return STATUS_BUFFER_OVERFLOW;
        }
    }

    st = RtlMultiByteToUnicodeN(DestinationString->Buffer,
                                DestinationString->Length,
                                &Index,
                                SourceString->Buffer,
                                SourceString->Length
                                );

    if (!NT_SUCCESS(st)) {

        if (AllocateDestinationString) {
            RtlFreeHeap(RtlProcessHeap(), 0, DestinationString->Buffer);
            DestinationString->Buffer = NULL;
        }

        return st;
    }

    DestinationString->Buffer[Index / sizeof(WCHAR)] = UNICODE_NULL;

    return STATUS_SUCCESS;
}

///
/// < RTL Error/Exception API Implementation >
///

ULONG
WINAPI
RtlSetLastNTError(
    IN NTSTATUS NtError
)
{
    ULONG DosError;
    DosError = RtlNtStatusToDosError(NtError);
    NtCurrentTeb()->LastErrorValue = DosError;
    return DosError;
}

BOOLEAN
NTAPI
RtlDispatchException(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT Context
)
{
    UNREFERENCED_PARAMETER(ExceptionRecord);
    UNREFERENCED_PARAMETER(Context);
    return FALSE;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4717) // RtlRaiseStatus is recursive by design
#endif
VOID
NTAPI
RtlRaiseStatus(
    IN NTSTATUS Status
)
{
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT Context;

    //
    // Capture the context.
    //
    RtlCaptureContext(&Context);

    //
    // Create an exception record
    //
    ExceptionRecord.ExceptionAddress = _ReturnAddress();
    ExceptionRecord.ExceptionCode = Status;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.NumberParameters = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;

    //
    // Write the context flag
    //
    Context.ContextFlags = CONTEXT_FULL;

    //
    // Check if a user mode debugger is active.
    //
    if (RtlpCheckForActiveDebugger()) {

        //
        // Raise an exception immediately.
        //
        NtRaiseException(&ExceptionRecord, &Context, TRUE);

    } else {

        //
        // Dispatch the exception.
        //
        RtlDispatchException(&ExceptionRecord, &Context);

        //
        // Raise exception if we got here.
        //
        Status = NtRaiseException(&ExceptionRecord, &Context, FALSE);
    }

    //
    // If we returned, raise a status.
    //
    RtlRaiseStatus(Status);
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif


///
/// < RTL Image API Implementation >
///

NTSTATUS
NTAPI
RtlImageNtHeaderEx(
    IN UINT32 Flags,
    IN VOID *Base,
    IN UINT64 Size,
    OUT PPE_NT_HEADERS* OutHeaders
)
{
    PPE_NT_HEADERS NtHeaders;
    UINT32 e_lfanew;
    BOOLEAN RangeCheck;
    NTSTATUS Status;

    NtHeaders = NULL;

    if (OutHeaders != NULL)
        *OutHeaders = NULL;

    if (OutHeaders == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & ~RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Base == NULL || Base == (VOID*)((SSIZE_T)-1)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    RangeCheck = (BOOLEAN)((Flags & RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) == 0);
    if (RangeCheck) {
        if (Size < sizeof(PE_DOS_HEADER)) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    //
    // Exception handling is not available in the boot loader, and exceptions
    // were not historically caught here in kernel mode. Drivers are considered
    // trusted, so we can't get an exception here due to a bad file, but we
    // could take an inpage error.
    //
    if (((PPE_DOS_HEADER)Base)->e_magic != PE_DOS_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto Exit;
    }

    e_lfanew = (UINT32)((PPE_DOS_HEADER)Base)->e_lfanew;

    if (RangeCheck) {
        if ((e_lfanew >= Size) ||
            (e_lfanew >= (~((UINT32)0) - SIZEOF_PE_SIGNATURE - sizeof(PE_FILE_HEADER))) ||
            ((e_lfanew + SIZEOF_PE_SIGNATURE + sizeof(PE_FILE_HEADER)) >= Size)) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    NtHeaders = (PPE_NT_HEADERS)((UINT8*)Base + e_lfanew);

    //
    // In kernelmode, do not cross from usermode address to kernelmode address.
    //
    if ((UINT64)Base < 0xFFFFFFFFFFFF0000) {
        if ((UINT64)NtHeaders >= 0xFFFFFFFFFFFF0000) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }

        //
        // Note that this check is slightly overeager since IMAGE_NT_HEADERS has
        // a builtin array of data_directories that may be larger than the image
        // actually has. A better check would be to add FileHeader.SizeOfOptionalHeader,
        // after ensuring that the FileHeader does not cross the u/k boundary.
        //
        if ((UINT64)((UINT8*)NtHeaders + sizeof(PE_NT_HEADERS)) >= 0xFFFFFFFFFFFF0000) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    if (NtHeaders->Signature != PE_NT_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto Exit;
    }

    Status = STATUS_SUCCESS;

Exit:
    if (NT_SUCCESS(Status))
        *OutHeaders = NtHeaders;

    return Status;
}

PPE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    IN VOID *Base
)
{
    PPE_NT_HEADERS NtHeaders;

    (VOID)RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK,
                             Base,
                             0,
                             &NtHeaders
                             );

    return NtHeaders;
}

PPE_SECTION_HEADER
NTAPI
RtlImageRvaToSection(
    IN CONST PE_NT_HEADERS *NtHeader,
    IN VOID *BaseAddress,
    IN UINT32 Rva
)
{
    PPE_SECTION_HEADER Section;
    UINT32 Va;
    UINT32 Count;

    Count = NtHeader->FileHeader.NumberOfSections;
    Section = PE_FIRST_SECTION(NtHeader);

    while (Count--) {
        Va = Section->VirtualAddress;

        if ((Va <= Rva) && (Rva < Va + Section->SizeOfRawData))
            return Section;

        Section++;
    }

    return NULL;
}

PPE_SECTION_HEADER
NTAPI
RtlImageFindSectionByName(
    IN CONST PE_NT_HEADERS *NtHeader,
    IN VOID *BaseAddress,
    IN CONST CHAR *SectionName
)
{
    PPE_SECTION_HEADER Section;
    UINT32 Count;
    CHAR *SectionName1, *SectionName2;

    Count = NtHeader->FileHeader.NumberOfSections;
    Section = PE_FIRST_SECTION(NtHeader);

    while (Count--) {
        SectionName1 = (CHAR*)SectionName;
        SectionName2 = (CHAR*)Section->Name;

        while (*SectionName1) {
            if (*SectionName1 != *SectionName2)
                break;
            SectionName1 += 1;
            SectionName2 += 1;
        }

        if (!(*SectionName1 - *SectionName2))
            return Section;

        Section++;
    }

    return NULL;
}

VOID*
NTAPI
RtlImageRvaToVa(
    IN CONST PE_NT_HEADERS *NtHeader,
    IN VOID* BaseAddress,
    IN UINT32 Rva,
    IN OUT PE_SECTION_HEADER **SectionHeader
)
{
    PPE_SECTION_HEADER Section = NULL;

    if (SectionHeader) {
        Section = *SectionHeader;
    }

    if ((Section == NULL) ||
        (Rva < Section->VirtualAddress) ||
        (Rva >= Section->VirtualAddress + Section->SizeOfRawData)) {

        Section = RtlImageRvaToSection(NtHeader, BaseAddress, Rva);

        if (Section == NULL) {
            return NULL;
        }

        if (SectionHeader) {
            *SectionHeader = Section;
        }
    }

    return (VOID*)((UINT8*)BaseAddress + Rva + (Section->PointerToRawData - Section->VirtualAddress));
}

VOID*
NTAPI
RtlImageDirectoryEntryToData(
    IN VOID *BaseAddress,
    IN BOOLEAN MappedAsImage,
    IN UINT16 Directory,
    OUT UINT32 *Size OPTIONAL
)
{
    PPE_NT_HEADERS NtHeader;
    UINT32 Va;

    //
    // Magic flag for non-mapped images
    //
    if ((SIZE_T)BaseAddress & 1) {
        BaseAddress = (VOID*)((SIZE_T)BaseAddress & ~1);
        MappedAsImage = FALSE;
    }

    //
    // Grab the NT header.
    //
    NtHeader = RtlImageNtHeader(BaseAddress);
    if (!NtHeader) {
        return NULL;
    }

    //
    // Verify Directory parameter.
    //
    if (Directory >= OPTIONAL_HEADER_VALUE(NtHeader, NumberOfRvaAndSizes)) {
        return NULL;
    }

    //
    // Determine Virtual Address and Size of the Directory.
    //
    Va = DATA_DIRECTORY_RVA(NtHeader, Directory);
    if (Va == 0) {
        return NULL;
    }

    *Size = DATA_DIRECTORY_SIZE(NtHeader, Directory);

    //
    // Check if its mapped as an ordinary file or not.
    //
    if (MappedAsImage || Va < NtHeader->OptionalHeader.SizeOfHeaders) {
        return (VOID*)((ULONG_PTR)BaseAddress + Va);
    }

    //
    // Image mapped as ordinary file, we must find raw pointer.
    //
    return RtlImageRvaToVa(NtHeader, BaseAddress, Va, NULL);
}

