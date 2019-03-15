#include "wow64.h"

#include "rtl.h"

#if defined(_X86_)

ULONG64
WINAPIV
Wow64CallX64(
    IN ULONG64 FuncAddress,
    IN ULONG ArgCount,
    IN ...
)
{
    va_list Args;
    va_start( Args, ArgCount );

    ULONG BackupEsp;
    WORD BackupFs;

    WOW64_REGISTER64 Rax;
    WOW64_REGISTER64 Rcx;
    WOW64_REGISTER64 Rdx;
    WOW64_REGISTER64 R8;
    WOW64_REGISTER64 R9;
    WOW64_REGISTER64 StackArgs;
    WOW64_REGISTER64 StackArgCount;

    Rax.Dword64 = 0;
    Rcx.Dword64 = 0;
    Rdx.Dword64 = 0;
    R8.Dword64 = 0;
    R9.Dword64 = 0;

    if (ArgCount > 0)
    {
        Rcx.Dword64 = va_arg( Args, ULONG64 );
        if (--ArgCount > 0)
        {
            Rdx.Dword64 = va_arg( Args, ULONG64 );
            if (--ArgCount > 0)
            {
                R8.Dword64 = va_arg( Args, ULONG64 );
                if (--ArgCount > 0)
                {
                    R9.Dword64 = va_arg( Args, ULONG64 );
                }
            }
        }
    }

    StackArgs.Dword64 = (ULONG64)&va_arg( Args, ULONG64 );
    StackArgCount.Dword64 = (ULONG64)ArgCount;

    BackupEsp = 0;
    BackupFs = 0;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4409) // annoying warning C4409: illegal instruction size

    __asm
    {
        //
        // Reset FS segment, to properly handle RFG
        //
        mov BackupFs, fs
        mov eax, 0x2B
        mov fs, ax

        //
        // Backup the original esp in BackupEsp variable
        //
        mov BackupEsp, esp

        //
        // Align stack to a 16-byte boundary. Without having the stack aligned, some syscalls may 
        // return errors (actually, for syscalls it is sufficient to align to 8, but SSE opcodes 
        // requires 16-byte alignment), it will be further adjusted according to the number of 
        // arguments above 4.
        //
        and esp, 0xFFFFFFF0

        //
        // Start decoding as x64 instructions.
        //
        WOW64_BEGIN_X64( 0x33 )

        //
        // The below code is compiled as x86 inline asm, but it is executed as x64 code
        // that's why it need sometimes WOW64_REX_W() macro, right column contains detailed
        // transcription how it will be interpreted by CPU
        //
        WOW64_REX_W mov ecx, Rcx.Dword[0]               // mov rcx, qword ptr [Rcx]
        WOW64_REX_W mov edx, Rdx.Dword[0]               // mov rdx, qword ptr [Rdx]

        push R8.Dword64                                 // push qword ptr [R8]
        WOW64_POP_X64( WOW64_R8 )                       // pop r8
        push R9.Dword64                                 // push qword ptr [R9]
        WOW64_POP_X64( WOW64_R9 )                       // pop r9

        WOW64_REX_W mov eax, StackArgCount.Dword[0]     // mov rax, qword ptr [StackArgCount]
        
        //
        // Final stack adjustment, depending on whether we have more than 4 arguments.
        //
        test al, 1                                      // test al, 1
        jnz skip_stack_adjustment                       // jnz skip_stack_adjustment
        sub esp, 8                                      // sub rsp, 8
skip_stack_adjustment:
        push edi;                                       // push rdi
        WOW64_REX_W mov edi, StackArgs.Dword[0]         // mov rdi, qword ptr [StackArgs]

        //
        // Put the rest of arguments on the stack.
        //
        WOW64_REX_W test eax, eax                       // test rax, rax
        jz create_shadow_space                          // je create_shadow_space
        WOW64_REX_W lea edi, dword ptr [edi+8*eax-8]    // lea rdi, [rdi+rax*8-8]

set_next_arg: 
        WOW64_REX_W test eax, eax                       // test rax, rax
        jz create_shadow_space                          // je create_shadow_space
        push dword ptr [edi]                            // push qword ptr [rdi]
        WOW64_REX_W sub edi, 8                          // sub rdi, 8
        WOW64_REX_W sub eax, 1                          // sub rax, 1
        jmp set_next_arg                                // jmp set_next_arg

        //
        // Create stack shadow space for debug registers, as per MSDN.
        //
create_shadow_space:     
        WOW64_REX_W sub esp, 0x20                       // sub rsp, 20h

        //
        // Finally, call the function!
        //
        call FuncAddress                                // call qword ptr [FuncAddress]

        //
        // Cleanup the stack.
        //
        WOW64_REX_W mov ecx, StackArgCount.Dword[0]     // mov rcx, qword ptr [StackArgCount]
        WOW64_REX_W lea esp, dword ptr [esp+8*ecx+0x20] // lea rsp, [rsp+rcx*8+20h]
        pop edi                                         // pop rdi

        //
        // Set the return value.
        //
        WOW64_REX_W mov Rax.Dword[0], eax               // mov qword ptr [Rax], rax

        //
        // Go back to decoding instructions as 32bit.
        //
        WOW64_END_X64( 0x23 )

        //
        // Restore the stack.
        //
        mov ax, ds
        mov ss, ax
        mov esp, BackupEsp

        //
        // Restore the FS segment.
        //
        mov ax, BackupFs
        mov fs, ax
    }

