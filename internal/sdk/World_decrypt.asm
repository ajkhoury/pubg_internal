        .CODE

; uint64_t DecryptWorldAsm(uint64_t WorldEncrypted);
PUBLIC DecryptWorldAsm
DecryptWorldAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     eax, ecx                ; RCX = WorldEncrypted
        xor     eax, 0e74173ebh
        add     eax, 04a0ae296h
        xor     eax, 0d04b917dh
        mov     dword ptr [rbp],eax
        shr     rcx, 32
        xor     ecx, 034c0a658h
        add     ecx, 049d54956h
        xor     ecx, 051ea10f2h
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptWorldAsm ENDP

        END