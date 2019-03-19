        .CODE

; uint64_t DecryptObjectsAsm(uint64_t ObjectsEncrypted);
PUBLIC DecryptObjectsAsm
DecryptObjectsAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        lea     eax, [rcx+06147A54Eh]   ; RCX = ObjectsEncrypted
        shr     rcx, 32
        xor     eax, 0D0D09030h
        add     ecx, 040E97A20h
        xor     ecx, 030D030D0h
        mov     dword ptr [rbp], eax
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptObjectsAsm ENDP

; uint32_t DecryptObjectInternalIndexAsm(uint32_t InternalIndexEncrypted);
PUBLIC DecryptObjectInternalIndexAsm
DecryptObjectInternalIndexAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx                ; EBX = InternalIndexEncrypted
        xor     ebx, 0B2102611h
        ror     ebx, 9
        mov     esi, ebx
        shl     esi, 16
        xor     esi, 018E4DED2h
        xor     esi, ebx
        mov     eax, esi

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectInternalIndexAsm ENDP

; uint32_t DecryptObjectFlagsAsm(uint32_t ObjectFlagsEncrypted);
PUBLIC DecryptObjectFlagsAsm
DecryptObjectFlagsAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx                ; EBX = ObjectFlagsEncrypted
        xor     ebx, 041433CB0h
        rol     ebx, 6
        not     ebx
        mov     eax, ebx

        add     rsp, 8
        pop     rbp
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

        mov     rbx, rcx                ; RBX = ClassEncrypted
        mov     rsi, 078D362BC613F492Ah
        mov     rbp, 0A220152F73771A11h
        xor     rbx, rsi
        rol     rbx, 3
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
        push    rbp
        sub     rsp, 8
;       mov     rbp, rsp

        mov     rdi, rcx                ; RDI = ClassEncrypted
        mov     rax, 0A636DED1017A006Fh
        xor     rdi, rax
        ror     rdi, 23
        mov     rbx, rdi
        shl     rbx, 32
        mov     rax, 03046B74562569154h
        xor     rbx, rax
        xor     rbx, rdi
        mov     rax, rbx

        add     rsp, 8
        pop     rbp
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

        mov     ebx, ecx                ; Index
        mov     edi, edx                ; Number

        ; Index decryption.
        xor     ebx, 0D5052678h
        rol     ebx, 14
        mov     eax, ebx
        shl     eax, 16
        xor     eax, 025CD3F7Bh
        xor     eax, ebx
        mov     dword ptr [r8], eax

        ; Number decryption
        xor     edi, 03CB04143h
        ror     edi, 10
        mov     eax, edi
        shl     eax, 16
        xor     eax, edi
        xor     eax, 0CD252F5Ch
        mov     dword ptr [r9], eax

        add     rsp, 8
        pop     rbp
        pop     rdi
        pop     rbx
        ret
DecryptObjectFNameAsm ENDP

        END