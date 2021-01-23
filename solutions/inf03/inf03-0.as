.text
.global f

a .req r0

f:
mul a, r3, a
mul a, r3, a
mul r1, r3, r1
add a, r1, a
add a, r2, a
bx lr