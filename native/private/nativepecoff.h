/**
 * EfiBootkit
 * Copyright (c) 2017 Aidan Khoury. All rights reserved.
 *
 * @file pecoff.h
 * @author Aidan Khoury (dude719)
 * @date 10/29/2017
 */

#ifndef _NATIVE_PECOFF_H_
#define _NATIVE_PECOFF_H_

#include "nativecommon.h"


/**
 * Image Format
 */

#ifndef _MAC

#pragma pack(push, 4)				// 4 byte packing is the default

#define PE_DOS_SIGNATURE                    0x5A4D      //! MZ
#define PE_OS2_SIGNATURE                    0x454E      //! NE
#define PE_OS2_SIGNATURE_LE                 0x454C      //! LE
#define PE_VXD_SIGNATURE                    0x454C      //! LE
#define PE_NT_SIGNATURE	                    0x00004550  //! PE00

#define SIZEOF_PE_SIGNATURE                 4

#pragma pack(push, 2)				// 16 bit headers are 2 byte packed

#else

#pragma pack(push, 1)

#define PE_DOS_SIGNATURE					0x4D5A      //! MZ
#define PE_OS2_SIGNATURE					0x4E45      //! NE
#define PE_OS2_SIGNATURE_LE					0x4C45      //! LE
#define PE_NT_SIGNATURE						0x50450000  //! PE00

#define SIZEOF_PE_SIGNATURE                 4

#endif

#define PE32(hdr)   (((PPE_NT_HEADERS)(hdr))->OptionalHeader.Magic == PE_NT_OPTIONAL_HDR32_MAGIC)
#define PE64(hdr)   (((PPE_NT_HEADERS)(hdr))->OptionalHeader.Magic == PE_NT_OPTIONAL_HDR64_MAGIC)

#define FILE_HEADER_VALUE(hdr, val) \
    (((PPE_NT_HEADERS)(hdr))->FileHeader.val)

#define PE_SECTIONS(hdr)    \
    (PE64(hdr) ? ((PPE_SECTION_HEADER)((PPE_NT_HEADERS64)(hdr)+1)) : ((PPE_SECTION_HEADER)((PPE_NT_HEADERS32)(hdr)+1)))

#define OPTIONAL_HEADER32_VALUE(hdr, val)   (((PPE_NT_HEADERS32)(hdr))->OptionalHeader.val)
#define OPTIONAL_HEADER64_VALUE(hdr, val)   (((PPE_NT_HEADERS64)(hdr))->OptionalHeader.val)
#define OPTIONAL_HEADER_VALUE(hdr, val) \
    (PE64(hdr) ? OPTIONAL_HEADER64_VALUE(hdr, val) : OPTIONAL_HEADER32_VALUE(hdr, val))

#define PE_IMAGE_SIZE(hdr)  \
    (PE64(hdr) ? ((PPE_NT_HEADERS64)(hdr))->OptionalHeader.SizeOfImage : ((PPE_NT_HEADERS32)(hdr))->OptionalHeader.SizeOfImage)

#define LOAD_CONFIG_DIRECTORY32_VALUE(hdr, dir, val) (((PPE_LOAD_CONFIG_DIRECTORY32)(dir))->val)
#define LOAD_CONFIG_DIRECTORY64_VALUE(hdr, dir, val) (((PPE_LOAD_CONFIG_DIRECTORY64)(dir))->val)
#define LOAD_CONFIG_DIRECTORY_VALUE(hdr, dir, val)  \
    (PE64(hdr) ? LOAD_CONFIG_DIRECTORY64_VALUE(hdr, dir, val) : LOAD_CONFIG_DIRECTORY32_VALUE(hdr, dir, val))

#define DATA_DIRECTORY32(hdr, dir)      (&((PPE_NT_HEADERS32)(hdr))->OptionalHeader.DataDirectory[(dir)])
#define DATA_DIRECTORY64(hdr, dir)      (&((PPE_NT_HEADERS64)(hdr))->OptionalHeader.DataDirectory[(dir)])
#define DATA_DIRECTORY(hdr, dir)        \
    (PE64(hdr) ? DATA_DIRECTORY64(hdr, dir) : DATA_DIRECTORY32(hdr, dir))

#define DATA_DIRECTORY32_RVA(hdr, dir)  (((PPE_NT_HEADERS32)(hdr))->OptionalHeader.DataDirectory[(dir)].VirtualAddress)
#define DATA_DIRECTORY64_RVA(hdr, dir)  (((PPE_NT_HEADERS64)(hdr))->OptionalHeader.DataDirectory[(dir)].VirtualAddress)
#define DATA_DIRECTORY_RVA(hdr, dir)    \
    (PE64( hdr ) ? DATA_DIRECTORY64_RVA(hdr, dir) : DATA_DIRECTORY32_RVA(hdr, dir))

#define DATA_DIRECTORY32_SIZE(hdr, dir) (((PPE_NT_HEADERS32)(hdr))->OptionalHeader.DataDirectory[(dir)].Size)
#define DATA_DIRECTORY64_SIZE(hdr, dir) (((PPE_NT_HEADERS64)(hdr))->OptionalHeader.DataDirectory[(dir)].Size)
#define DATA_DIRECTORY_SIZE(hdr, dir)   \
    (PE64( hdr ) ? DATA_DIRECTORY64_SIZE(hdr, dir) : DATA_DIRECTORY32_SIZE(hdr, dir))

#define DATA_DIRECTORY32_ADDRESS(base, hdr, dir)    (((UINTN)DATA_DIRECTORY32_RVA(hdr, dir)) + ((UINTN)(base)))
#define DATA_DIRECTORY64_ADDRESS(base, hdr, dir)    (((UINTN)DATA_DIRECTORY64_RVA(hdr, dir)) + ((UINTN)(base)))
#define DATA_DIRECTORY_ADDRESS(base, hdr, dir)  \
    (PE64( hdr ) ? DATA_DIRECTORY64_ADDRESS(base, hdr, dir) : DATA_DIRECTORY32_ADDRESS(base, hdr, dir))

typedef struct _PE_DOS_HEADER {
    UINT16 e_magic;					    //!< 0x00 Magic number
    UINT16 e_cblp;					    //!< 0x02 Bytes on last page of file
    UINT16 e_cp;						//!< 0x04 Pages in file
    UINT16 e_crlc;					    //!< 0x06 Relocations
    UINT16 e_cparhdr;					//!< 0x08 Size of header in paragraphs
    UINT16 e_minalloc;				    //!< 0x0A Minimum extra paragraphs needed
    UINT16 e_maxalloc;				    //!< 0x0C Maximum extra paragraphs needed
    UINT16 e_ss;						//!< 0x0E Initial (relative) SS value
    UINT16 e_sp;						//!< 0x10 Initial SP value
    UINT16 e_csum;					    //!< 0x12 Checksum
    UINT16 e_ip;						//!< 0x14 Initial IP value
    UINT16 e_cs;						//!< 0x16 Initial (relative) CS value
    UINT16 e_lfarlc;					//!< 0x18 File address of relocation table
    UINT16 e_ovno;					    //!< 0x1A Overlay number
    UINT16 e_res[4];					//!< 0x1C Reserved words
    UINT16 e_oemid;					    //!< 0x1E OEM identifier (for e_oeminfo)
    UINT16 e_oeminfo;					//!< 0x20 OEM information; e_oemid specific
    UINT16 e_res2[10];				    //!< 0x22 Reserved words
    INT32 e_lfanew;					    //!< 0x24 File address of new exe header
} PE_DOS_HEADER, *PPE_DOS_HEADER;

#ifndef _MAC
#pragma pack(pop)					// Back to 4 byte packing
#endif

/**
* File Header Format
*/

typedef struct _PE_FILE_HEADER {
    UINT16 Machine;                     //!< 0x00 The architecture type of the computer.
    UINT16 NumberOfSections;            //!< 0x02 The number of sections. This indicates the size of the section table, which immediately follows the headers.
    UINT32 TimeDateStamp;				//!< 0x04 The low 32 bits of the time stamp of the image. This represents the date and time the image was created by the linker.
    UINT32 PointerToSymbolTable;		//!< 0x08 The offset of the symbol table, in bytes, or zero if no COFF symbol table exists.
    UINT32 NumberOfSymbols;			    //!< 0x0C The number of symbols in the symbol table.
    UINT16 SizeOfOptionalHeader;        //!< 0x10 The size of the optional header, in bytes. This value is 0 for object files.
    UINT16 Characteristics;             //!< 0x12 The characteristics of the image.
} PE_FILE_HEADER, *PPE_FILE_HEADER;
#define PE_SIZEOF_FILE_HEADER				20 // 0x14

#define PE_FILE_RELOCS_STRIPPED					0x0001  //! Relocation info stripped from file.
#define PE_FILE_EXECUTABLE_IMAGE				0x0002  //! File is executable  (i.e. no unresolved external references).
#define PE_FILE_LINE_NUMS_STRIPPED				0x0004  //! Line nunbers stripped from file.
#define PE_FILE_LOCAL_SYMS_STRIPPED				0x0008  //! Local symbols stripped from file.
#define PE_FILE_AGGRESIVE_WS_TRIM				0x0010  //! Aggressively trim working set
#define PE_FILE_LARGE_ADDRESS_AWARE				0x0020  //! App can handle >2gb addresses
#define PE_FILE_BYTES_REVERSED_LO				0x0080  //! Bytes of machine word are reversed.
#define PE_FILE_32BIT_MACHINE					0x0100  //! 32 bit word machine.
#define PE_FILE_DEBUG_STRIPPED					0x0200  //! Debugging info stripped from file in .DBG file
#define PE_FILE_REMOVABLE_RUN_FROM_SWAP			0x0400  //! If Image is on removable media, copy and run from the swap file.
#define PE_FILE_NET_RUN_FROM_SWAP				0x0800  //! If Image is on Net, copy and run from the swap file.
#define PE_FILE_SYSTEM							0x1000  //! System File.
#define PE_FILE_DLL								0x2000  //! File is a DLL.
#define PE_FILE_UP_SYSTEM_ONLY					0x4000  //! File should only be run on a UP machine
#define PE_FILE_BYTES_REVERSED_HI				0x8000  //! Bytes of machine word are reversed.

#define PE_FILE_MACHINE_UNKNOWN					0
#define PE_FILE_MACHINE_I386					0x014C  //! Intel 386.
#define PE_FILE_MACHINE_R3000					0x0162  //! MIPS little-endian, 0x160 big-endian
#define PE_FILE_MACHINE_R4000					0x0166  //! MIPS little-endian
#define PE_FILE_MACHINE_R10000					0x0168  //! MIPS little-endian
#define PE_FILE_MACHINE_WCEMIPSV2				0x0169  //! MIPS little-endian WCE v2
#define PE_FILE_MACHINE_ALPHA					0x0184  //! Alpha_AXP
#define PE_FILE_MACHINE_SH3						0x01A2  //! SH3 little-endian
#define PE_FILE_MACHINE_SH3DSP					0x01A3  //! SH3DSP little-endian
#define PE_FILE_MACHINE_SH3E					0x01A4  //! SH3E little-endian
#define PE_FILE_MACHINE_SH4						0x01A6  //! SH4 little-endian
#define PE_FILE_MACHINE_SH5						0x01A8  //! SH5
#define PE_FILE_MACHINE_ARM						0x01C0  //! ARM Little-Endian
#define PE_FILE_MACHINE_THUMB					0x01C2  //! ARM Thumb/Thumb-2 Little-Endian
#define PE_FILE_MACHINE_ARMNT					0x01C4  //! ARM Thumb-2 Little-Endian
#define PE_FILE_MACHINE_AM33					0x01d3
#define PE_FILE_MACHINE_POWERPC					0x01F0  //! IBM PowerPC Little-Endian
#define PE_FILE_MACHINE_POWERPCFP				0x01F1
#define PE_FILE_MACHINE_IA64					0x0200  //! Intel 64
#define PE_FILE_MACHINE_MIPS16					0x0266  //! MIPS
#define PE_FILE_MACHINE_ALPHA64					0x0284  //! ALPHA64
#define PE_FILE_MACHINE_MIPSFPU					0x0366  //! MIPS
#define PE_FILE_MACHINE_MIPSFPU16				0x0466  //! MIPS
#define PE_FILE_MACHINE_AXP64					PE_FILE_MACHINE_ALPHA64
#define PE_FILE_MACHINE_TRICORE					0x0520  //! Infineon
#define PE_FILE_MACHINE_CEF						0x0CEF
#define PE_FILE_MACHINE_EBC						0x0EBC  //! EFI Byte Code
#define PE_FILE_MACHINE_AMD64					0x8664  //! AMD64 (K8)
#define PE_FILE_MACHINE_M32R					0x9041  //! M32R little-endian
#define PE_FILE_MACHINE_CEE						0xC0EE

/**
* Directory Format
*/

typedef struct _PE_DATA_DIRECTORY {
    UINT32 VirtualAddress;			    //!< 0x00 The relative virtual address of the table.
    UINT32 Size;					    //!< 0x04 The size of the table, in bytes.
} PE_DATA_DIRECTORY, *PPE_DATA_DIRECTORY;
#define PE_NUMBEROF_DIRECTORY_ENTRIES    16

/**
* Optional Header Format
*/

