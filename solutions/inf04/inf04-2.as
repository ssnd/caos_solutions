
.text

.global main
main:
push {r4, r5,r7, r8, lr}
ldr r4, .stdin
ldr r4, [r4]
ldr r5, .stdout
ldr r5, [r5]
mov r7, #48
mov r8, #57

.loop:
mov r0, r4
bl fgetc
cmp r0, #0
blt .endloop
cmp r0, r7
blt .loop
cmp r0, r8
bgt .loop
mov r1, r5
bl fputc
b .loop
.endloop:
pop {r4,r5,r7,r8, lr}
bx lr
//test
.stdin: .word stdin
.stdout: .word stdout