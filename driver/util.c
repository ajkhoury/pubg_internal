/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file util.c
 * @author Aidan Khoury (ajkhoury)
 * @date 8/30/2018
 */

#include "util.h"

#include "file.h"
#include "image.h"
#include "mm.h"
#include "process.h"
#include "loader.h"

#include "log.h"

static PVOID UtlpNtKernelBase = NULL;
static ULONG UtlpNtKernelSize = 0;

static PKSERVICE_TABLE_DESCRIPTOR UtlpServiceDescriptorTable = NULL;
static PKSERVICE_TABLE_DESCRIPTOR UtlpServiceDescriptorTableShadow = NULL;

static KSPIN_LOCK UtlpLock;

CONST UNICODE_STRING UtlpSystemRootName = RTL_CONSTANT_STRING(L"\\SystemRoot");

//
// .text:140154F10                      KiSystemServiceStart proc near          ; DATA XREF: KiServiceInternal+5A
// .text:140154F10 48 89 A3 90 00 00 00     mov     [rbx+90h], rsp
// .text:140154F17 8B F8                    mov     edi, eax
// .text:140154F19 C1 EF 07                 shr     edi, 7 <------- sigKiSystemServiceStart
// .text:140154F1C 83 E7 20                 and     edi, 20h
// .text:140154F1F 25 FF 0F 00 00           and     eax, 0FFFh
//
#define KI_SYSTEM_SERVICE_START_SIGNATURE { 0xC1, 0xEF, 0x07, 0x83, 0xE7, \
                                            0x20, 0x25, 0xFF, 0x0F, 0x00, 0x00 }


//
// Private Implementation
//

static
PVOID
UtlpFindNtKernelBaseByPointer(
    IN PVOID Pointer,
    OUT ULONG *Size
)
{
    PUCHAR CurrentPage;
    PIMAGE_DOS_HEADER DosHdr;
    PIMAGE_NT_HEADERS NtHdr;
    ULONG Count;

    //
    // Align to a page size.
    //
    CurrentPage = (PUCHAR)PAGE_ALIGN(Pointer);

    //
    // Enumerate pages backwards until we find the DOS header of the kernel.
    //
    for (Count = 0; CurrentPage; CurrentPage -= PAGE_SIZE) {

        //
        // Dont go back farther than 2048 pages.
        //
        if ((++Count) > 2048) {
            break;
        }

        //
        // Is this a DOS header?
        //
        DosHdr = (PIMAGE_DOS_HEADER)CurrentPage;

        //
        // If this address is not mapped in, there will be a PAGE_FAULT_IN_NONPAGED_AREA.
        //
        if (!UtlIsAddressValid((ULONG_PTR)DosHdr) || DosHdr->e_magic != IMAGE_DOS_SIGNATURE) {
            continue;
        }

        //
        // Validate the NT header.
        //
        NtHdr = (PIMAGE_NT_HEADERS64)((ULONG_PTR)DosHdr + DosHdr->e_lfanew);
        if ((ULONG_PTR)NtHdr >= (ULONG_PTR)Pointer || (ULONG_PTR)NtHdr <= (ULONG_PTR)DosHdr) {
            continue;
        }

        //
        // Make sure the header is mapped in!
        //
        if (!UtlIsAddressValid((ULONG_PTR)NtHdr) || NtHdr->Signature != IMAGE_NT_SIGNATURE) {
            continue;
        }

        //
        // Found the kernel base!
        //
        if (ARGUMENT_PRESENT(Size)) {
            *Size = NtHdr->OptionalHeader.SizeOfImage;
        }

        return DosHdr;
    }

    if (ARGUMENT_PRESENT(Size)) {
        *Size = 0;
    }

    return NULL;
}

static
NTSTATUS
UtlpFindSystemServiceDescriptorTables(
    OUT PVOID *FoundSsdt,
    OUT PVOID *FoundSsdtShadow
)
{
    NTSTATUS Status;
    PVOID KernelBase;
    PUCHAR Found, SearchEnd;
    PUCHAR SsdtBase;
    CONST UCHAR NTSIG(KiSystemServiceStart)[] = KI_SYSTEM_SERVICE_START_SIGNATURE;

    if (!ARGUMENT_PRESENT(FoundSsdt) ||
        !ARGUMENT_PRESENT(FoundSsdtShadow)) {
        return STATUS_INVALID_PARAMETER;
    }

    KernelBase = UtlGetNtKernelBase(NULL);
    if (!KernelBase) {
        return STATUS_NOT_FOUND;
    }

    //
    // Our goal is to determine the KeServiceDescriptorTable address.
    //
    // .text:140154F24                     KiSystemServiceRepeat proc near         ; CODE XREF: KiSystemServiceExit+32Bj
    // .text:140154F24 4C 8D 15 55 38 25 00     lea     r10, KeServiceDescriptorTable <--- Here is what we're looking for
    // .text:140154F2B 4C 8D 1D 0E 38 25 00     lea     r11, KeServiceDescriptorTableShadow
    // .text:140154F32 F7 43 78 40 00 00 00     test    dword ptr [rbx+78h], 40h
    // .text:140154F39 74 13                    jz      short loc_140154F4E
    // .text:140154F3B F7 43 78 00 00 08 00     test    dword ptr [rbx+78h], 80000h
    // .text:140154F42 74 07                    jz      short loc_140154F4B
    // .text:140154F44 4C 8D 1D 75 38 25 00     lea     r11, KeServiceDescriptorTableFilter
    //
    Status = UtlFindPatternInSection(KernelBase,
                                     SECTION_TEXT,
                                     0xCC,
                                     NTSIG(KiSystemServiceStart),
                                     sizeof(NTSIG(KiSystemServiceStart)),
                                     (PVOID *)&Found
                                     );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    SsdtBase = NULL;
    SearchEnd = Found + 64;
    while (Found < SearchEnd) {
        if (*Found == 0x4C && *((USHORT*)(Found + 1)) == 0x158D) {
            SsdtBase = Found;
            break;
        }
        ++Found;
    }

    if (!SsdtBase) {
        return STATUS_NOT_FOUND;
    }

    *FoundSsdt = UtlMemOperandTargetAddress(SsdtBase, 0);
    *FoundSsdtShadow = UtlMemOperandTargetAddress(SsdtBase + 3 + sizeof(LONG), 0);

    return STATUS_SUCCESS;
}



