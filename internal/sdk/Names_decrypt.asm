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

        mov     QWORD PTR [rbp],0
        mov     eax,ecx                     ; RCX = NamesEncrypted 
        rol     eax,16
        add     eax,06cec3cd4h
        rol     eax,16
        xor     eax,06cec3cd4h
        mov     DWORD PTR [rbp],eax
        shr     rcx,32
        ror     ecx,8
        add     ecx,06b536b54h
        ror     ecx,8
        xor     ecx,094ac94ach
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
        push    r8
        push    r9
        push    rdx
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     r9,rcx                  ; RCX = IndexEncrypted
        shr     r9,32
        mov     QWORD PTR [rbp],0
        mov     edx,ecx
        shr     edx,16
        movzx   eax,cx
        xor     edx,eax
        and     ecx,0ffff0000h
        or      edx,ecx
        add     edx,04e8e5652h
        mov     ecx,edx
        shr     ecx,16
        movzx   eax,cx
        ror     ax,8
        movzx   r8d,ax
        shl     r8d,16
        movzx   eax,dx
        xor     ecx,eax
        or      r8d,ecx
        xor     r8d,04e8e5652h
        mov     DWORD PTR [rbp],r8d
        mov     edx,r9d
        shr     edx,16
        movzx   eax,dx
        xor     ax,r9w
        ror     ax,8
        movzx   ecx,ax
        ror     dx,8
        movzx   edx,dx
        shl     edx,16
        or      edx,ecx
        add     edx,04d114d92h
        mov     eax,edx
        shr     eax,16
        xor     ax,dx
        ror     ax,8
        movzx   ecx,ax
        and     edx,0ffff0000h
        or      ecx,edx
        xor     ecx,0b2eeb26eh
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
        pop     rbx
        pop     rdx
        pop     r9
        pop     r8
        ret
DecryptNameEntryIndexAsm ENDP

; uint64_t DecryptChunksAsm(uint64_t ChunksEncrypted)
PUBLIC DecryptChunksAsm
DecryptChunksAsm PROC
        push    rbx
        push    rdi
        push    rsi
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     QWORD PTR [rbp],0
        lea     eax,[rcx-019d971c6h]    ; RCX = ChunksEncrypted
        xor     eax,0e6268e3ah
        mov     DWORD PTR [rbp],eax
        shr     rcx,32
        sub     ecx,01a061a86h
        xor     ecx,01a061a86h
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rsi
        pop     rdi
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

        mov     eax,ecx                 ; RCX = NumElementsEncrypted
        mov     QWORD PTR [rbp],0
        ror     eax,8
        add     eax,047674b09h
        shr     rcx,32
        ror     eax,8
        sub     ecx,039973957h
        xor     eax,047674b09h
        xor     ecx,039973957h
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