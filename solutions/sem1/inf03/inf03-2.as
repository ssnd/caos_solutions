	.text
	.global solve

// aliases
i   .req r4
temp .req r5
res .req r6
C   .req r0
D   .req r1
E   .req r2
F   .req r3

solve:
	push {r4, r5, r6}
	mov i, #0 // i = 0


loop: 
	mov res, #0 // res = 0
	mov temp, i 		// temp = x
	mul temp, i, temp       // temp *= x
	mul temp, i, temp	// temp *= x
	mul temp, C, temp      // temp *= C
	add res, temp, res      // res += temp
	
	mov temp, i		// temp = x
	mul temp, i, temp	// temp *= x
	mul temp, D, temp 	// temp *= D
	add res, temp, res	// res += temp
	
	mov temp, i		// temp = x
	mul temp, E, temp 	// temp *= E
	add res, temp, res	/// res += temp
	
	add res, F, res		// res += F temp
	cmp res, #0		// res == 0

	beq endfor
	add  i, i, #1		// ++i
	cmp i, #254
	beq endfor

	
	b loop
endfor:
	mov r0, i


	pop {r4,r5,r6}
	bx lr 