//
// Public Implementation
//

/**
 * Search for substring String2 in String1.
 *
 * @param[in] String1          The string to search for a substring in.
 * @param[in] String2          The substring to search for.
 * @param[in] CaseInsensitive  Whether the search should be case insensitive or not.
 * @return  A pointer to the found substring String2 inside String1.
 *          NULL if not found.
 */
WCHAR*
UTLAPI
UtlWcsStr(
    IN CONST WCHAR* String1,
    IN CONST WCHAR* String2,
    IN BOOLEAN CaseInsensitive
)
{
    WCHAR *p1, *p2;
    WCHAR *cp = (WCHAR*)String1; // cast away const!

    if (CaseInsensitive) {

        while (*cp) {
            for (p1 = cp, p2 = (WCHAR*)String2; // cast away const!
                 *p1 && *p2 && towlower(*p1) == towlower(*p2);
                 ++p1, ++p2)
                ;

            if (!*p2)
                return cp;

            ++cp;
        }

    } else {

        while (*cp) {
            p1 = cp;
            p2 = (WCHAR*)String2; // cast away const!

            while (*p1 && *p2 && !(*p1 - *p2))
                p1++, p2++;

            if (!*p2)
                return cp;

            ++cp;
        }
    }

    return NULL;
}

/**
 * Compares two unicode strings to determine whether one string is a suffix
 * of the other. This is an exact implementation of the RtlSuffixUnicodeString
 * routine which is not exported by ntoskrnl until Windows 10.
 *
 * @param[in] Suffix           The string to check if its a suffix of \p String.
 * @param[in] String           The string to check if for the suffix \p Suffix.
 * @param[in] CaseInSensitive  TRUE if should ignore case.
 * @return  TRUE if String2 is a suffix of String1.
 */
BOOLEAN
UTLAPI
UtlSuffixUnicodeString(
    IN PCUNICODE_STRING Suffix,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInSensitive
)
{
    return String->Length >= Suffix->Length &&
            RtlCompareUnicodeStrings(String->Buffer +
                                        (String->Length - Suffix->Length) / sizeof(WCHAR),
                                     Suffix->Length / sizeof(WCHAR),
                                     Suffix->Buffer,
                                     Suffix->Length / sizeof(WCHAR),
                                     CaseInSensitive) == 0;
}

/**
 * Converts an ascii representation of a number into a 32-bit integer.
 *
 * @param[in] str The ascii string representation of a number.
 * @return  The 32-bit value integer of the ascii string.
 */
int
UTLAPI
UtlAsciiStrToInteger(
    IN CONST CHAR *str
)
{
    int c;          /* current char */
    int sign;       /* if '-', then negative, otherwise positive */
    int total;      /* current total */

    /* skip whitespace */
    while (isspace((int)(unsigned char)*str)) {
        ++str;
    }

    c = (int)(unsigned char)*str++;
    sign = c;                           /* save sign indication */
    if (c == '-' || c == '+') {
        c = (int)(unsigned char)*str++; /* skip sign */
    }
    total = 0;

    while (isdigit(c)) {
        total = 10 * total + (c - '0'); /* accumulate digit */
        c = (int)(unsigned char)*str++; /* get next char */
    }

    if (sign == '-') {
        return -total;
    } else {
        return total;                   /* return result, negated if necessary */
    }
}

/**
 * Converts a wide-character representation of a number into a 32-bit integer.
 *
 * @param[in] str The wide-character string representation of a number.
 * @return  The 32-bit value integer of the wide-character string.
 */
int
UTLAPI
UtlWideStrToInteger(
    IN CONST WCHAR *str
)
{
    wint_t c;       /* current char */
    wint_t sign;    /* if '-', then negative, otherwise positive */
    int total;      /* current total */

    /* skip whitespace */
    while (iswspace((wint_t)*str)) {
        ++str;
    }

    c = (wint_t)*str++;
    sign = c;                           /* save sign indication */
    if (c == L'-' || c == L'+') {
        c = (wint_t)*str++;             /* skip sign */
    }
    total = 0;

    while (iswdigit(c)) {
        total = 10 * total + (c - L'0');/* accumulate digit */
        c = (wint_t)*str++;             /* get next char */
    }

    if (sign == L'-') {
        return -total;
    } else {
        return total;                   /* return result, negated if necessary */
    }
}

/**
 * Extracts the file name from the supplied file path.
 *
 * @param[in] FilePath  The file path to extract the file name from.
 * @param[out] FileName The extracted file name.
 * @return  STATUS_SUCCESS on success.
 *          STATUS_NOT_FOUND if no explicit file name was found.
 */
