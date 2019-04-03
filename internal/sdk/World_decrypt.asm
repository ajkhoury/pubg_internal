        .CODE

; unsigned __int64 DecryptWorldAsm(unsigned __int64 WorldEncrypted);
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

; unsigned __int64 DecryptPersistentLevelAsm(unsigned __int64 PersistentLevelEncrypted);
PUBLIC DecryptPersistentLevelAsm
DecryptPersistentLevelAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     eax,ecx
        ror     eax,8
        sub     eax,038183477h
        shr     rcx,32
        ror     eax,8
        add     ecx,046e84629h
        xor     eax,0c7e7cb89h
        xor     ecx,0b917b9d7h
        mov     DWORD PTR [rbp],eax
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptPersistentLevelAsm ENDP

; unsigned __int64 DecryptCurrentLevelAsm(unsigned __int64 CurrentLevelEncrypted);
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

; unsigned __int64 DecryptActorsAsm(unsigned __int64 ActorsEncrypted);
PUBLIC DecryptActorsAsm
DecryptActorsAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        lea     eax, [rcx-1AE496EBh]    ; RCX = ActorsEncrypted
        xor     eax, 55B5619Bh
        shr     rcx, 32
        add     ecx, 2E80624Bh
        mov     DWORD PTR [rbp], eax
        xor     ecx, 2B452B85h
        mov     DWORD PTR [rbp+4], ecx
        mov     rax, QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptActorsAsm ENDP

        END