#pragma warning(pop)
#endif // _MSC_VER

    return Rax.Dword64;
}

ULONG64
Wow64TebX64(
    VOID
)
{
    WOW64_REGISTER64 R12;
    R12.Dword64 = 0;

    //
    // Start decoding as x64 instructions
    //
    WOW64_BEGIN_X64( 0x33 );

    //
    // R12 register should always contain the pointer to the x64 TEB in 
    // a WoW64 processes
    //
    WOW64_PUSH_X64( WOW64_R12 );
    __asm pop R12.Dword[0]

    //
    // Return to decoding as 32bit instructions again.
    //
    WOW64_END_X64( 0x23 );

    return R12.Dword64;
}

BOOL
Wow64CopyMemoryX64( 
    OUT PVOID DestAddress, 
    IN ULONG64 SourceAddress, 
    IN SIZE_T Size 
)
{
    SIZE_T Sz;
    WOW64_REGISTER64 Source;

    if (!DestAddress || !SourceAddress || !Size)
        return FALSE;

    Sz = Size;
    Source.Dword64 = SourceAddress;

    __asm
    {
        WOW64_BEGIN_X64( 0x33 )

        push edi                                // push rdi
        push esi                                // push rsi

        mov edi, DestAddress                    // mov edi, dword ptr [DestAddress] => high part of RDI is zeroed
        WOW64_REX_W mov esi, Source.Dword[0]    // mov rsi, qword ptr [Source]
        mov ecx, Sz                             // mov ecx, dword ptr [Sz] => high part of RCX is zeroed

        mov eax, ecx                            // mov eax, ecx
        and eax, 3                              // and eax, 3
        shr ecx, 2                              // shr ecx, 2

        rep movsd                               // rep movs dword ptr [rdi], dword ptr [rsi]

        test eax, eax                           // test eax, eax
        je done                                 // je done
        cmp eax, 1                              // cmp eax, 1
        je move_bytes                           // je move_bytes

//move_words:
        movsw                                   // movs word ptr [rdi], word ptr [rsi]
        cmp eax, 2                              // cmp eax, 2
        je done                                 // je done

move_bytes:
        movsb                                   // movs byte ptr [rdi], byte ptr [rsi]

done:
        pop esi                                 // pop      rsi
        pop edi                                 // pop      rdi

        WOW64_END_X64( 0x23 )
    }

    return TRUE;
}