NTSTATUS
UTLAPI
UtlExtractFileName(
    IN PCUNICODE_STRING FilePath,
    OUT PUNICODE_STRING FileName
)
{
    USHORT Index;

    if (!FilePath || !FileName) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Make sure the file path string is not empty.
    //
    if (FilePath->Length >= sizeof(WCHAR)) {

        //
        // Iterate from end making way towards beginning, searching for the 
        // first slash character, indicating a directory.
        //
        for (Index = (FilePath->Length / sizeof(WCHAR)) - 1; Index != 0; Index--) {
            if (FilePath->Buffer[Index] == L'\\' || FilePath->Buffer[Index] == L'/') {
                FileName->Buffer = &FilePath->Buffer[Index + 1];
                FileName->Length = FilePath->Length - (Index + 1) * sizeof(WCHAR);
                FileName->MaximumLength = FilePath->MaximumLength - (Index + 1) * sizeof(WCHAR);
                return STATUS_SUCCESS;
            }
        }
    }

    //
    // Most likely just a file name then.
    //
    *FileName = *FilePath;
    return STATUS_NOT_FOUND;
}

/**
 * Extracts the directory from the supplied file path.
 *
 * @param[in] FilePath       The file path to extract the directory from.
 * @param[out] FileDirectory The extracted directory.
 * @return  STATUS_SUCCESS on success.
 *          STATUS_NOT_FOUND if no explicit directory was found.
 */
NTSTATUS
UTLAPI
UtlExtractDirectory(
    IN PCUNICODE_STRING FilePath,
    OUT PUNICODE_STRING FileDirectory
)
{
    USHORT Index;

    if (!FilePath || !FileDirectory) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Make sure the file path string is not empty.
    //
    if (FilePath->Length >= sizeof(WCHAR)) {

        //
        // Iterate from end making way towards beginning, searching for the 
        // first slash character, indicating a directory.
        //
        for (Index = (FilePath->Length / sizeof(WCHAR)) - 1; Index != 0; Index--) {
            if (FilePath->Buffer[Index] == L'\\' || FilePath->Buffer[Index] == L'/') {
                FileDirectory->Buffer = FilePath->Buffer;
                FileDirectory->Length = (Index + 1) * sizeof(WCHAR);
                FileDirectory->MaximumLength = FileDirectory->Length;
                break;
            }
        }
    }

    //
    // Most likely just a directory then.
    //
    *FileDirectory = *FilePath;
    return STATUS_NOT_FOUND;
}

/**
 * Determines if a given virtual address is valid by executing a canonical
 * check on the virtual address and checking for an associated physical
 * address.
 *
 * @param[in] Address  The virtual address to check.
 * @return  TRUE if the virtual address is valid.
 *          FALSE if the virtual address is not valid.
 */
BOOLEAN
UTLAPI
UtlIsAddressValid(
    IN ULONG_PTR Address
)
{
    ULONG_PTR TopPart;
    PHYSICAL_ADDRESS PhysAddress;

    //
    // Canonical check. Bits 48 to 63 must match bit 47.
    //
    TopPart = (Address >> 47);
    if (TopPart & 1) {

        //
        // Top must be 0x1FFFF
        //
        if (TopPart != 0x1FFFF) {
            return FALSE;
        }

    } else {

        //
        // Top must be 0.
        //
        if (TopPart != 0) {
            return FALSE;
        }
    }

    //
    // If canonical check failed, it better be less than MAXINT64.
    //
    if (Address < 0x7FFFFFFFFFFFFFFFULL) {
        return TRUE;
    }

    //
    // Lastly, check if it's a physical address.
    //
    PhysAddress = MmGetPhysicalAddress((PVOID)Address);
    return (PhysAddress.QuadPart != 0);
}

/**
 * Get the ntoskrnl build number.
 *
 * @param[out] BuildNumberPtr  Optionally return the pointer to the build
 *                             number in the ntoskrnl image.
 * @return  The nt build number. 0 if the nt build number was not found.
 */
USHORT
UTLAPI
UtlGetNtKernelBuild(
    OUT PUSHORT *BuildNumberPtr OPTIONAL
)
{
    NTSTATUS Status;
    PVOID KernelBase;
    PUSHORT NtBuildNumberPtr = NULL;
    USHORT BuildNumber = 0;
    CONST UNICODE_STRING NtBuildNumberUstr = RTL_CONSTANT_STRING(L"NtBuildNumber");

    if (!UtlpNtKernelBase) {
        KernelBase = UtlGetNtKernelBase(NULL);
    } else {
        KernelBase = UtlpNtKernelBase;
    }

    if (!KernelBase) {
        goto Exit;
    }

    Status = LdrFindExportAddressUnicode(KernelBase, &NtBuildNumberUstr, &NtBuildNumberPtr);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    BuildNumber = *NtBuildNumberPtr;

Exit:
    if (ARGUMENT_PRESENT(BuildNumberPtr)) {
        *BuildNumberPtr = NtBuildNumberPtr;
    }
    return BuildNumber;
}

/**
 * Get the ntoskrnl build lab string.
 *
 * @return  Returns the pointer to the NtBuildLab string in the ntoskrnl image.
 *          Returns NULL if not found.
 */