typedef struct _PE_OPTIONAL_HEADER {
    UINT16 Magic;                       //!< 0x00 The state of the image file.
    UINT8 MajorLinkerVersion;           //!< 0x02 The major version number of the linker.
    UINT8 MinorLinkerVersion;           //!< 0x03 The minor version number of the linker.
    UINT32 SizeOfCode;                  //!< 0x04 The size of the code section, in bytes, or the sum of all such sections if there are multiple code sect
    UINT32 SizeOfInitializedData;       //!< 0x08 The size of the initialized data section, in bytes, or the sum of all such sections if there are multip
    UINT32 SizeOfUninitializedData;     //!< 0x0C The size of the uninitialized data section, in bytes, or the sum of all such sections if there are mult
    UINT32 AddressOfEntryPoint;         //!< 0x10 A pointer to the entry point function, relative to the image base address.
    UINT32 BaseOfCode;                  //!< 0x14 A pointer to the beginning of the code section, relative to the image base.
    UINT32 BaseOfData;                  //!< 0x18 A pointer to the beginning of the data section, relative to the image base.
    UINT32 ImageBase;                   //!< 0x1C The preferred address of the first byte of the image when it is loaded in memory.
    UINT32 SectionAlignment;            //!< 0x20 The alignment of sections loaded in memory, in bytes.
    UINT32 FileAlignment;               //!< 0x24 The alignment of the raw data of sections in the image file, in bytes.
    UINT16 MajorOSVersion;			    //!< 0x28 The major version number of the required operating system.
    UINT16 MinorOSVersion;			    //!< 0x2A The minor version number of the required operating system.
    UINT16 MajorImageVersion;           //!< 0x2C The major version number of the image.
    UINT16 MinorImageVersion;           //!< 0x2E The minor version number of the image.
    UINT16 MajorSubsystemVersion;       //!< 0x30 The major version number of the subsystem.
    UINT16 MinorSubsystemVersion;       //!< 0x32 The minor version number of the subsystem.
    UINT32 Win32VersionValue;           //!< 0x34 This member is reserved and must be 0.
    UINT32 SizeOfImage;                 //!< 0x38 The size of the image, in bytes, including all headers.
    UINT32 SizeOfHeaders;               //!< 0x3C The combined size of all headers (e_lfanew, file header, optional header, all section headers).
    UINT32 CheckSum;                    //!< 0x40 The image file checksum.
    UINT16 Subsystem;                   //!< 0x44 The subsystem required to run this image.
    UINT16 DllCharacteristics;          //!< 0x46 The DLL characteristics of the image.
    UINT32 SizeOfStackReserve;          //!< 0x48 The number of bytes to reserve for the stack.
    UINT32 SizeOfStackCommit;           //!< 0x4C The number of bytes to commit for the stack.
    UINT32 SizeOfHeapReserve;           //!< 0x50 The number of bytes to reserve for the local heap.
    UINT32 SizeOfHeapCommit;            //!< 0x54 The number of bytes to commit for the local heap.
    UINT32 LoaderFlags;                 //!< 0x58 This member is obsolete.
    UINT32 NumberOfRvaAndSizes;         //!< 0x5C The number of directory entries in the remainder of the optional header.
    PE_DATA_DIRECTORY DataDirectory[PE_NUMBEROF_DIRECTORY_ENTRIES]; //!< 0x60 A pointer to the PE_DATA_DIRECTORY structures in the data directory.
} PE_OPTIONAL_HEADER32, *PPE_OPTIONAL_HEADER32;

typedef struct _PE_ROM_OPTIONAL_HEADER {
    UINT16 Magic;                       //!< 0x00 The state of the image file.
    UINT8 MajorLinkerVersion;           //!< 0x02 The major version number of the linker.
    UINT8 MinorLinkerVersion;           //!< 0x03 The minor version number of the linker.
    UINT32 SizeOfCode;                  //!< 0x04 The size of the code section, in bytes, or the sum of all such sections if there are multiple code sections.
    UINT32 SizeOfInitializedData;       //!< 0x08 The size of the initialized data section, in bytes, or the sum of all such sections if there are multiple initialized data sections.
    UINT32 SizeOfUninitializedData;     //!< 0x0C The size of the uninitialized data section, in bytes, or the sum of all such sections if there are multiple uninitialized data sections.
    UINT32 AddressOfEntryPoint;         //!< 0x10 A pointer to the entry point function, relative to the image base address.
    UINT32 BaseOfCode;                  //!< 0x14 A pointer to the beginning of the code section, relative to the image base.
    UINT32 BaseOfData;                  //!< 0x18 A pointer to the beginning of the data section, relative to the image base.
    UINT32 BaseOfBss;                   //!< 0x1C A pointer to the beginning of the bss section, relative to the image base.
    UINT32 GprMask;                     //!< 0x20
    UINT32 CprMask[4];                  //!< 0x24
    UINT32 GpValue;                     //!< 0x34
} PE_ROM_OPTIONAL_HEADER, *PPE_ROM_OPTIONAL_HEADER;

typedef struct _PE_OPTIONAL_HEADER64 {
    UINT16 Magic;                       //!< 0x00 The state of the image file.
    UINT8 MajorLinkerVersion;           //!< 0x02 The major version number of the linker.
    UINT8 MinorLinkerVersion;           //!< 0x03 The minor version number of the linker.
    UINT32 SizeOfCode;                  //!< 0x04 The size of the code section, in bytes, or the sum of all such sections if there are multiple code sections.
    UINT32 SizeOfInitializedData;       //!< 0x08 The size of the initialized data section, in bytes, or the sum of all such sections if there are multiple initialized data sections.
    UINT32 SizeOfUninitializedData;     //!< 0x0C The size of the uninitialized data section, in bytes, or the sum of all such sections if there are multiple uninitialized data sections.
    UINT32 AddressOfEntryPoint;         //!< 0x10 A pointer to the entry point function, relative to the image base address.
    UINT32 BaseOfCode;                  //!< 0x14 A pointer to the beginning of the code section, relative to the image base.
    UINT64 ImageBase;					//!< 0x18 The preferred address of the first byte of the image when it is loaded in memory.
    UINT32 SectionAlignment;            //!< 0x20 The alignment of sections loaded in memory, in bytes.
    UINT32 FileAlignment;               //!< 0x24 The alignment of the raw data of sections in the image file, in bytes.
    UINT16 MajorOSVersion;			    //!< 0x28 The major version number of the required operating system.
    UINT16 MinorOSVersion;			    //!< 0x2A The minor version number of the required operating system.
    UINT16 MajorImageVersion;           //!< 0x2C The major version number of the image.
    UINT16 MinorImageVersion;           //!< 0x2E The minor version number of the image.
    UINT16 MajorSubsystemVersion;       //!< 0x30 The major version number of the subsystem.
    UINT16 MinorSubsystemVersion;       //!< 0x32 The minor version number of the subsystem.
    UINT32 Win32VersionValue;           //!< 0x34 This member is reserved and must be 0.
    UINT32 SizeOfImage;                 //!< 0x38 The size of the image, in bytes, including all headers.
    UINT32 SizeOfHeaders;               //!< 0x3C The combined size of all headers (e_lfanew, file header, optional header, all section headers).
    UINT32 CheckSum;                    //!< 0x40 The image file checksum.
    UINT16 Subsystem;                   //!< 0x44 The subsystem required to run this image.
    UINT16 DllCharacteristics;          //!< 0x46 The DLL characteristics of the image.
    UINT64 SizeOfStackReserve;		    //!< 0x48 The number of bytes to reserve for the stack.
    UINT64 SizeOfStackCommit;			//!< 0x50 The number of bytes to commit for the stack.
    UINT64 SizeOfHeapReserve;			//!< 0x58 The number of bytes to reserve for the local heap.
    UINT64 SizeOfHeapCommit;			//!< 0x60 The number of bytes to commit for the local heap.
    UINT32 LoaderFlags;                 //!< 0x68 This member is obsolete.
    UINT32 NumberOfRvaAndSizes;         //!< 0x6C The number of directory entries in the remainder of the optional header.
    PE_DATA_DIRECTORY DataDirectory[PE_NUMBEROF_DIRECTORY_ENTRIES]; //!< 0x70 A pointer to the PE_DATA_DIRECTORY structures in the data directory.
} PE_OPTIONAL_HEADER64, *PPE_OPTIONAL_HEADER64;

#define PE_NT_OPTIONAL_HDR32_MAGIC			0x10B
#define PE_NT_OPTIONAL_HDR64_MAGIC			0x20B
#define PE_ROM_OPTIONAL_HDR_MAGIC			0x107

#if defined(_M_X64) || defined(__x86_64__)
typedef PE_OPTIONAL_HEADER64				PE_OPTIONAL_HEADER;
typedef PPE_OPTIONAL_HEADER64				PPE_OPTIONAL_HEADER;
#define PE_NT_OPTIONAL_HDR_MAGIC			PE_NT_OPTIONAL_HDR64_MAGIC
#else
typedef PE_OPTIONAL_HEADER32				PE_OPTIONAL_HEADER;
typedef PPE_OPTIONAL_HEADER32				PPE_OPTIONAL_HEADER;
#define PE_NT_OPTIONAL_HDR_MAGIC			PE_NT_OPTIONAL_HDR32_MAGIC
#endif

typedef struct _PE_NT_HEADERS64 {
    UINT32 Signature;                       //!< 0x00 A 4-byte signature identifying the file as a PE image.
    PE_FILE_HEADER FileHeader;				//!< 0x04 An PE_FILE_HEADER structure that specifies the file header.
    PE_OPTIONAL_HEADER64 OptionalHeader;	//!< 0x18 An PE_OPTIONAL_HEADER64 structure that specifies the optional file header.
} PE_NT_HEADERS64, *PPE_NT_HEADERS64;

typedef struct _PE_NT_HEADERS32 {
    UINT32 Signature;                       //!< 0x00 A 4-byte signature identifying the file as a PE image.
    PE_FILE_HEADER FileHeader;				//!< 0x04 An PE_FILE_HEADER structure that specifies the file header.
    PE_OPTIONAL_HEADER32 OptionalHeader;	//!< 0x18 An PE_OPTIONAL_HEADER32 structure that specifies the optional file header.
} PE_NT_HEADERS32, *PPE_NT_HEADERS32;

typedef struct _PE_ROM_HEADERS {
    PE_FILE_HEADER FileHeader;				//!< 0x00 An PE_FILE_HEADER structure that specifies the file header.
    PE_ROM_OPTIONAL_HEADER OptionalHeader;	//!< 0x14 An PE_ROM_OPTIONAL_HEADER structure that specifies the optional file header.
} PE_ROM_HEADERS, *PPE_ROM_HEADERS;

#if defined(_M_X64) || defined(__x86_64__)
typedef PE_NT_HEADERS64						PE_NT_HEADERS;
typedef PPE_NT_HEADERS64					PPE_NT_HEADERS;
#else
typedef PE_NT_HEADERS32						PE_NT_HEADERS;
typedef PPE_NT_HEADERS32					PPE_NT_HEADERS;
#endif

/**
* Some helpful PE functions.
*/

/**
* @name PeDosHeader
* @brief Obtain the specified image's DOS header.
* @param[in] ImageBase - The image base address.
* @return A pointer to this image's DOS header.
*/
PPE_DOS_HEADER PeDosHeader( IN CONST VOID* ImageBase );

/**
* @name PeNtHeader
* @brief Obtain the specified image's NT headers.
* @param[in] ImageBase - The image base address.
* @return A pointer to this image's NT headers.
*/
PPE_NT_HEADERS PeNtHeader( IN CONST VOID* ImageBase );

/** PE_FIRST_SECTION doesn't need 32/64 versions since the file header is the same either way. */
#define PE_FIRST_SECTION(NtHeader) ((PPE_SECTION_HEADER)                \
    (((SIZE_T)(NtHeader)) +                                             \
     FIELD_OFFSET( PE_NT_HEADERS, OptionalHeader ) +                    \
     ((PPE_NT_HEADERS)(NtHeader))->FileHeader.SizeOfOptionalHeader      \
    ))

/**
* Subsystem Values
*/
#define PE_SUBSYSTEM_UNKNOWN						0	//! Unknown subsystem.
#define PE_SUBSYSTEM_NATIVE							1	//! Image doesn't require a subsystem.
#define PE_SUBSYSTEM_WINDOWS_GUI					2	//! Image runs in the Windows GUI subsystem.
#define PE_SUBSYSTEM_WINDOWS_CUI					3	//! Image runs in the Windows character subsystem.
#define PE_SUBSYSTEM_OS2_CUI						5	//! image runs in the OS/2 character subsystem.
#define PE_SUBSYSTEM_POSIX_CUI						7	//! image runs in the Posix character subsystem.
#define PE_SUBSYSTEM_NATIVE_WINDOWS					8	//! image is a native Win9x driver.
#define PE_SUBSYSTEM_WINDOWS_CE_GUI					9	//! Image runs in the Windows CE subsystem.
#define PE_SUBSYSTEM_EFI_APPLICATION				10	//! Extensible Firmware Interface (EFI) application.
#define PE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER		11	//! EFI driver with boot services.
#define PE_SUBSYSTEM_EFI_RUNTIME_DRIVER				12	//! EFI driver with run-time services.
#define PE_SUBSYSTEM_EFI_ROM						13	//! EFI ROM image.
#define PE_SUBSYSTEM_XBOX							14	//! Xbox system.
#define PE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION		16	//! Boot application.

/**
* DllCharacteristics Entries
*/
//      PE_LIBRARY_PROCESS_INIT					0x0001	//! Reserved.
//      PE_LIBRARY_PROCESS_TERM					0x0002	//! Reserved.
//      PE_LIBRARY_THREAD_INIT					0x0004	//! Reserved.
//      PE_LIBRARY_THREAD_TERM					0x0008	//! Reserved.
#define PE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA	0x0020  //! Image can handle a high entropy 64-bit virtual address space.
#define PE_DLLCHARACTERISTICS_DYNAMIC_BASE		0x0040  //! DLL can move.
#define PE_DLLCHARACTERISTICS_FORCE_INTEGRITY	0x0080  //! Code Integrity Image
#define PE_DLLCHARACTERISTICS_NX_COMPAT			0x0100  //! Image is NX compatible
#define PE_DLLCHARACTERISTICS_NO_ISOLATION		0x0200  //! Image understands isolation and doesn't want it
#define PE_DLLCHARACTERISTICS_NO_SEH			0x0400  //! Image does not use SEH.  No SE handler may reside in this image
#define PE_DLLCHARACTERISTICS_NO_BIND			0x0800  //! Do not bind this image.
#define PE_DLLCHARACTERISTICS_APPCONTAINER		0x1000  //! Image should execute in an AppContainer
#define PE_DLLCHARACTERISTICS_WDM_DRIVER		0x2000  //! Driver uses WDM model
#define PE_DLLCHARACTERISTICS_GUARD_CF			0x4000  //! Image supports Control Flow Guard.
#define PE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE  0x8000 //! The image is terminal server aware.

