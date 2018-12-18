#include "gpio.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"
#include "strings.h"
#include "printf.h"
#include "timer.h"
#include "ringbuffer.h"
#include "interrupts.h"
#include "assert.h"
#include "timer.h"

static const int CLK  = GPIO_PIN23;
const unsigned int DATA = GPIO_PIN24; 
const unsigned int BYTE = 8;
const unsigned int MAX_LEN = 3;

static unsigned int global_modifiers = 0; //Carries the values of each modifier function

rb_t *bit_buf;
static unsigned int bit_counter = 0; //counts how many bits have been sent
static unsigned int bit_stream = 0; //loads those bits into a stream
static unsigned int working_scancode = 0; //the scancode as given by the eight data bits
static unsigned int parity_check = 0;
static unsigned int last_interrupt = 0;

void wait_for_falling_clock_edge() {
    while (gpio_read(CLK) == 0) {}
    while (gpio_read(CLK) == 1) {}
}

void reset_stream(){
    bit_counter = 0;
    parity_check = 0;
    working_scancode = 0;
}


void gather_interrupt_scancode(unsigned int pc){
    assert(gpio_check_and_clear_event(CLK));
    int curr_time = timer_get_ticks();
    if(curr_time > (last_interrupt + 3000)){
        //printf("Extra Long Interrupt\n");
        reset_stream();
    }
    last_interrupt = curr_time;
    /*Start Bit*/
    if(bit_counter==0){
        if(gpio_read(DATA)){
            //reset_stream();
            //printf("Failed: Start Bit\n");
        } else {
            bit_counter++;
        }
        return; //does not evaluate the rest of the function
    }
    /*Data Bits*/
    if((bit_counter > 0) && (bit_counter < 9)){
        int curr_bit = gpio_read(DATA);
        working_scancode = working_scancode | (curr_bit << (bit_counter-1));
        parity_check += curr_bit;
        bit_counter++;
        return;
    }
    /*Parity Bit*/
    if(bit_counter == 9){
        parity_check += gpio_read(DATA);
        parity_check %= 2;
        if(parity_check != 1){
            //printf("Failed Parity\n");
            reset_stream();
            return;
        }
        bit_counter++;
        return;
    }
    /*End Bit*/
    if(bit_counter == 10){
        if(!gpio_read(DATA)){
            reset_stream();
            return;
        }
    }
    /*If the program reaches this line, it has stored a valid scancode*/
    rb_enqueue(bit_buf, working_scancode);
    reset_stream();
}

void setup_interrupts(){
    gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE);
    bool ok = interrupts_attach_handler(gather_interrupt_scancode);
    assert(ok);
    interrupts_enable_source(INTERRUPTS_GPIO3);
    interrupts_global_enable();
}

void keyboard_init(void) 
{
    gpio_set_input(CLK); 
    gpio_set_pullup(CLK); 
 
    gpio_set_input(DATA); 
    gpio_set_pullup(DATA); 

    setup_interrupts();
    bit_buf = rb_new();
}


void wait_for_start_bit(void){
    wait_for_falling_clock_edge();
}

/*Old version -- Does not utilize interrupts*/
/*There is no equivalent interrupt version because the function becomes a single line
    once the wait_for_falling_clock_edge() function is removed. */
unsigned int keyboard_read_bit(void){
    wait_for_falling_clock_edge();
    return gpio_read(DATA);
}

/*New version, utilizing interrupts*/
unsigned char keyboard_read_scancode(void) 
{
    while(1){
        if(!rb_empty(bit_buf)){
            unsigned int scancode;
            rb_dequeue(bit_buf, &scancode);
            //printf("Scancode: %02x\n", (unsigned char)scancode);
            return (unsigned char)scancode;
        }
    }
}

/*
//Old Version, which did not utilize interrupts
unsigned char keyboard_read_scancode(void) 
{
    int dataBits[BYTE];
    int parityCheck = 0;
    wait_for_falling_clock_edge();
    if(gpio_read(DATA)){
    printf("Start Bit Improperly Calibrated");
        return 0;
    }

    int currChar = 0;
    for(int i = 0; i < BYTE; i++){
        int currBit = keyboard_read_bit();
        currChar =  currChar | (currBit << i);
        parityCheck += currBit;
    }

    parityCheck += keyboard_read_bit();
    parityCheck %= 2;
    int stopBit = keyboard_read_bit();
    if(parityCheck != 1){
        printf("Parity Bit Improperly Calibrated \n");
    }
    if(stopBit != 1){
        printf("End Bit Improperly Calibrated \n");
    }
    return currChar;
}
/*********************/