CONST CHAR*
UTLAPI
UtlGetNtKernelBuildLab(
    VOID
)
{
    NTSTATUS Status;
    PVOID KernelBase;
    PVOID NtBuildLabPtr;
    CONST UNICODE_STRING NtBuildLabUstr = RTL_CONSTANT_STRING(L"NtBuildLab");

    if (!UtlpNtKernelBase) {
        KernelBase = UtlGetNtKernelBase(NULL);
    } else {
        KernelBase = UtlpNtKernelBase;
    }

    if (!KernelBase) {
        return NULL;
    }

    Status = LdrFindExportAddressUnicode(KernelBase, &NtBuildLabUstr, &NtBuildLabPtr);
    if (!NT_SUCCESS(Status)) {
        return NULL;
    }

    return (CONST CHAR*)NtBuildLabPtr;
}


NTKERNELAPI VOID NTAPI __chkstk(VOID);

/**
 * Get the ntoskrnl base address and size.
 *
 * @param[out] Size  Optionally return the size of the ntoskrnl image.
 * @return  The base address of the ntoskrnl image.
 *          NULL if not found.
 */
PVOID
UTLAPI
UtlGetNtKernelBase(
    OUT PULONG Size OPTIONAL
)
{
    if (!UtlpNtKernelBase) {
        UtlpNtKernelBase = UtlpFindNtKernelBaseByPointer((PVOID)__chkstk, &UtlpNtKernelSize);
    }

    if (ARGUMENT_PRESENT(Size)) {
        *Size = UtlpNtKernelSize;
    }

    return UtlpNtKernelBase;
}

/**
 * Gets the system service descriptor address for the given service number.
 *
 * @param[in] ServiceTableIndex Supplies index of the service table.
 * @return  The service table address.
 *          NULL if not found.
 */
PKSERVICE_TABLE_DESCRIPTOR
UTLAPI
UtlGetServiceDescriptorTable(
    IN ULONG ServiceTableIndex OPTIONAL
)
{
    NTSTATUS Status;

    if (!UtlpServiceDescriptorTable || !UtlpServiceDescriptorTableShadow) {

        Status = UtlpFindSystemServiceDescriptorTables(
                                    (PVOID*)&UtlpServiceDescriptorTable,
                                    (PVOID*)&UtlpServiceDescriptorTableShadow
                                    );
        if (!NT_SUCCESS(Status)) {
            return NULL;
        }

        LOG_INFO("SSDT Base = 0x%p", UtlpServiceDescriptorTable->Base);
        LOG_INFO("SSDT Count = 0x%p", UtlpServiceDescriptorTable->Count);
        LOG_INFO("SSDT Limit = 0x%X", UtlpServiceDescriptorTable->Limit);
        LOG_INFO("SSDT Number = 0x%p", UtlpServiceDescriptorTable->Number);
    }

    if ((ServiceTableIndex > NUMBER_SERVICE_TABLES - 1) ||
        ((UtlpServiceDescriptorTable[ServiceTableIndex].Base == NULL) &&
        (UtlpServiceDescriptorTableShadow[ServiceTableIndex].Base == NULL))) {

        return NULL;

    } else {

        if (ServiceTableIndex != WIN32K_SERVICE_INDEX) {
            return &UtlpServiceDescriptorTable[ServiceTableIndex];
        } else {
            return &UtlpServiceDescriptorTableShadow[ServiceTableIndex];
        }
    }
}

/**
 * Gets the service descriptor entry function address for the given service
 * number.
 *
 * @param[in] ServiceTable  Supplies the service table to get the entry from.
 * @param[in] ServiceNumber The service number used as an index into the
 *                          system service descriptor table.
 * @return  The found service function address.
 *          NULL if not found.
 */
PVOID
UTLAPI
UtlGetServiceDescriptorTableEntry(
    IN PKSERVICE_TABLE_DESCRIPTOR ServiceTable OPTIONAL,
    IN ULONG ServiceNumber
)
{
    LONG ServiceOffset;

    if (!ServiceTable) {

        //
        // Get the System Service Descriptor Table by default.
        //
        ServiceTable = UtlGetServiceDescriptorTable(SYSTEM_SERVICE_INDEX);
    }

    //
    // Check if the table is valid.
    //
    if (!ServiceTable || ServiceNumber >= ServiceTable->Limit) {
        return NULL;
    }

    //
    // Calculate the service entry offset.
    //
    ServiceOffset = *((LONG*)ServiceTable->Base + ServiceNumber) >> SERVICE_OFFSET_SHIFT;

    //
    // Calculate the service entry address.
    //
    return (PVOID)((ULONG_PTR)ServiceTable->Base + ServiceOffset);
}

/**
 * Get the certificate data for the specified file.
 *
 * @param[in] Filename      The filename of the file to retrieve the
 *                          certificate data for.
 * @param[out] Certificate  The resulting certificate data.
 * @param[out] CertificateLength  The resulting certificate data length.
 * @return  STATUS_SUCCESS on successful retrieval of the certificate data.
 *          The appropriate error status code if a failure occurred.
 */
