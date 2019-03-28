        .CODE

; uint64_t DecryptNamesAsm(uint64_t NamesEncrypted)
PUBLIC DecryptNamesAsm
DecryptNamesAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rax, rcx                ; RCX = NamesEncrypted
        xor     eax,048c1fc42h
        not     eax
        sub     eax,02d6db572h
        xor     eax,080ac4933h
        mov     DWORD PTR [rbp],eax
        shr     rcx,32
        xor     ecx,0e3923917h
        not     ecx
        sub     ecx,02e322eb2h
        xor     ecx,0995fe85ah
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
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

        mov     rax, rcx                ; RCX = IndexEncrypted
        xor     eax,0193e03bdh
        rol     eax,8
        sub     eax,04bcb9bf4h
        xor     eax,0b43e03bdh
        rol     eax,8
        xor     eax,0b434640ch
        mov     DWORD PTR [rbp],eax
        shr     rcx,32
        xor     ecx,07e6dc6e8h
        ror     ecx,16
        sub     ecx,04c744c74h
        xor     ecx,0196dc6e8h
        ror     ecx,16
        xor     ecx,04c744c74h
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

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

        mov     rax,rcx
        rol     eax,16
        sub     eax,073f3234ch
        rol     eax,16
        xor     eax,08c0cdcb4h
        mov     DWORD PTR [rbp],eax
        shr     rcx,32
        ror     ecx,8
        sub     ecx,074cc74cch
        ror     ecx,8
        xor     ecx,074cc74cch
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

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

        mov     rax,rcx
        not     eax
        shr     rcx,32
        add     eax,049618a8fh
        not     ecx
        not     eax
        sub     ecx,045fd2a88h
        sub     eax,052f2a63dh
        not     ecx
        not     eax
        add     ecx,02ce22ca3h
        sub     eax,0279e7571h
        not     ecx
        xor     eax,052f2a63ch
        add     ecx,04902d578h
        xor     ecx,02ce22ca2h
        mov     DWORD PTR [rbp],eax
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        ret
DecryptNumElementsAsm ENDP

        END