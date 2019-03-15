/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file image.c
 * @author Aidan Khoury (ajkhoury)
 * @date 9/4/2018
 */

#include "image.h"
#include "file.h"
#include "loader.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4706) // assignment within conditional expression
#endif

static
PVOID
RtlpImageDirectoryEntryToData32(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    IN PIMAGE_NT_HEADERS32 NtHeaders
    );

static
PVOID
RtlpImageDirectoryEntryToData64(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    IN PIMAGE_NT_HEADERS64 NtHeaders
    );

#if defined(_MSC_VER)
#if defined(ALLOC_PRAGMA)
//#pragma alloc_text(PAGE,RtlpImageDirectoryEntryToData32)
//#pragma alloc_text(PAGE,RtlpImageDirectoryEntryToData64)
#endif // ALLOC_PRAGMA
#endif // _MSC_VER 


NTSTATUS
IMAGEAPI
RtlImageNtHeaderEx(
    IN ULONG Flags,
    IN PVOID Base,
    IN SIZE_T Size,
    OUT PIMAGE_NT_HEADERS* OutHeaders
)
{
    PIMAGE_NT_HEADERS NtHeaders = 0;
    ULONG e_lfanew = 0;
    BOOLEAN RangeCheck = 0;
    NTSTATUS Status = 0;

    if (OutHeaders != NULL) {
        *OutHeaders = NULL;
    }
    if (OutHeaders == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if ((Flags & ~(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK)) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Base == NULL || Base == (PVOID)(LONG_PTR)-1) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    RangeCheck = (BOOLEAN)((Flags & IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) == 0);
    if (RangeCheck) {
        if (Size < sizeof(IMAGE_DOS_HEADER)) {
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
    if (((PIMAGE_DOS_HEADER)Base)->e_magic != IMAGE_DOS_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto Exit;
    }

    e_lfanew = (ULONG)((PIMAGE_DOS_HEADER)Base)->e_lfanew;

    if (RangeCheck) {
        if (e_lfanew >= Size
        #define SIZEOF_PE_SIGNATURE 4
            || e_lfanew >= (MAXULONG - SIZEOF_PE_SIGNATURE - sizeof(IMAGE_FILE_HEADER))
            || (e_lfanew + SIZEOF_PE_SIGNATURE + sizeof(IMAGE_FILE_HEADER)) >= Size
            ) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    NtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)Base + e_lfanew);

    //
    // In kernelmode, do not cross from usermode address to kernelmode address.
    //
    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)NtHeaders >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }

        //
        // Note that this check is slightly overeager since IMAGE_NT_HEADERS has
        // a builtin array of data_directories that may be larger than the image
        // actually has. A better check would be to add FileHeader.SizeOfOptionalHeader,
        // after ensuring that the FileHeader does not cross the u/k boundary.
        //
        if ((PVOID)((ULONG_PTR)NtHeaders + sizeof(IMAGE_NT_HEADERS)) >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }

    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto Exit;
    }

    Status = STATUS_SUCCESS;

Exit:
    if (NT_SUCCESS(Status)) {
        *OutHeaders = NtHeaders;
    }
    return Status;
}

PIMAGE_NT_HEADERS
IMAGEAPI
RtlImageNtHeader(
    IN PVOID Base
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    (VOID)RtlImageNtHeaderEx(IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, Base, 0, &NtHeaders);
    return NtHeaders;
}


PIMAGE_SECTION_HEADER
IMAGEAPI
RtlImageSectionTableFromVirtualAddress(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    UNREFERENCED_PARAMETER(Base);

    NtSection = IMAGE_FIRST_SECTION(NtHeaders);
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) {
        if ((UINT32)Address >= NtSection->VirtualAddress &&
            (UINT32)Address < NtSection->VirtualAddress + NtSection->SizeOfRawData) {
            return NtSection;
        }
        ++NtSection;
    }

    return NULL;
}

