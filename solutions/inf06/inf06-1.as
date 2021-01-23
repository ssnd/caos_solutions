
.global my_sin
.intel_syntax noprefix
.text

my_sin:
	push ebp
	mov ebp, esp
	sub  esp, 8

	movsd xmm0, [ebp + 8]
	movsd xmm1, [ebp + 8]
	cvtsi2sd xmm2, eax
	movsd xmm3, [ebp + 8]


	mov eax, 1
	mov ebx, -1

	mulsd xmm3, xmm3
	cvtsi2sd xmm2, ebx
	mulsd xmm3, xmm2

for:
	mulsd xmm1, xmm3
	add eax, 1
	cvtsi2sd xmm2, eax
	divsd xmm1, xmm2
	add eax, 1
	cvtsi2sd xmm2, eax

	divsd xmm1, xmm2
	movsd xmm3, xmm0
	//cmp
	addsd xmm0, xmm1

	comisd xmm0, xmm3
	jne for

end:
	movsd [ebp - 8], xmm0
	fld  qword ptr [ebp - 8]
	mov esp, ebp

	pop  ebp
	ret