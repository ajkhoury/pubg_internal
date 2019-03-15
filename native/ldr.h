#pragma once

#include "native.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Windows Loader table of DLL HashLinks
 */
#define LDR_HASH_TABLE_ENTRIES      (32)
#define LDRP_HASH_MASK              (LDR_HASH_TABLE_ENTRIES - 1)
#define LDR_GET_HASH_ENTRY(Value)   (Value & LDRP_HASH_MASK)

/* LdrLockLoaderLock Flags */
#define LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS   0x00000001
#define LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY          0x00000002

/* Ldr lock disposition flags */
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_INVALID             0
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED       1
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED   2

/* LdrUnlockLoaderLock Flags */
#define LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS 0x00000001

#define LDR_LOAD_DLL_VALID_FLAGS \
    (LOAD_LIBRARY_SAFE_CURRENT_DIRS | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32   \
    | LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR \
    | LOAD_WITH_ALTERED_SEARCH_PATH | DONT_RESOLVE_DLL_REFERENCES)


/**
 * @brief Find the module entry for a specified address.
 * @param[in] Address       - The address that corresponds to a module entry.
 * @param[out] FoundEntry   - The found entry. NULL if not found
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindEntryForAddress(
    IN CONST PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *FoundEntry
    );

/**
 * @brief Find the module entry for the supplied module base address.
 * @param[in] BaseAddress   The address that corresponds to a module entry.
 * @param[out] FoundEntry   The found entry. NULL if not found.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindEntryForBase(
    IN CONST PVOID BaseAddress,
    OUT PLDR_DATA_TABLE_ENTRY *FoundEntry
    );

/**
 * @brief Find the base of a module by it's name.
 * @param[in] ModuleList    - The module list containing the modules to search.
 * @param[in] ModuleName    - The unicode name of the module to find the base address of.
 * @param[out] ModuleBase   - The returned module base of the found module.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindModuleBaseUnicode(
    IN PLIST_ENTRY ModuleList,
    IN CONST WCHAR *ModuleName,
    OUT PVOID *ModuleBase
    );

/**
 * @brief Find the base of a module by it's name.
 * @param[in] ModuleList   The module list containing the modules to search.
 * @param[in] ModuleName   The ascii name of the module to find the base address of.
 * @param[out] ModuleBase  The returned module base of the found module.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindModuleBaseAscii(
    IN PLIST_ENTRY ModuleList,
    IN CHAR *ModuleName,
    OUT PVOID *ModuleBase
    );

/**
 * @brief Find the handle of a module by it's name or path.
 * @param[in] Flags              Find flags. 
 * @param[in] DllCRC32           The possible CRC32 checksum name of the module to find.
 * @param[in] DllPath            The possible path to the module to find.
 * @param[in] DllCharacteristics The characteristics of the module to find.
 * @param[out] DllName           The possible name of the module to find.
 * @param[out] DllHandle         Pointer to the found module handle.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindModuleHandleEx(
    IN UINT32 Flags,
    IN UINT32 DllCRC32 OPTIONAL,
    IN PWSTR DllPath OPTIONAL,
    IN PUINT32 DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle OPTIONAL
    );

//! LdrFindModuleHandleEx Flags
#define LDR_FIND_MODULE_HANDLE_EX_UNCHANGED_REFCOUNT    (0x00000001)
#define LDR_FIND_MODULE_HANDLE_EX_PIN                   (0x00000002)

/**
 * @brief Find the handle of a module by it's name or path.
 * @param[in] DllPath            The possible path to the module to find.
 * @param[in] DllCharacteristics The characteristics of the module to find.
 * @param[out] DllName           The possible name of the module to find.
 * @param[out] DllHandle         Pointer to the found module handle.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindModuleHandle(
    IN PWSTR DllPath OPTIONAL,
    IN PUINT32 DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

/**
 * @brief Get export procedure address from the target image base.
 * @param[in] BaseAddress       The target image base to search the export table for.
 * @param[in] Name              The unicode name of the export to find.
 * @param[in] Ordinal           The ordinal number of the export to find.
 * @param[out] ProcedureAddress The returned procedure address of the found export.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindExportAddressUnicode(
    IN PVOID BaseAddress,
    IN WCHAR *Name,
    IN UINT32 Ordinal,
    OUT PVOID *ProcedureAddress
    );

/**
 * @brief Get export procedure address from the target image base.
 * @param[in] BaseAddress       The target image base to search the export table for.
 * @param[in] Name              The ascii name of the export to find.
 * @param[in] Ordinal           The ordinal number of the export to find.
 * @param[out] ProcedureAddress The returned procedure address of the found export.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindExportAddressAscii(
    IN PVOID BaseAddress,
    IN CHAR *Name,
    IN UINT32 Ordinal,
    OUT PVOID *ProcedureAddress
    );

/**
 * @brief Get export procedure address from the target image base.
 * @param[in] BaseAddress       The target image base to search the export table for.
 * @param[in] Crc32Name         The CRC32 checksum of the ascii name of the export to find.
 * @param[in] Ordinal           The ordinal number of the export to find.
 * @param[out] ProcedureAddress The returned procedure address of the found export.
 * @return Status value.
 */
NTSTATUS
NTAPI
LdrFindExportAddressCrc32(
    IN PVOID BaseAddress,
    IN UINT32 Crc32Name,
    IN UINT32 Ordinal,
    OUT PVOID *ProcedureAddress
    );

#ifdef __cplusplus
} // extern "C" 
#endif