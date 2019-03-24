        .CODE

; uint64_t DecryptObjectsAsm(uint64_t ObjectsEncrypted);
PUBLIC DecryptObjectsAsm
DecryptObjectsAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rdx,rcx
        mov     ecx,edx
        movzx   eax,dx
        shr     ecx,10h
        mov     r8,rdx
        xor     ecx,eax
        and     edx,0ffff0000h
        shr     r8,20h
        or      ecx,edx
        add     ecx,01818b868h
        movzx   eax,cx
        mov     edx,ecx
        and     ecx,0ffff0000h
        shr     edx,10h
        xor     edx,eax
        or      edx,ecx
        mov     ecx,r8d
        shr     ecx,10h
        xor     edx,01818b868h
        xor     r8w,cx
        mov     dword ptr [rbp],edx
        movzx   eax,cx
        rol     r8w,8
        ror     ax,8
        movzx   edx,ax
        movzx   eax,r8w
        shl     edx,10h
        or      edx,eax
        add     edx,017671768h
        mov     ecx,edx
        shr     ecx,10h
        movzx   eax,cx
        xor     cx,dx
        ror     ax,8
        rol     cx,8
        movzx   r8d,ax
        shl     r8d,10h
        movzx   eax,cx
        or      r8d,eax
        xor     r8d,0e898e898h
        mov     dword ptr [rbp+4],r8d
        mov     rax,qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectsAsm ENDP

; uint32_t DecryptObjectIndexAsm(uint32_t InternalIndexEncrypted);
PUBLIC DecryptObjectIndexAsm
DecryptObjectIndexAsm PROC
        push    rbx
        push    rdi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx
        xor     ebx, 0d898e095h
        rol     ebx, 0dh
        mov     edi, ebx
        shl     edi, 10h
        xor     edi, ebx
        xor     edi, 091454e39h
        mov     eax, edi

        add     rsp, 8
        pop     rbp
        pop     rdi
        pop     rbx
        ret
DecryptObjectIndexAsm ENDP

; uint32_t DecryptObjectFlagsAsm(uint32_t ObjectFlagsEncrypted);
PUBLIC DecryptObjectFlagsAsm
DecryptObjectFlagsAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx                ; EBX = ObjectFlagsEncrypted
        xor     ebx, 090d24e96h
        ror     ebx, 4
        not     ebx
        mov     eax, ebx

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectFlagsAsm ENDP

; uint64_t DecryptObjectOuterAsm(uint64_t OuterEncrypted);
PUBLIC DecryptObjectOuterAsm
DecryptObjectOuterAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
;       mov     rbp, rsp

        mov     rsi, 0d92556a5c1f69e0eh
        mov     rbp, 0e6098b0ad5a609a5h
        mov     rbx, rcx                ; RBX = ClassEncrypted
        xor     rbx, rsi
        ror     rbx, 7
        mov     rax, rbx
        shl     rax, 32
        xor     rax, rbp
        xor     rax, rbx

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectOuterAsm ENDP

; uint64_t DecryptObjectClassAsm(uint64_t ClassEncrypted);
PUBLIC DecryptObjectClassAsm
DecryptObjectClassAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
;       mov     rbp, rsp

        mov     rdi, rcx                    ; RDI = ClassEncrypted
        mov     rax, 09f7c4e3e9913b649h
        xor     rdi, rax
        rol     rdi, 13h
        mov     rbx, rdi
        shl     rbx, 32
        mov     rax, 05f346a8b67ac8a1bh
        xor     rbx, rax
        xor     rbx, rdi
        mov     rax, rbx

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectClassAsm ENDP

; void DecryptObjectFNameAsm(const int32_t InNameIndexEncrypted, 
;                            const int32_t InNameNumberEncrypted,
;                            int32_t* OutIndex,
;                            int32_t* OutNumber);
PUBLIC DecryptObjectFNameAsm
DecryptObjectFNameAsm PROC
        push    rbx
        push    rdi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx                ; EBX = IndexEncrypted
        mov     edi, edx                ; EDI = NumberEncrypted

        ; Index decryption.
        xor     ebx, 07a193ec7h
        ror     ebx, 12
        mov     eax, ebx
        shl     eax, 16
        xor     eax, 01bb2de72h
        xor     eax, ebx
        mov     dword ptr [r8], eax
        ; Number decryption
        xor     edi, 04e9690d2h
        rol     edi, 12
        mov     eax, edi
        shl     eax, 16
        xor     eax, edi
        xor     eax, 0b21bba5bh
        mov     dword ptr [r9], eax

        add     rsp, 8
        pop     rbp
        pop     rdi
        pop     rbx
        ret
DecryptObjectFNameAsm ENDP

        END