NTSTATUS
UTLAPI
UtlGetFileCertificate(
    IN WCHAR* Filename,
    OUT LPWIN_CERTIFICATE *Certificate,
    OUT ULONG *CertificateLength
)
{
    NTSTATUS Status;
    UNICODE_STRING FilenameUnicode;
    HANDLE FileHandle;
    ULONG CertLength;
    LPWIN_CERTIFICATE CertData;

    if (!ARGUMENT_PRESENT(Certificate) || !ARGUMENT_PRESENT(CertificateLength)) {
        return STATUS_INVALID_PARAMETER;
    }

    CertData = NULL;
    CertLength = 0;
    RtlInitUnicodeString(&FilenameUnicode, Filename);

    Status = FileOpen(&FilenameUnicode,
                      SYNCHRONIZE | GENERIC_READ | FILE_READ_ATTRIBUTES,
                      FILE_SHARE_READ,
                      FILE_SYNCHRONOUS_IO_NONALERT,
                      &FileHandle);

    if (NT_SUCCESS(Status)) {

        Status = ImageGetCertificateData(FileHandle,
                                         0,
                                         NULL,
                                         &CertLength);

        if (Status == STATUS_BUFFER_OVERFLOW) {

            CertData = MmAllocateNonPaged(CertLength);
            if (!CertData) {
                FileClose(FileHandle);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            Status = ImageGetCertificateData(FileHandle,
                                             0,
                                             CertData,
                                             &CertLength);
            if (!NT_SUCCESS(Status)) {
                ExFreePoolWithTag(CertData, 'treC');
                CertData = NULL;
            }
        }

        FileClose(FileHandle);
    }

    if (CertData != NULL) {
        *Certificate = CertData;
        *CertificateLength = CertLength;
    } else {
        *Certificate = NULL;
        *CertificateLength = 0;
    }

    return Status;
}

/**
 * Free the certificate data that was previously retrieved via the
 * UtlGetFileCertificate routine.
 *
 * @param[in] Certificate  The resulting certificate data retrived from
 *                         calling the UtlGetFileCertificate routine.
 */
VOID
UTLAPI
UtlFreeCertificate(
    IN LPWIN_CERTIFICATE Certificate
)
{
    if (Certificate) {
        MmFreeNonPaged(Certificate);
    }
}

/**
 * Calculates the call address from a call instruction (E8 ? ? ? ?)
 *
 * @param[in] CallInstruction   The call instruction to get the address of the
 *                              called function from.
 * @return  The address of the called function.
 *          NULL if invalid.
 */
PVOID
UTLAPI
UtlCallTargetAddress(
    IN PVOID CallInstruction
)
{
    INT32 RelativeCallOffset; // has to be signed!

    if (CallInstruction != NULL) {

        // call rip+4000000h
        if (*(UINT8 *)CallInstruction == 0xE8) {
            RelativeCallOffset = *(INT32 *)((UINT8 *)CallInstruction + 1);
            return (PVOID)((UINT8 *)CallInstruction + (RelativeCallOffset + 1 + sizeof(INT32)));
        }

        // call qword ptr [rip+4000000h]
        if (*(UINT8 *)CallInstruction == 0xFF && *((UINT8 *)CallInstruction + 1) == 0x15) {
            RelativeCallOffset = *(INT32 *)((UINT8 *)CallInstruction + 2);
            return (PVOID)((UINT8 *)CallInstruction + (RelativeCallOffset + 2 + sizeof(INT32)));
        }
    }

    return NULL;
}

/**
 * Calculates the jmp target location from a jmp instruction (E9 ? ? ? ?)
 *
 * @param[in] JmpInstruction    The jmp instruction to get the address of the jmp
 *                              location from.
 * @return  The address of the jmp location. NULL if invalid.
 */
PVOID
UTLAPI
UtlJmpTargetAddress(
    IN PVOID JmpInstruction
)
{
    INT32 RelativeJmpOffset; // has to be signed!

    if (JmpInstruction != NULL) {

        if (*(UINT8 *)JmpInstruction == 0xE9 || *(UINT8 *)JmpInstruction == 0xEB) {
            RelativeJmpOffset = *(INT32 *)((UINT8 *)JmpInstruction + 1);
            return (PVOID)((UINT8 *)JmpInstruction + (RelativeJmpOffset + 1 + sizeof(INT32)));
        }
    }

    return NULL;
}

/**
 * Calculates the memory operand address location from a memory instruction.
 *
 * @param[in] MemInstruction    The memory instruction to get the address of the
 *                              memory operand location from.
 * @param[in] AdditionalBias    An additional bias added to the instruction address
 *                              when calculating the memory operand address.
 * @return  The address of the memory operand location. NULL if invalid.
 */
PVOID
UTLAPI
UtlMemOperandTargetAddress(
    IN PVOID MemInstruction,
    IN INT32 AdditionalBias
)
{
    INT32 RelativeMemoryOffset; // has to be signed!

    if (MemInstruction != NULL) {
        RelativeMemoryOffset = *(INT32 *)((UINT8 *)MemInstruction + 3);
        return (PVOID)((UINT8 *)MemInstruction + RelativeMemoryOffset +
                                                    (3 + sizeof(INT32) + AdditionalBias));
    }

    return NULL;
}

/**
 * Searches a byte pattern from a given address range WITHOUT any wilcards.
 *
 * @param[in] SearchBase    An address to start search.
 * @param[in] SearchSize    A length to search in bytes.
 * @param[in] Pattern       A byte pattern to search.
 * @param[in] PatternSize   A size of \a pattern.
 * @return  An address of the first occurrence of the patten if found,
  *         returns NULL otherwise.
 */
PVOID
UTLAPI
UtlMemMem(
    IN const VOID *SearchBase,
    IN SIZE_T SearchSize,
    IN const VOID *Pattern,
    IN SIZE_T PatternSize
)
{
    PUCHAR Base;
    SIZE_T Index;

    if (PatternSize > SearchSize) {
        return NULL;
    }

    for (Index = 0, Base = (PUCHAR)SearchBase;
         Index <= SearchSize - PatternSize;
         Index++) {
        if (RtlCompareMemory(Pattern, Base + Index, PatternSize) == PatternSize) {
            return (PVOID)(Base + Index);
        }
    }

    return NULL;
}

/**
 * Execute a memory comparison operation with an additional wildcard byte check.
 *
 * @param[in] Buf1      The first buffer to compare against the buffer in Buf2.
 * @param[in] Wildcard  The wildcard byte.
 * @param[in] Buf2      The second comparison buffer.
 * @param[in] Size      The size to compare (should be the size of Buf2)
 * @return  Returns an integral value indicating the relationship between the
 *          content of the memory blocks:
 *
 *          <0 The first byte that does not match in both memory blocks has a
 *             lower value in Buf1 than in Buf2.
 *          0  The contents of both memory blocks are equal.
 *          >0 The first byte that does not match in both memory blocks has a
 *             greater value in Buf1 than in Buf2.
 */
int
UTLAPI
UtlMemCmp(
    IN const UCHAR *Buf1,
    IN const UCHAR Wildcard,
    IN const UCHAR *Buf2,
    IN SIZE_T Size
)
{
    if (!Size) {
        return 0;
    }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4057) // differs in indirection to slightly different base types ... 
#endif
    while (--Size && ((*(CHAR *)Buf1 == *(CHAR *)Buf2) || (*(UCHAR *)Buf2 == Wildcard))) {
        Buf1 = (CHAR *)Buf1 + 1;
        Buf2 = (CHAR *)Buf2 + 1;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    return (int)(*(UCHAR *)Buf1 - *(UCHAR *)Buf2);
}

/**
 * Find a byte pattern in a specified range.
 *
 * @param[in] Pattern       The signature pattern to search for.
 * @param[in] PatternSize   The byte signature length.
 * @prama[in] Wildcard      The wildcard byte.
 * @param[in] SearchBase    The start address to search for the signature pattern.
 * @param[in] SearchSize    The size of the range to search for the signature pattern.
 * @return  The pointer to the found byte signature. NULL if not found.
 */
PVOID
UTLAPI
UtlFindPattern(
    IN CONST VOID *SearchBase,
    IN SIZE_T SearchSize,
    IN UCHAR Wildcard,
    IN CONST UCHAR *Pattern,
    IN SIZE_T PatternSize
)
{
    const UCHAR LastByte = Pattern[PatternSize - 1];
    const UCHAR *Search = SearchBase;
    const UCHAR *SearchEnd = Search + SearchSize - PatternSize;

    while (Search < SearchEnd) {
        if (Search[PatternSize - 1] == LastByte &&
            UtlMemCmp(Search, Wildcard, Pattern, PatternSize) == 0) {
            return (PVOID)Search;
        }
        ++Search;
    }

    return NULL;
}

/**
 * Find a byte pattern in a specific section of a module given by the Base.
 *
 * @param[in] Base          The base of the module to search in.
 * @param[in] SectionName   The 8-byte section tag.
 * @prama[in] Wildcard      The wildcard byte.
 * @param[in] Pattern       The signature pattern to search for.
 * @param[in] PatternSize   The byte signature length.
 * @param[out] FoundAddress The resulting pointer to the beginning of the
 *                          pattern found in the module's section.
 * @return  STATUS_SUCCESS if found.
 *          STATUS_NOT_FOUND if the pattern was not found.
 *          Or the appropriate error status code if a failure occured.
 */
NTSTATUS
UTLAPI
UtlFindPatternInSection(
    IN PVOID Base,
    IN ULONGLONG SectionName,
    IN UCHAR Wildcard,
    IN CONST UCHAR *Pattern,
    IN SIZE_T PatternSize,
    OUT PVOID *FoundAddress
)
{
    NTSTATUS Status;
    PIMAGE_NT_HEADERS64 NtHeaders;
    PIMAGE_SECTION_HEADER Section;
    USHORT SectionIndex;
    PVOID Found;

    if (!Base || !ARGUMENT_PRESENT(FoundAddress)) {
        return STATUS_INVALID_PARAMETER;
    }

    *FoundAddress = NULL;

    Status = RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, Base, 0, &NtHeaders);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Section = IMAGE_FIRST_SECTION(NtHeaders);
    SectionIndex = NtHeaders->FileHeader.NumberOfSections;
    while (SectionIndex > 0) {

        if (*((ULONGLONG*)Section->Name) == SectionName) {

            Found = UtlFindPattern((CONST VOID*)((ULONG_PTR)Base + Section->VirtualAddress),
                                   (SIZE_T)Section->Misc.VirtualSize,
                                   Wildcard,
                                   Pattern,
                                   PatternSize
                                   );
            if (Found != NULL) {
                *FoundAddress = Found;
                return STATUS_SUCCESS;
            }
        }

        SectionIndex -= 1;
        Section += 1;
    }

    return STATUS_NOT_FOUND;
}

