#include "ldr.h"
#include "rtl.h"

#include <stdlib.h>

/**
 * The default '.DLL' extension
 */
CONST UNICODE_STRING LdrpApiDefaultExtension = RTL_CONSTANT_STRING(L".DLL");

/**
 * Windows Loader table of DLL HashLinks
 */
PLIST_ENTRY LdrpHashTable = NULL;

/**
 * Atomic Loader Lock Acquisition Count
 */
PULONG LdrpLoaderLockAcquisitionCount = NULL;

/**
 * Determines if the ldr has been initialized
 */
BOOLEAN LdrpInLdrInit = FALSE;

static
UINT32
LdrpCrc32Checksum(
    IN CONST VOID* Data,
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

BOOLEAN
NTAPI
LdrpFindLoadedDllByCrc32(
    IN UINT32 DllCrc32,
    IN BOOLEAN FullPath,
    OUT PLDR_DATA_TABLE_ENTRY *LdrEntry
)
{
    PLIST_ENTRY ListHead, ListEntry;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    CHAR DllNameBuffer[MAX_PATH + 1];
    ANSI_STRING DllNameAnsi;
    PUNICODE_STRING DllName;
    UINT32 DllNameCrc32;

    //
    // Loop the module list.
    //
    ListHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead) {
        //
        // Get the current entry and advance to the next one immediately.
        //
        LdrDataTableEntry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        ListEntry = ListEntry->Flink;

        //
        // Check if it's being unloaded.
        //
        if (!LdrDataTableEntry->InMemoryOrderLinks.Flink)
            continue;

        //
        // Get the full DLL path or just the base name.
        //
        DllName = (FullPath != FALSE) ? &LdrDataTableEntry->FullDllName : &LdrDataTableEntry->BaseDllName;

        //
        // Convert to an ANSI string.
        //
        RtlInitEmptyAnsiString(&DllNameAnsi, DllNameBuffer, sizeof(DllNameBuffer));
        RtlUnicodeStringToAnsiString(&DllNameAnsi, DllName, FALSE);

        //
        // Calculate the CRC32 checksum for this DLL.
        //
        DllNameCrc32 = LdrpCrc32Checksum(DllNameAnsi.Buffer, DllNameAnsi.Length);

        //
        // Check if name CRC32 checksum matches.
        //
        if (DllCrc32 == DllNameCrc32) {
            *LdrEntry = LdrDataTableEntry;
            return TRUE;
        }
    }

    //
    // Not found.
    //
    return FALSE;
}