/**
* Directory Entries
*/
#define PE_DIRECTORY_ENTRY_EXPORT					0	//! Export Directory
#define PE_DIRECTORY_ENTRY_IMPORT					1	//! Import Directory
#define PE_DIRECTORY_ENTRY_RESOURCE					2	//! Resource Directory
#define PE_DIRECTORY_ENTRY_EXCEPTION				3	//! Exception Directory
#define PE_DIRECTORY_ENTRY_SECURITY					4	//! Security Directory
#define PE_DIRECTORY_ENTRY_BASERELOC				5	//! Base Relocation Table
#define PE_DIRECTORY_ENTRY_DEBUG					6	//! Debug Directory
//      PE_DIRECTORY_ENTRY_COPYRIGHT				7	//! (X86 usage)
#define PE_DIRECTORY_ENTRY_ARCHITECTURE				7	//! Architecture Specific Data
#define PE_DIRECTORY_ENTRY_GLOBALPTR				8	//! RVA of GP
#define PE_DIRECTORY_ENTRY_TLS						9	//! TLS Directory
#define PE_DIRECTORY_ENTRY_LOAD_CONFIG				10  //! Load Configuration Directory
#define PE_DIRECTORY_ENTRY_BOUND_IMPORT				11  //! Bound Import Directory in headers
#define PE_DIRECTORY_ENTRY_IAT						12  //! Import Address Table
#define PE_DIRECTORY_ENTRY_DELAY_IMPORT				13  //! Delay Load Import Descriptors
#define PE_DIRECTORY_ENTRY_COM_DESCRIPTOR			14  //! COM Runtime descriptor

/**
* Section header format.
*/

#define PE_SIZEOF_SHORT_NAME              8

typedef struct _PE_SECTION_HEADER {
    UINT8 Name[PE_SIZEOF_SHORT_NAME];		//!< 0x00 An 8-byte, null-padded UTF-8 string.
    union {
        UINT32 PhysicalAddress;             //!< 0x08 The file address.
        UINT32 VirtualSize;                 //!< 0x08 The total size of the section when loaded into memory, in bytes.
    } Misc;
    UINT32 VirtualAddress;                  //!< 0x0C The address of the first byte of the section when loaded into memory, relative to the image base.
    UINT32 SizeOfRawData;                   //!< 0x10 The size of the initialized data on disk, in bytes.
    UINT32 PointerToRawData;                //!< 0x14 A file pointer to the first page within the COFF file.
    UINT32 PointerToRelocations;            //!< 0x18 A file pointer to the beginning of the relocation entries for the section.
    UINT32 PointerToLineNumbers;            //!< 0x1C A file pointer to the beginning of the line-number entries for the section.
    UINT16 NumberOfRelocations;             //!< 0x20 The number of relocation entries for the section.
    UINT16 NumberOfLineNumbers;             //!< 0x22 The number of line-number entries for the section.
    UINT32 Characteristics;                 //!< 0x24 The characteristics of the image.
} PE_SECTION_HEADER, *PPE_SECTION_HEADER;
#define PE_SIZEOF_SECTION_HEADER			40 // 0x28

/**
* Section characteristics.
*/
//      PE_SCN_TYPE_REG					    0x00000000  //! Reserved.
//      PE_SCN_TYPE_DSECT				    0x00000001  //! Reserved.
//      PE_SCN_TYPE_NOLOAD				    0x00000002  //! Reserved.
//      PE_SCN_TYPE_GROUP				    0x00000004  //! Reserved.
#define PE_SCN_TYPE_NO_PAD				    0x00000008  //! Reserved.
//      PE_SCN_TYPE_COPY				    0x00000010  //! Reserved.

#define PE_SCN_CNT_CODE                     0x00000020  //! Section contains code.
#define PE_SCN_CNT_INITIALIZED_DATA         0x00000040  //! Section contains initialized data.
#define PE_SCN_CNT_UNINITIALIZED_DATA       0x00000080  //! Section contains uninitialized data.

#define PE_SCN_LNK_OTHER                    0x00000100  //! Reserved.
#define PE_SCN_LNK_INFO                     0x00000200  //! Section contains comments or some other type of information.
//      PE_SCN_TYPE_OVER                    0x00000400  //! Reserved.
#define PE_SCN_LNK_REMOVE                   0x00000800  //! Section contents will not become part of image.
#define PE_SCN_LNK_COMDAT                   0x00001000  //! Section contents comdat.
//                                          0x00002000  //! Reserved.
//      PE_SCN_MEM_PROTECTED - Obsolete     0x00004000
#define PE_SCN_NO_DEFER_SPEC_EXC            0x00004000  //! Reset speculative exceptions handling bits in the TLB entries for this section.
#define PE_SCN_GPREL                        0x00008000  //! Section content can be accessed relative to GP
#define PE_SCN_MEM_FARDATA                  0x00008000
//      PE_SCN_MEM_SYSHEAP  - Obsolete      0x00010000
#define PE_SCN_MEM_PURGEABLE                0x00020000
#define PE_SCN_MEM_16BIT                    0x00020000
#define PE_SCN_MEM_LOCKED                   0x00040000
#define PE_SCN_MEM_PRELOAD                  0x00080000

#define PE_SCN_ALIGN_1BYTES                 0x00100000  //!
#define PE_SCN_ALIGN_2BYTES                 0x00200000  //!
#define PE_SCN_ALIGN_4BYTES                 0x00300000  //!
#define PE_SCN_ALIGN_8BYTES                 0x00400000  //!
#define PE_SCN_ALIGN_16BYTES                0x00500000  //! Default alignment if no others are specified.
#define PE_SCN_ALIGN_32BYTES                0x00600000  //!
#define PE_SCN_ALIGN_64BYTES                0x00700000  //!
#define PE_SCN_ALIGN_128BYTES               0x00800000  //!
#define PE_SCN_ALIGN_256BYTES               0x00900000  //!
#define PE_SCN_ALIGN_512BYTES               0x00A00000  //!
#define PE_SCN_ALIGN_1024BYTES              0x00B00000  //!
#define PE_SCN_ALIGN_2048BYTES              0x00C00000  //!
#define PE_SCN_ALIGN_4096BYTES              0x00D00000  //!
#define PE_SCN_ALIGN_8192BYTES              0x00E00000  //!
#define PE_SCN_ALIGN_MASK                   0x00F00000

#define PE_SCN_LNK_NRELOC_OVFL              0x01000000  //! Section contains extended relocations.
#define PE_SCN_MEM_DISCARDABLE              0x02000000  //! Section can be discarded.
#define PE_SCN_MEM_NOT_CACHED               0x04000000  //! Section is not cachable.
#define PE_SCN_MEM_NOT_PAGED                0x08000000  //! Section is not pageable.
#define PE_SCN_MEM_SHARED                   0x10000000  //! Section is shareable.
#define PE_SCN_MEM_EXECUTE                  0x20000000  //! Section is executable.
#define PE_SCN_MEM_READ                     0x40000000  //! Section is readable.
#define PE_SCN_MEM_WRITE                    0x80000000  //! Section is writeable.

/**
* TLS Characteristic Flags
*/
#define PE_SCN_SCALE_INDEX                  0x00000001  //! Tls index is scaled

#ifndef _MAC
#pragma pack(push, 2)				// Symbols, relocs, and linenumbers are 2 byte packed
#endif

/**
* Symbol Format
*/

typedef struct _PE_SYMBOL {
    union {
        UINT8 ShortName[8];
        struct {
            UINT32 Short;   //!< if 0, use LongName
            UINT32 Long;    //!< offset into string table
        } Name;
        UINT32 LongName[2]; //!< PBYTE [2]
    } N;
    UINT32 Value;
    INT16 SectionNumber;
    UINT16 Type;
    UINT8 StorageClass;
    UINT8 NumberOfAuxSymbols;
} PE_SYMBOL;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_SYMBOL __unaligned *PPE_SYMBOL;
#else
typedef PE_SYMBOL *PPE_SYMBOL;
#endif

#define PE_SIZEOF_SYMBOL                  18

typedef struct _PE_SYMBOL_EX {
    union {
        UINT8 ShortName[8];
        struct {
            UINT32 Short;     //!< if 0, use LongName
            UINT32 Long;      //!< offset into string table
        } Name;
        UINT32 LongName[2];   //!< PBYTE  [2]
    } N;
    UINT32 Value;
    INT32 SectionNumber;
    UINT16 Type;
    UINT8 StorageClass;
    UINT8 NumberOfAuxSymbols;
} PE_SYMBOL_EX;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_SYMBOL_EX __unaligned *PPE_SYMBOL_EX;
#else
typedef PE_SYMBOL_EX *PPE_SYMBOL_EX;
#endif

/**
* Section values.
*/
// Symbols have a section number of the section in which they are
// defined. Otherwise, section numbers have the following meanings:
#define PE_SYM_UNDEFINED				(INT16)0        //! Symbol is undefined or is common.
#define PE_SYM_ABSOLUTE					(INT16)(-1)	    //! Symbol is an absolute value.
#define PE_SYM_DEBUG					(INT16)(-2)	    //! Symbol is a special debug item.
#define PE_SYM_SECTION_MAX				0xFEFF          //! Values 0xFF00-0xFFFF are special
#define PE_SYM_SECTION_MAX_EX			0x7FFFFFFF

/**
* Fundamental Type Values
*/
#define PE_SYM_TYPE_NULL				0x0000  //! no type.
#define PE_SYM_TYPE_VOID				0x0001  //!
#define PE_SYM_TYPE_CHAR				0x0002  //! type character.
#define PE_SYM_TYPE_SHORT				0x0003  //! type short integer.
#define PE_SYM_TYPE_INT					0x0004  //!
#define PE_SYM_TYPE_LONG				0x0005  //!
#define PE_SYM_TYPE_FLOAT				0x0006  //!
#define PE_SYM_TYPE_DOUBLE				0x0007  //!
#define PE_SYM_TYPE_STRUCT				0x0008  //!
#define PE_SYM_TYPE_UNION				0x0009  //!
#define PE_SYM_TYPE_ENUM				0x000A  //! enumeration.
#define PE_SYM_TYPE_MOE					0x000B  //! member of enumeration.
#define PE_SYM_TYPE_BYTE				0x000C  //!
#define PE_SYM_TYPE_WORD				0x000D  //!
#define PE_SYM_TYPE_UINT				0x000E  //!
#define PE_SYM_TYPE_DWORD				0x000F  //!
#define PE_SYM_TYPE_PCODE				0x8000  //!

/**
* Type (derived) values.
*/
#define PE_SYM_DTYPE_NULL					0   //! no derived type.
#define PE_SYM_DTYPE_POINTER				1   //! pointer.
#define PE_SYM_DTYPE_FUNCTION				2   //! function.
#define PE_SYM_DTYPE_ARRAY					3   //! array.

/**
* Storage Classes
*/
#define PE_SYM_CLASS_END_OF_FUNCTION		(UINT8)(-1)
#define PE_SYM_CLASS_NULL					0x0000
#define PE_SYM_CLASS_AUTOMATIC				0x0001
#define PE_SYM_CLASS_EXTERNAL				0x0002
#define PE_SYM_CLASS_STATIC					0x0003
#define PE_SYM_CLASS_REGISTER				0x0004
#define PE_SYM_CLASS_EXTERNAL_DEF			0x0005
#define PE_SYM_CLASS_LABEL					0x0006
#define PE_SYM_CLASS_UNDEFINED_LABEL		0x0007
#define PE_SYM_CLASS_MEMBER_OF_STRUCT		0x0008
#define PE_SYM_CLASS_ARGUMENT				0x0009
#define PE_SYM_CLASS_STRUCT_TAG				0x000A
#define PE_SYM_CLASS_MEMBER_OF_UNION		0x000B
#define PE_SYM_CLASS_UNION_TAG				0x000C
#define PE_SYM_CLASS_TYPE_DEFINITION		0x000D
#define PE_SYM_CLASS_UNDEFINED_STATIC		0x000E
#define PE_SYM_CLASS_ENUM_TAG				0x000F
#define PE_SYM_CLASS_MEMBER_OF_ENUM			0x0010
#define PE_SYM_CLASS_REGISTER_PARAM			0x0011
#define PE_SYM_CLASS_BIT_FIELD				0x0012

#define PE_SYM_CLASS_FAR_EXTERNAL			0x0044  //

#define PE_SYM_CLASS_BLOCK					0x0064
#define PE_SYM_CLASS_FUNCTION				0x0065
#define PE_SYM_CLASS_END_OF_STRUCT			0x0066
#define PE_SYM_CLASS_FILE					0x0067
#define PE_SYM_CLASS_SECTION				0x0068
#define PE_SYM_CLASS_WEAK_EXTERNAL			0x0069

#define PE_SYM_CLASS_CLR_TOKEN				0x006B

/**
* Type Packing Constants
*/
#define N_BTMASK                            0x000F
#define N_TMASK                             0x0030
#define N_TMASK1                            0x00C0
#define N_TMASK2                            0x00F0
#define N_BTSHFT                            4
#define N_TSHIFT                            2

/**
* MACROS
*/

/** Basic Type of  x */
#define BTYPE(x) ((x) & N_BTMASK)

/** Is x a pointer? */
#ifndef ISPTR
#define ISPTR(x) (((x) & N_TMASK) == (PE_SYM_DTYPE_POINTER << N_BTSHFT))
#endif

/** Is x a function? */
#ifndef ISFCN
#define ISFCN(x) (((x) & N_TMASK) == (PE_SYM_DTYPE_FUNCTION << N_BTSHFT))
#endif

/** Is x an array? */
#ifndef ISARY
#define ISARY(x) (((x) & N_TMASK) == (PE_SYM_DTYPE_ARRAY << N_BTSHFT))
#endif

/** Is x a structure, union, or enumeration TAG? */
#ifndef ISTAG
#define ISTAG(x) ((x)==PE_SYM_CLASS_STRUCT_TAG || (x)==PE_SYM_CLASS_UNION_TAG || (x)==PE_SYM_CLASS_ENUM_TAG)
#endif

