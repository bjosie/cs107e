#include "printf.h"
#include "gpio.h"
#include "gpioextra.h"

#define STEPS 8
#define SAMPS 4

const int step_button[] = {
  100, //GPIO_PIN14,
  GPIO_PIN18,
  GPIO_PIN24,
  GPIO_PIN8,
  GPIO_PIN12,
  GPIO_PIN20,
  GPIO_PIN26,
  GPIO_PIN13,
};

const int step_led[] = {
  101, //GPIO_PIN15,
  GPIO_PIN23,
  GPIO_PIN25,
  GPIO_PIN7,
  GPIO_PIN16,
  GPIO_PIN21,
  GPIO_PIN19,
  GPIO_PIN6,
};

const int samp_button[]  = {
  GPIO_PIN11,
  GPIO_PIN9,
  GPIO_PIN22,
  GPIO_PIN27,
};

const int PLAY_BUTTON = GPIO_PIN4;
const int REC_BUTTON = GPIO_PIN5;

void print_pins(){
  printf("hello, world!\n");
  for (int i = 0; i < STEPS; i++){
    printf("Step Button %d: %d", i, step_button[i]);
    printf("    Step LED %d: %d\n", i, step_led[i]);
  }
  for (int i = 0; i < SAMPS; i++){
    printf("Sample Button %d: %d\n", i, samp_button[i]);
  }
  printf("Play Button: %d", PLAY_BUTTON);
  printf("Record Button: %d\n", REC_BUTTON);
}

void gpio_IO_init(){
    //Configure Step Buttons and LEDs
    for(int i = 0; i < STEPS; i++){
        gpio_set_input(step_button[i]);
        gpio_set_pullup(step_button[i]);
        gpio_set_output(step_led[i]);
        if (i < SAMPS){
            gpio_set_input(samp_button[i]);
            gpio_set_pullup(samp_button[i]);
        }
    }
    gpio_set_input(PLAY_BUTTON);
    gpio_set_pullup(PLAY_BUTTON);
    gpio_set_input(REC_BUTTON);
    gpio_set_pullup(REC_BUTTON);  
}


void main() {
    print_pins();
    gpio_IO_init();
}