BOOLEAN
NTAPI
LdrpFindLoadedDllByName(
    IN PWSTR DllPath,
    IN PUNICODE_STRING DllName,
    IN BOOLEAN LookupHash,
    IN BOOLEAN RedirectedDll,
    OUT PLDR_DATA_TABLE_ENTRY *LdrEntry
)
{
    PEB_LDR_DATA* LdrData;
    ULONG NtdllHash, NtdllHashIndex;
    LDR_DATA_TABLE_ENTRY* NtdllEntry;
    PLIST_ENTRY NtdllHashHead;
    UINT8* NtdllBase;
    UINT8* NtdllEndAddress;
    PLIST_ENTRY NextHashEntry;

    ULONG HashValue, HashIndex;
    PLIST_ENTRY ListHead, ListEntry;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PWCHAR Wch;
    WCHAR NameBuffer[MAX_PATH + 1];
    ULONG Length;
    UNICODE_STRING DllName1;
    PUNICODE_STRING DllName2;
    BOOLEAN FullPath;

    //
    // Check if a dll name was provided.
    //
    if (!(DllName->Buffer) || !(DllName->Buffer[0])) {
        return FALSE;
    }

    //
    // Look in the hash table if flag was set
    //
LookInHash:
    if (LookupHash) {
        //
        // Find the hash table inside ntdll if we haven't already
        //
        if (!LdrpHashTable) {

            LdrData = (PEB_LDR_DATA*)(NtCurrentPeb()->Ldr);
            NtdllEntry = CONTAINING_RECORD(LdrData->InInitializationOrderModuleList.Flink,
                                           LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);

            RtlHashUnicodeString(&NtdllEntry->BaseDllName,
                                 TRUE,
                                 HASH_STRING_ALGORITHM_DEFAULT,
                                 &NtdllHash
                                 );

            NtdllHashIndex = NtdllHash & 0x1F;

            NtdllBase = (UINT8 *)NtdllEntry->DllBase;
            NtdllEndAddress = (UINT8 *)NtdllBase + (NtdllEntry->SizeOfImage - 1);

            //
            // Scan hash list to the head (head is located within ntdll).
            //
            NtdllHashHead = NULL;
            NextHashEntry = (PLIST_ENTRY)NtdllEntry->HashLinks.Flink;
            while(NextHashEntry != (PLIST_ENTRY)&NtdllEntry->HashLinks) {

                if ((UINT8 *)NextHashEntry >= NtdllBase && (UINT8 *)NextHashEntry < NtdllEndAddress) {
                    NtdllHashHead = NextHashEntry;
                    break;
                }

                NextHashEntry = NextHashEntry->Flink;
            }

            //
            // Get pointer to LdrpHashTable 
            //
            if (NtdllHashHead != NULL) {
                LdrpHashTable = NtdllHashHead - NtdllHashIndex;
            }
        }

        //
        // Use the loader's hash table to try and find the entry
        //
        if (LdrpHashTable) {

            //
            // Get hash index.
            //
            RtlHashUnicodeString(DllName, TRUE, HASH_STRING_ALGORITHM_DEFAULT, &HashValue);
            HashIndex = (ULONG)LDR_GET_HASH_ENTRY(HashValue);

            //
            // Traverse the hash table list.
            //
            ListHead = LdrpHashTable + HashIndex;
            ListEntry = ListHead->Flink;
            while (ListEntry != ListHead) {
                LdrDataTableEntry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, HashLinks);

                //
                // Check if the base name of the module matches.
                //
                if (RtlEqualUnicodeString(DllName, &LdrDataTableEntry->BaseDllName, TRUE)) {
                    *LdrEntry = LdrDataTableEntry;
                    return TRUE;
                }

                ListEntry = ListEntry->Flink;
            }
        }

        //
        // Module was not found, so loop module list.
        //
        DllName1 = *DllName;

        goto ModuleList;
    }

    //
    // Check if there is a full path in this DLL.
    //
    FullPath = FALSE;
    Wch = DllName->Buffer;
    while (*Wch) {

        //
        // Check for a slash in the current position.
        //
        if ((*Wch == L'\\') || (*Wch == L'/')) {

            //
            // Found the slash, so dll name contains a path.
            //
            FullPath = TRUE;

            //
            // Setup full dll name string.
            //
            DllName1.Buffer = NameBuffer;

            //
            // TODO: Should call LdrpSearchPath instead.
            //
            Length = RtlDosSearchPath_U(
                            DllPath ? DllPath : NtCurrentPeb()->ProcessParameters->DllPath.Buffer,
                            DllName->Buffer,
                            NULL,
                            sizeof(NameBuffer) - sizeof(UNICODE_NULL),
                            DllName1.Buffer,
                            NULL
                            );

            //
            // Check if that was successful.
            //
            if (!Length || Length > (sizeof(NameBuffer) - sizeof(UNICODE_NULL))) {
                return FALSE;
            }

            //
            // Full dll name is found.
            //
            DllName1.Length = (USHORT)Length;
            DllName1.MaximumLength = DllName1.Length + sizeof(UNICODE_NULL);

            break;
        }

        Wch++;
    }

    //
    // Go check the hash table.
    //
    if (!FullPath && !LookupHash) {
        LookupHash = TRUE;
        goto LookInHash;
    }

    //
    // Loop the module list.
    //
