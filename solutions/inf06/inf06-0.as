.intel_syntax noprefix
.text
.global calc
.global vector_sum

calc:
	push ebp
	mov ebp, esp

	fld qword ptr [ebp+8]
	fadd qword ptr [ebp+16]

	fild dword ptr [ebp + 32]
	fadd qword ptr [ebp+24]

	fdivp st(1), st
	pop ebp

	ret



vector_sum:
	push ebp
	mov ebp, esp
	push esi
	push edi 
	push eax
	push edx
	push ecx
	mov ecx, 0

	mov eax, [ebp+8]
	mov esi,[ebp+12]
	mov edi, [ebp+16]
	mov edx,[ebp+20]

loop:
	movaps xmm0, [esi+ecx*4]
	movaps xmm1, [edi+ecx*4]
	addps xmm0, xmm1
	movaps [edx+ecx*4], xmm0
	add ecx, 4
	cmp ecx, eax
	jl loop

	pop ecx
	pop edx
	pop eax
	pop edi 
	pop esi
	pop ebp

	ret





