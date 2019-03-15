        .CODE

; uint64_t DecryptObjectsAsm(uint64_t ObjectsEncrypted);
PUBLIC DecryptObjectsAsm
DecryptObjectsAsm PROC
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        lea     eax, [rcx-11FBFC43h]
        shr     rcx, 20h
        add     ecx, 5333C6E8h
        rol     eax, 10h
        ror     ecx, 8
        sub     eax, 2DD2F6DCh
        sub     ecx, 49C57051h
        rol     eax, 10h
        ror     ecx, 8
        xor     eax, 49290567h
        xor     ecx, 37F93739h
        mov     dword ptr [rbp], eax
        mov     dword ptr [rbp+4], ecx
        mov     rax, qword ptr [rbp]

        add     rsp, 8
        pop     rbp
        ret
DecryptObjectsAsm ENDP

; uint32_t DecryptObjectFlagsAsm(uint32_t ObjectFlagsEncrypted);
PUBLIC DecryptObjectFlagsAsm
DecryptObjectFlagsAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, ecx
        xor     ebx, 0BC52AD05h
        rol     ebx, 5
        mov     eax, ebx

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptObjectFlagsAsm ENDP

; uint32_t DecryptObjectInternalIndexAsm(uint64_t InternalIndexEncrypted);
PUBLIC DecryptObjectInternalIndexAsm
DecryptObjectInternalIndexAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rbx, rcx
        xor     ebx, 0EE560124h
        ror     ebx, 0Ch
        mov     eax, ebx
        shl     eax, 10h
        xor     eax, 6043A386h
        xor     eax, ebx

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptObjectInternalIndexAsm ENDP

; uint64_t DecryptObjectClassAsm(uint64_t ClassEncrypted);
PUBLIC DecryptObjectClassAsm
DecryptObjectClassAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rbx, rcx
        mov     rax, 43C5A3874B72E28Fh
        xor     rbx, rax
        rol     rbx, 0Ah
        mov     rax, rbx
        shl     rax, 20h
        mov     rcx, 19D87E75578E7F55h
        xor     rax, rcx
        xor     rax, rbx

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptObjectClassAsm ENDP

; uint64_t DecryptObjectOuterAsm(uint64_t OuterEncrypted);
PUBLIC DecryptObjectOuterAsm
DecryptObjectOuterAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     rbx, rcx
        mov     rax, 961B170F8231F67Ch
        xor     rbx, rax
        rol     rbx, 20h
        mov     rdx, rbx
        shl     rdx, 20h
        mov     rax, 0AACFBDB7837CB396h
        xor     rdx, rax
        xor     rdx, rbx
        mov     rax, rdx

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptObjectOuterAsm ENDP

; void DecryptObjectFNameAsm(const FNameEncrypted* InNameEncrypted, int32_t* OutIndex, int32_t* OutNumber);
PUBLIC DecryptObjectFNameAsm
DecryptObjectFNameAsm PROC
        push    rbx
        push    rbp
        sub     rsp, 8
        mov     rbp, rsp

        mov     ebx, dword ptr [rcx]    ; Index
        xor     ebx, 0DF808891h
        rol     ebx, 0Dh
        mov     eax, ebx
        shl     eax, 10h
        xor     eax, 1F7BE9D4h
        xor     eax, ebx
        mov     dword ptr [rdx], eax
        mov     ebx, dword ptr [rcx+4]  ; Number
        xor     ebx, 0AD05BC52h
        ror     ebx, 0Bh
        mov     eax, ebx
        shl     eax, 10h
        xor     eax, ebx
        xor     eax, 7B1F773Bh
        mov     dword ptr [r8], eax

        add     rsp, 8
        pop     rbp
        pop     rbx
        ret
DecryptObjectFNameAsm ENDP

        END