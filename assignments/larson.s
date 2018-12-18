/*
 * LARSON SCANNER
 *
 * Currently this code is the blink code copied from lab 1.
 *
 * Modify the code to implement the larson scanner for assignment 1.
 *
 * Make sure to use GPIO pins 20-23 (or 20-27) for your scanner.
 */

.equ DELAY1, 0x0D0000
.equ DELAY2, 0x00D000
.equ DELAY3, 0x00A000

/*Use these values instead to slow it down to a perceptible speed*/
//.equ DELAY1, 0x0D00000
//.equ DELAY2, 0x00D0000
//.equ DELAY3, 0x00A0000

.equ LEDCOUNT, 7
.equ DIM, 3
.equ DIMMER, 1
.equ STARTPIN, 20

// configure GPIO 20-27 for output
ldr r0, FSEL2

mov r1, #1    //store 001 at the end

mov r7, #LEDCOUNT    //counter

setupLoop: 
lsl r1, #3   //shift the digits 3 to the left
add r1, #1
subs r7, #1
bne setupLoop
str r1, [r0]
//output has been configured

mov r10, #LEDCOUNT //will determine the pins used in the loop


totalReset:
mov r4, #0b0
mov r1, #(0b1<< 20)
mov r2, #0
mov r3, #0


upLoop: 
    add r4, r4, #1 //count the number of times the program has looped. Will stop when r4 == #LEDCOUNT

    mov r7, #DELAY3 //determines how much time is spent on each LED before moving up to the next
    LEDwait1:
        /*In this loop, it is crucial that the LED is set before it is cleared, else it will
        stay on maximum brightness. Additionally, the exact number of times looped, using #DIM and
        #DIMMER here, provides extremely subtle changes in brightness*/
        mov r8, #DIM 
        upDimSET:
            ldr r0, SET0
            str r2, [r0]
            subs r8, #1
        bne upDimSET
        
        mov r8, #DIM
        upDimCLR:
            ldr r0, CLR0
            str r2, [r0]
            subs r8, #1
        bne upDimCLR
        
        
        mov r8, #DIMMER
        upDimmerSET:
            ldr r0, SET0
            str r3, [r0]
            subs r8, #1
        bne upDimmerSET

        mov r8, #DIMMER
        upDimmerCLR:
            ldr r0, CLR0
            str r3, [r0]
            subs r8, #1
        bne upDimmerCLR

        
        ldr r0, SET0
        str r1, [r0] //turn on the brightest LED

        subs r7, #1
    bne LEDwait1

    // set GPIO 20 lo
    ldr r0, CLR0
    str r1, [r0] //turn all 3 brightnesses off
    str r2, [r0]
    str r3, [r0] 

    mov r3, r2  //shift the set of LEDs used
    mov r2, r1
    lsl r1, #1
    cmp r10, r4 //guage whether it is at the end of the LED count
bne upLoop


//set between up and down

mov r4, #0b0 //reset LED counter

ldr r0, CLR0 
str r3, [r0] //only two LEDs on

lsr r1, #1 //fix increment
lsr r2, #1 

ldr r0, SET0
str r1, [r0] //keep brightest LED on


mov r7, #DELAY2
waitMid1:
    mov r8, #DIM
    midDimLoop:
        ldr r0, CLR0
        str r2, [r0]
        ldr r0, SET0
        str r2, [r0]
        subs r8, #1
    bne midDimLoop
    subs r7, #1
bne waitMid1


ldr r0, CLR0
str r2, [r0] //only the brightest LED on

mov r7, #DELAY1
waitMid2:
    subs  r7, #1
bne waitMid2

mov r2, #0 //these registers will be initialized at the end of the next loop
mov r3, #0

//take the LEDs back down
downLoop:
add r4, r4, #1


// delay
mov r7, #DELAY3
wait2:

    mov r8, #DIM
    downDimSET:
        ldr r0, SET0
        str r2, [r0]
        subs r8, #1
    bne downDimSET

    mov r8, #DIM
    downDimCLR:
        ldr r0, CLR0
        str r2, [r0]
        subs r8, #1
    bne downDimCLR
    
    
    mov r8, #DIMMER
    downDimmerSET:
        ldr r0, SET0
        str r3, [r0]      
        subs r8, #1
    bne downDimmerSET   

    mov r8, #DIMMER
    downDimmerCLR:
        ldr r0, CLR0
        str r3, [r0]
        subs r8, #1
    bne downDimmerCLR
    

    // set GPIO 20 high
    ldr r0, SET0
    str r1, [r0] 
    subs r7, #1

bne wait2

//turn all 3 brightnesses off
    ldr r0, CLR0
    str r1, [r0] 
    str r2, [r0]
    str r3, [r0] 

    mov r3, r2  //shift the set of LEDs used
    mov r2, r1
    lsr r1, #1
    cmp r10, r4 //guage whether it is at the end of the LED count

bne downLoop

//set between down and reset

ldr r0, CLR0
str r3, [r0]
mov r4, #0b0

lsl r1, #1
lsl r2, #1

ldr r0, SET0
str r1, [r0]


mov r7, #DELAY2
waitEnd:
    mov r8, #DIM
    endDimLoop:
        ldr r0, CLR0
        str r2, [r0]
        ldr r0, SET0
        str r2, [r0]
        subs r8, #1
    bne endDimLoop
    subs r7, #1
bne waitEnd


ldr r0, CLR0
str r2, [r0]

mov r7, #DELAY1
waitEnd2:
    subs  r7, #1
bne waitEnd2

mov r2, #0
mov r3, #0

b totalReset




FSEL0: .word 0x20200000
FSEL1: .word 0x20200004
FSEL2: .word 0x20200008
SET0:  .word 0x2020001C
SET1:  .word 0x20200020
CLR0:  .word 0x20200028
CLR1:  .word 0x2020002C

