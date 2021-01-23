.global dot_product
.intel_syntax noprefix
.text

dot_product:
	subss xmm0, xmm0
	mov rcx, -4

.for1:
	add rcx, 4
	cmp rdi, 4
	jb .for2
	movups xmm1, [rsi+4*rcx]
	movups xmm2, [rdx+4*rcx]
	dpps xmm1, xmm2, 0xFF
	addps xmm0, xmm1

	sub rdi, 4
	jmp .for1

.for2:
	cmp rdi, 0
	jz .end
	movups xmm1, [rsi + 4*rcx]
	movups xmm2, [rdx + 4*rcx]
	mulss xmm1, xmm2
	addss xmm0, xmm1
	sub rdi, 1
	add rcx, 1
	jmp .for2
.end:
	ret
