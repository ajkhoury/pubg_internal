        .CODE

; uint64_t DecryptNamesAsm(uint64_t NamesEncrypted)
PUBLIC DecryptNamesAsm
DecryptNamesAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rdx, rcx            ; RDX = NamesEncrypted
        mov     r8, rdx
        shr     r8, 32
        mov     ecx, edx
        shr     ecx, 16
        movzx   eax, cx
        xor     ax, dx
        ror     cx, 8
        movzx   ecx, cx
        shl     ecx, 16
        movzx   eax, ax
        or      ecx, eax
        add     ecx, 0D8D878A8h
        mov     edx, ecx
        shr     edx, 16
        movzx   eax, dx
        xor     ax, cx
        ror     dx, 8
        movzx   ecx, dx
        shl     ecx, 16
        movzx   eax, ax
        or      ecx, eax
        xor     ecx, 0D8D878A8h
        mov     dword ptr [rbp], ecx
        mov     eax, r8d
        shr     eax, 16
        xor     ax, r8w
        rol     ax, 8
        movzx   ecx, ax
        and     r8d, 0FFFF0000h
        or      ecx, r8d
        add     ecx, 0D7A7D7A8h
        mov     eax, ecx
        shr     eax, 16
        xor     ax, cx
        rol     ax, 8
        movzx   eax, ax
        and     ecx, 0FFFF0000h
        or      eax, ecx
        xor     eax, 028582858h
        mov     dword ptr [rbp+4], eax
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptNamesAsm ENDP

; uint64_t DecryptNameEntryIndexAsm(uint64_t IndexEncrypted)
PUBLIC DecryptNameEntryIndexAsm
DecryptNameEntryIndexAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     eax, ecx                ; RCX = IndexEncrypted
        xor     eax, 0D7C473EBh
        sub     eax, 045856DDAh
        xor     eax, 030BEE1CDh
        mov     dword ptr [rbp],eax
        shr     rcx, 32
        xor     ecx, 02443A658h
        sub     ecx, 0469A461Ah
        xor     ecx, 091D9E042h
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptNameEntryIndexAsm ENDP

; uint64_t DecryptChunksAsm(uint64_t ChunksEncrypted)
PUBLIC DecryptChunksAsm
DecryptChunksAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        lea     eax, [rcx+04E9103BDh]    ; RCX = ChunksEncrypted
        rol     eax, 16
        sub     eax, 06CE5173Ch
        rol     eax, 16
        xor     eax, 0A989E507h
        mov     dword ptr [rbp], eax
        shr     rcx, 32
        sub     ecx, 04C3F3918h
        ror     ecx, 8
        add     ecx, 07766EF4Fh
        ror     ecx, 8
        xor     ecx, 0D759D799h
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptChunksAsm ENDP

; uint64_t DecryptNumElementsAsm(uint64_t NumElementsEncrypted)
PUBLIC DecryptNumElementsAsm
DecryptNumElementsAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        lea     eax, [rcx+076B6DEAAh]   ; RCX = NumElementsEncrypted
        xor     eax, 076B6DEAAh
        shr     rcx, 32
        add     ecx, 0756975EAh
        mov     dword ptr [rbp], eax
        xor     ecx, 08A968A16h
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptNumElementsAsm ENDP

        END