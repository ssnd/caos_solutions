	.text
.global R
.data
    R:.word 3

.addr_R: .word R
.addr_A: .word A
.addr_B: .word B
.addr_C: .word C
.addr_D: .word D


.global calculate
calculate: 
	mov r2, #0

	ldr r0, .addr_A
	ldr r0, [r0]
	ldr r1, .addr_B
	ldr r1, [r1]
	mul r0, r1, r0
	add r2, r0, r2

	ldr r0, .addr_C
	ldr r0, [r0]
	ldr r1, .addr_D
	ldr r1, [r1]
	mul r0, r1, r0
	add r2, r0, r2

	ldr r0, .addr_R
	ldr r1, [r0]
	str r2, [r0]	

	bx lr
