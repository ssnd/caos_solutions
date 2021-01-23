.global summ
.intel_syntax noprefix
summ:
        push    ebp
        mov     ebp, esp
        push ebx
        mov edi, 0
        mov esi, [ebp + 8]
        mov ebx, [ebp + 12]
        mov ecx, [ebp + 16]
        mov edx, [ebp + 20]
.loop:
        mov eax, [ebx + 4 * edi]
        add eax, [ecx  + 4 * edi]
        mov [edx + 4 * edi], eax
        add edi, 1
        cmp edi, esi
        jl  .loop
.end:
        pop ebx
        pop  ebp
        ret