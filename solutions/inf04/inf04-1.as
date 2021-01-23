.data
output_string: .asciz "%d\n"
input_string: .string "%d %d"
x1: .word 0
x2: .word 0

.text

.global main
.extern printf
.extern scanf
main:
	push {lr}
	ldr r0,  =input_string
	ldr r1,  =x1
	ldr r2,  =x2
	bl scanf
	ldr r1, =x1
	ldr r2, =x2
	ldr r1, [r1]
	ldr r2, [r2]

	add r1, r2, r1
	ldr r0, =output_string

	bl printf

	pop {lr}
	bx lr