/**
 * Find a the start or base of a function by a given pointer inside the function.
 *
 * @param[in] Ptr           The pointer inside of the function to find the start of.
 * @param[in] MaxSearchSize The maximum size to traverse backwards until the start.
 * @param[out] FoundAddress The returned found address of the start of the function.
 *                          NULL if not found.
 * @return  STATUS_SUCCESS if the function start was found.
 *          STATUS_NOT_FOUND if the function start was not found.
 *          Or the appropriate error status code if a failure occured.
 */
NTSTATUS
UTLAPI
UtlFindFunctionStartFromPtr(
    IN PVOID Ptr,
    IN SIZE_T MaxSearchSize OPTIONAL,
    OUT PVOID *FoundAddress
)
{
    PUINT8 P;
    SIZE_T Index;
    PUINT16 Pad16;
    PUINT64 Pad64;
    PUINT8 PadMisaligned;
    PVOID Found;

    if (!ARGUMENT_PRESENT(FoundAddress)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!MaxSearchSize) {
        MaxSearchSize = MAXULONG_PTR;
    }

    P = Ptr;
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
        else if (*Pad64 == 0x0000000000000000) {
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
        return STATUS_NOT_FOUND;
    }

    *FoundAddress = Found;
    return STATUS_SUCCESS;
}

