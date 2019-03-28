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

        mov     rax,rcx
        shr     rcx,32
        xor     eax,067de73ebh
        xor     ecx,0b45da658h
        sub     eax,035759deah
        sub     ecx,036aa362ah
        xor     eax,0d05411fdh
        xor     ecx,051f79072h
        mov     DWORD PTR [rbp],eax
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

        mov     ebx, ecx
        xor     ebx,01a02e929h
        ror     ebx,14
        mov     esi,ebx
        shl     esi,16
        xor     esi,ebx
        xor     esi,0a825ef1eh
        mov     eax,esi

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
        xor     ebx,0827741e1h
        rol     ebx,3
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
        mov     rsi,0b8213cea5665c11bh
        mov     rbp,035bc2b9281e9ae11h
        xor     rbx,rsi
        rol     rbx,6
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

        mov     rdi, rcx                    ; RDI = ClassEncrypted
        mov     rax,0a73bef1e35b16a80h
        xor     rdi,rax
        ror     rdi,20
        mov     rbx,rdi
        shl     rbx,16
        mov     rax,036afb043acad5969h
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

        ; Index decryption.
        xor     ebx,041e18277h
        ror     ebx,13
        mov     eax,ebx
        shl     eax,16
        xor     eax,ebx
        xor     eax,0d08566cfh
        mov     DWORD PTR [r8],eax
        ; Number decryption
        xor     edi,048e0de23h
        ror     edi,5
        mov     eax,edi
        shl     eax,16
        xor     eax,edi
        xor     eax,085d0dc4ch
        mov     DWORD PTR [r9],eax

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptObjectFNameAsm ENDP

        END