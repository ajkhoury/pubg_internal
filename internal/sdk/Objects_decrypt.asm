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

        mov     eax,ecx                 ; RCX = ObjectsEncrypted
        shr     rcx,32
        xor     eax,0c95d03bdh
        xor     ecx,02e8cc6e8h
        rol     ecx,8
        add     eax,064e4545ch
        add     ecx,063db63dch
        xor     eax,0b957e1h
        xor     ecx,0c98cc6e8h
        mov     DWORD PTR [rbp],eax
        rol     ecx,8
        xor     ecx,09c249c24h
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

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
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx            ; EBX = InternalIndexEncrypted
        mov     edi,ebx
        shl     edi,16
        xor     r12d,r12d
        xor     edi,01092215dh
        xor     edi,ebx
        mov     eax,edi

        add     rsp, 8
        pop     rbp
        pop     rsi
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
        xor     ebx,04c6d20beh
        ror     ebx,0fh
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

        mov     rbx, rcx                ; RBX = OuterEncrypted
        mov     rsi,017c5d6ba13b06525h
        mov     rbp,09cf3dd6d41702e7eh
        xor     rbx,rsi
        ror     rbx,0ch
        mov     rax,rbx
        shl     rax,32
        xor     rax,rbp
        xor     rax,rbx

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

        mov     rdi, rcx                ; RDI = ClassEncrypted
        mov     rax,01a511c71fcf834ebh
        xor     rdi,rax
        rol     rdi,1eh
        mov     rbx,rdi
        shl     rbx,32
        mov     rax,0c5a53184ee3b0cafh
        xor     rbx,rax
        xor     rbx,rdi
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
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx                ; EBX = IndexEncrypted
        mov     edi, edx                ; EDI = NumberEncrypted

        xor    ebx,020be4c6dh
        rol    ebx,1
        mov    eax,ebx
        shl    eax,16
        xor    eax,ebx
        xor    eax,0ee1586e3h
        mov    DWORD PTR [r8],eax

        xor    edi,04a359959h
        rol    edi,9
        mov    eax,edi
        shl    eax,16
        xor    eax,edi
        xor    eax,015eeb119h
        mov    DWORD PTR [r9],eax

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectFNameAsm ENDP

        END