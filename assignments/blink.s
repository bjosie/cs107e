.equ DELAY, 0x3F0000

// configure GPIO 20 for output
ldr r0, FSEL0
mov r1, #0b001 << (4 * 3)
str r1, [r0]
//mov r1, #1

// set bit 20
mov r1, #(0b1<<4)
mov r3, #(1<<21)

loop: 

// set GPIO 20 high
ldr r0, SET0
str r1, [r0] 
str r3, [r0]

/*
// delay
mov r2, #DELAY
wait1:
    
    ldr r0, SET0
    str r3, [r0] 
    ldr r0, CLR0
    str r3, [r0] 
    ldr r0, SET0
    str r3, [r0] 
    ldr r0, CLR0
    str r3, [r0]   
    subs r2, #1
    bne wait1

// set GPIO 20 low
ldr r0, CLR0
str r1, [r0]
str r3, [r0]
*/
// delay

mov r2, #DELAY
wait2:
    subs r2, #2
    bne wait2


b loop

FSEL0: .word 0x20200000
FSEL1: .word 0x20200004
FSEL2: .word 0x20200008
SET0:  .word 0x2020001C
SET1:  .word 0x20200020
CLR0:  .word 0x20200028
CLR1:  .word 0x2020002C

