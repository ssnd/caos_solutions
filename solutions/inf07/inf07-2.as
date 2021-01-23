#include <asm/unistd_32.h>

	.intel_syntax noprefix
	.text
	.global _start

_start:
	mov eax, __NR_brk
	mov ebx, 0
	int 0x80
	
	mov esi, eax
	add esi, 4096
	
	mov eax, __NR_brk
	mov ebx, esi
	int 0x80
	sub esi, 4096

.for:
	mov eax, __NR_read
	mov ebx, 0
	mov ecx, esi
	mov edx, 4096
	int 0x80

	cmp eax, 0
	je .end

	mov edi, eax

	mov eax, __NR_write
	mov ebx, 1
	mov ecx, esi
	mov edx, edi
	int 0x80
	jmp .for

.end:
	sub esi, 4096
	mov eax, __NR_brk
	mov ebx, esi
	int 0x80

	mov eax, __NR_exit
	mov ebx, 0
	int 0x80


