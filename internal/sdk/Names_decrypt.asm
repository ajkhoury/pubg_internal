        .CODE

; uint64_t DecryptNamesAsm(uint64_t NamesEncrypted)
PUBLIC DecryptNamesAsm
DecryptNamesAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        lea     eax, [rcx+01005753eh]   ; RCX = NamesEncrypted
        xor     eax, 0e0e06020h
        mov     dword ptr [rbp], eax
        shr     rcx, 32
        sub     ecx, 0e7875f0h
        xor     ecx, 020e020e0h
        mov     dword ptr [rbp+4], ecx
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
        xor     eax, 018a2fc42h
        not     eax
        add     eax, 02c2ba5eh
        xor     eax, 0809fb9e3h
        mov     dword ptr [rbp], eax
        shr     rcx, 32
        xor     ecx, 0b3733917h
        not     ecx
        add     ecx, 019d011eh
        xor     ecx, 019ee380ah
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

        mov     eax, ecx                ; RCX = ChunksEncrypted
        not     eax
        sub     eax, 0e2ef2c1h
        xor     eax, 0e2ef2c0h
        mov     dword ptr [rbp],eax
        shr     rcx, 32
        rol     ecx, 8
        not     ecx
        add     ecx, 070de709fh
        rol     ecx, 8
        xor     ecx, 070de709eh
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
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rdx, rcx
        mov     r8, rcx
        movzx   eax,dx
        mov     ecx,edx
        mov     r9,rdx
        shr     ecx,10h
        xor     ecx,eax
        shr     r9,20h
        and     edx,0ffff0000h
        or      ecx,edx
        add     ecx,07ebec622h
        mov     edx,ecx
        movzx   ecx,cx
        shr     edx,10h
        xor     ecx,edx
        movzx   eax,dx
        ror     ax,8
        mov     edx,r9d
        movzx   r8d,ax
        shr     edx,10h
        movzx   eax,dx
        shl     r8d,10h
        xor     ax,r9w
        ror     dx,8
        ror     ax,8
        or      r8d,ecx
        movzx   ecx,ax
        xor     r8d,07ebec622h
        movzx   edx,dx
        shl     edx,10h
        or      edx,ecx
        mov     dword ptr [rbp],r8d
        add     edx,07de17d62h
        mov     eax,edx
        shr     eax,10h
        xor     ax,dx
        and     edx,0ffff0000h
        ror     ax,8
        movzx   ecx,ax
        or      ecx,edx
        xor     ecx,0821e829eh
        mov     dword ptr [rbp+4],ecx
        mov     rax,qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptNumElementsAsm ENDP

        END