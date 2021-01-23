	.text
	.global summ

// aliases
i    .req r3
x0   .req r0
N    .req r1
X    .req r2
temp .req r4
 
summ:
	push {r4}
	mov i, #0 			// i = 0
loop:
	cmp i, N  			// i < N
	bge endfor 			// -> endfor
	ldr temp, [X, i, LSL #2] 	// temp = X[i]
	add x0, x0, temp 		// x0+=temp
	add i,i, #1 			// ++i
	b loop
endfor:
	mov r1, r0
	pop {r4}

	bx lr 