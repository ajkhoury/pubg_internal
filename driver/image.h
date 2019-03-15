/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file image.h
 * @author Aidan Khoury (ajkhoury)
 * @date 9/4/2018
 */

#ifndef _BLACKOUT_DRIVER_IMAGE_H_
#define _BLACKOUT_DRIVER_IMAGE_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

// Image API calling convention
#define IMAGEAPI  NTAPI

NTSTATUS
IMAGEAPI
RtlImageNtHeaderEx(
    IN ULONG Flags,
    IN PVOID Base,
    IN SIZE_T Size,
    OUT PIMAGE_NT_HEADERS* OutHeaders
    );

#define IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK (0x00000001)

PIMAGE_NT_HEADERS
IMAGEAPI
RtlImageNtHeader(
    IN PVOID Base
    );

PIMAGE_SECTION_HEADER
IMAGEAPI
RtlImageSectionTableFromVirtualAddress(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    );

PVOID
IMAGEAPI
RtlImageAddressInSectionTable(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    );

PVOID
IMAGEAPI
RtlImageDirectoryEntryToData(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

PIMAGE_SECTION_HEADER
IMAGEAPI
RtlImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    );

PVOID
IMAGEAPI
RtlImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    );

NTSTATUS
IMAGEAPI
ImageGetCertificateData(
    IN HANDLE FileHandle,
    IN ULONG CertificateIndex,
    OUT LPWIN_CERTIFICATE Certificate,
    IN OUT PULONG RequiredLength
    );

NTSTATUS
IMAGEAPI
ImageGetCertificateHeader(
    IN HANDLE FileHandle,
    IN ULONG CertificateIndex,
    IN OUT LPWIN_CERTIFICATE CertificateHeader
    );


#endif // _BLACKOUT_DRIVER_IMAGE_H_