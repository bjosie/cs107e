#include "gpio.h"
#include "assert.h"

 
struct gpio {
    unsigned int fsel[6];
    unsigned int reservedA;
    unsigned int set[2];
    unsigned int reservedB;
    unsigned int clr[2];
    unsigned int reservedC;
    unsigned int lev[2];
} ;

volatile struct gpio *gpio =  (struct gpio*)0x20200000;


void gpio_init(void) {
}

void gpio_set_function(unsigned int pin, unsigned int function) {

   /*Determines which FSEL you are using*/
   int pinSet = pin/10;
   unsigned int *functionRegister = &(gpio -> fsel[pinSet]);

   /*Determines which pin you are writing to within the FSEL register*/
   int pinPos = pin%10;
   
   /*Bitmasking to set function of pin while retaining all other current functions*/
   unsigned int functionClear = (7 << (3*pinPos));
   unsigned int currPinIO = ~functionClear;
   unsigned int currSetIO = currPinIO & *functionRegister;
   functionClear = (function << (3*pinPos));
   currSetIO = currSetIO | functionClear;
   *functionRegister = currSetIO;
}

unsigned int gpio_get_function(unsigned int pin) {
   
   /*Determines which FSEL you are using*/
   int pinSet = pin/10;
   unsigned int *functionRegister = &(gpio -> fsel[pinSet]); 
   
   /*Determines which pin you are writing to within the FSEL register*/
   int pinPos = pin%10;

   unsigned int pinIO = *functionRegister >> (pinPos*3);
   unsigned int function = pinIO & 7;
    return function;
}

void gpio_set_input(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

void gpio_set_output(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int value) {
    //assert(pin == GPIO_PIN4);
    unsigned int index = 0;
    if (pin > 31){
        index = 1;
        pin = pin - 32;
    }
    unsigned int *currRegister;
    if (value){
        currRegister = &(gpio->set[index]);
        //assert(currRegister == 0x2020001c)
    } else {
        currRegister = &(gpio->clr[index]);
    }
    *currRegister = (1 << pin);
    
}

unsigned int gpio_read(unsigned int pin) {
    unsigned int index = 0;
    if (pin > 31){
        index = 1;
        pin = pin - 32;
    }
    volatile unsigned int *currRegister;
    currRegister = &(gpio->lev[index]);
    unsigned int value = *currRegister >> pin;
    value = value & 1;
    return value;
}


