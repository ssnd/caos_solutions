.global summ
.global everyday795
.intel_syntax noprefix

summ:

	push ebp
	mov ebp, esp
	push ebx

	mov edi, 0
	mov esi, N
	mov ebx, A
	mov ecx, B
	mov edx, R

.loop:
	mov eax, 0
	mov eax, [ebx + 4 * edi]
	add eax, [ecx  + 4 * edi]

	mov [edx + 4 * edi], eax
	add edi, 1
	cmp edi, esi
	jl .loop

.end:
	pop ebx
	pop  ebp
	ret


everyday795:
	push ebp
	mov ebp, esp
	push ebx
	sub esp, 12

	mov eax, esp
	add eax, 8
	mov [esp + 4], eax

	mov edi, [ebp + 8] 
	mov esi, [ebp + 12]

	mov ecx, offset scanf_str
	mov [esp], ecx
	call scanf

	mov ebx, [esp + 8]
	imul ebx, edi
	add ebx, esi

	mov ecx, offset printf_str

	mov [esp], ecx
	mov [esp + 4], ebx

	call printf

	add esp,12

	pop ebx
	pop ebp
	ret


printf_str:
	.string "%d\n"
scanf_str:
	.string "%d"