/**
 * Given a symbolic link name this routine returns a string with the
 * links destination and a handle to the open link name.
 * Can only be called at PASSIVE_LEVEL.
 *
 * @NOTE Caller must close the LinkHandle!
 *
 * @param[in] SymbolicLinkName  The given symbolic link name.
 * @param[in,out] SymbolicLink  The resolved symbolic link target.
 * @param[out] LinkHandle       The resolve link handle.
 * @return  STATUS_SUCCESS on success.
 *          The appropriate error status on failure.
 */
NTSTATUS
UTLAPI
UtlGetSymbolicLink(
    IN PCUNICODE_STRING SymbolicLinkName,
    IN OUT PUNICODE_STRING SymbolicLink,
    OUT PHANDLE LinkHandle
)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN Allocated;
    UNICODE_STRING SymbolicLinkTarget;
    WCHAR SymbolicLinkTargetBuffer[MAXIMUM_FILENAME_LENGTH];
    HANDLE TempLinkHandle;
    ULONG SymbolicLinkLength;

    //
    // Open and query the symbolic link.
    //
    InitializeObjectAttributes(&ObjectAttributes,
                              (PUNICODE_STRING)SymbolicLinkName,
                               OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenSymbolicLinkObject(&TempLinkHandle, GENERIC_READ, &ObjectAttributes);
    if (Status == STATUS_SUCCESS) {

        //
        // Get the size of the symbolic link string.
        //
        SymbolicLinkLength = 0;
        Allocated = FALSE;

        RtlInitEmptyUnicodeString(&SymbolicLinkTarget,
                                  SymbolicLinkTargetBuffer,
                                  sizeof(SymbolicLinkTargetBuffer));

        Status = ZwQuerySymbolicLinkObject(TempLinkHandle,
                                           &SymbolicLinkTarget,
                                           &SymbolicLinkLength);

        if (Status == STATUS_BUFFER_TOO_SMALL && SymbolicLinkLength > 0) {

            //
            // Allocate memory for the symbolic link.
            //
            SymbolicLinkTarget.Buffer = MmAllocateNonPaged(SymbolicLinkLength);
            SymbolicLinkTarget.Length = 0;
            SymbolicLinkTarget.MaximumLength = (USHORT)SymbolicLinkLength;

            Allocated = TRUE;

            Status = ZwQuerySymbolicLinkObject(TempLinkHandle,
                                               &SymbolicLinkTarget,
                                               &SymbolicLinkLength);
        }

        if (Status == STATUS_SUCCESS) {

            if (SymbolicLink->MaximumLength < SymbolicLinkTarget.MaximumLength) {
                Status = STATUS_BUFFER_TOO_SMALL;
            } else {
                SymbolicLink->Length = SymbolicLinkTarget.Length;
                RtlCopyMemory(SymbolicLink->Buffer,
                              SymbolicLinkTarget.Buffer,
                              SymbolicLinkTarget.Length);
                *LinkHandle = TempLinkHandle;
            }
        }

        if (Allocated) {
            MmFreeNonPaged(SymbolicLinkTarget.Buffer);
        }
    }

    return Status;
}

/**
 * Closes a handle opened by the UtlGetSymbolicLink routine or the OS
 * internal ZwQuerySymbolicLinkObject API routine.
 *
 * @param[in] LinkHandle  The symbolic link handle to be closed.
 * @return  The appropriate status value. STATUS_SUCCESS on success.
 */
NTSTATUS
UTLAPI
UtlCloseSymbolicLink(
    IN HANDLE LinkHandle
)
{
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    PS_PREVIOUS_MODE_KERNEL(&PreviousMode);
    Status = NtClose(LinkHandle);
    PS_PREVIOUS_MODE_RESTORE(PreviousMode);
    return Status;
}

/**
 * Transform a given full path in one of these forms:
 *
 *      \Device\Harddisk0\Partition1\WINDOWS
 *      \Device\BootDevice\WINDOWS
 *
 * And translates it into the symoblic name for the drive so it
 * looks more like this:
 *
 *      C:\WINDOWS
 *
 * @param[in,out] Source The supplied full path to be translated to the
 *                       symbolic drive name.
 * @return  The appropriate status value.
 *          STATUS_UNSUCCESSFUL if the supplied full path is ill formed.
 *          STATUS_SUCCESS if the full path was translated successfully.
 */