PVOID
IMAGEAPI
RtlImageAddressInSectionTable(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
)
{
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = RtlImageSectionTableFromVirtualAddress(NtHeaders, Base, Address);
    if (NtSection != NULL) {
        return (PVOID)((ULONG_PTR)Base +
            ((ULONG_PTR)Address - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData);
    }

    return NULL;
}

static
PVOID
RtlpImageDirectoryEntryToData32(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    IN PIMAGE_NT_HEADERS32 NtHeaders
)
{
    ULONG DirectoryAddress;

    if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        return NULL;
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[DirectoryEntry].VirtualAddress)) {
        return NULL;
    }

    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)((ULONG_PTR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
            return NULL;
        }
    }

    *Size = NtHeaders->OptionalHeader.DataDirectory[DirectoryEntry].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return (PVOID)((ULONG_PTR)Base + DirectoryAddress);
    }

    return RtlImageAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress);
}

static
PVOID
RtlpImageDirectoryEntryToData64(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    IN PIMAGE_NT_HEADERS64 NtHeaders
)
{
    ULONG DirectoryAddress;

    if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        return NULL;
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[DirectoryEntry].VirtualAddress)) {
        return NULL;
    }

    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)((ULONG_PTR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
            return NULL;
        }
    }

    *Size = NtHeaders->OptionalHeader.DataDirectory[DirectoryEntry].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return (PVOID)((ULONG_PTR)Base + DirectoryAddress);
    }

    return RtlImageAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress);
}

PVOID
IMAGEAPI
RtlImageDirectoryEntryToData(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
)
{
    PIMAGE_NT_HEADERS NtHeaders;

    if (LDR_IS_DATAFILE(Base)) {
        Base = LDR_DATAFILE_TO_VIEW(Base);
        MappedAsImage = FALSE;
    }

    NtHeaders = RtlImageNtHeader(Base);
    if (!NtHeaders) {
        return NULL;
    }

    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

        return RtlpImageDirectoryEntryToData32(Base,
                                               MappedAsImage,
                                               DirectoryEntry,
                                               Size,
                                               (PIMAGE_NT_HEADERS32)NtHeaders
                                               );

    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

        return RtlpImageDirectoryEntryToData64(Base,
                                               MappedAsImage,
                                               DirectoryEntry,
                                               Size,
                                               (PIMAGE_NT_HEADERS64)NtHeaders
                                               );
    } else {

        return NULL;
    }
}

#if !defined(NTOS_KERNEL_RUNTIME) && !defined(BLDR_KERNEL_RUNTIME)

PIMAGE_SECTION_HEADER
IMAGEAPI
RtlImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
)
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    UNREFERENCED_PARAMETER(Base);

    NtSection = IMAGE_FIRST_SECTION(NtHeaders);
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData) {
            return NtSection;
        }
        ++NtSection;
    }

    return NULL;
}

PVOID
IMAGEAPI
RtlImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
)
{
    PIMAGE_SECTION_HEADER NtSection;

    if (!ARGUMENT_PRESENT(LastRvaSection) ||
        (NtSection = *LastRvaSection) == NULL ||
        Rva < NtSection->VirtualAddress ||
        Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData) {
        NtSection = RtlImageRvaToSection(NtHeaders, Base, Rva);
    }

    if (NtSection != NULL) {
        if (LastRvaSection != NULL) {
            *LastRvaSection = NtSection;
        }

        return (PVOID)((ULONG_PTR)Base +
                            (Rva - NtSection->VirtualAddress) +
                                                    NtSection->PointerToRawData);
    }

    return NULL;
}

#endif // !NTOS_KERNEL_RUNTIME && !BLDR_KERNEL_RUNTIME

