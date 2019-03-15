#pragma once

#include "native.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

///
/// < RTL Heap API >
///

#define RtlProcessHeap() \
    (NtCurrentPeb()->ProcessHeap)

PVOID
NTAPI
RtlGetProcessHeap(
    VOID
    );

NTSYSAPI
PVOID
NTAPI
RtlDestroyHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
PVOID
NTAPI
RtlAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags OPTIONAL,
    IN SIZE_T Size
    );

NTRTLAPI
BOOLEAN
NTAPI
RtlFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags OPTIONAL,
    IN PVOID BaseAddress
    );

NTSYSAPI
SIZE_T
NTAPI
RtlSizeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlZeroHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSYSAPI
VOID
NTAPI
RtlProtectHeap(
    IN PVOID HeapHandle,
    IN BOOLEAN MakeReadOnly
    );


///
/// < RTL String API >
///

/**
 * Initializes a counted string of Unicode characters.
 *
 * @param[in,out]   DestinationString   A pointer to the UNICODE_STRING structure to be initialized.
 * @param[in]       SourceString        A pointer to a null-terminated wide-character string.
 *                                      This string is used to initialize the counted string pointed to by DestinationString.
 */
VOID
NTAPI
RtlInitUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    );

/**
 * Initializes an empty counted Unicode string.
 *
 * @param[out] DestinationString    Pointer to the UNICODE_STRING structure to be initialized.
 * @param[in] Buffer                Pointer to a caller-allocated buffer to be used to contain a WCHAR string.
 * @param[in] BufferSize            Length, in bytes, of the buffer that Buffer points to.
 */       
FORCEINLINE
VOID
RtlInitEmptyUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN PWCHAR Buffer,
    IN USHORT BufferSize
)
{
    memset(UnicodeString, 0, sizeof(*UnicodeString));
    UnicodeString->MaximumLength = BufferSize;
    UnicodeString->Buffer = Buffer;
}

/**
 * Initializes an empty counted ANSI string.
 *
 * @param[out] DestinationString    Pointer to the ANSI_STRING structure to be initialized.
 * @param[in] Buffer                Pointer to a caller-allocated buffer to be used to contain a CHAR string.
 * @param[in] BufferSize            Length, in bytes, of the buffer that Buffer points to.
 */
FORCEINLINE
VOID
RtlInitEmptyAnsiString( 
    OUT PANSI_STRING AnsiString, 
    IN PCHAR Buffer, 
    IN USHORT BufferSize 
)
{
    AnsiString->Length = 0;
    AnsiString->MaximumLength = BufferSize;
    AnsiString->Buffer = Buffer;
}

/**
 * Initializes a counted string of Unicode characters.
 *
 * @param[in,out]   DestinationString   A pointer to the UNICODE_STRING structure to be initialized.
 * @param[in]       SourceString        A pointer to a null-terminated wide-character string.
 *                                      This string is used to initialize the counted string pointed to by DestinationString.
 */
NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString OPTIONAL
    );

/**
 * Frees the string buffer that was previously allocated on the heap.
 *
 * @param[in,out] UnicodeString  A pointer to the Unicode string whose buffer was previously allocated
 */
VOID
NTAPI
RtlFreeUnicodeString(
    IN OUT PUNICODE_STRING UnicodeString
    );

/**
 * Copies a source string to a destination string.
 *
 * @param[in,out]   DestinationString    pointer to the destination string buffer.
 * @param[in]       SourceString         pointer to the source string buffer.
 */
VOID
NTAPI
RtlCopyUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString OPTIONAL
    );

/**
 * Concatenates two Unicode strings.
 *
 * @param[in,out] Destination    Pointer to a buffered Unicode string.
 * @param[in]     Source         Pointer to the buffered string to be concatenated.
 *
 * @return Status value.
 */
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString(
    IN OUT PUNICODE_STRING Destination,
    IN PCUNICODE_STRING Source
    );

/**
 * Converts the specified Unicode character to uppercase.
 *
 * @param[in] SourceCharacter    Specifies the character to convert.
 *
 * @return The uppercase value of the Unicode character.
 */
FORCEINLINE
WCHAR
NTAPI
RtlUpcaseUnicodeChar(
    IN WCHAR SourceCharacter
)
{
    return ((SourceCharacter >= L'a') && (SourceCharacter <= L'z')) ?
        (SourceCharacter - L'a' + L'A') : SourceCharacter;
}

/**
 * Converts the specified Unicode character to lowercase
 *
 * @param[in] SourceCharacter   Specifies the character to convert.
 *
 * @return The lowercase value of the Unicode character.
 */