BOOL 
Wow64CompareMemoryX64( 
    IN PVOID DestAddress, 
    IN ULONG64 SourceAddress, 
    IN SIZE_T Size
)
{
    BOOL Result;
    SIZE_T Sz;
    WOW64_REGISTER64 Source;

    if (!DestAddress || !SourceAddress || !Size)
        return FALSE;

    Result = FALSE;
    Sz = Size;
    Source.Dword64 = SourceAddress;

    __asm
    {
        WOW64_BEGIN_X64( 0x33 )

        push edi                                // push rdi
        push esi                                // push rsi

        mov edi, DestAddress                    // mov edi, dword ptr [DestAddress] => high part of RDI is zeroed
        WOW64_REX_W mov esi, Source.Dword[0]    // mov rsi, qword ptr [Source]
        mov ecx, Sz                             // mov ecx, dword ptr [Sz] => high part of RCX is zeroed

        mov eax, ecx                            // mov eax, ecx
        and eax, 3                              // and eax, 3
        shr ecx, 2                              // shr ecx, 2

        repe cmpsd                              // repe cmps dword ptr [rsi], dword ptr [rdi]
        jnz done                                // jnz done

        test eax, eax                           // test eax, eax
        je success                              // je success
        cmp eax, 1                              // cmp eax, 1
        je compare_bytes                        // je compare_bytes

//compare_words:
        cmpsw                                   // cmps word ptr [rsi], word ptr [rdi]
        jnz done                                // jnz done
        cmp eax, 2                              // cmp eax, 2
        je success                              // je success

compare_bytes:
        cmpsb                                   // cmps byte ptr [rsi], byte ptr [rdi]
        jnz done                                // jnz done

success:
        mov Result, 1;                          // mov byte ptr [Result], 1
done:
        pop esi;                                // pop rsi
        pop edi;                                // pop rdi

        WOW64_END_X64( 0x23 )
    }

    return Result;
}

ULONG64
WINAPI
Wow64GetModuleHandleX64(
    IN LPCWSTR lpModuleName
)
{
    ULONG64 ModuleHandle;
    TEB64 Teb64;
    PEB64 Peb64;
    PEB_LDR_DATA64 PebLdrData;
    LDR_DATA_TABLE_ENTRY64 LdrDataTableEntry;
    PWSTR BaseDllName;
    ULONG64 ListHead;       // PLIST_ENTRY64
    ULONG64 CurrentEntry;   // PLIST_ENTRY64
    LIST_ENTRY64 NextEntry;

    ModuleHandle = 0;

    Wow64CopyMemoryX64( &Teb64, Wow64TebX64( ), sizeof( TEB64 ) );
    Wow64CopyMemoryX64( &Peb64, Teb64.ProcessEnvironmentBlock, sizeof( PEB64 ) );  
    Wow64CopyMemoryX64( &PebLdrData, Peb64.Ldr, sizeof( PEB_LDR_DATA64 ) );

    ListHead = Peb64.Ldr + FIELD_OFFSET( PEB_LDR_DATA64, InLoadOrderModuleList );
    CurrentEntry = PebLdrData.InLoadOrderModuleList.Flink;
    do
    {
        Wow64CopyMemoryX64( &NextEntry, CurrentEntry, sizeof( LIST_ENTRY64 ) );
        Wow64CopyMemoryX64( &LdrDataTableEntry, WOW64_CONTAINING_RECORD_X64( CurrentEntry, LDR_DATA_TABLE_ENTRY64, InLoadOrderLinks ), sizeof( LDR_DATA_TABLE_ENTRY64 ) );

        BaseDllName = (PWSTR)malloc( LdrDataTableEntry.BaseDllName.MaximumLength );
        if (!BaseDllName)
            return 0;

        Wow64CopyMemoryX64( BaseDllName, LdrDataTableEntry.BaseDllName.Buffer, LdrDataTableEntry.BaseDllName.MaximumLength );

        if (_wcsicmp( lpModuleName, BaseDllName ) == 0)
            ModuleHandle = LdrDataTableEntry.DllBase;

        free( BaseDllName );

        if (ModuleHandle != 0)
            break;

        CurrentEntry = NextEntry.Flink;
    } while (CurrentEntry != ListHead);

    return ModuleHandle;
}

