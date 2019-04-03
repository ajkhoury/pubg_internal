        .CODE

;; unsigned __int64 DecryptWorldAsm(unsigned __int64 WorldEncrypted);
;PUBLIC DecryptWorldAsm
;DecryptWorldAsm PROC
;        push    rbx
;        push    rbp
;        sub     rsp, 8
;        mov     rbp, rsp
;
;        mov     rax, rcx
;        xor     eax, 0e93303bdh
;        ror     eax, 16
;        sub     eax, 07bfb0bc4h
;        xor     eax, 0843303bdh
;        ror     eax, 16
;        xor     eax, 08404f43ch
;        mov     DWORD PTR [rbp],eax
;        mov     rax, rcx
;        shr     rax, 32
;        xor     eax, 04e62c6e8h
;        ror     eax, 8
;        sub     eax, 07c447c44h
;        xor     eax, 0e962c6e8h
;        ror     eax, 8
;        xor     eax, 07c447c44h
;        mov     DWORD PTR [rbp+4], eax
;        mov     rax, QWORD PTR [rbp]
;
;        add     rsp, 8
;        pop     rbp
;        pop     rbx
;        ret
;DecryptWorldAsm ENDP
;
;; unsigned __int64 DecryptCurrentLevelAsm(unsigned __int64 CurrentLevelEncrypted);
;PUBLIC DecryptCurrentLevelAsm
;DecryptCurrentLevelAsm PROC
;        push    rbx
;        push    rbp
;        sub     rsp, 8
;        mov     rbp, rsp
;
;        mov     rdx, rcx                ; RDX = CurrentLevelEncrypted
;        mov     eax, edx
;        xor     eax, 0B8CCFC42h
;        not     eax
;        add     eax, 62229AFEh
;        xor     eax, 80119943h
;        mov     DWORD PTR [rbp], eax
;        shr     rdx, 32
;        xor     edx, 539D3917h
;        not     edx
;        add     edx, 613D61BEh
;        xor     edx, 0D9A058AAh
;        mov     DWORD PTR [rbp+4], edx
;        mov     rax, QWORD PTR [rbp]
;
;        add     rsp, 8
;        pop     rbp
;        pop     rbx
;        ret
;DecryptCurrentLevelAsm ENDP

; unsigned __int64 DecryptPersistentLevelAsm(unsigned __int64 PersistentLevelEncrypted);
PUBLIC DecryptPersistentLevelAsm
DecryptPersistentLevelAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     eax,ecx             ; RCX = PersistentLevelEncrypted
        mov     QWORD PTR [rbp],0
        rol     eax,8
        shr     rcx,32
        not     eax
        ror     ecx,16
        add     eax,021017d0fh
        not     ecx
        rol     eax,8
        xor     eax,0defe82f0h
        sub     ecx,05f515f91h
        ror     ecx,16
        xor     ecx,0a0aea06eh
        mov     DWORD PTR [rbp],eax
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptPersistentLevelAsm ENDP

; unsigned __int64 DecryptActorsAsm(unsigned __int64 ActorsEncrypted);
PUBLIC DecryptActorsAsm
DecryptActorsAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     r8, rcx             ; R8 = ActorsEncrypted
        mov     ecx,r8d
        mov     QWORD PTR [rbp],0
        shr     ecx,16
        mov     r9,r8
        movzx   eax,cx
        shr     r9,32
        ror     ax,8
        movzx   edx,ax
        movzx   eax,r8w
        xor     ecx,eax
        shl     edx,16
        or      edx,ecx
        add     edx,078789808h
        mov     ecx,edx
        shr     ecx,16
        movzx   eax,cx
        ror     ax,8
        movzx   r8d,ax
        movzx   eax,dx
        xor     ecx,eax
        shl     r8d,16
        or      r8d,ecx
        mov     eax,r9d
        shr     eax,16
        xor     r8d,078789808h
        xor     ax,r9w
        mov     DWORD PTR [rbp],r8d
        rol     ax,8
        and     r9d,0ffff0000h
        movzx   edx,ax
        or      edx,r9d
        add     edx,077077708h
        mov     eax,edx
        shr     eax,16
        xor     ax,dx
        and     edx,0ffff0000h
        rol     ax,8
        movzx   ecx,ax
        or      ecx,edx
        xor     ecx,088f888f8h
        mov     DWORD PTR [rbp+4],ecx
        mov     rax,QWORD PTR [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptActorsAsm ENDP

        END