FORCEINLINE
WCHAR
NTAPI
RtlDowncaseUnicodeChar(
    IN WCHAR SourceCharacter
)
{
    return ((SourceCharacter >= L'A') && (SourceCharacter <= L'Z')) ?
        (SourceCharacter - L'A' + L'a') : SourceCharacter;
}

/**
 * Compares two Unicode strings.
 *
 * @param[in] String1           Pointer to the first string.
 * @param[in] Length1           Length of the first string.
 * @param[in] String2           Pointer to the second string.
 * @param[in] Length2           Length of the second string.
 * @param[in] CaseInSensitive   If TRUE, case should be ignored when doing the comparison.
 *
 * @return A signed value that gives the results of the comparison.
 */
LONG
NTAPI
RtlCompareUnicodeStrings(
    IN CONST PWCHAR String1,
    IN UINT32 Length1,
    IN CONST PWCHAR String2,
    IN UINT32 Length2,
    IN BOOLEAN CaseInSensitive
    );

/**
 * Compares two Unicode strings.
 *
 * @param[in] String1            Pointer to the first string.
 * @param[in] String2            Pointer to the second string.
 * @param[in] CaseInSensitive    If TRUE, case should be ignored when doing the comparison.
 *
 * @return A signed value that gives the results of the comparison.
 */
FORCEINLINE
LONG
NTAPI
RtlCompareUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
)
{
    return RtlCompareUnicodeStrings(String1->Buffer,
                                    String1->Length,
                                    String2->Buffer,
                                    String2->Length,
                                    CaseInSensitive
                                    );
}

/**
 * Compares two Unicode strings to determine whether they are equal.
 *
 * @param[in] String1            Pointer to the first Unicode string.
 * @param[in] String2            Pointer to the second Unicode string.
 * @param[in] CaseInSensitive    If TRUE, case should be ignored when doing the comparison.
 *
 * @return Returns TRUE if the two Unicode strings are equal; otherwise, it returns FALSE.
 */
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

/**
 * Creates a hash value from a given Unicode string and hash algorithm.
 *
 * @param[in] String             A pointer to a UNICODE_STRING structure that contains the Unicode string to be converted to a hash value.
 * @param[in] CaseInSensitive    If CaseInSensitive is TRUE, a lowercase and uppercase string hash to the same value.
 * @param[in] HashAlgorithm      The hash algorithm to use.
 * @param[out] HashValue         A pointer to a 32 bit variable that receives the hash value.
 *
 * @return Returns STATUS_SUCCESS on success, or the appropriate NTSTATUS value on failure.
 */
NTSTATUS
NTAPI
RtlHashUnicodeString(
    IN PCUNICODE_STRING String,
    IN BOOLEAN CaseInSensitive,
    IN UINT32 HashAlgorithm,
    OUT UINT32 *HashValue
    );
#define HASH_STRING_ALGORITHM_DEFAULT  (0)
#define HASH_STRING_ALGORITHM_X65599   (1)
#define HASH_STRING_ALGORITHM_INVALID  (0xFFFFFFFF)

/**
 * Determines how many bytes are needed to represent a Unicode string as an ANSI string.
 *
 * @param[out] BytesInMultiByteString   Returns the number of bytes for the ANSI equivalent of the Unicode string pointed to by UnicodeString.
 * @param[in] UnicodeString             The Unicode source string for which the ANSI length is calculated.
 * @param[in] BytesInUnicodeString      The number of bytes in the string pointed to by UnicodeString.
 *
 * @return STATUS_SUCCESS if the count was successful. Otherwise the appropriate status value is returned.
 */
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
    OUT PULONG BytesInMultiByteString,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

/**
 * Calculates the number of bytes required for a null-terminated ANSI string that is equivalent to a 
 * specified Unicode string.
 *
 * @param[in] UnicodeString  Pointer to the Unicode string for which to compute the number of bytes required 
 *                           for an equivalent null-terminated ANSI string.
 *
 * @return  The number of bytes required for an equivalent null-terminated ANSI string.
 */
ULONG
NTAPI
RtlUnicodeStringToAnsiSize(
    IN PUNICODE_STRING UnicodeString
    );

/**
 * Calculates the number of bytes that are required for a null-terminated
 * Unicode string that is equivalent to a specified ANSI string.
 *
 * @param[in] AnsiString    Pointer to the ANSI string for which to compute
 *                          the number of bytes that are required for an
 *                          equivalent null-terminated Unicode string.
 *
 * @return  The number of bytes that are required for an equivalent
 *          null-terminated Unicode string, if the ANSI string can be
 *          translated into an Unicode string by using the current system
 *          locale information. Otherwise, this routine returns zero.
 */