ULONG64
WINAPI
Wow64GetNtdllX64( 
    VOID 
)
{
    static ULONG64 s_Wow64NtdllX64 = 0;

    if (!s_Wow64NtdllX64)
        s_Wow64NtdllX64 = Wow64GetModuleHandleX64( L"ntdll.dll" );

    return s_Wow64NtdllX64;
}

ULONG64
WINAPI
Wow64GetExportAddressX64(
    IN ULONG64 BaseAddress,
    IN LPCSTR pszName
)
{
    BOOL Result;
    ULONG64 ImageBase;
    //ULONG NameLength;
    PE_DOS_HEADER DosHeader;
    PE_NT_HEADERS64 NtHeader;
    ULONG ExportVa;
    ULONG ExportSize;
    PPE_EXPORT_DIRECTORY ExportDirectory;
    PUSHORT OrdsTable;
    PULONG NamesTable;
    PULONG FuncsTable;
    ULONG FuncIndex;
    USHORT OrdIndex;
    PCHAR FuncName;
    ULONG64 ExportAddress = 0;

    ImageBase = BaseAddress;

    //
    // Procure and validate the headers of the module.
    //
    Result = Wow64CopyMemoryX64( &DosHeader, ImageBase, sizeof( PE_DOS_HEADER ) );
    if (!Result || DosHeader.e_magic != PE_DOS_SIGNATURE)
        return FALSE;
    Result = Wow64CopyMemoryX64( &NtHeader, ImageBase + DosHeader.e_lfanew, sizeof( PE_NT_HEADERS64 ) );
    if (!Result || NtHeader.Signature != IMAGE_NT_SIGNATURE)
        return FALSE;

    //
    // Make sure its a 64 bit executable.
    //
    if (!PE64( &NtHeader ))
        return FALSE;

    //
    // Determine the virtual address and size of the export directory.
    //
    ExportVa = DATA_DIRECTORY_RVA( &NtHeader, IMAGE_DIRECTORY_ENTRY_EXPORT );
    if (!ExportVa)
        return FALSE;
    ExportSize = DATA_DIRECTORY_SIZE( &NtHeader, IMAGE_DIRECTORY_ENTRY_EXPORT );

    //
    // Allocate data for the export directory.
    //
    ExportDirectory = (PPE_EXPORT_DIRECTORY)NTDLL_RtlAllocateHeap( RtlProcessHeap( ), 0, ExportSize );
    if (!ExportDirectory)
        return FALSE;

    if (!Wow64CopyMemoryX64( ExportDirectory, ImageBase + ExportVa, ExportSize ))
    {
        NTDLL_RtlFreeHeap( RtlProcessHeap( ), 0, ExportDirectory );
        return FALSE;
    }

    FuncsTable = (PULONG)((PCHAR)ExportDirectory + ExportDirectory->AddressOfFunctions - ExportVa);
    NamesTable = (PULONG)((PCHAR)ExportDirectory + ExportDirectory->AddressOfNames - ExportVa);
    OrdsTable = (PUSHORT)((PCHAR)ExportDirectory + ExportDirectory->AddressOfNameOrdinals - ExportVa);

    for (FuncIndex = 0; FuncIndex < ExportDirectory->NumberOfFunctions; ++FuncIndex)
    {
        OrdIndex = 0xFFFF;
        FuncName = NULL;

        if ((ULONG)pszName <= 0x0000FFFF)
        {
            //
            // Find by index
            //
            OrdIndex = (USHORT)FuncIndex;
        }
        else if ((ULONG)pszName > 0x0000FFFF && FuncIndex < ExportDirectory->NumberOfNames)
        {
            //
            // Find by name
            //
            FuncName = (PCHAR)ExportDirectory + NamesTable[FuncIndex] - ExportVa;
            OrdIndex = OrdsTable[FuncIndex];
        }
        else
        {
            //
            // Never found
            //
            break;
        }

        //
        // Check if this export is a match
        //
        if (((ULONG_PTR)pszName <= 0xFFFF && (USHORT)(ULONG_PTR)pszName == (OrdIndex + ExportDirectory->Base)) ||
            ((ULONG_PTR)pszName > 0xFFFF && strcmp( FuncName, pszName ) == 0))
        {
            ExportAddress = ImageBase + FuncsTable[OrdIndex];

            //
            // AFAIK exports are rarely ever exported in the kernel, so just
            // break out with the found export address here instead of searching 
            // for any forwarded exports.
            //
            break;
        }
    }

    NTDLL_RtlFreeHeap( RtlProcessHeap( ), 0, ExportDirectory );
    return ExportAddress;
}

