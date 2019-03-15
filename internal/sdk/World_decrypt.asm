        .CODE

; uint64_t DecryptWorldAsm(uint64_t WorldEncrypted);
PUBLIC DecryptWorldAsm
DecryptWorldAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rdx, rcx
        mov     r8, rcx
        shr     rdx, 32
        mov     qword ptr [rbp], 0
        mov     ecx, r8d
        shr     ecx, 16
        movzx   eax, cx
        xor     ax, r8w
        rol     cx, 8
        movzx   ecx, cx
        shl     ecx, 16
        movzx   eax, ax
        or      ecx, eax
        add     ecx, 0C36357CDh
        mov     eax, ecx
        shr     eax, 16
        xor     ax, cx
        and     ecx, 0FFFF0000h
        movzx   eax, ax
        or      ecx, eax
        xor     ecx, 0C36357CDh
        mov     dword ptr [rbp], ecx
        mov     eax, edx
        shr     eax, 16
        xor     ax, dx
        ror     ax, 8
        movzx   eax, ax
        and     edx, 0FFFF0000h
        or      eax, edx
        add     eax, 42AC42EDh
        mov     ecx, eax
        shr     ecx, 16
        movzx   edx, cx
        xor     dx, ax
        rol     cx, 8
        movzx   r9d, cx
        shl     r9d, 16
        ror     dx, 8
        movzx   eax, dx
        or      r9d, eax
        xor     r9d, 0BD53BD13h
        mov     dword ptr [rbp+4], r9d
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptWorldAsm ENDP

        END