static
NTSTATUS
UtlpExtractDriveString(
    IN OUT PUNICODE_STRING Source
)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG Index = 0;
    ULONG NumberOfSlashes = 0;

    while ((Index < Source->Length / sizeof(WCHAR)) && NumberOfSlashes != 4) {
        if (Source->Buffer[Index] == L'\\') {
            NumberOfSlashes++;
        }
        Index++;
    }

    //
    // Either of the following forms are acceptable:
    //
    //      \Device\Harddisk0\Partition1\WINDOWS
    //      \Device\BootDevice\WINDOWS
    //
    if ((NumberOfSlashes == 4 || NumberOfSlashes == 3) && Index > 1) {

        //
        // Simply backspace one character.
        //
        Index--;

        //
        // Keep backspacing until we find the first slash starting from 
        // the ending index.
        //
        if (NumberOfSlashes == 3) {
            while (Source->Buffer[Index] != L'\\')
                --Index;
        }

        Source->Buffer[Index] = L'\0';
        Source->Length = (USHORT)(Index * sizeof(WCHAR));
        Status = STATUS_SUCCESS;
    }

    return Status;
}

/**
 * On success this routine copies the system root path (ie C:\Windows) into the
 * given SystemRootPath parameter.
 *
 * @param[in,out] SystemRootPath  The given output buffer to copy the resolved
 *                                system root path into.
 * @return  STATUS_SUCCESS on success. The appropriate error status on failure.
 */
NTSTATUS
UTLAPI
UtlGetSystemRootPath(
    IN OUT PUNICODE_STRING SystemRootPath
)
{
    NTSTATUS Status;
    NTSTATUS ReturnStatus = STATUS_UNSUCCESSFUL;
    UNICODE_STRING SystemRootSymbolicLink1;
    WCHAR SystemRootSymbolicLink1Buffer[MAXIMUM_FILENAME_LENGTH];
    UNICODE_STRING SystemRootSymbolicLink2;
    WCHAR SystemRootSymbolicLink2Buffer[MAXIMUM_FILENAME_LENGTH];
    UNICODE_STRING SystemDosRootPath;
    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;
    HANDLE LinkHandle;
    ULONG FullPathLength;

    RtlInitEmptyUnicodeString(&SystemRootSymbolicLink1,
                              SystemRootSymbolicLink1Buffer,
                              sizeof(SystemRootSymbolicLink1Buffer));

    //
    // Get the full path for the system root directory
    //
    Status = UtlGetSymbolicLink(&UtlpSystemRootName,
                                &SystemRootSymbolicLink1,
                                &LinkHandle
                                );

    if (Status == STATUS_SUCCESS) {

        //
        // At this stage we have the full path but its in one of these forms:
        //
        //      \Device\Harddisk0\Partition1\WINDOWS
        //      \Device\BootDevice\WINDOWS
        //
        // So lets try to get the symoblic name for this drive so it looks
        // more like this:
        //
        //      C:\WINDOWS
        //
        //LOG_DEBUG("Full System Root Path: %wZ", &SystemRootSymbolicLink1);
        FullPathLength = SystemRootSymbolicLink1.Length;
        UtlCloseSymbolicLink(LinkHandle);

        //
        // Remove the path so we can query the drive letter
        //
        Status = UtlpExtractDriveString(&SystemRootSymbolicLink1);
        if (Status == STATUS_SUCCESS) {

            //
            // We've added a NULL termination character so we must reflect that in the 
            // total length.
            //
            FullPathLength = FullPathLength - sizeof(WCHAR);

            //
            // Check for overflow.
            //
            if (SystemRootPath->MaximumLength >= FullPathLength) {

                RtlInitEmptyUnicodeString(&SystemRootSymbolicLink2,
                                          SystemRootSymbolicLink2Buffer,
                                          sizeof(SystemRootSymbolicLink2Buffer)
                                          );

                //
                // Query the drive letter
                //
                Status = UtlGetSymbolicLink(&SystemRootSymbolicLink1,
                                            &SystemRootSymbolicLink2,
                                            &LinkHandle
                                            );

                if (Status == STATUS_SUCCESS) {

                    Status = IoGetDeviceObjectPointer(&SystemRootSymbolicLink2,
                                                      SYNCHRONIZE | FILE_ANY_ACCESS,
                                                      &FileObject,
                                                      &DeviceObject
                                                      );
                    if (Status == STATUS_SUCCESS) {

                        ObReferenceObject(DeviceObject);

                        //
                        // Get the dos name for the drive
                        //
                        Status = IoVolumeDeviceToDosName(DeviceObject, &SystemDosRootPath);

                        if (Status == STATUS_SUCCESS && SystemDosRootPath.Buffer != NULL) {

                            //
                            // Drive
                            //
                            RtlCopyMemory(SystemRootPath->Buffer,
                                          SystemDosRootPath.Buffer,
                                          SystemDosRootPath.Length);

                            //
                            // Drive Slash
                            //
                            RtlCopyMemory(SystemRootPath->Buffer +
                                            (SystemDosRootPath.Length / sizeof(WCHAR)),
                                          L"\\",
                                          sizeof(WCHAR));

                            //
                            // Drive Slash Directory
                            //
                            RtlCopyMemory(SystemRootPath->Buffer +
                                            (SystemDosRootPath.Length / sizeof(WCHAR)) + 1,
                                          SystemRootSymbolicLink1.Buffer +
                                            (SystemRootSymbolicLink1.Length / sizeof(WCHAR)) + 1,
                                          FullPathLength - SystemRootSymbolicLink1.Length);

                            SystemRootPath->Length = (SystemDosRootPath.Length + sizeof(WCHAR)) +
                                        ((USHORT)FullPathLength - SystemRootSymbolicLink1.Length);

                            ExFreePool(SystemDosRootPath.Buffer);

                            ReturnStatus = STATUS_SUCCESS;
                        }

                        ObDereferenceObject(DeviceObject);
                    }

                    UtlCloseSymbolicLink(LinkHandle);
                }

            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }

    return ReturnStatus;
}