ModuleList:
    ListHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead) {

        //
        // Get the current entry and advance to the next one immediately.
        //
        LdrDataTableEntry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        ListEntry = ListEntry->Flink;

        //
        // Check if it's being unloaded
        //
        if (!LdrDataTableEntry->InMemoryOrderLinks.Flink) {
            continue;
        }

        //
        // Check if the name matches.
        //
        DllName2 = (FullPath != FALSE) ? &LdrDataTableEntry->FullDllName : &LdrDataTableEntry->BaseDllName;
        if (RtlEqualUnicodeString(&DllName1, DllName2, TRUE)) {
            *LdrEntry = LdrDataTableEntry;
            return TRUE;
        }
    }

    //
    // Not found.
    //
    return FALSE;
}

NTSTATUS
NTAPI
LdrFindEntryForAddress(
    IN CONST PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *FoundEntry
)
{
    PLIST_ENTRY ModuleListHead, NextEntry;
    PLDR_DATA_TABLE_ENTRY ModuleEntry;

    if (!FoundEntry) {
        return STATUS_INVALID_PARAMETER;
    }

    ModuleListHead = &NtCurrentPeb()->Ldr->InMemoryOrderModuleList;
    NextEntry = ModuleListHead->Flink;
    while(NextEntry != ModuleListHead) {
        
        ModuleEntry = CONTAINING_RECORD(NextEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (ModuleEntry->DllBase <= Address &&
            (ULONG_PTR)Address < (ULONG_PTR)ModuleEntry->DllBase + ModuleEntry->SizeOfImage) {
            *FoundEntry = ModuleEntry;
            return STATUS_SUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

    *FoundEntry = NULL;
    return STATUS_NOT_FOUND;
}

NTSTATUS
NTAPI
LdrFindEntryForBase(
    IN CONST PVOID BaseAddress,
    OUT PLDR_DATA_TABLE_ENTRY *FoundEntry
)
{
    PLIST_ENTRY ModuleListHead, NextEntry;
    PLDR_DATA_TABLE_ENTRY ModuleEntry;

    if (!FoundEntry) {
        return STATUS_INVALID_PARAMETER;
    }

    ModuleListHead = &NtCurrentPeb()->Ldr->InMemoryOrderModuleList;
    NextEntry = ModuleListHead->Flink;
    while (NextEntry != ModuleListHead) {

        ModuleEntry = CONTAINING_RECORD(NextEntry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        if (ModuleEntry->DllBase == BaseAddress) {
            *FoundEntry = ModuleEntry;
            return STATUS_SUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

    *FoundEntry = NULL;
    return STATUS_NOT_FOUND;
}

NTSTATUS
NTAPI
LdrFindModuleBaseUnicode(
    IN PLIST_ENTRY ModuleList,
    IN CONST WCHAR *ModuleName,
    OUT PVOID *ModuleBase
)
{
    PLIST_ENTRY NextEntry;
    PLDR_DATA_TABLE_ENTRY ModuleEntry;
    UNICODE_STRING ModuleNameUnicode;

    if (!ModuleList || !ModuleName || !ModuleBase) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&ModuleNameUnicode, ModuleName);

    NextEntry = ModuleList->Flink;
    while(NextEntry != ModuleList) {

        ModuleEntry = CONTAINING_RECORD(NextEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        if (RtlEqualUnicodeString(&ModuleNameUnicode, &ModuleEntry->BaseDllName, TRUE)) {
            *ModuleBase = ModuleEntry->DllBase;
            return STATUS_SUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

    *ModuleBase = NULL;
    return STATUS_NOT_FOUND;
}

NTSTATUS
NTAPI
LdrFindModuleBaseAscii(
    IN PLIST_ENTRY ModuleList,
    IN CHAR *ModuleName,
    OUT PVOID *ModuleBase
)
{
    NTSTATUS Status;
    ANSI_STRING ModuleNameAnsi;
    UNICODE_STRING ModuleNameUnicode;
    WCHAR ModuleNameWideChar[256];

    RtlInitAnsiStringInline(&ModuleNameAnsi, ModuleName);

    ModuleNameUnicode.Length = sizeof(ModuleNameWideChar) - sizeof(ModuleNameWideChar[0]);
    ModuleNameUnicode.MaximumLength = sizeof(ModuleNameWideChar);
    ModuleNameUnicode.Buffer = ModuleNameWideChar;
    Status = RtlAnsiStringToUnicodeString(&ModuleNameUnicode, &ModuleNameAnsi, FALSE);

    if (Status == STATUS_SUCCESS) {
        Status = LdrFindModuleBaseUnicode(ModuleList, ModuleNameWideChar, ModuleBase);
    }

    return Status;
}

NTSTATUS
NTAPI
LdrFindModuleHandleEx(
    IN UINT32 Flags,
    IN UINT32 DllCRC32 OPTIONAL,
    IN PWSTR DllPath OPTIONAL,
    IN PUINT32 DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle OPTIONAL
)
{
    NTSTATUS Status;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    WCHAR RawDllNameBuffer[MAX_PATH];
    UNICODE_STRING RedirectName, RawDllName;
    PWCHAR p1, p2, p3;
    BOOLEAN RedirectedDll;
    ULONG Length;

    //
    // Check if we should find by a CRC32 checksum.
    //
    if (DllCRC32 != 0) {

        //
        // Do the lookup.
        //
        if (LdrpFindLoadedDllByCrc32(DllCRC32, (ULONG_PTR)DllPath & 1 ? TRUE : FALSE, &LdrEntry)) {

            //
            // Return success
            //
            Status = STATUS_SUCCESS;

        } else {

            //
            // Make sure to NULL LdrEntry!
            //
            LdrEntry = NULL;
            Status = STATUS_NOT_FOUND;
        }

        //
        // Exit if we found the module, or if we didn't provide a DllName
        //
        if (NT_SUCCESS(Status) || !DllName) {
            goto Exit;
        }
    }

    //
    // Initialize strings
    //
    RtlInitEmptyUnicodeString(&RawDllName, NULL, 0);
    RedirectName = *DllName;

    //
    // Initialize state
    //
    RedirectedDll = FALSE;
    LdrEntry = NULL;

    //
    // Clear the handle.
    //
    if (DllHandle)
        *DllHandle = NULL;

    //
    // Check for a valid flag combination.
    //
    if ((Flags & ~(LDR_FIND_MODULE_HANDLE_EX_PIN | LDR_FIND_MODULE_HANDLE_EX_UNCHANGED_REFCOUNT)) ||
        (!DllHandle && !(Flags & LDR_FIND_MODULE_HANDLE_EX_PIN))) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    //
    // Set default failure code.
    //
    Status = STATUS_DLL_NOT_FOUND;

    //
    // Find the name without the extension.
    //
    p1 = RedirectName.Buffer;
    p2 = NULL;
    p3 = &p1[RedirectName.Length / sizeof(WCHAR)];
    while (p1 != p3) {
        if (*p1++ == L'.')
            p2 = p1;
        else if (*p1 == L'\\')
            p2 = NULL;
    }

    //
    // Check if no extension was found or if we got a slash.
    //
    if (!(p2) || (*p2 == L'\\') || (*p2 == L'/')) {

        //
        // Check that we have space to add one
        //
        Length = RedirectName.Length + LdrpApiDefaultExtension.Length + sizeof(UNICODE_NULL);
        if (Length >= UNICODE_STRING_MAX_BYTES) {

            //
            // No space to add the extension.
            //
            Status = STATUS_NAME_TOO_LONG;
            goto Exit;
        }

        //
        // Setup the string.
        //
        RawDllName.MaximumLength = (USHORT)Length;
        RawDllName.Buffer = RawDllNameBuffer;

        //
        // Copy the string and add extension
        //
        RtlCopyUnicodeString(&RawDllName, &RedirectName);
        RtlAppendUnicodeStringToString(&RawDllName, &LdrpApiDefaultExtension);

    } else {

        //
        // Check if there's something in the name.
        //
        Length = RedirectName.Length;
        if (Length) {

            //
            // Check and remove trailing period
            //
            if (RedirectName.Buffer[Length / sizeof(WCHAR) - sizeof(ANSI_NULL)] == '.') {
                // Decrease the size by one character.
                RedirectName.Length -= sizeof(WCHAR);
            }
        }

        //
        // Setup the string
        //
        RawDllName.MaximumLength = RedirectName.Length + sizeof(WCHAR);
        RawDllName.Buffer = RawDllNameBuffer;

        //
        // Copy the string
        //
        RtlCopyUnicodeString(&RawDllName, &RedirectName);
    }

    //
    // Finally, do the lookup.
    //
    if (LdrpFindLoadedDllByName(DllPath,
                                &RawDllName,
                                ((ULONG_PTR)DllPath & 1) ? TRUE : FALSE,
                                RedirectedDll, &LdrEntry)) {
        //
        // Return success.
        //
        Status = STATUS_SUCCESS;

    } else {

        //
        // Make sure to NULL LdrEntry!
        //
        LdrEntry = NULL;
    }

Exit:
    //
    // The success path must have a valid loader entry.
    //
    if ((LdrEntry != NULL) != NT_SUCCESS(Status)) {
        RaiseException(STATUS_INVALID_PARAMETER, EXCEPTION_NONCONTINUABLE, 0, NULL);
        return Status;
    }

    //
    // Check if we got an entry and success.
    //
    if (LdrEntry != NULL && NT_SUCCESS(Status)) {

        //
        // Check if the caller is requesting the handle.
        //
        if (DllHandle)
            *DllHandle = LdrEntry->DllBase;
    }

    //
    // Return resulting status.
    //
    return Status;
}

NTSTATUS
NTAPI
LdrFindModuleHandle(
    IN PWSTR DllPath OPTIONAL,
    IN PUINT32 DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
)
{
    return LdrFindModuleHandleEx(LDR_FIND_MODULE_HANDLE_EX_UNCHANGED_REFCOUNT,
                                 0,
                                 DllPath,
                                 DllCharacteristics,
                                 DllName,
                                 DllHandle
                                 );
}

NTSTATUS
NTAPI
LdrFindExportAddressUnicode(
    IN PVOID BaseAddress,
    IN WCHAR *Name,
    IN UINT32 Ordinal,
    OUT PVOID *ProcedureAddress
)
{
    NTSTATUS Status;
    UNICODE_STRING NameUnicode;
    ANSI_STRING NameAnsi;
    CHAR NameAscii[256];
    CHAR *NameAsciiPtr;

    if (Name) {

        RtlInitUnicodeString(&NameUnicode, Name);
        NameAnsi.Buffer = NameAscii;
        NameAnsi.Length = sizeof(NameAscii) - sizeof(NameAscii[0]);
        NameAnsi.MaximumLength = sizeof(NameAscii);
        Status = RtlUnicodeStringToAnsiString(&NameAnsi, &NameUnicode, FALSE);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }

        NameAsciiPtr = NameAnsi.Buffer;

    } else {

        NameAsciiPtr = NULL;
    }

    Status = LdrFindExportAddressAscii(BaseAddress, NameAsciiPtr, Ordinal, ProcedureAddress);

Exit:
    return Status;
}

NTSTATUS
NTAPI
LdrFindExportAddressAscii(
    IN PVOID BaseAddress,
    IN CHAR *Name,
    IN UINT32 Ordinal,
    OUT PVOID *ProcedureAddress
)
{
    PPE_NT_HEADERS NtHeader;
    PPE_EXPORT_DIRECTORY ExportDirectory;
    UINT32 ExportDirectorySize;
    UINT16 *OrdsTable;
    UINT32 *NamesTable;
    UINT32 *FuncsTable;

    UINT32 FunctionIndex;
    UINT16 OrdIndex;
    SIZE_T NameLength;
    CHAR* FuncName;

    CHAR* ForwardImportDescriptor;
    CHAR* ForwardExportName;
    CHAR ForwardModuleName[256];
    WCHAR ForwardModuleNameUnicode[256];
    size_t Converted;
    UNICODE_STRING DestinationString;
    UINT32 ForwardModuleNameLength;
    UINT32 ForwardExportOrdinal;
    PVOID ForwardModuleBase;

    NTSTATUS Status;
    PVOID FoundExportAddress;

    if (!BaseAddress) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK,
                                BaseAddress,
                                0,
                                &NtHeader
                                );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = STATUS_PROCEDURE_NOT_FOUND;
    FoundExportAddress = NULL;

    //
    // Get the pointer to the export directory
    //
    ExportDirectory = RtlImageDirectoryEntryToData(BaseAddress,
                                                   TRUE,
                                                   PE_DIRECTORY_ENTRY_EXPORT,
                                                   &ExportDirectorySize
                                                   );
    if (!ExportDirectory) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    FuncsTable = (UINT32 *)((UINT8 *)BaseAddress + ExportDirectory->AddressOfFunctions);

    //
    // If it's an ordinal
    //
    if (!Name) {

        if (Ordinal > 0) {

            //
            // Don't need to worry about odd behavior here because ordinals cannot be forwarded (as far as I know)
            //
            *ProcedureAddress = (PVOID)((UINT8 *)BaseAddress + FuncsTable[Ordinal - 1]);
            return STATUS_SUCCESS;
        }

        //
        // Bad params!
        //
        return STATUS_INVALID_PARAMETER;
    }

    NameLength = (SIZE_T)strlen(Name);
    if (!NameLength) {

        //
        // Bad params!
        //
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Get names and ordinals tables.
    //
    OrdsTable = (UINT16 *)((UINT8 *)BaseAddress + ExportDirectory->AddressOfNameOrdinals);
    NamesTable = (UINT32 *)((UINT8 *)BaseAddress + ExportDirectory->AddressOfNames);

    for (FunctionIndex = 0; FunctionIndex < ExportDirectory->NumberOfFunctions; ++FunctionIndex) {

        OrdIndex = 0xFFFF;
        FuncName = NULL;

        if (FunctionIndex < ExportDirectory->NumberOfNames) {

            //
            // Find by name
            //
            FuncName = (CHAR *)((UINT8 *)BaseAddress + NamesTable[FunctionIndex]);
            OrdIndex = OrdsTable[FunctionIndex];

        } else {

            //
            // Never found
            //
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        //
        // Compare the procedure name
        //
        if (memcmp(FuncName, Name, NameLength) == 0) {

            //
            // Found export address
            //
            FoundExportAddress = (PVOID)((UINT8 *)BaseAddress + FuncsTable[OrdIndex]);

            //
            // Status success!
            //
            Status = STATUS_SUCCESS;

            //
            // Check for forwarded export.
            //
            if ((FoundExportAddress >= (PVOID)ExportDirectory) &&
                (FoundExportAddress <= (PVOID)((UINT8 *)ExportDirectory + ExportDirectorySize))) {

                ForwardImportDescriptor = (CHAR *)FoundExportAddress;
                ForwardExportName = strchr(ForwardImportDescriptor, '.');

                if (ForwardExportName != NULL) {

                    //
                    // Move ahead one character to get export name.
                    //
                    ++ForwardExportName;

                    //
                    // Get forward module name length.
                    //
                    ForwardModuleNameLength = (UINT32)(ForwardExportName - ForwardImportDescriptor);

                    //
                    // Zero out memory for forward module name.
                    //
                    RtlZeroMemory(ForwardModuleName, sizeof(ForwardModuleName));

                    //
                    // Copy name over into buffer.
                    //
                    RtlCopyMemory(ForwardModuleName, ForwardImportDescriptor, ForwardModuleNameLength);

#ifdef __GNUC__     // Disable annoying warning -Wmultichar
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#endif
                    //
                    // Concatenate '.dll' and null-terminate
                    //
                    *(ULONG *)(ForwardModuleName + ForwardModuleNameLength - 1) = (ULONG)'lld.';
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
                    //
                    // Create unicode string structure for forward module name.
                    //
                    mbstowcs_s(&Converted, ForwardModuleNameUnicode, 256, ForwardModuleName, 256);
                    RtlInitUnicodeString(&DestinationString, ForwardModuleNameUnicode);

                    //
                    // Get the forward module base       
                    //
                    Status = LdrFindModuleHandle(NULL, NULL, &DestinationString, &ForwardModuleBase);
                    if (!NT_SUCCESS(Status)) {

                        //
                        // Module not found!
                        //
                        return Status;
                    }

                    //
                    // Check if this is an ordinal in the forwarder chain.
                    //
                    if (strchr(ForwardExportName, '#')) {

                        //
                        // Forward by ordinal
                        //
                        ForwardExportOrdinal = (UINT32)atoi(ForwardExportName + 1);
                        ForwardExportName = NULL;

                    } else {

                        //
                        // Forward by name
                        //
                        ForwardExportOrdinal = 0;
                    }

                    //
                    // Recurse to follow the forwarded export chain.
                    //
                    return LdrFindExportAddressAscii(ForwardModuleBase,
                                                     ForwardExportName,
                                                     ForwardExportOrdinal,
                                                     &FoundExportAddress
                                                     );

                } else {

                    Status = STATUS_PROCEDURE_NOT_FOUND;
                }
            }

            break;
        }
    }

    if (NT_SUCCESS(Status)) {

        //
        // Return the found procedure address.
        //
        *ProcedureAddress = (PVOID)FoundExportAddress;
    }

    return Status;
}

NTSTATUS
NTAPI
LdrFindExportAddressCrc32(
    IN PVOID BaseAddress,
    IN UINT32 Crc32Name,
    IN UINT32 Ordinal,
    OUT PVOID *ProcedureAddress
)
{
    PPE_NT_HEADERS NtHeader;
    PPE_EXPORT_DIRECTORY ExportDirectory;
    UINT32 ExportDirectorySize;
    UINT16 *OrdsTable;
    UINT32 *NamesTable;
    UINT32 *FuncsTable;

    UINT32 FunctionIndex;
    UINT16 OrdIndex;
    SIZE_T NameLength;
    CHAR* FuncName;
    UINT32 Crc32FuncName;

    CHAR* ForwardImportDescriptor;
    CHAR* ForwardExportName;
    CHAR ForwardModuleName[256];
    WCHAR ForwardModuleNameUnicode[256];
    size_t Converted;
    UNICODE_STRING DestinationString;
    UINT32 ForwardModuleNameLength;
    UINT32 ForwardExportOrdinal;
    PVOID ForwardModuleBase;

    NTSTATUS Status;
    PVOID FoundExportAddress;

    if (BaseAddress == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK,
                                BaseAddress,
                                0,
                                &NtHeader
                                );
    if (!NT_SUCCESS(Status)) {

        //
        // Not a valid PE file
        //
        return Status;
    }

    //
    // Initial status value.
    //
    Status = STATUS_PROCEDURE_NOT_FOUND;
    FoundExportAddress = NULL;

    //
    // Get the pointer to the export directory
    //
    ExportDirectory = RtlImageDirectoryEntryToData(BaseAddress,
                                                   TRUE,
                                                   PE_DIRECTORY_ENTRY_EXPORT,
                                                   &ExportDirectorySize
                                                   );
    if (!ExportDirectory) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    FuncsTable = (UINT32 *)((UINT8 *)BaseAddress + ExportDirectory->AddressOfFunctions);

    //
    // If it's an ordinal
    //
    if (!Crc32Name) {

        //
        // Are we looking for an ordinal?
        //
        if (Ordinal > 0) {

            //
            // No need to worry about odd behavior here because ordinals are never forwarded (as far as I know?)
            //
            *ProcedureAddress = (PVOID)((UINT8 *)BaseAddress + FuncsTable[Ordinal - 1]);
            return STATUS_SUCCESS;
        }

        //
        // Bad params
        //
        return STATUS_INVALID_PARAMETER;
    }

    OrdsTable = (UINT16 *)((UINT8 *)BaseAddress + ExportDirectory->AddressOfNameOrdinals);
    NamesTable = (UINT32 *)((UINT8 *)BaseAddress + ExportDirectory->AddressOfNames);

    for (FunctionIndex = 0; FunctionIndex < ExportDirectory->NumberOfFunctions; ++FunctionIndex) {

        OrdIndex = 0xFFFF;
        FuncName = NULL;
        Crc32FuncName = 0;

        if (FunctionIndex < ExportDirectory->NumberOfNames) {

            //
            // Find by name
            //
            FuncName = (CHAR *)((UINT8 *)BaseAddress + NamesTable[FunctionIndex]);
            OrdIndex = OrdsTable[FunctionIndex];

        } else {

            //
            // Never found :(
            //
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        //
        // Compare the checksums of the procedure names
        //
        NameLength = strlen(FuncName);
        if (NameLength) {

            //
            // Calculate this export name's CRC32 checksum
            //
            Crc32FuncName = LdrpCrc32Checksum(FuncName, NameLength);
            if (Crc32Name == Crc32FuncName) {

                //
                // Found export address
                //
                FoundExportAddress = (PVOID)((UINT8 *)BaseAddress + FuncsTable[OrdIndex]);

                //
                // Status success!
                //
                Status = STATUS_SUCCESS;

                //
                // Check for forwarded export
                //
                if ((FoundExportAddress >= (PVOID)ExportDirectory) &&
                    (FoundExportAddress <= (PVOID)((UINT8 *)ExportDirectory + ExportDirectorySize))) {

                    ForwardImportDescriptor = (CHAR*)FoundExportAddress;
                    ForwardExportName = strchr(ForwardImportDescriptor, '.');
                    if (ForwardExportName != NULL) {

                        //
                        // Move ahead one character to get export name
                        //
                        ++ForwardExportName;

                        //
                        // Get forward module name length
                        //
                        ForwardModuleNameLength = (UINT32)(ForwardExportName - ForwardImportDescriptor);

                        // Zero out memory for forward module name
                        RtlZeroMemory(ForwardModuleName, sizeof(ForwardModuleName));
                        // Copy name over into buffer
                        RtlCopyMemory(ForwardModuleName, ForwardImportDescriptor, ForwardModuleNameLength);


#ifdef __GNUC__         // Disable annoying warning -Wmultichar
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#endif
                        //
                        // Concatenate '.dll' and null-terminate
                        //
                        *(UINT32 *)(ForwardModuleName + ForwardModuleNameLength - 1) = (UINT32)'lld.';
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
                        //
                        // Create unicode string structure for forward module name
                        //
                        mbstowcs_s(&Converted, ForwardModuleNameUnicode, 256, ForwardModuleName, 256);
                        RtlInitUnicodeString(&DestinationString, ForwardModuleNameUnicode);

                        //
                        // Get the forward module base       
                        //
                        Status = LdrFindModuleHandle(NULL, NULL, &DestinationString, &ForwardModuleBase);
                        if (!NT_SUCCESS(Status)) {

                            //
                            // Module not found!
                            //
                            return Status;
                        }

                        //
                        // Check if this is an ordinal in the forwarder chain.
                        //
                        if (strchr(ForwardExportName, '#')) {

                            //
                            // Forward by ordinal.
                            //
                            ForwardExportOrdinal = (UINT32)atoi(ForwardExportName + 1);
                            ForwardExportName = NULL;

                        } else {

                            //
                            // Forward by name
                            //
                            ForwardExportOrdinal = 0;
                        }

                        //
                        // Recurse to follow the forwarded export chain.
                        //
                        return LdrFindExportAddressAscii(ForwardModuleBase,
                                                         ForwardExportName,
                                                         ForwardExportOrdinal,
                                                         &FoundExportAddress
                                                         );

                    } else {

                        Status = STATUS_PROCEDURE_NOT_FOUND;
                    }
                }

                break;
            }
        }
    }

    if (NT_SUCCESS(Status)) {

        //
        // Return the found procedure address.
        //
        *ProcedureAddress = (PVOID)FoundExportAddress;
    }

    return Status;
}
