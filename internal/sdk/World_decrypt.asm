        .CODE

; uint64_t DecryptWorldAsm(uint64_t WorldEncrypted);
PUBLIC DecryptWorldAsm
DecryptWorldAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rax, rcx
        not     eax
        xor     eax, 0775d03bdh
        not     eax
        sub     eax, 06dadf532h
        xor     eax, 0800f0973h
        mov     dword ptr [rbp], eax
        mov     rax, rcx
        shr     rax, 32
        not     eax
        xor     eax, 0dc8cc6e8h
        not     eax
        sub     eax, 06ef26e72h
        xor     eax, 0197ea89ah
        mov     dword ptr [rbp+4], eax
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptWorldAsm ENDP

        END