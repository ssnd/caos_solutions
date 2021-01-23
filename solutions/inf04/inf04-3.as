.text

.global main
.extern printf
.extern fgetc
.extern fputc
.extern realloc
.extern malloc
.extern free
.stdin: .word stdin
.stdout: .word stdout
main:
	
    push {lr, r4, r5, r6, r7, r8, r9}
	
	ldr r4, .stdin
	ldr r4, [r4]
	ldr r5, .stdout
	ldr r5, [r5]
	
	mov r6, #0  // i = 1 (malloc memory for a single integer)
	mov r0, r6  //  
	bl malloc 	//  malloc(0) 

	mov r7, r0  // int* -> r7

loop:
	mov r0, r4
	bl fgetc
	mov r8, r0
	cmp r8, #0
	blt loop2 // endfor if == '4'

	add r6, r6, #4 // r6 += 4
	/////debug///////////////////////
	//ldr r0, =output_string	/////
	//mov r1, r6				/////
	//bl printf					/////
	////// debug ////////////////////
	mov r1, r6
	mov r0, r7	// r0 -> r7
	bl realloc
	mov r7, r0	
	mov r9, r6

	sub r9, r9, #4
	str r8, [r7, r9]

	b loop

loop2:
	mov r1, r5
	ldr r0, [r7, r9]
	bl fputc
	sub r9, r9, #4
	cmp r9, #-4	
	bne loop2

	mov r0, r7
	bl free
    pop {lr, r4, r5, r6, r7, r8, r9}

    bx lr