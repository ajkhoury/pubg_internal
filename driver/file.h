/**
 * Blackout Driver
 * Copyright (c) 2018-2019 Aidan Khoury. All rights reserved.
 *
 * @file file.h
 * @author Aidan Khoury (ajkhoury)
 * @date 9/4/2018
 */

#ifndef _BLACKOUT_DRIVER_FILE_H_
#define _BLACKOUT_DRIVER_FILE_H_
#if defined(_MSC_VER)
#pragma once
#endif

#include "ntapi.h"

#define FILEAPI NTAPI


// Observant readers may notice that 2 new fields,
// 'fReadOnly' and 'Version' have been added to
// the LOADED_IMAGE structure after 'fDOSImage'.
// This does not change the size of the structure
// from previous headers.  That is because while
// 'fDOSImage' is a byte, it is padded by the
// compiler to 4 bytes.  So the 2 new fields are
// slipped into the extra space.

typedef struct _LOADED_IMAGE {
    PSTR                  ModuleName;
    HANDLE                hFile;
    PUCHAR                MappedAddress;
    PIMAGE_NT_HEADERS     FileHeader;
    PIMAGE_SECTION_HEADER LastRvaSection;
    ULONG                 NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;
    ULONG                 Characteristics;
    BOOLEAN               fSystemImage;
    BOOLEAN               fDOSImage;
    BOOLEAN               fReadOnly;
    UCHAR                 Version;
    LIST_ENTRY            Links;
    ULONG                 SizeOfImage;
} LOADED_IMAGE, *PLOADED_IMAGE;

typedef struct _MAPPED_IMAGE {
    PLOADED_IMAGE         LoadedImage;
    ULONG                 AllocationGranularity;
    HANDLE                FileMapping;
    ULONG                 NumberOfBytesMapped;
    ULONG                 CertDataDirectoryVa;
    PVOID                 ViewBase;
    SIZE_T                ViewSize;
    ULONG_PTR             Unknown;
} MAPPED_IMAGE, *PMAPPED_IMAGE;

NTSTATUS
FILEAPI
FileInitialize(
    IN PVOID KernelBase
    );

BOOLEAN
FILEAPI
FileExists(
    IN PUNICODE_STRING FilePath
    );

NTSTATUS
FILEAPI
FileCreate(
    IN PUNICODE_STRING FilePath,
    IN ULONG DesiredAccess,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    OUT PHANDLE Handle
    );

NTSTATUS
FILEAPI
FileOpen(
    IN PUNICODE_STRING FilePath,
    IN ULONG DesiredAccess,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions,
    OUT PHANDLE Handle
    );

NTSTATUS
FILEAPI
FileClose(
    IN HANDLE FileHandle
    );

NTSTATUS
FILEAPI
FileRead(
    IN HANDLE FileHandle,
    IN ULONG Offset,
    OUT PUCHAR Buffer,
    IN ULONG Size,
    OUT PULONG ReturnSize
    );

NTSTATUS
FILEAPI
FileGetSize(
    IN HANDLE FileHandle,
    OUT PLARGE_INTEGER FileSize
    );

NTSTATUS
FILEAPI
FileCreateMapping(
    IN HANDLE FileHandle OPTIONAL,
    IN PSECURITY_ATTRIBUTES SecurityAttributes OPTIONAL,
    IN ULONG ProtectionFlags,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN CONST WCHAR *Name OPTIONAL,
    OUT PHANDLE SectionMapping
    );

NTSTATUS
FILEAPI
FileMapView(
    IN HANDLE FileMappingHandle,
    IN ULONG DesiredAccess,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN SIZE_T NumberOfBytesToMap,
    IN PVOID BaseAddress OPTIONAL,
    OUT PVOID *ViewBaseAddress
    );

NTSTATUS
FILEAPI
FileFlushView(
    IN PVOID ViewBaseAddress,
    IN SIZE_T NumberOfBytesToFlush
    );

NTSTATUS
FILEAPI
FileUnmapView(
    IN PVOID ViewBaseAddress
    );

NTSTATUS
FILEAPI
FileMap(
    IN HANDLE FileHandle,
    OUT PMAPPED_IMAGE MappedImage,
    IN BOOLEAN ReadOnly
    );

NTSTATUS
FILEAPI
FileCalculateImagePtrs(
    IN PMAPPED_IMAGE MappedImage,
    IN ULONG NumberOfBytesMapped
    );

NTSTATUS
FILEAPI
FileUnmap(
    IN PMAPPED_IMAGE MappedImage
    );

#define FILE_BEGIN 0 // The starting point is zero or the beginning of the file.
#define FILE_CURRENT 1 // The starting point is the current value of the file pointer.
#define FILE_END 2 // The starting point is the current end-of-file position.

NTSTATUS
FILEAPI
FileSeek(
    IN HANDLE FileHandle,
    IN PLARGE_INTEGER DistanceToSeek,
    IN ULONG SeekMethod
    );

NTSTATUS
FILEAPI
FileSetEnd(
    IN HANDLE FileHandle
    );

#endif // _BLACKOUT_DRIVER_FILE_H_