int keyboard_read_sequence(unsigned char seq[])
{
    int length = 0;
    while(length < 3){
        seq[length] = keyboard_read_scancode();
        if (!(seq[length] == 0xf0 || seq[length] == 0xe0)){
            length++;
            break;
        }
        length++;
    }
    return length;
}

void set_hold_modifiers(keyboard_modifiers_t modifier, keyboard_action_t action){
        if (action == KEYBOARD_ACTION_DOWN){
            global_modifiers = global_modifiers | modifier;
        } else {
            global_modifiers = global_modifiers ^ modifier;
        }
}

void toggle_modifiers(keyboard_modifiers_t modifier, keyboard_action_t action){
    if (action == KEYBOARD_ACTION_DOWN){
        global_modifiers = global_modifiers ^ modifier;
    }
}

key_event_t keyboard_read_event(void) 
{
    unsigned char seq[3];
    int length = keyboard_read_sequence(seq);

    keyboard_action_t currAction = KEYBOARD_ACTION_DOWN;
    if (length > 1){
        unsigned char scanAction = seq[length-2];
        if(scanAction == 0xf0){
            currAction = KEYBOARD_ACTION_UP;
        }
    }

    key_event_t event;

    event.action = currAction;
    event.key = ps2_keys[seq[length-1]];
    event.seq_len = length;
    memcpy(event.seq, seq, length);

    unsigned char ch = event.key.ch;
    if(ch == PS2_KEY_SHIFT){
        set_hold_modifiers(KEYBOARD_MOD_SHIFT, event.action);
    } else if (ch == PS2_KEY_ALT) {
        set_hold_modifiers(KEYBOARD_MOD_ALT, event.action);
    } else if (ch == PS2_KEY_CTRL) {
        set_hold_modifiers(KEYBOARD_MOD_CTRL, event.action);
    } else if (ch == PS2_KEY_CAPS_LOCK) {
        toggle_modifiers(KEYBOARD_MOD_CAPS_LOCK, event.action);
    } else if (ch == PS2_KEY_NUM_LOCK) {
        toggle_modifiers(KEYBOARD_MOD_NUM_LOCK, event.action);
    } else if (ch == PS2_KEY_SCROLL_LOCK) {
        toggle_modifiers(KEYBOARD_MOD_SCROLL_LOCK, event.action);
    }  

    event.modifiers = global_modifiers;

    return event;
}



unsigned char keyboard_read_next(void) 
{
    key_event_t currEvent = keyboard_read_event();
    ps2_key_t currKey = currEvent.key;

    /* Should Only Return a character under the following criteria:                        */
    /*   1. The key is part of a "make" sequence, not "break"                              */
    /*   2. The key is a valid ascii character, not a special key such as SHIFT, ALT, etc. */
    while (currEvent.action == KEYBOARD_ACTION_UP || currKey.ch < ' ' || currKey.ch > '~'){ 
        if((currKey.ch == '\n' || currKey.ch == '\b') && currEvent.action == KEYBOARD_ACTION_DOWN){
            break;
        }
        currEvent = keyboard_read_event();
        currKey = currEvent.key;
    }

    if((global_modifiers & KEYBOARD_MOD_SHIFT) == KEYBOARD_MOD_SHIFT){
        if(currKey.other_ch){ //is not NULL
            return currKey.other_ch;
        }
    }

    if((global_modifiers & KEYBOARD_MOD_CAPS_LOCK) == KEYBOARD_MOD_CAPS_LOCK){
        if(currKey.other_ch){ //is not NULL
            if (currKey.ch >= 'a' && currKey.ch <= 'z'){ //CAPS_LOCK only affects letters
                return currKey.other_ch;
            }
        }
    }

    printf("Curr Char = %c\n", currKey.ch);
    return currKey.ch;
}