static
NTSTATUS
MapCertificate(
    IN PMAPPED_IMAGE MappedImage,
    IN ULONG DataDirectoryVa,
    IN LONG SizeAdjustment,
    OUT LPWIN_CERTIFICATE *Certificate,
    OUT PULONG CertificateSize
)
{
    NTSTATUS Status;
    PLOADED_IMAGE LoadedImage;
    PVOID ViewBase;
    ULONG NumberOfBytesToMap;
    LARGE_INTEGER DisatanceToSeek, SectionSize;
    HANDLE NewFileMapping;
    PVOID NewMappedBase;
    LARGE_INTEGER FileOffset;
    PVOID CertificateMappedView;

    ViewBase = MappedImage->ViewBase;

    if (ViewBase) {
        if (!LoadedImage->fReadOnly) {
            FileFlushView(ViewBase, MappedImage->ViewSize);
            ViewBase = MappedImage->ViewBase;
        }
        FileUnmapView(ViewBase);
        MappedImage->ViewBase = NULL;
    }

    if (SizeAdjustment != 0) {
        if (LoadedImage->fReadOnly) {
            return STATUS_INVALID_PARAMETER;
        }

        FileFlushView(LoadedImage->MappedAddress, MappedImage->NumberOfBytesMapped);
        FileUnmapView(LoadedImage->MappedAddress);
        LoadedImage->MappedAddress = NULL;

        FileClose(MappedImage->FileMapping);
        MappedImage->FileMapping = NULL;

        LoadedImage->SizeOfImage += SizeAdjustment;

        NumberOfBytesToMap = 0xA00000;
        if (LoadedImage->SizeOfImage < 0xA00000)
            NumberOfBytesToMap = LoadedImage->SizeOfImage;

        MappedImage->NumberOfBytesMapped = NumberOfBytesToMap;

        DisatanceToSeek.QuadPart = LoadedImage->SizeOfImage;
        FileSeek(LoadedImage->hFile, &DisatanceToSeek, FILE_BEGIN);
        FileSetEnd(LoadedImage->hFile);

        SectionSize.QuadPart = LoadedImage->SizeOfImage;
        Status = FileCreateMapping(LoadedImage->hFile,
                                   NULL,
                                   PAGE_READWRITE,
                                   &SectionSize,
                                   NULL,
                                   &NewFileMapping);

        MappedImage->FileMapping = NewFileMapping;

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        Status = FileMapView(MappedImage->FileMapping,
                             FILE_MAP_WRITE,
                             NULL,
                             MappedImage->NumberOfBytesMapped,
                             NULL,
                             &NewMappedBase);

        LoadedImage->MappedAddress = NewMappedBase;

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        Status = FileCalculateImagePtrs(MappedImage, MappedImage->NumberOfBytesMapped);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    MappedImage->CertDataDirectoryVa = DataDirectoryVa - DataDirectoryVa % MappedImage->AllocationGranularity;

    NumberOfBytesToMap = LoadedImage->SizeOfImage - MappedImage->CertDataDirectoryVa;
    MappedImage->ViewSize = NumberOfBytesToMap;

    FileOffset.QuadPart = MappedImage->CertDataDirectoryVa;
    Status = FileMapView(MappedImage->FileMapping,
                         LoadedImage->fReadOnly != 0 ? PAGE_READWRITE : PAGE_READONLY,
                         &FileOffset,
                         NumberOfBytesToMap,
                         NULL,
                         &CertificateMappedView);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *Certificate = (LPWIN_CERTIFICATE)((ULONG_PTR)CertificateMappedView + DataDirectoryVa - MappedImage->CertDataDirectoryVa);
    *CertificateSize = MappedImage->CertDataDirectoryVa + (ULONG)MappedImage->ViewSize - DataDirectoryVa;

    return Status;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4701) // potentially uninitialized local variable
#endif
static
BOOLEAN
FindCertificate(
    IN PMAPPED_IMAGE MappedImage,
    IN ULONG Index,
    OUT LPWIN_CERTIFICATE *Certificate
)
{
    PLOADED_IMAGE LoadedImage;
    PIMAGE_DATA_DIRECTORY DataDirectory;
    ULONG_PTR CurrentCert;
    ULONG_PTR LastCert;
    ULONG CurrentIdx;
    BOOLEAN rc;

    LoadedImage = MappedImage->LoadedImage;

    if (LoadedImage->fDOSImage) {
        // No way this could have a certificate.
        return FALSE;
    }

    rc = FALSE;

    if (LoadedImage->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        DataDirectory = &((PIMAGE_NT_HEADERS32)(LoadedImage->FileHeader))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
    } else if (LoadedImage->FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        DataDirectory = &((PIMAGE_NT_HEADERS64)(LoadedImage->FileHeader))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
    } else {
        goto Exit; // Not an interesting file type.
    }

    //
    // Check if the cert pointer is at least reasonable.
    //
    if (!DataDirectory->VirtualAddress ||
        !DataDirectory->Size ||
        (DataDirectory->VirtualAddress + DataDirectory->Size > LoadedImage->SizeOfImage)) {
        goto Exit;
    }

    //
    // We're not looking at an empty security slot or an invalid (past the image boundary) value.
    // Let's see if we can find it.
    //
    CurrentIdx = 0;
    CurrentCert = (ULONG_PTR)(LoadedImage->MappedAddress + DataDirectory->VirtualAddress);
    LastCert = CurrentCert + DataDirectory->Size;

    while (CurrentCert < LastCert) {
        if (CurrentIdx == Index) {
            rc = TRUE;
            break;
        }
        CurrentIdx++;
        CurrentCert += ((LPWIN_CERTIFICATE)CurrentCert)->dwLength;
        CurrentCert = (CurrentCert + 7) & ~7;   // align it.
    }

Exit:
    if (rc == TRUE) {
        *Certificate = (LPWIN_CERTIFICATE)CurrentCert;
    }

    return rc;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

NTSTATUS
IMAGEAPI
ImageGetCertificateData(
    IN HANDLE FileHandle,
    IN ULONG CertificateIndex,
    OUT LPWIN_CERTIFICATE Certificate,
    IN OUT PULONG RequiredLength
)
{
    NTSTATUS Status;
    MAPPED_IMAGE MappedImage;
    LOADED_IMAGE LoadedImage;
    LPWIN_CERTIFICATE ImageCert;

    MappedImage.LoadedImage = &LoadedImage;

    Status = FileMap(FileHandle, &MappedImage, TRUE);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (FindCertificate(&MappedImage, CertificateIndex, &ImageCert)) {
        if (*RequiredLength < ImageCert->dwLength) {
            *RequiredLength = ImageCert->dwLength;
            Status = STATUS_BUFFER_OVERFLOW;
        } else {
            RtlCopyMemory(Certificate, ImageCert, ImageCert->dwLength);
        }
    } else {
        Status = STATUS_NOT_FOUND;
    }

    FileUnmap(&MappedImage);

    return Status;
}

NTSTATUS
IMAGEAPI
ImageGetCertificateHeader(
    IN HANDLE FileHandle,
    IN ULONG CertificateIndex,
    IN OUT LPWIN_CERTIFICATE CertificateHeader
)
{
    NTSTATUS Status;
    MAPPED_IMAGE MappedImage;
    LOADED_IMAGE LoadedImage;
    LPWIN_CERTIFICATE ImageCert;

    MappedImage.LoadedImage = &LoadedImage;

    Status = FileMap(FileHandle, &MappedImage, TRUE);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (FindCertificate(&MappedImage, CertificateIndex, &ImageCert)) {
        RtlCopyMemory(CertificateHeader, ImageCert, sizeof(WIN_CERTIFICATE));
    } else {
        Status = STATUS_NOT_FOUND;
    }

    FileUnmap(&MappedImage);

    return Status;
}



#if defined(_MSC_VER)
#pragma warning(pop)
#endif