#ifndef INCREF
#define INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(PE_SYM_DTYPE_POINTER<<N_BTSHFT)|((x)&N_BTMASK))
#endif
#ifndef DECREF
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))
#endif

#pragma pack(push, 2)

typedef struct PE_AUX_SYMBOL_TOKEN_DEF {
    UINT16 bAuxType;					//!< PE_AUX_SYMBOL_TYPE
    UINT16 bReserved;					//!< Must be 0
    UINT32 SymbolTableIndex;
    UINT8 rgbReserved[12];			    //!< Must be 0
} PE_AUX_SYMBOL_TOKEN_DEF;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_AUX_SYMBOL_TOKEN_DEF __unaligned *PPE_AUX_SYMBOL_TOKEN_DEF;
#else
typedef PE_AUX_SYMBOL_TOKEN_DEF *PPE_AUX_SYMBOL_TOKEN_DEF;
#endif

#pragma pack(pop)

/**
* Auxiliary entry format.
*/

typedef union _PE_AUX_SYMBOL {
    struct {
        UINT32 TagIndex;                        //!< struct, union, or enum tag index
        union {
            struct {
                UINT16 Linenumber;              //!< declaration line number
                UINT16 Size;                    //!< size of struct, union, or enum
            } LnSz;
            UINT32 TotalSize;
        } Misc;
        union {
            struct {                            //!< if ISFCN, tag, or .bb
                UINT32 PointerToLinenumber;
                UINT32 PointerToNextFunction;
            } Function;
            struct {                            //!< if ISARY, up to 4 dimen.
                UINT16 Dimension[4];
            } Array;
        } FcnAry;
        UINT16 TvIndex;                         //!< tv index
    } Sym;
    struct {
        UINT8 Name[PE_SIZEOF_SYMBOL];
    } File;
    struct {
        UINT32 Length;                          //!< section length
        UINT16 NumberOfRelocations;             //!< number of relocation entries
        UINT16 NumberOfLinenumbers;             //!< number of line numbers
        UINT32 CheckSum;                        //!< checksum for communal
        INT16 Number;                           //!< section number to associate with
        UINT8 Selection;                        //!< communal selection type
        UINT8 bReserved;
        INT16 HighNumber;                       //!< high bits of the section number
    } Section;
    PE_AUX_SYMBOL_TOKEN_DEF TokenDef;
    struct {
        UINT32 crc;
        UINT8 rgbReserved[14];
    } CRC;
} PE_AUX_SYMBOL;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_AUX_SYMBOL __unaligned *PPE_AUX_SYMBOL;
#else
typedef PE_AUX_SYMBOL *PPE_AUX_SYMBOL;
#endif

typedef union _PE_AUX_SYMBOL_EX {
    struct {
        UINT32 WeakDefaultSymIndex;            //!< the weak extern default symbol index
        UINT32 WeakSearchType;
        UINT8 rgbReserved[12];
    } Sym;
    struct {
        UINT8 Name[sizeof( PE_SYMBOL_EX )];
    } File;
    struct {
        UINT32 Length;                          //!< section length
        UINT16 NumberOfRelocations;             //!< number of relocation entries
        UINT16 NumberOfLinenumbers;             //!< number of line numbers
        UINT32 CheckSum;                        //!< checksum for communal
        INT16 Number;                           //!< section number to associate with
        UINT8 Selection;                        //!< communal selection type
        UINT8 bReserved;
        INT16 HighNumber;                       //!< high bits of the section number
        UINT8 rgbReserved[2];
    } Section;
    struct {
        PE_AUX_SYMBOL_TOKEN_DEF TokenDef;
        UINT8  rgbReserved[2];
    };
    struct {
        UINT32 crc;
        UINT8 rgbReserved[16];
    } CRC;
} PE_AUX_SYMBOL_EX;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_AUX_SYMBOL_EX __unaligned *PPE_AUX_SYMBOL_EX;
#else
typedef PE_AUX_SYMBOL_EX *PPE_AUX_SYMBOL_EX;
#endif

typedef enum PE_AUX_SYMBOL_TYPE {
    PE_AUX_SYMBOL_TYPE_TOKEN_DEF = 1,
} PE_AUX_SYMBOL_TYPE;

/**
* Communal selection types.
*/

#define PE_COMDAT_SELECT_NODUPLICATES    1
#define PE_COMDAT_SELECT_ANY             2
#define PE_COMDAT_SELECT_SAME_SIZE       3
#define PE_COMDAT_SELECT_EXACT_MATCH     4
#define PE_COMDAT_SELECT_ASSOCIATIVE     5
#define PE_COMDAT_SELECT_LARGEST         6
#define PE_COMDAT_SELECT_NEWEST          7

#define PE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define PE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define PE_WEAK_EXTERN_SEARCH_ALIAS      3

/**
* Relocation Format
*/

typedef struct _PE_RELOCATION {
    union {
        UINT32 VirtualAddress;
        UINT32 RelocCount;			//!< Set to the real count when PE_SCN_LNK_NRELOC_OVFL is set
    };
    UINT32 SymbolTableIndex;
    UINT16 Type;
} PE_RELOCATION;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_RELOCATION __unaligned *PPE_RELOCATION;
#else
typedef PE_RELOCATION *PPE_RELOCATION;
#endif


/**
* Based Relocation Types
*/
#define PE_REL_BASED_ABSOLUTE				0
#define PE_REL_BASED_HIGH					1
#define PE_REL_BASED_LOW					2
#define PE_REL_BASED_HIGHLOW				3
#define PE_REL_BASED_HIGHADJ				4
#define PE_REL_BASED_MACHINE_SPECIFIC_5		5
#define PE_REL_BASED_RESERVED				6
#define PE_REL_BASED_MACHINE_SPECIFIC_7		7
#define PE_REL_BASED_MACHINE_SPECIFIC_8		8
#define PE_REL_BASED_MACHINE_SPECIFIC_9		9
#define PE_REL_BASED_DIR64					10

/**
* Platform-specific Based Relocation Types
*/
#define PE_REL_BASED_IA64_IMM64				9
#define PE_REL_BASED_MIPS_JMPADDR			5
#define PE_REL_BASED_MIPS_JMPADDR16			9
#define PE_REL_BASED_ARM_MOV32				5
#define PE_REL_BASED_THUMB_MOV32			7

/**
* I386 Relocation Types
*/
#define PE_REL_I386_ABSOLUTE			0x0000  //! Reference is absolute, no relocation is necessary
#define PE_REL_I386_DIR16				0x0001  //! Direct 16-bit reference to the symbols virtual address
#define PE_REL_I386_REL16				0x0002  //! PC-relative 16-bit reference to the symbols virtual address
#define PE_REL_I386_DIR32				0x0006  //! Direct 32-bit reference to the symbols virtual address
#define PE_REL_I386_DIR32NB				0x0007  //! Direct 32-bit reference to the symbols virtual address, base not included
#define PE_REL_I386_SEG12				0x0009  //! Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
#define PE_REL_I386_SECTION				0x000A
#define PE_REL_I386_SECREL				0x000B
#define PE_REL_I386_TOKEN				0x000C  //! clr token
#define PE_REL_I386_SECREL7				0x000D  //! 7 bit offset from base of section containing target
#define PE_REL_I386_REL32				0x0014  //! PC-relative 32-bit reference to the symbols virtual address

/**
* MIPS relocation types.
*/
#define PE_REL_MIPS_ABSOLUTE			0x0000  //! Reference is absolute, no relocation is necessary
#define PE_REL_MIPS_REFHALF				0x0001
#define PE_REL_MIPS_REFWORD				0x0002
#define PE_REL_MIPS_JMPADDR				0x0003
#define PE_REL_MIPS_REFHI				0x0004
#define PE_REL_MIPS_REFLO				0x0005
#define PE_REL_MIPS_GPREL				0x0006
#define PE_REL_MIPS_LITERAL				0x0007
#define PE_REL_MIPS_SECTION				0x000A
#define PE_REL_MIPS_SECREL				0x000B
#define PE_REL_MIPS_SECRELLO			0x000C  //! Low 16-bit section relative referemce (used for >32k TLS)
#define PE_REL_MIPS_SECRELHI			0x000D  //! High 16-bit section relative reference (used for >32k TLS)
#define PE_REL_MIPS_TOKEN				0x000E  //! clr token
#define PE_REL_MIPS_JMPADDR16			0x0010
#define PE_REL_MIPS_REFWORDNB			0x0022
#define PE_REL_MIPS_PAIR				0x0025

/**
* Alpha Relocation types
*/
#define PE_REL_ALPHA_ABSOLUTE			0x0000
#define PE_REL_ALPHA_REFLONG			0x0001
#define PE_REL_ALPHA_REFQUAD			0x0002
#define PE_REL_ALPHA_GPREL32			0x0003
#define PE_REL_ALPHA_LITERAL			0x0004
#define PE_REL_ALPHA_LITUSE				0x0005
#define PE_REL_ALPHA_GPDISP				0x0006
#define PE_REL_ALPHA_BRADDR				0x0007
#define PE_REL_ALPHA_HINT				0x0008
#define PE_REL_ALPHA_INLINE_REFLONG		0x0009
#define PE_REL_ALPHA_REFHI				0x000A
#define PE_REL_ALPHA_REFLO				0x000B
#define PE_REL_ALPHA_PAIR				0x000C
#define PE_REL_ALPHA_MATCH				0x000D
#define PE_REL_ALPHA_SECTION			0x000E
#define PE_REL_ALPHA_SECREL				0x000F
#define PE_REL_ALPHA_REFLONGNB			0x0010
#define PE_REL_ALPHA_SECRELLO			0x0011  //! Low 16-bit section relative reference
#define PE_REL_ALPHA_SECRELHI			0x0012  //! High 16-bit section relative reference
#define PE_REL_ALPHA_REFQ3				0x0013  //! High 16 bits of 48 bit reference
#define PE_REL_ALPHA_REFQ2				0x0014  //! Middle 16 bits of 48 bit reference
#define PE_REL_ALPHA_REFQ1				0x0015  //! Low 16 bits of 48 bit reference
#define PE_REL_ALPHA_GPRELLO			0x0016  //! Low 16-bit GP relative reference
#define PE_REL_ALPHA_GPRELHI			0x0017  //! High 16-bit GP relative reference

/**
* IBM PowerPC relocation types
*/
#define PE_REL_PPC_ABSOLUTE				0x0000  //! NOP
#define PE_REL_PPC_ADDR64				0x0001  //! 64-bit address
#define PE_REL_PPC_ADDR32				0x0002  //! 32-bit address
#define PE_REL_PPC_ADDR24				0x0003  //! 26-bit address, shifted left 2 (branch absolute)
#define PE_REL_PPC_ADDR16				0x0004  //! 16-bit address
#define PE_REL_PPC_ADDR14				0x0005  //! 16-bit address, shifted left 2 (load doubleword)
#define PE_REL_PPC_REL24				0x0006  //! 26-bit PC-relative offset, shifted left 2 (branch relative)
#define PE_REL_PPC_REL14				0x0007  //! 16-bit PC-relative offset, shifted left 2 (br cond relative)
#define PE_REL_PPC_TOCREL16				0x0008  //! 16-bit offset from TOC base
#define PE_REL_PPC_TOCREL14				0x0009  //! 16-bit offset from TOC base, shifted left 2 (load doubleword)

#define PE_REL_PPC_ADDR32NB				0x000A  //! 32-bit addr w/o image base
#define PE_REL_PPC_SECREL				0x000B  //! va of containing section (as in an image sectionhdr)
#define PE_REL_PPC_SECTION				0x000C  //! sectionheader number
#define PE_REL_PPC_IFGLUE				0x000D  //! substitute TOC restore instruction iff symbol is glue code
#define PE_REL_PPC_IMGLUE				0x000E  //! symbol is glue code; virtual address is TOC restore instruction
#define PE_REL_PPC_SECREL16				0x000F  //! va of containing section (limited to 16 bits)
#define PE_REL_PPC_REFHI				0x0010
#define PE_REL_PPC_REFLO				0x0011
#define PE_REL_PPC_PAIR					0x0012
#define PE_REL_PPC_SECRELLO				0x0013  //! Low 16-bit section relative reference (used for >32k TLS)
#define PE_REL_PPC_SECRELHI				0x0014  //! High 16-bit section relative reference (used for >32k TLS)
#define PE_REL_PPC_GPREL				0x0015
#define PE_REL_PPC_TOKEN				0x0016  //! clr token

#define PE_REL_PPC_TYPEMASK				0x00FF  //! mask to isolate above values in PE_RELOCATION.Type

// Flag bits in PE_RELOCATION.TYPE
#define PE_REL_PPC_NEG					0x0100  //! subtract reloc value rather than adding it
#define PE_REL_PPC_BRTAKEN				0x0200  //! fix branch prediction bit to predict branch taken
#define PE_REL_PPC_BRNTAKEN				0x0400  //! fix branch prediction bit to predict branch not taken
#define PE_REL_PPC_TOCDEFN				0x0800  //! toc slot defined in file (or, data in toc)

