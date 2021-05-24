#include <asm/unistd_32.h>
	.intel_syntax noprefix
	.text
	.global _start

_start:
	mov eax, __NR_brk
	mov ebx, 0
	int 0x80

	mov edi, eax
	mov esi, eax
	mov ebp, 0

.loop:
	add esi, 4096
	mov eax, __NR_brk
	mov ebx, esi
	int 0x80

	sub esi, 4096

	mov eax, __NR_read
	mov ebx, 0
	mov ecx, esi
	mov edx, 4096
	int 0x80

	add ebp, eax

	cmp eax, 4096
	jl .ecx

	add esi, 4096
	jmp .loop

.ecx:
	mov ecx, ebp
.loop2:
	sub ecx, 1
	cmp ecx, 0
	jle .first

	movb al, [edi + ecx]
	cmp al, 10
	jz .print_str

	jmp .loop2

.print_str:
	push ecx
	push edi 

	mov esi, ebp
	add edi, ecx

	add edi, 1
	sub esi, ecx
	sub esi, 1

	mov eax, __NR_write
	mov ebx, 1
	mov ecx, edi
	mov edx, esi
	int 0x80

	sub edi, 1

	mov eax, __NR_write
	mov ebx, 1
	mov ecx, edi
	mov edx, 1

	int 0x80

	pop edi
	pop ecx
	mov ebp, ecx

	jmp .loop2

.first:
	mov eax, __NR_write
	mov ebx, 1
	mov ecx, edi
	mov edx, ebp
	int 0x80

.end:
	mov eax, __NR_exit
	mov ebx, 0
	int 0x80