UINT32
NTAPI
RtlAnsiStringToUnicodeSize(
    IN PCANSI_STRING AnsiString
    );

/**
 * Translates the specified Unicode string into a new ANSI character string.
 *
 * @param[out] MultiByteString           Pointer to a caller-allocated buffer to receive
 *                                       the translated string. 
 *
 * @param[in] MaxBytesInMultiByteString Maximum number of bytes to be written to MultiByteString.
 *
 * @param[out] BytesInMultiByteString    Pointer to a caller-allocated variable that receives
 *                                       the length, in bytes, of the translated string.
 *
 * @param[in] UnicodeString              Pointer to the Unicode source string to be translated.
 *
 * @param[in] BytesInUnicodeString       Size, in bytes, of the string at UnicodeString.
 *
 * @return  STATUS_SUCCESS if the translation was successful.
 *          Otherwise the appropriate status value is returned.
 */
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
    OUT PCHAR MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PCWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

/**
 * This functions converts the specified ansi source string into a Unicode string.
 *
 * @param[out] UnicodeString            Returns a unicode string that is equivalent to
 *                                      the ansi source string.
 *
 * @param[in] MaxBytesInUnicodeString   Supplies the maximum number of bytes to be written
 *                                      to UnicodeString. If this causes UnicodeString to
 *                                      be a  truncated equivalent of MultiByteString, no
 *                                      error condition results.
 *
 * @param[out] BytesInUnicodeString     Returns the number of bytes in the returned
 *                                      unicode string pointed to by UnicodeString.
 *
 * @param[in] MultiByteString           Supplies the ansi source string that is to be
 *                                      converted to unicode. For single-byte character
 *                                      sets, this address CAN be the same as UnicodeString.
 *
 * @param[in] BytesInMultiByteString    The number of bytes in the string pointed to
 *                                      by MultiByteString.
 *
 * @return  STATUS_SUCCESS if the count was successful.
 *          Otherwise the appropriate status value is returned.
 */
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
    OUT WCHAR *UnicodeString,
    IN UINT32 MaxBytesInUnicodeString,
    OUT UINT32 *BytesInUnicodeString OPTIONAL,
    IN CONST CHAR *MultiByteString,
    IN UINT32 BytesInMultiByteString
    );

/**
 * @brief Converts the specified Unicode source string into an ANSI string.
 *
 * @param[in,out] DestinationString      A pointer to an ANSI_STRING structure to hold the converted ANSI string.
 * @param[in] SourceString               The UNICODE_STRING structure that contains the source string to be converted to ANSI.
 * @param[in] AllocateDestinationString  Controls allocation of the buffer space for the DestinationString.
 *
 * @return STATUS_SUCCESS if the Unicode string was converted to ANSI. Otherwise the appropriate status value is returned.
 */
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    IN OUT PANSI_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

/**
 * This functions converts the specified ansi source string into a
 * Unicode string. The translation is done with respect to the
 * current system locale information.
 *
 * @param DestinationString         Returns a unicode string that is equivalent to
 *                                  the ansi source string. The maximum length field is
 *                                  only set if AllocateDestinationString is TRUE.
 *
 * @param SourceString              Supplies the ansi source string that is to be converted
 *                                  to unicode.
 *
 * @param AllocateDestinationString Supplies a flag that controls whether or
 *                                  not this API allocates the buffer space for the destination
 *                                  string.  If it does, then the buffer must be deallocated using
 *                                  RtlFreeUnicodeString (note that only storage for
 *                                  DestinationString->Buffer is allocated by this API).
 *
 * @return  SUCCESS if the conversion was successful
 *
 *          !SUCCESS if the operation failed. No storage was allocated and no
 *              conversion was done.  None.
 */
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCANSI_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );


///
/// < RTL Error API >
///

ULONG
WINAPI
RtlSetLastNTError(
    IN NTSTATUS NtError
    );

FORCEINLINE
VOID
NTAPI
RtlSetLastWin32Error(
    IN ULONG LastError
)
{
    NtCurrentTeb()->LastErrorValue = LastError;
}

FORCEINLINE
ULONG
NTAPI
RtlGetLastWin32Error(
    VOID
)
{
    return NtCurrentTeb()->LastErrorValue;
}

FORCEINLINE
BOOLEAN
NTAPI
RtlpCheckForActiveDebugger(
    VOID
)
{
    // Return the flag in the PEB
    return NtCurrentPeb()->BeingDebugged;
}

BOOLEAN
NTAPI
RtlDispatchException(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT Context
    );

VOID
NTAPI
RtlRaiseStatus(
    IN NTSTATUS Status
    );