/**
* Hitachi SH3 relocation types.
*/
#define PE_REL_SH3_ABSOLUTE				0x0000  //! No relocation
#define PE_REL_SH3_DIRECT16				0x0001  //! 16 bit direct
#define PE_REL_SH3_DIRECT32				0x0002  //! 32 bit direct
#define PE_REL_SH3_DIRECT8				0x0003  //! 8 bit direct, -128..255
#define PE_REL_SH3_DIRECT8_WORD			0x0004  //! 8 bit direct .W (0 ext.)
#define PE_REL_SH3_DIRECT8_LONG			0x0005  //! 8 bit direct .L (0 ext.)
#define PE_REL_SH3_DIRECT4				0x0006  //! 4 bit direct (0 ext.)
#define PE_REL_SH3_DIRECT4_WORD			0x0007  //! 4 bit direct .W (0 ext.)
#define PE_REL_SH3_DIRECT4_LONG			0x0008  //! 4 bit direct .L (0 ext.)
#define PE_REL_SH3_PCREL8_WORD			0x0009  //! 8 bit PC relative .W
#define PE_REL_SH3_PCREL8_LONG			0x000A  //! 8 bit PC relative .L
#define PE_REL_SH3_PCREL12_WORD			0x000B  //! 12 LSB PC relative .W
#define PE_REL_SH3_STARTOF_SECTION		0x000C  //! Start of EXE section
#define PE_REL_SH3_SIZEOF_SECTION		0x000D  //! Size of EXE section
#define PE_REL_SH3_SECTION				0x000E  //! Section table index
#define PE_REL_SH3_SECREL				0x000F  //! Offset within section
#define PE_REL_SH3_DIRECT32_NB			0x0010  //! 32 bit direct not based
#define PE_REL_SH3_GPREL4_LONG			0x0011  //! GP-relative addressing
#define PE_REL_SH3_TOKEN				0x0012  //! clr token
#define PE_REL_SHM_PCRELPT				0x0013  //! Offset from current
//  instruction in longwords
//  if not NOMODE, insert the
//  inverse of the low bit at
//  bit 32 to select PTA/PTB
#define PE_REL_SHM_REFLO				0x0014  //! Low bits of 32-bit address
#define PE_REL_SHM_REFHALF				0x0015  //! High bits of 32-bit address
#define PE_REL_SHM_RELLO				0x0016  //! Low bits of relative reference
#define PE_REL_SHM_RELHALF				0x0017  //! High bits of relative reference
#define PE_REL_SHM_PAIR					0x0018  //! offset operand for relocation

#define PE_REL_SH_NOMODE				0x8000  //! relocation ignores section mode

/**
* ARM Relocation Types
*/
#define PE_REL_ARM_ABSOLUTE				0x0000  //! No relocation required
#define PE_REL_ARM_ADDR32				0x0001  //! 32 bit address
#define PE_REL_ARM_ADDR32NB				0x0002  //! 32 bit address w/o image base
#define PE_REL_ARM_BRANCH24				0x0003  //! 24 bit offset << 2 & sign ext.
#define PE_REL_ARM_BRANCH11				0x0004  //! Thumb: 2 11 bit offsets
#define PE_REL_ARM_TOKEN				0x0005  //! clr token
#define PE_REL_ARM_GPREL12				0x0006  //! GP-relative addressing (ARM)
#define PE_REL_ARM_GPREL7				0x0007  //! GP-relative addressing (Thumb)
#define PE_REL_ARM_BLX24				0x0008
#define PE_REL_ARM_BLX11				0x0009
#define PE_REL_ARM_SECTION				0x000E  //! Section table index
#define PE_REL_ARM_SECREL				0x000F  //! Offset within section
#define PE_REL_ARM_MOV32A				0x0010  //! ARM: MOVW/MOVT
#define PE_REL_ARM_MOV32				0x0010  //! ARM: MOVW/MOVT (deprecated)
#define PE_REL_ARM_MOV32T				0x0011  //! Thumb: MOVW/MOVT
#define PE_REL_THUMB_MOV32				0x0011  //! Thumb: MOVW/MOVT (deprecated)
#define PE_REL_ARM_BRANCH20T			0x0012  //! Thumb: 32-bit conditional B
#define PE_REL_THUMB_BRANCH20			0x0012  //! Thumb: 32-bit conditional B (deprecated)
#define PE_REL_ARM_BRANCH24T			0x0014  //! Thumb: 32-bit B or BL
#define PE_REL_THUMB_BRANCH24			0x0014  //! Thumb: 32-bit B or BL (deprecated)
#define PE_REL_ARM_BLX23T				0x0015  //! Thumb: BLX immediate
#define PE_REL_THUMB_BLX23				0x0015  //! Thumb: BLX immediate (deprecated)

#define PE_REL_AM_ABSOLUTE				0x0000
#define PE_REL_AM_ADDR32				0x0001
#define PE_REL_AM_ADDR32NB				0x0002
#define PE_REL_AM_CALL32				0x0003
#define PE_REL_AM_FUNCINFO				0x0004
#define PE_REL_AM_REL32_1				0x0005
#define PE_REL_AM_REL32_2				0x0006
#define PE_REL_AM_SECREL				0x0007
#define PE_REL_AM_SECTION				0x0008
#define PE_REL_AM_TOKEN					0x0009

/**
* x64 Relocations
*/
#define PE_REL_AMD64_ABSOLUTE			0x0000  //! Reference is absolute, no relocation is necessary
#define PE_REL_AMD64_ADDR64				0x0001  //! 64-bit address (VA).
#define PE_REL_AMD64_ADDR32				0x0002  //! 32-bit address (VA).
#define PE_REL_AMD64_ADDR32NB			0x0003  //! 32-bit address w/o image base (RVA).
#define PE_REL_AMD64_REL32				0x0004  //! 32-bit relative address from byte following reloc
#define PE_REL_AMD64_REL32_1			0x0005  //! 32-bit relative address from byte distance 1 from reloc
#define PE_REL_AMD64_REL32_2			0x0006  //! 32-bit relative address from byte distance 2 from reloc
#define PE_REL_AMD64_REL32_3			0x0007  //! 32-bit relative address from byte distance 3 from reloc
#define PE_REL_AMD64_REL32_4			0x0008  //! 32-bit relative address from byte distance 4 from reloc
#define PE_REL_AMD64_REL32_5			0x0009  //! 32-bit relative address from byte distance 5 from reloc
#define PE_REL_AMD64_SECTION			0x000A  //! Section index
#define PE_REL_AMD64_SECREL				0x000B  //! 32 bit offset from base of section containing target
#define PE_REL_AMD64_SECREL7			0x000C  //! 7 bit unsigned offset from base of section containing target
#define PE_REL_AMD64_TOKEN				0x000D  //! 32 bit metadata token
#define PE_REL_AMD64_SREL32				0x000E  //! 32 bit signed span-dependent value emitted into object
#define PE_REL_AMD64_PAIR				0x000F
#define PE_REL_AMD64_SSPAN32			0x0010  //! 32 bit signed span-dependent value applied at link time

/**
* IA64 Relocation Types
*/
#define PE_REL_IA64_ABSOLUTE			0x0000
#define PE_REL_IA64_IMM14				0x0001
#define PE_REL_IA64_IMM22				0x0002
#define PE_REL_IA64_IMM64				0x0003
#define PE_REL_IA64_DIR32				0x0004
#define PE_REL_IA64_DIR64				0x0005
#define PE_REL_IA64_PCREL21B			0x0006
#define PE_REL_IA64_PCREL21M			0x0007
#define PE_REL_IA64_PCREL21F			0x0008
#define PE_REL_IA64_GPREL22				0x0009
#define PE_REL_IA64_LTOFF22				0x000A
#define PE_REL_IA64_SECTION				0x000B
#define PE_REL_IA64_SECREL22			0x000C
#define PE_REL_IA64_SECREL64I			0x000D
#define PE_REL_IA64_SECREL32			0x000E
//
#define PE_REL_IA64_DIR32NB				0x0010
#define PE_REL_IA64_SREL14				0x0011
#define PE_REL_IA64_SREL22				0x0012
#define PE_REL_IA64_SREL32				0x0013
#define PE_REL_IA64_UREL32				0x0014
#define PE_REL_IA64_PCREL60X			0x0015  //! This is always a BRL and never converted
#define PE_REL_IA64_PCREL60B			0x0016
#define PE_REL_IA64_PCREL60F			0x0017
#define PE_REL_IA64_PCREL60I			0x0018
#define PE_REL_IA64_PCREL60M			0x0019
#define PE_REL_IA64_IMMGPREL64			0x001A
#define PE_REL_IA64_TOKEN				0x001B  //! clr token
#define PE_REL_IA64_GPREL32				0x001C
#define PE_REL_IA64_ADDEND				0x001F

/**
* CEF Relocation Types
*/
#define PE_REL_CEF_ABSOLUTE				0x0000  //! Reference is absolute, no relocation is necessary
#define PE_REL_CEF_ADDR32				0x0001  //! 32-bit address (VA).
#define PE_REL_CEF_ADDR64				0x0002  //! 64-bit address (VA).
#define PE_REL_CEF_ADDR32NB				0x0003  //! 32-bit address w/o image base (RVA).
#define PE_REL_CEF_SECTION				0x0004  //! Section index
#define PE_REL_CEF_SECREL				0x0005  //! 32 bit offset from base of section containing target
#define PE_REL_CEF_TOKEN				0x0006  //! 32 bit metadata token

/**
* CLR Relocation Types
*/
#define PE_REL_CEE_ABSOLUTE          0x0000  //! Reference is absolute, no relocation is necessary
#define PE_REL_CEE_ADDR32            0x0001  //! 32-bit address (VA).
#define PE_REL_CEE_ADDR64            0x0002  //! 64-bit address (VA).
#define PE_REL_CEE_ADDR32NB          0x0003  //! 32-bit address w/o image base (RVA).
#define PE_REL_CEE_SECTION           0x0004  //! Section index
#define PE_REL_CEE_SECREL            0x0005  //! 32 bit offset from base of section containing target
#define PE_REL_CEE_TOKEN             0x0006  //! 32 bit metadata token

#define PE_REL_M32R_ABSOLUTE         0x0000  //! No relocation required
#define PE_REL_M32R_ADDR32           0x0001  //! 32 bit address
#define PE_REL_M32R_ADDR32NB         0x0002  //! 32 bit address w/o image base
#define PE_REL_M32R_ADDR24           0x0003  //! 24 bit address
#define PE_REL_M32R_GPREL16          0x0004  //! GP relative addressing
#define PE_REL_M32R_PCREL24          0x0005  //! 24 bit offset << 2 & sign ext.
#define PE_REL_M32R_PCREL16          0x0006  //! 16 bit offset << 2 & sign ext.
#define PE_REL_M32R_PCREL8           0x0007  //! 8 bit offset << 2 & sign ext.
#define PE_REL_M32R_REFHALF          0x0008  //! 16 MSBs
#define PE_REL_M32R_REFHI            0x0009  //! 16 MSBs; adj for LSB sign ext.
#define PE_REL_M32R_REFLO            0x000A  //! 16 LSBs
#define PE_REL_M32R_PAIR             0x000B  //! Link HI and LO
#define PE_REL_M32R_SECTION          0x000C  //! Section table index
#define PE_REL_M32R_SECREL32         0x000D  //! 32 bit section relative reference
#define PE_REL_M32R_TOKEN            0x000E  //! clr token

#define PE_REL_EBC_ABSOLUTE          0x0000  //! No relocation required
#define PE_REL_EBC_ADDR32NB          0x0001  //! 32 bit address w/o image base
#define PE_REL_EBC_REL32             0x0002  //! 32-bit relative address from byte following reloc
#define PE_REL_EBC_SECTION           0x0003  //! Section table index
#define PE_REL_EBC_SECREL            0x0004  //! Offset within section

#define PE_EXT_IMM64(Val, Address, Size, InstPos, ValPos)  /* Intel-IA64-Filler */                              \
    Val |= (((UINT64)((*(Address) >> (InstPos)) & (((UINT64)1 << (Size)) - 1))) << (ValPos))  // Intel-IA64-Filler

#define PE_INS_IMM64(Val, Address, Size, InstPos, ValPos)  /* Intel-IA64-Filler */                              \
    *(UINT32*)(Address) = (*(UINT32*)(Address) & ~(((1 << (Size)) - 1) << (InstPos))) | /* Intel-IA64-Filler */ \
    ((UINT32)((((UINT64)(Val) >> (ValPos)) & (((UINT64)1 << (Size)) - 1))) << (InstPos))  // Intel-IA64-Filler

/**
* Intel-IA64-Filler
*/
#define FILLER_EMARCH_ENC_I17_IMM7B_INST_WORD_X         3
#define FILLER_EMARCH_ENC_I17_IMM7B_SIZE_X              7
#define FILLER_EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X     4
#define FILLER_EMARCH_ENC_I17_IMM7B_VAL_POS_X           0

#define FILLER_EMARCH_ENC_I17_IMM9D_INST_WORD_X         3
#define FILLER_EMARCH_ENC_I17_IMM9D_SIZE_X              9
#define FILLER_EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X     18
#define FILLER_EMARCH_ENC_I17_IMM9D_VAL_POS_X           7

#define FILLER_EMARCH_ENC_I17_IMM5C_INST_WORD_X         3
#define FILLER_EMARCH_ENC_I17_IMM5C_SIZE_X              5
#define FILLER_EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X     13
#define FILLER_EMARCH_ENC_I17_IMM5C_VAL_POS_X           16

#define FILLER_EMARCH_ENC_I17_IC_INST_WORD_X            3
#define FILLER_EMARCH_ENC_I17_IC_SIZE_X                 1
#define FILLER_EMARCH_ENC_I17_IC_INST_WORD_POS_X        12
#define FILLER_EMARCH_ENC_I17_IC_VAL_POS_X              21

#define FILLER_EMARCH_ENC_I17_IMM41a_INST_WORD_X        1
#define FILLER_EMARCH_ENC_I17_IMM41a_SIZE_X             10
#define FILLER_EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X    14
#define FILLER_EMARCH_ENC_I17_IMM41a_VAL_POS_X          22

#define FILLER_EMARCH_ENC_I17_IMM41b_INST_WORD_X        1
#define FILLER_EMARCH_ENC_I17_IMM41b_SIZE_X             8
#define FILLER_EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X    24
#define FILLER_EMARCH_ENC_I17_IMM41b_VAL_POS_X          32

#define FILLER_EMARCH_ENC_I17_IMM41c_INST_WORD_X        2
#define FILLER_EMARCH_ENC_I17_IMM41c_SIZE_X             23
#define FILLER_EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X    0
#define FILLER_EMARCH_ENC_I17_IMM41c_VAL_POS_X          40

#define FILLER_EMARCH_ENC_I17_SIGN_INST_WORD_X          3
#define FILLER_EMARCH_ENC_I17_SIGN_SIZE_X               1
#define FILLER_EMARCH_ENC_I17_SIGN_INST_WORD_POS_X      27
#define FILLER_EMARCH_ENC_I17_SIGN_VAL_POS_X            63

