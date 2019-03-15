        .CODE

; uint64_t DecryptNamesAsm(uint64_t NamesEncrypted);
PUBLIC DecryptNamesAsm
DecryptNamesAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rax, rcx
        ror     ecx, 8
        not     ecx
        shr     rax, 20h
        add     ecx, 5131EDDFh
        not     eax
        ror     ecx, 8
        sub     eax, 2F812FC1h
        xor     ecx, 0AECE1220h
        xor     eax, 0D07ED03Eh
        mov     dword ptr [rbp], ecx
        mov     dword ptr [rbp+4], eax
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptNamesAsm ENDP

; uint64_t DecryptChunksAsm(uint64_t ChunksEncrypted);
PUBLIC DecryptChunksAsm
DecryptChunksAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     r9, rcx
        shr     r9, 20h
        mov     eax, ecx
        shr     eax, 10h
        xor     ax, cx
        ror     ax, 8
        movzx   eax, ax
        and     ecx, 0FFFF0000h
        or      eax, ecx
        add     eax, 8ECE9612h
        mov     ecx, eax
        shr     ecx, 10h
        movzx   edx, cx
        xor     dx, ax
        ror     cx, 8
        movzx   r8d, cx
        shl     r8d, 10h
        ror     dx, 8
        movzx   eax, dx
        or      r8d, eax
        xor     r8d, 8ECE9612h
        mov     dword ptr [rbp], r8d
        mov     ecx, r9d
        shr     ecx, 10h
        movzx   eax, cx
        xor     ax, r9w
        ror     cx, 8
        movzx   ecx, cx
        shl     ecx, 10h
        movzx   eax, ax
        or      ecx, eax
        add     ecx, 8DD18D52h
        mov     eax, ecx
        shr     eax, 10h
        xor     ax, cx
        and     ecx, 0FFFF0000h
        movzx   eax, ax
        or      ecx, eax
        xor     ecx, 722E72AEh
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptChunksAsm ENDP

PUBLIC DecryptNumElementsAsm
DecryptNumElementsAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rdx, rcx
        lea     eax, [rdx+3BB8A8Fh]
        rol     eax, 8
        add     eax, 428A9DB0h
        ror     eax, 10h
        xor     eax, 0AFCF1321h
        mov     dword ptr [rbp], eax
        mov     rax, rdx
        shr     rax, 20h
        add     eax, 745CD578h
        ror     eax, 10h
        add     eax, 31DD0439h
        ror     eax, 8
        xor     eax, 0D17FD13Fh
        mov     dword ptr [rbp+4], eax
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptNumElementsAsm ENDP

PUBLIC DecryptNameEntryIndexAsm
DecryptNameEntryIndexAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     r9, rcx
        mov     r8, r9
        shr     r8, 20h
        mov     ecx, r9d
        shr     ecx, 10h
        movzx   eax, cx
        rol     ax, 8
        movzx   edx, ax
        shl     edx, 10h
        movzx   eax, r9w
        xor     ecx, eax
        or      edx, ecx
        add     edx, 33D3075Dh
        mov     ecx, edx
        shr     ecx, 10h
        movzx   eax, dx
        xor     ecx, eax
        and     edx, 0FFFF0000h
        or      ecx, edx
        xor     ecx, 33D3075Dh
        mov     dword ptr [rbp], ecx
        mov     eax, r8d
        shr     eax, 10h
        xor     ax, r8w
        ror     ax, 8
        movzx   ecx, ax
        and     r8d, 0FFFF0000h
        or      ecx, r8d
        add     ecx, 0B23CB27Dh
        mov     edx, ecx
        shr     edx, 10h
        movzx   eax, dx
        xor     ax, cx
        ror     ax, 8
        movzx   ecx, ax
        rol     dx, 8
        movzx   eax, dx
        shl     eax, 10h
        or      ecx, eax
        xor     ecx, 4DC34D83h
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptNameEntryIndexAsm ENDP

        END