NTRTLAPI
ULONG
NTAPI
RtlNtStatusToDosError(
    IN NTSTATUS Status
    );


///
/// < RTL AVL Table API >
///

NTSYSAPI
VOID
RtlInitializeGenericTable(
    PRTL_GENERIC_TABLE            Table,
    PRTL_GENERIC_COMPARE_ROUTINE  CompareRoutine,
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_FREE_ROUTINE     FreeRoutine,
    PVOID                         TableContext
    );



///
/// < RTL Path API >
///

NTRTLAPI // Imported cause fuck this implementation!
ULONG
NTAPI
RtlDosSearchPath_U(
    IN PCWSTR Path,
    IN PCWSTR FileName,
    IN PCWSTR Extension,
    IN ULONG Size,
    IN PWSTR Buffer,
    OUT PWSTR *PartName
    );


///
/// < RTL Image API >
///

/**
 * Validate and return the NT header for the given base of image.
 *
 * @param[in] Flags         Flags for validating the NT header.
 * @param[in] BaseOfImage   The base address of the image to get the NT header for.       
 * @param[in] Size          The size of the image.
 * @param[out] Headers      The pointer to the resulting NT header.
 *
 * @return Status value.
 */
NTSTATUS
NTAPI
RtlImageNtHeaderEx(
    IN UINT32 Flags,
    IN VOID *BaseOfImage,
    IN UINT64 Size,
    OUT PPE_NT_HEADERS* Headers
    );
// RtlImageNtHeaderEx Flags
#ifndef RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK
#define RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK 0x00000001
#endif

/**
 * Validate and return the NT header for the given base of image.
 *
 * @param[in] BaseOfImage   The base address of the image to get the NT header for.
 *
 * @return The pointer to the resulting NT header.
 */
PPE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    IN VOID *BaseOfImgae
    );

/**
 * Find a section header by a given RVA.
 *
 * @param[in] NtHeader       The NT header of the image to find the section in.
 * @param[in] BaseAddress    The base address of the image.
 * @param[in] Rva            The relative virtual address that is inside the section to find.
 *
 * @return  The pointer to the found section header.
 *          NULL if not found.
 */
PPE_SECTION_HEADER
NTAPI
RtlImageRvaToSection(
    IN CONST PE_NT_HEADERS *NtHeader,
    IN VOID *BaseAddress,
    IN UINT32 Rva
    );

/**
 * Find a section header by its given name.
 *
 * @param[in] NtHeader      The NT header of the image to find the section in.
 * @param[in] BaseAddress   The base address of the image.
 * @param[in] SectionName   The name of the section to find.
 *
 * @return  The pointer to the found section header.
 *          NULL if not found.
 */
PPE_SECTION_HEADER
NTAPI
RtlImageFindSectionByName(
    IN CONST PE_NT_HEADERS *NtHeader,
    IN VOID *BaseOfImage,
    IN CONST CHAR *SectionName
    );

/**
 * Locates a relative virtual address (RVA) within the image header of a file 
 * that is mapped as a file and returns the virtual address of the corresponding byte in the file.
 *
 * @param NtHeader       A pointer to an IMAGE_NT_HEADERS structure. 
 * @param BaseAddress    The base address of an image that is mapped into memory through a call to the MapViewOfFile function.
 * @param Rva            The relative virtual address to be located.
 * @param SectionHeader  A pointer to an IMAGE_SECTION_HEADER structure that specifies the last RVA section.
 *
 * @return  If the function succeeds, the return value is the virtual address in the mapped file.
 */
VOID*
NTAPI
RtlImageRvaToVa(
    IN CONST PE_NT_HEADERS *NtHeader,
    IN VOID* BaseAddress,
    IN UINT32 Rva,
    IN OUT PE_SECTION_HEADER **SectionHeader
    );

/**
 * Obtains access to image-specific data.
 *
 * @param[in] BaseAddress       A pointer to the base address of the image. The 'MZ' signature.
 * @param[in] MappedAsImage     If this parameter is TRUE, the file is mapped by the system as an image. 
 *                              If the flag is FALSE, the file is mapped as a data file by the MapViewOfFile.
 * @param[in] Directory         The index number of the desired directory entry.
 * @param[out] Size             A pointer to a variable that receives the size of the data for the directory entry, in bytes.
 *
 * @return  If the function succeeds, the return value is a pointer to the directory entry's data.
 */
VOID*
NTAPI
RtlImageDirectoryEntryToData(
    IN VOID *BaseAddress,
    IN BOOLEAN MappedAsImage,
    IN UINT16 Directory,
    OUT UINT32 *Size OPTIONAL
    );


#ifdef __cplusplus
}
#endif // __cplusplus