#define FILLER_X3_OPCODE_INST_WORD_X                    3
#define FILLER_X3_OPCODE_SIZE_X                         4
#define FILLER_X3_OPCODE_INST_WORD_POS_X                28
#define FILLER_X3_OPCODE_SIGN_VAL_POS_X                 0

#define FILLER_X3_I_INST_WORD_X                         3
#define FILLER_X3_I_SIZE_X                              1
#define FILLER_X3_I_INST_WORD_POS_X                     27
#define FILLER_X3_I_SIGN_VAL_POS_X                      59

#define FILLER_X3_D_WH_INST_WORD_X                      3
#define FILLER_X3_D_WH_SIZE_X                           3
#define FILLER_X3_D_WH_INST_WORD_POS_X                  24
#define FILLER_X3_D_WH_SIGN_VAL_POS_X                   0

#define FILLER_X3_IMM20_INST_WORD_X                     3
#define FILLER_X3_IMM20_SIZE_X                          20
#define FILLER_X3_IMM20_INST_WORD_POS_X                 4
#define FILLER_X3_IMM20_SIGN_VAL_POS_X                  0

#define FILLER_X3_IMM39_1_INST_WORD_X                   2
#define FILLER_X3_IMM39_1_SIZE_X                        23
#define FILLER_X3_IMM39_1_INST_WORD_POS_X               0
#define FILLER_X3_IMM39_1_SIGN_VAL_POS_X                36

#define FILLER_X3_IMM39_2_INST_WORD_X                   1
#define FILLER_X3_IMM39_2_SIZE_X                        16
#define FILLER_X3_IMM39_2_INST_WORD_POS_X               16
#define FILLER_X3_IMM39_2_SIGN_VAL_POS_X                20

#define FILLER_X3_P_INST_WORD_X                         3
#define FILLER_X3_P_SIZE_X                              4
#define FILLER_X3_P_INST_WORD_POS_X                     0
#define FILLER_X3_P_SIGN_VAL_POS_X                      0

#define FILLER_X3_TMPLT_INST_WORD_X                     0
#define FILLER_X3_TMPLT_SIZE_X                          4
#define FILLER_X3_TMPLT_INST_WORD_POS_X                 0
#define FILLER_X3_TMPLT_SIGN_VAL_POS_X                  0

#define FILLER_X3_BTYPE_QP_INST_WORD_X                  2
#define FILLER_X3_BTYPE_QP_SIZE_X                       9
#define FILLER_X3_BTYPE_QP_INST_WORD_POS_X              23
#define FILLER_X3_BTYPE_QP_INST_VAL_POS_X               0

#define FILLER_X3_EMPTY_INST_WORD_X                     1
#define FILLER_X3_EMPTY_SIZE_X                          2
#define FILLER_X3_EMPTY_INST_WORD_POS_X                 14
#define FILLER_X3_EMPTY_INST_VAL_POS_X                  0

/**
* Line Number Format
*/
typedef struct _PE_LINENUMBER {
    union {
        UINT32 SymbolTableIndex;        //!< Symbol table index of function name if Linenumber is 0.
        UINT32 VirtualAddress;          //!< Virtual address of line number.
    } Type;
    UINT16 Linenumber;                  //!< Line number.
} PE_LINENUMBER;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_LINENUMBER __unaligned *PPE_LINENUMBER;
#else
typedef PE_LINENUMBER *PPE_LINENUMBER;
#endif

#ifndef _MAC
#pragma pack(pop)					// Back to 4 byte packing again
#endif

/**
* Based Relocation Format
*/
typedef struct _PE_BASE_RELOCATION {
    UINT32 VirtualAddress;			    //!< RVA of the relocation block.
    UINT32 SizeOfBlock;				    //!< SizeOfBlock-sizeof(PE_BASE_RELOCATION) indicates how many TypeOffsets follow the SizeOfBlock.
                                        //  UINT16    TypeOffset[1];
} PE_BASE_RELOCATION;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_BASE_RELOCATION __unaligned *PPE_BASE_RELOCATION;
#else
typedef PE_BASE_RELOCATION *PPE_BASE_RELOCATION;
#endif


/**
* Archive Format
*/
#define PE_ARCHIVE_START_SIZE				8
#define PE_ARCHIVE_START					"!<arch>\n"
#define PE_ARCHIVE_END						"`\n"
#define PE_ARCHIVE_PAD						"\n"
#define PE_ARCHIVE_LINKER_MEMBER			"/               "
#define PE_ARCHIVE_LONGNAMES_MEMBER			"//              "

typedef struct _PE_ARCHIVE_MEMBER_HEADER {
    UINT8 Name[16];                     //!< File member name - '/' terminated.
    UINT8 Date[12];                     //!< File member date - decimal.
    UINT8 UserID[6];                    //!< File member user id - decimal.
    UINT8 GroupID[6];                   //!< File member group id - decimal.
    UINT8 Mode[8];                      //!< File member mode - octal.
    UINT8 Size[10];                     //!< File member size - decimal.
    UINT8 EndHeader[2];                 //!< String to end header.
} PE_ARCHIVE_MEMBER_HEADER, *PPE_ARCHIVE_MEMBER_HEADER;
#define PE_SIZEOF_ARCHIVE_MEMBER_HDR		60

/**
* DLL Support
*/

/**
* Export Format
*/
typedef struct _PE_EXPORT_DIRECTORY {
    UINT32 Characteristics;             //!< The characteristics of the export directory.
    UINT32 TimeDateStamp;               //!< Time and date stamp.
    UINT16 MajorVersion;                //!< Major Version.
    UINT16 MinorVersion;                //!< Minor Version.
    UINT32 Name;                        //!< Name of export.
    UINT32 Base;                        //!< Base of the export directory.
    UINT32 NumberOfFunctions;           //!< Number of exported functions.
    UINT32 NumberOfNames;               //!< Number of exported names.
    UINT32 AddressOfFunctions;          //!< RVA to the addresses of the functions.
    UINT32 AddressOfNames;              //!< RVA to the addresses of the names.
    UINT32 AddressOfNameOrdinals;		//!< RVA to the name ordinals.
} PE_EXPORT_DIRECTORY, *PPE_EXPORT_DIRECTORY;

/**
* Import Format
*/
typedef struct _PE_IMPORT_BY_NAME {
    UINT16 Hint;
    CHAR Name[1];
} PE_IMPORT_BY_NAME, *PPE_IMPORT_BY_NAME;

#pragma pack(push, 8)		// Use align 8 for the 64-bit IAT.

typedef struct _PE_THUNK_DATA64 {
    union {
        UINT64 ForwarderString;		    //!< PBYTE
        UINT64 Function;				//!< PDWORD
        UINT64 Ordinal;				    //!< Orindal value.
        UINT64 AddressOfData;			//!< Points to the address in the IAT or to an PE_IMPORT_BY_NAME struct.
    };
} PE_THUNK_DATA64;
typedef PE_THUNK_DATA64 *PPE_THUNK_DATA64;

#pragma pack(pop)			// Back to 4 byte packing

typedef struct _PE_THUNK_DATA32 {
    union {
        UINT32 ForwarderString;		    //!< PBYTE
        UINT32 Function;				//!< PDWORD
        UINT32 Ordinal;				    //!< Orindal value.
        UINT32 AddressOfData;			//!< Points to the address in the IAT or to an PE_IMPORT_BY_NAME struct.
    };
} PE_THUNK_DATA32;
typedef PE_THUNK_DATA32 *PPE_THUNK_DATA32;

#define PE_ORDINAL_FLAG64				0x8000000000000000
#define PE_ORDINAL_FLAG32				0x80000000
#define PE_ORDINAL64(Ordinal)			((Ordinal) & 0xFFFF)
#define PE_ORDINAL32(Ordinal)			((Ordinal) & 0xFFFF)
#define PE_SNAP_BY_ORDINAL64(Ordinal)	(((Ordinal) & PE_ORDINAL_FLAG64) != 0)
#define PE_SNAP_BY_ORDINAL32(Ordinal)	(((Ordinal) & PE_ORDINAL_FLAG32) != 0)

typedef struct _PE_IMPORT_DESCRIPTOR {
    union {
        UINT32 Characteristics;	        //!< Characteristics of import, 0 for terminating null import descriptor.
        UINT32 OriginalFirstThunk;      //!< RVA to original unbound IAT (PPE_THUNK_DATA)
    };
    UINT32 TimeDateStamp;               //!< 0 if not bound, -1 if bound, and real date\time stamp in PE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND), otherwise date/time stamp of DLL bound to (Old BIND)
    UINT32 ForwarderChain;              //!< -1 if no forwarders.
    UINT32 Name;                        //!< RVA to the name of the DLL.
    UINT32 FirstThunk;                  //!< RVA to IAT (if bound this IAT has actual addresses)
} PE_IMPORT_DESCRIPTOR;
#if defined(_MSC_VER) && (defined(_M_X64) || defined(__x86_64__))
typedef PE_IMPORT_DESCRIPTOR __unaligned *PPE_IMPORT_DESCRIPTOR;
#else
typedef PE_IMPORT_DESCRIPTOR *PPE_IMPORT_DESCRIPTOR;
#endif

/**
* New format import descriptors pointed to by DataDirectory[PE_DIRECTORY_ENTRY_BOUND_IMPORT]
*/
typedef struct _PE_BOUND_IMPORT_DESCRIPTOR {
    UINT32 TimeDateStamp;               //!< Time and date stamp.
    UINT16 OffsetModuleName;            //!< Offset to module name.
    UINT16 NumberOfModuleForwarderRefs; //!< The number of PE_BOUND_FORWARDER_REF structures that follow.
                                        //PE_BOUND_FORWARDER_REF ForwarderRefs[0]	//!< Array of zero or more PE_BOUND_FORWARDER_REF follows
} PE_BOUND_IMPORT_DESCRIPTOR, *PPE_BOUND_IMPORT_DESCRIPTOR;

typedef struct _PE_BOUND_FORWARDER_REF {
    UINT32 TimeDateStamp;               //!< Time and date stamp.
    UINT16 OffsetModuleName;            //!< Offset to module name.
    UINT16 Reserved;                    //!< Reserved.
} PE_BOUND_FORWARDER_REF, *PPE_BOUND_FORWARDER_REF;

typedef struct _PE_DELAYLOAD_DESCRIPTOR {
    union {
        UINT32 AllAttributes;
        struct {
            UINT32 RvaBased : 1;          //!< Delay load version 2
            UINT32 ReservedAttribs : 31;  //!< Reserved.
        };
    } Attributes;
    UINT32 DllNameRVA;                  //!< RVA to the name of the target library (NULL-terminate ASCII string)
    UINT32 ModuleHandleRVA;             //!< RVA to the HMODULE caching location (PHMODULE)
    UINT32 ImportAddressTableRVA;       //!< RVA to the start of the IAT (PPE_THUNK_DATA)
    UINT32 ImportNameTableRVA;	        //!< RVA to the start of the name table (PPE_THUNK_DATA::AddressOfData)
    UINT32 BoundImportAddressTableRVA;	//!< RVA to an optional bound IAT
    UINT32 UnloadInformationTableRVA;   //!< RVA to an optional unload info table
    UINT32 TimeDateStamp;               //!< 0 if not bound, otherwise, date/time of the target DLL
} PE_DELAYLOAD_DESCRIPTOR, *PPE_DELAYLOAD_DESCRIPTOR;
typedef const PE_DELAYLOAD_DESCRIPTOR *PCPE_DELAYLOAD_DESCRIPTOR;


/**
* Thread Local Storage
*/

#if defined(__llvm__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wattributes"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

// TLS Callback function type
typedef VOID( NTAPI *PPE_TLS_CALLBACK )(VOID*, UINT32, VOID*);

#if defined(__llvm__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

typedef struct _PE_TLS_DIRECTORY64 {
    UINT64 StartAddressOfRawData;		//!< Start address of the raw data.
    UINT64 EndAddressOfRawData;		    //!< End address of the raw data.
    UINT64 AddressOfIndex;			    //!< Address of index (pointer to TLS index - PDWORD)
    UINT64 AddressOfCallBacks;		    //!< Address of the callbacks (PPE_TLS_CALLBACK*)
    UINT32 SizeOfZeroFill;              //!< Size of zero fill.
    union {
        UINT32 Characteristics;         //!< Characteristics.
        struct {
            UINT32 Reserved0 : 20;
            UINT32 Alignment : 4;
            UINT32 Reserved1 : 8;
        };
    };
} PE_TLS_DIRECTORY64;
typedef PE_TLS_DIRECTORY64 * PPE_TLS_DIRECTORY64;

typedef struct _PE_TLS_DIRECTORY32 {
    UINT32 StartAddressOfRawData;		//!< Start address of the raw data.
    UINT32 EndAddressOfRawData;		    //!< End address of the raw data.
    UINT32 AddressOfIndex;			    //!< Address of index (pointer to TLS index - PDWORD)
    UINT32 AddressOfCallBacks;		    //!< Address of the callbacks (PPE_TLS_CALLBACK*)
    UINT32 SizeOfZeroFill;			    //!< Size of zero fill.
    union {
        UINT32 Characteristics;		    //!< Characteristics.
        struct {
            UINT32 Reserved0 : 20;
            UINT32 Alignment : 4;
            UINT32 Reserved1 : 8;
        };
    };
} PE_TLS_DIRECTORY32;
typedef PE_TLS_DIRECTORY32 *PPE_TLS_DIRECTORY32;

#if defined(_M_X64) || defined(__x86_64__)
#define PE_ORDINAL_FLAG					PE_ORDINAL_FLAG64
#define PE_ORDINAL(Ordinal)				PE_ORDINAL64(Ordinal)
typedef PE_THUNK_DATA64					PE_THUNK_DATA;
typedef PPE_THUNK_DATA64				PPE_THUNK_DATA;
#define PE_SNAP_BY_ORDINAL(Ordinal)		PE_SNAP_BY_ORDINAL64(Ordinal)
typedef PE_TLS_DIRECTORY64				PE_TLS_DIRECTORY;
typedef PPE_TLS_DIRECTORY64				PPE_TLS_DIRECTORY;
#else
#define PE_ORDINAL_FLAG					PE_ORDINAL_FLAG32
#define PE_ORDINAL(Ordinal)				PE_ORDINAL32(Ordinal)
typedef PE_THUNK_DATA32					PE_THUNK_DATA;
typedef PPE_THUNK_DATA32				PPE_THUNK_DATA;
#define PE_SNAP_BY_ORDINAL(Ordinal)		PE_SNAP_BY_ORDINAL32(Ordinal)
typedef PE_TLS_DIRECTORY32				PE_TLS_DIRECTORY;
typedef PPE_TLS_DIRECTORY32				PPE_TLS_DIRECTORY;
#endif

