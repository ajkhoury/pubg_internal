        .CODE

; uint64_t DecryptWorldAsm(uint64_t WorldEncrypted);
PUBLIC DecryptWorldAsm
DecryptWorldAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rax, rcx
        xor     eax, 0e93303bdh
        ror     eax, 16
        sub     eax, 07bfb0bc4h
        xor     eax, 0843303bdh
        ror     eax, 16
        xor     eax, 08404f43ch
        mov     DWORD PTR [rbp],eax
        mov     rax, rcx
        shr     rax, 32
        xor     eax, 04e62c6e8h
        ror     eax, 8
        sub     eax, 07c447c44h
        xor     eax, 0e962c6e8h
        ror     eax, 8
        xor     eax, 07c447c44h
        mov     DWORD PTR [rbp+4], eax
        mov     rax, QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptWorldAsm ENDP

; uint64_t DecryptCurrentLevelAsm(uint64_t CurrentLevelEncrypted);
PUBLIC DecryptCurrentLevelAsm
DecryptCurrentLevelAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rdx, rcx                ; RDX = CurrentLevelEncrypted
        mov     eax, edx
        xor     eax, 0B8CCFC42h
        not     eax
        add     eax, 62229AFEh
        xor     eax, 80119943h
        mov     DWORD PTR [rbp], eax
        shr     rdx, 32
        xor     edx, 539D3917h
        not     edx
        add     edx, 613D61BEh
        xor     edx, 0D9A058AAh
        mov     DWORD PTR [rbp+4], edx
        mov     rax, QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptCurrentLevelAsm ENDP

        END