ULONG64
WINAPI
Wow64VirtualAllocExX64(
    IN HANDLE hProcess,
    IN OUT ULONG64 lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect
)
{
    static ULONG64 s_Wow64NtAllocateVirtualMemory = 0;

    if (!s_Wow64NtAllocateVirtualMemory)
    {
        s_Wow64NtAllocateVirtualMemory = Wow64GetExportAddressX64( Wow64GetNtdllX64( ), "NtAllocateVirtualMemory" );
        if (!s_Wow64NtAllocateVirtualMemory)
            return 0;
    }

    ULONG64 Result;
    ULONG64 Address = lpAddress;
    ULONG64 Size = dwSize;

    Result = Wow64CallX64( s_Wow64NtAllocateVirtualMemory, 6, 
        (ULONG64)hProcess, 
        (ULONG64)&Address,
        (ULONG64)0, 
        (ULONG64)&Size,
        (ULONG64)flAllocationType, 
        (ULONG64)flProtect );

    if (Result != STATUS_SUCCESS)
    {
        RtlSetLastWin32Error( NTDLL_RtlNtStatusToDosError( (DWORD)Result ) );
        return FALSE;
    }
    
    return Address;
}

NTSTATUS
NTAPI
Wow64NtQuerySystemInformationX64(
    IN ULONG SystemInformationClass,
    IN OUT PVOID SystemInformation OPTIONAL,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
)
{
    static ULONG64 s_Wow64NtQuerySystemInformationX64 = 0;

    if (!s_Wow64NtQuerySystemInformationX64)
    {
        s_Wow64NtQuerySystemInformationX64 = Wow64GetExportAddressX64( Wow64GetNtdllX64( ), "NtQuerySystemInformation" );
        if (!s_Wow64NtQuerySystemInformationX64)
            return 0;
    }

    NTSTATUS Status;
    ULONG64 ResultLength = (ULONG64)ReturnLength;
    ULONG64 InformationClass = (ULONG64)SystemInformationClass;
    ULONG64 Information = (ULONG64)SystemInformation;
    ULONG64 InformationLength = (ULONG64)SystemInformationLength;

    Status = (NTSTATUS)Wow64CallX64( s_Wow64NtQuerySystemInformationX64, 4,
        InformationClass,
        Information,
        InformationLength,
        ResultLength );

    return Status;
}

NTSTATUS
NTAPI
Wow64NtQueryObjectX64(
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    IN OUT PVOID ObjectInformation OPTIONAL,
    IN ULONG ObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
)
{
    static ULONG64 s_Wow64NtQueryObjectX64 = 0;

    if (!s_Wow64NtQueryObjectX64)
    {
        s_Wow64NtQueryObjectX64 = Wow64GetExportAddressX64( Wow64GetNtdllX64( ), "NtQueryObject" );
        if (!s_Wow64NtQueryObjectX64)
            return 0;
    }

    NTSTATUS Status;
    ULONG64 ObjectHandle = (ULONG64)Handle;
    ULONG64 InformationClass = (ULONG64)ObjectInformationClass;
    ULONG64 Information = (ULONG64)ObjectInformation;
    ULONG64 InformationLength = (ULONG64)ObjectInformationLength;
    ULONG64 ResultLength = (ULONG64)ReturnLength;

    Status = (NTSTATUS)Wow64CallX64( s_Wow64NtQueryObjectX64, 5,
        ObjectHandle,
        InformationClass,
        Information,
        InformationLength,
        ResultLength );

    return Status;
}

#endif