/**
* Resource Format
*/

// Resource directory consists of two counts, following by a variable length
// array of directory entries.  The first count is the number of entries at
// beginning of the array that have actual names associated with each entry.
// The entries are in ascending order, case insensitive strings.  The second
// count is the number of entries that immediately follow the named entries.
// This second count identifies the number of entries that have 16-bit integer
// Ids as their name.  These entries are also sorted in ascending order.
//
// This structure allows fast lookup by either name or number, but for any
// given resource entry only one form of lookup is supported, not both.
// This is consistant with the syntax of the .RC file and the .RES file.

typedef struct _PE_RESOURCE_DIRECTORY {
    UINT32 Characteristics;                 //!< Resource directory characteristics.
    UINT32 TimeDateStamp;                   //!< Time and date stamp.
    UINT16 MajorVersion;                    //!< Major version.
    UINT16 MinorVersion;                    //!< Minor version.
    UINT16 NumberOfNamedEntries;            //!< Number of named entries.
    UINT16 NumberOfIdEntries;               //!< Number of ID entries.
                                            //PE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[1];
} PE_RESOURCE_DIRECTORY, *PPE_RESOURCE_DIRECTORY;

#define PE_RESOURCE_NAME_IS_STRING        0x80000000
#define PE_RESOURCE_DATA_IS_DIRECTORY     0x80000000

// Each directory contains the 32-bit Name of the entry and an offset,
// relative to the beginning of the resource directory of the data associated
// with this directory entry.  If the name of the entry is an actual text
// string instead of an integer Id, then the high order bit of the name field
// is set to one and the low order 31-bits are an offset, relative to the
// beginning of the resource directory of the string, which is of type
// PE_RESOURCE_DIRECTORY_STRING.  Otherwise the high bit is clear and the
// low-order 16-bits are the integer Id that identify this resource directory
// entry. If the directory entry is yet another resource directory (i.e. a
// subdirectory), then the high order bit of the offset field will be
// set to indicate this.  Otherwise the high bit is clear and the offset
// field points to a resource data entry.

typedef struct _PE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            UINT32 NameOffset : 31;           //!< Offset to the resource name.
            UINT32 NameIsString : 1;          //!< True if the entry is a resource with a name.
        };
        UINT32 Name;                        //!< Address of the name if its a named resource.
        UINT16 Id;                          //!< The ID if its an ID resource.
    };
    union {
        UINT32 OffsetToData;                //!< Offset to the data.
        struct {
            UINT32 OffsetToDirectory : 31;    //!< Offset to the next directory.
            UINT32 DataIsDirectory : 1;       //!< True if the entry data is a directory
        };
    };
} PE_RESOURCE_DIRECTORY_ENTRY, *PPE_RESOURCE_DIRECTORY_ENTRY;

// For resource directory entries that have actual string names, the Name
// field of the directory entry points to an object of the following type.
// All of these string objects are stored together after the last resource
// directory entry and before the first resource data object.  This minimizes
// the impact of these variable length objects on the alignment of the fixed
// size directory entry objects.

typedef struct _PE_RESOURCE_DIRECTORY_STRING {
    UINT16 Length;                          //!< Length of the string in Unicode characters, in bytes.
    INT8 NameString[1];                     //!< Pointer to the ANSI string.
} PE_RESOURCE_DIRECTORY_STRING, *PPE_RESOURCE_DIRECTORY_STRING;

typedef struct _PE_RESOURCE_DIR_STRING_U {
    UINT16 Length;                          //!< Length of the string in Unicode characters, NOT in bytes.
    INT16 NameString[1];                    //!< Pointer to the Unicode string.
} PE_RESOURCE_DIR_STRING_U, *PPE_RESOURCE_DIR_STRING_U;

// Each resource data entry describes a leaf node in the resource directory
// tree.  It contains an offset, relative to the beginning of the resource
// directory of the data for the resource, a size field that gives the number
// of bytes of data at that offset, a CodePage that should be used when
// decoding code point values within the resource data.  Typically for new
// applications the code page would be the unicode code page.

typedef struct _PE_RESOURCE_DATA_ENTRY {
    UINT32 OffsetToData;                    //!< Offset to the data of the resource.
    UINT32 Size;                            //!< Size of the resource data.
    UINT32 CodePage;                        //!< Code page.
    UINT32 Reserved;                        //!< Reserved for use by the operating system.
} PE_RESOURCE_DATA_ENTRY, *PPE_RESOURCE_DATA_ENTRY;

/**
* Load Configuration Directory Entry
*/
typedef struct _PE_LOAD_CONFIG_DIRECTORY32 {
    UINT32 Size;			                //!< The size of the structure.
    UINT32 TimeDateStamp;	                //!< The date and time stamp value.
    UINT16 MajorVersion;	                //!< The major version number.
    UINT16 MinorVersion;	                //!< The minor version number.
    UINT32 GlobalFlagsClear;                //!< The global flags that control system behavior.
    UINT32 GlobalFlagsSet;					//!< The global flags that control system behavior.
    UINT32 CriticalSectionDefaultTimeout;   //!< The critical section default time-out value.
    UINT32 DeCommitFreeBlockThreshold;      //!< The size of the minimum block that must be freed before it is freed (de-committed), in bytes.
    UINT32 DeCommitTotalFreeThreshold;      //!< The size of the minimum total memory that must be freed in the process heap before it is freed (de-committed), in bytes.
    UINT32 LockPrefixTable;                 //!< The VA of a list of addresses where the LOCK prefix is used.
    UINT32 MaximumAllocationSize;           //!< The maximum allocation size, in bytes.
    UINT32 VirtualMemoryThreshold;			//!< The maximum block size that can be allocated from heap segments, in bytes.
    UINT32 ProcessHeapFlags;                //!< The process heap flags.
    UINT32 ProcessAffinityMask;             //!< The process affinity mask.
    UINT16 CSDVersion;		                //!< The service pack version.
    UINT16 Reserved1;                       //!< Reserved for use by the operating system.
    UINT32 EditList;                        //!< Reserved for use by the system.
    UINT32 SecurityCookie;                  //!< A pointer to a cookie that is used by Visual C++ or GS implementation.
    UINT32 SEHandlerTable;                  //!< The VA of the sorted table of RVAs of each valid, unique handler in the image.
    UINT32 SEHandlerCount;                  //!< The count of unique handlers in the table.
    UINT32 GuardCFCheckFunctionPointer;		//!< Control flow guard (Win 8.1 and up) function pointer.
    UINT32 Reserved2;                       //!< Reserved for use by the operating system.
    UINT32 GuardCFFunctionTable;            //!< Pointer to the control flow guard function table. Only on Win 8.1 and up.
    UINT32 GuardCFFunctionCount;            //!< Count of functions under control flow guard. Only on Win 8.1 and up.
    UINT32 GuardFlags;                      //!< Flags for the control flow guard. Only on Win 8.1 and up.
} PE_LOAD_CONFIG_DIRECTORY32, *PPE_LOAD_CONFIG_DIRECTORY32;

typedef struct _PE_LOAD_CONFIG_DIRECTORY64 {
    UINT32 Size;                            //!< The size of the structure.
    UINT32 TimeDateStamp;                   //!< The date and time stamp value.
    UINT16 MajorVersion;                    //!< The major version number.
    UINT16 MinorVersion;                    //!< The minor version number.
    UINT32 GlobalFlagsClear;                //!< The global flags that control system behavior.
    UINT32 GlobalFlagsSet;                  //!< The global flags that control system behavior.
    UINT32 CriticalSectionDefaultTimeout;   //!< The critical section default time-out value.
    UINT64 DeCommitFreeBlockThreshold;      //!< The size of the minimum block that must be freed before it is freed (de-committed), in bytes.
    UINT64 DeCommitTotalFreeThreshold;      //!< The size of the minimum total memory that must be freed in the process heap before it is freed (de-committed), in bytes.
    UINT64 LockPrefixTable;                 //!< The VA of a list of addresses where the LOCK prefix is used.
    UINT64 MaximumAllocationSize;           //!< The maximum allocation size, in bytes.
    UINT64 VirtualMemoryThreshold;          //!< The maximum block size that can be allocated from heap segments, in bytes.
    UINT64 ProcessAffinityMask;             //!< The process affinity mask.
    UINT32 ProcessHeapFlags;                //!< The process heap flags.
    UINT16 CSDVersion;                      //!< The service pack version.
    UINT16 Reserved1;                       //!< Reserved for use by the operating system.
    UINT64 EditList;                        //!< Reserved for use by the system.
    UINT64 SecurityCookie;                  //!< A pointer to a cookie that is used by Visual C++ or GS implementation.
    UINT64 SEHandlerTable;                  //!< The VA of the sorted table of RVAs of each valid, unique handler in the image.
    UINT64 SEHandlerCount;                  //!< The count of unique handlers in the table.
    UINT64 GuardCFCheckFunctionPointer;     //!< Control flow guard (Win 8.1 and up) function pointer.
    UINT64 Reserved2;                       //!< Reserved for use by the operating system.
    UINT64 GuardCFFunctionTable;            //!< Pointer to the control flow guard function table. Only on Win 8.1 and up.
    UINT64 GuardCFFunctionCount;            //!< Count of functions under control flow guard. Only on Win 8.1 and up.
    UINT32 GuardFlags;                      //!< Flags for the control flow guard. Only on Win 8.1 and up.
} PE_LOAD_CONFIG_DIRECTORY64, *PPE_LOAD_CONFIG_DIRECTORY64;

#if defined(_M_X64) || defined(__x86_64__)
typedef PE_LOAD_CONFIG_DIRECTORY64     PE_LOAD_CONFIG_DIRECTORY;
typedef PPE_LOAD_CONFIG_DIRECTORY64    PPE_LOAD_CONFIG_DIRECTORY;
#else
typedef PE_LOAD_CONFIG_DIRECTORY32     PE_LOAD_CONFIG_DIRECTORY;
typedef PPE_LOAD_CONFIG_DIRECTORY32    PPE_LOAD_CONFIG_DIRECTORY;
#endif

#define PE_GUARD_CF_INSTRUMENTED						0x00000100
#define PE_GUARD_CFW_INSTRUMENTED						0x00000200
#define PE_GUARD_CF_FUNCTION_TABLE_PRESENT				0x00000400
#define PE_GUARD_SECURITY_COOKIE_UNUSED					0x00000800
#define PE_GUARD_PROTECT_DELAYLOAD_IAT					0x00001000
#define PE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION		0x00002000
#define PE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK			0xF0000000
#define PE_GUARD_CF_FUNCTION_TABLE_SIZE_SHIFT			28

/**
* WIN CE Exception table format
*/

//
// Function table entry format.  Function table is pointed to by the
// PE_DIRECTORY_ENTRY_EXCEPTION directory entry.
//

typedef struct _PE_CE_RUNTIME_FUNCTION_ENTRY {
    UINT32 FuncStart;
    UINT32 PrologLen : 8;
    UINT32 FuncLen : 22;
    UINT32 ThirtyTwoBit : 1;
    UINT32 ExceptionFlag : 1;
} PE_CE_RUNTIME_FUNCTION_ENTRY, *PPE_CE_RUNTIME_FUNCTION_ENTRY;

typedef struct _PE_ARM_RUNTIME_FUNCTION_ENTRY {
    UINT32 BeginAddress;
    union {
        UINT32 UnwindData;
        struct {
            UINT32 Flag : 2;
            UINT32 FunctionLength : 11;
            UINT32 Ret : 2;
            UINT32 H : 1;
            UINT32 Reg : 3;
            UINT32 R : 1;
            UINT32 L : 1;
            UINT32 C : 1;
            UINT32 StackAdjust : 10;
        };
    };
} PE_ARM_RUNTIME_FUNCTION_ENTRY, *PPE_ARM_RUNTIME_FUNCTION_ENTRY;

typedef struct _PE_ALPHA64_RUNTIME_FUNCTION_ENTRY {
    UINT64 BeginAddress;
    UINT64 EndAddress;
    UINT64 ExceptionHandler;
    UINT64 HandlerData;
    UINT64 PrologEndAddress;
} PE_ALPHA64_RUNTIME_FUNCTION_ENTRY, *PPE_ALPHA64_RUNTIME_FUNCTION_ENTRY;

typedef struct _PE_ALPHA_RUNTIME_FUNCTION_ENTRY {
    UINT32 BeginAddress;
    UINT32 EndAddress;
    UINT32 ExceptionHandler;
    UINT32 HandlerData;
    UINT32 PrologEndAddress;
} PE_ALPHA_RUNTIME_FUNCTION_ENTRY, *PPE_ALPHA_RUNTIME_FUNCTION_ENTRY;

typedef struct _PE_RUNTIME_FUNCTION_ENTRY {
    UINT32 BeginAddress;
    UINT32 EndAddress;
    union {
        UINT32 UnwindInfoAddress;
        UINT32 UnwindData;
    };
} _PE_RUNTIME_FUNCTION_ENTRY, *_PPE_RUNTIME_FUNCTION_ENTRY;

typedef _PE_RUNTIME_FUNCTION_ENTRY  PE_IA64_RUNTIME_FUNCTION_ENTRY;
typedef _PPE_RUNTIME_FUNCTION_ENTRY PPE_IA64_RUNTIME_FUNCTION_ENTRY;

#if defined(_AXP64_)
typedef  PE_ALPHA64_RUNTIME_FUNCTION_ENTRY  PE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef PPE_ALPHA64_RUNTIME_FUNCTION_ENTRY PPE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef  PE_ALPHA64_RUNTIME_FUNCTION_ENTRY  PE_RUNTIME_FUNCTION_ENTRY;
typedef PPE_ALPHA64_RUNTIME_FUNCTION_ENTRY PPE_RUNTIME_FUNCTION_ENTRY;
#elif defined(_ALPHA_)
typedef  PE_ALPHA_RUNTIME_FUNCTION_ENTRY  PE_RUNTIME_FUNCTION_ENTRY;
typedef PPE_ALPHA_RUNTIME_FUNCTION_ENTRY PPE_RUNTIME_FUNCTION_ENTRY;
#elif defined(_ARM_)
typedef  PE_ARM_RUNTIME_FUNCTION_ENTRY  PE_RUNTIME_FUNCTION_ENTRY;
typedef PPE_ARM_RUNTIME_FUNCTION_ENTRY PPE_RUNTIME_FUNCTION_ENTRY;
#else
typedef  _PE_RUNTIME_FUNCTION_ENTRY  PE_RUNTIME_FUNCTION_ENTRY;
typedef _PPE_RUNTIME_FUNCTION_ENTRY PPE_RUNTIME_FUNCTION_ENTRY;
#endif

/**
* Debug Format
*/

typedef struct _PE_DEBUG_DIRECTORY {
    UINT32 Characteristics;
    UINT32 TimeDateStamp;
    UINT16 MajorVersion;
    UINT16 MinorVersion;
    UINT32 Type;
    UINT32 SizeOfData;
    UINT32 AddressOfRawData;
    UINT32 PointerToRawData;
} PE_DEBUG_DIRECTORY, *PPE_DEBUG_DIRECTORY;

#define PE_DEBUG_TYPE_UNKNOWN          0
#define PE_DEBUG_TYPE_COFF             1
#define PE_DEBUG_TYPE_CODEVIEW         2
#define PE_DEBUG_TYPE_FPO              3
#define PE_DEBUG_TYPE_MISC             4
#define PE_DEBUG_TYPE_EXCEPTION        5
#define PE_DEBUG_TYPE_FIXUP            6
#define PE_DEBUG_TYPE_OMAP_TO_SRC      7
#define PE_DEBUG_TYPE_OMAP_FROM_SRC    8
#define PE_DEBUG_TYPE_BORLAND          9
#define PE_DEBUG_TYPE_RESERVED10       10
#define PE_DEBUG_TYPE_CLSID            11

typedef struct _PE_COFF_SYMBOLS_HEADER {
    UINT32 NumberOfSymbols;
    UINT32 LvaToFirstSymbol;
    UINT32 NumberOfLinenumbers;
    UINT32 LvaToFirstLinenumber;
    UINT32 RvaToFirstByteOfCode;
    UINT32 RvaToLastByteOfCode;
    UINT32 RvaToFirstByteOfData;
    UINT32 RvaToLastByteOfData;
} PE_COFF_SYMBOLS_HEADER, *PPE_COFF_SYMBOLS_HEADER;

#define FRAME_FPO       0
#define FRAME_TRAP      1
#define FRAME_TSS       2
#define FRAME_NONFPO    3

typedef struct _PE_FPO_DATA {
    UINT32 ulOffStart;              //!< offset 1st byte of function code
    UINT32 cbProcSize;              //!< # bytes in function
    UINT32 cdwLocals;               //!< # bytes in locals/4
    UINT16 cdwParams;               //!< # bytes in params/4
    UINT16 cbProlog : 8;            //!< # bytes in prolog
    UINT16 cbRegs : 3;              //!< # regs saved
    UINT16 fHasSEH : 1;             //!< TRUE if SEH in func
    UINT16 fUseBP : 1;              //!< TRUE if EBP has been allocated
    UINT16 reserved : 1;            //!< reserved for future use
    UINT16 cbFrame : 2;             //!< frame type
} PE_FPO_DATA, *PPE_FPO_DATA;
#define SIZEOF_RFPO_DATA 16

#define PE_DEBUG_MISC_EXENAME    1
typedef struct _PE_DEBUG_MISC {
    UINT32 DataType;                //!< type of misc data, see defines
    UINT32 Length;                  //!< total length of record, rounded to four byte multiple.
    UINT8 Unicode;                  //!< TRUE if data is unicode string
    UINT8 Reserved[3];              //!< Reserved
    UINT8 Data[1];                  //!< Actual data
} PE_DEBUG_MISC, *PPE_DEBUG_MISC;

// Function table extracted from MIPS/ALPHA/IA64 images.  Does not contain
// information needed only for runtime support.  Just those fields for
// each entry needed by a debugger.
typedef struct _PE_FUNCTION_ENTRY {
    UINT32 StartingAddress;
    UINT32 EndingAddress;
    UINT32 EndOfPrologue;
} PE_FUNCTION_ENTRY, *PPE_FUNCTION_ENTRY;

typedef struct _PE_FUNCTION_ENTRY64 {
    UINT64 StartingAddress;
    UINT64 EndingAddress;
    union {
        UINT64 EndOfPrologue;
        UINT64 UnwindInfoAddress;
    };
} PE_FUNCTION_ENTRY64, *PPE_FUNCTION_ENTRY64;

// Debugging information can be stripped from an image file and placed
// in a separate .DBG file, whose file name part is the same as the
// image file name part (e.g. symbols for CMD.EXE could be stripped
// and placed in CMD.DBG).  This is indicated by the PE_FILE_DEBUG_STRIPPED
// flag in the Characteristics field of the file header.  The beginning of
// the .DBG file contains the following structure which captures certain
// information from the image file.  This allows a debug to proceed even if
// the original image file is not accessable.  This header is followed by
// zero of more PE_SECTION_HEADER structures, followed by zero or more
// PE_DEBUG_DIRECTORY structures.  The latter structures and those in
// the image file contain file offsets relative to the beginning of the
// .DBG file.
//
// If symbols have been stripped from an image, the PE_DEBUG_MISC structure
// is left in the image file, but not mapped.  This allows a debugger to
// compute the name of the .DBG file, from the name of the image in the
// PE_DEBUG_MISC structure.
typedef struct _PE_SEPARATE_DEBUG_HEADER {
    UINT16 Signature;
    UINT16 Flags;
    UINT16 Machine;
    UINT16 Characteristics;
    UINT32 TimeDateStamp;
    UINT32 CheckSum;
    UINT32 ImageBase;
    UINT32 SizeOfImage;
    UINT32 NumberOfSections;
    UINT32 ExportedNamesSize;
    UINT32 DebugDirectorySize;
    UINT32 SectionAlignment;
    UINT32 Reserved[2];
} PE_SEPARATE_DEBUG_HEADER, *PPE_SEPARATE_DEBUG_HEADER;

typedef struct _PE_NON_PAGED_DEBUG_INFO {
    UINT16 Signature;
    UINT16 Flags;
    UINT32 Size;
    UINT16 Machine;
    UINT16 Characteristics;
    UINT32 TimeDateStamp;
    UINT32 CheckSum;
    UINT32 SizeOfImage;
    UINT64 ImageBase;
    //DebugDirectorySize
    //PE_DEBUG_DIRECTORY
} PE_NON_PAGED_DEBUG_INFO, *PPE_NON_PAGED_DEBUG_INFO;

#ifndef _MAC
#define PE_SEPARATE_DEBUG_SIGNATURE			0x4944
#define PE_NON_PAGED_DEBUG_SIGNATURE		0x494E
#else
#define PE_SEPARATE_DEBUG_SIGNATURE			0x4449  // DI
#define PE_NON_PAGED_DEBUG_SIGNATURE		0x4E49  // NI
#endif

#define PE_SEPARATE_DEBUG_FLAGS_MASK		0x8000
#define PE_SEPARATE_DEBUG_MISMATCH			0x8000  // when DBG was updated, the old checksum didn't match.

//  The .arch section is made up of headers, each describing an amask position/value
//  pointing to an array of PE_ARCHITECTURE_ENTRY's.  Each "array" (both the header
//  and entry arrays) are terminiated by a quadword of 0xffffffffL.
//
//  NOTE: There may be quadwords of 0 sprinkled around and must be skipped.

typedef struct _PE_ARCHITECTURE_HEADER {
    UINT32 AmaskValue : 1;                //!< 1 -> code section depends on mask bit
    INT32 Adummy1 : 7;                    //!< MBZ 0 -> new instruction depends on mask bit
    UINT32 AmaskShift : 8;                //!< Amask bit in question for this fixup
    INT32 Adummy2 : 16;                   //!< MBZ
    UINT32 FirstEntryRVA;               //!< RVA into .arch section to array of ARCHITECTURE_ENTRY's
} PE_ARCHITECTURE_HEADER, *PPE_ARCHITECTURE_HEADER;

typedef struct _PE_ARCHITECTURE_ENTRY {
    UINT32 FixupInstRVA;                //!< RVA of instruction to fixup
    UINT32 NewInst;                     //!< fixup instruction (see alphaops.h)
} PE_ARCHITECTURE_ENTRY, *PPE_ARCHITECTURE_ENTRY;

#pragma pack(pop)				// Back to the initial value

// The following structure defines the new import object.  Note the values of the first two fields,
// which must be set as stated in order to differentiate old and new import members.
// Following this structure, the linker emits two null-terminated strings used to recreate the
// import at the time of use.  The first string is the import's name, the second is the dll's name.

#define PE_IMPORT_OBJECT_HDR_SIG2		0xFFFF

typedef struct _PE_IMPORT_OBJECT_HEADER {
    UINT16 Sig1;                        //!< Must be PE_FILE_MACHINE_UNKNOWN
    UINT16 Sig2;                        //!< Must be PE_IMPORT_OBJECT_HDR_SIG2.
    UINT16 Version;
    UINT16 Machine;
    UINT32 TimeDateStamp;               //!< Time/date stamp
    UINT32 SizeOfData;	                //!< particularly useful for incremental links
    union {
        UINT16 Ordinal;                 //!< if grf & IMPORT_OBJECT_ORDINAL
        UINT16 Hint;                    //!< Hint
    };
    UINT16 Type : 2;                      //!< PE_IMPORT_TYPE
    UINT16 NameType : 3;                  //!< PE_IMPORT_NAME_TYPE
    UINT16 Reserved : 11;                 //!< Reserved. Must be zero.
} PE_IMPORT_OBJECT_HEADER;

typedef enum _PE_IMPORT_OBJECT_TYPE {
    PE_IMPORT_OBJECT_CODE = 0,
    PE_IMPORT_OBJECT_DATA = 1,
    PE_IMPORT_OBJECT_CONST = 2,
} PE_IMPORT_OBJECT_TYPE;

typedef enum _PE_IMPORT_OBJECT_NAME_TYPE {
    PE_IMPORT_OBJECT_ORDINAL = 0,          //!< Import by ordinal
    PE_IMPORT_OBJECT_NAME = 1,             //!< Import name == public symbol name.
    PE_IMPORT_OBJECT_NAME_NO_PREFIX = 2,   //!< Import name == public symbol name skipping leading ?, @, or optionally _.
    PE_IMPORT_OBJECT_NAME_UNDECORATE = 3,  //!< Import name == public symbol name skipping leading ?, @, or optionally _ and truncating at first @
} PE_IMPORT_OBJECT_NAME_TYPE;

#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines {
    // COM+ Header entry point flags.
    COMIMAGE_FLAGS_ILONLY = 0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED = 0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY = 0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED = 0x00000008,
    COMIMAGE_FLAGS_NATIVE_ENTRYPOINT = 0x00000010,
    COMIMAGE_FLAGS_TRACKDEBUGDATA = 0x00010000,

    // Version flags for image.
    COR_VERSION_MAJOR_V2 = 2,
    COR_VERSION_MAJOR = COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR = 5,
    COR_DELETED_NAME_LENGTH = 8,
    COR_VTABLEGAP_NAME_LENGTH = 8,

    // Maximum size of a NativeType descriptor.
    NATIVE_TYPE_MAX_CB = 1,
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE = 0xFF,

    // #defines for the MIH FLAGS
    IMAGE_COR_MIH_METHODRVA = 0x01,
    IMAGE_COR_MIH_EHRVA = 0x02,
    IMAGE_COR_MIH_BASICBLOCK = 0x08,

    // V-table constants
    COR_VTABLE_32BIT = 0x01,                // V-table slots are 32-bits in size.
    COR_VTABLE_64BIT = 0x02,                // V-table slots are 64-bits in size.
    COR_VTABLE_FROM_UNMANAGED = 0x04,       // If set, transition from unmanaged.
    COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN = 0x08,  // If set, transition from unmanaged with keeping the current appdomain.
    COR_VTABLE_CALL_MOST_DERIVED = 0x10,    // Call most derived method described by

    // EATJ constants
    IMAGE_COR_EATJ_THUNK_SIZE = 32,         // Size of a jump thunk reserved range.
    
    // Max name lengths
    MAX_CLASS_NAME = 1024,
    MAX_PACKAGE_NAME = 1024,
} ReplacesCorHdrNumericDefines;

// CLR 2.0 header structure.
typedef struct _PE_COR20_HEADER {
    // Header versioning
    UINT32 cb;
    UINT16 MajorRuntimeVersion;
    UINT16 MinorRuntimeVersion;
    // Symbol table and startup information
    PE_DATA_DIRECTORY MetaData;
    UINT32 Flags;
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set, EntryPointToken represents a managed entrypoint.
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set, EntryPointRVA represents an RVA to a native entrypoint.
    union {
        UINT32 EntryPointToken;
        UINT32 EntryPointRVA;
    };
    // Binding information
    PE_DATA_DIRECTORY Resources;
    PE_DATA_DIRECTORY StrongNameSignature;
    // Regular fixup and binding information
    PE_DATA_DIRECTORY CodeManagerTable;
    PE_DATA_DIRECTORY VTableFixups;
    PE_DATA_DIRECTORY ExportAddressTableJumps;
    // Precompiled image info (internal use only - set to zero)
    PE_DATA_DIRECTORY ManagedNativeHeader;
} PE_COR20_HEADER, *PPE_COR20_HEADER;

#endif // __IMAGE_COR20_HEADER_DEFINED__

// End Image Format

#endif // _NATIVE_PECOFF_H_
