#include "printf.h"
#include "gpio.h"
#include "gpioextra.h"
#include "timer.h"
#include "midi.h"
#include "interrupts.h"
#include "assert.h"

#define STEPS 8
#define SAMPS 4
#define MIDI_OUT GPIO_PIN17
//#define MIDI_CHANNEL 0

const int step_button[] = {
  GPIO_PIN4,
  GPIO_PIN18,
  GPIO_PIN24,
  GPIO_PIN8,
  GPIO_PIN12,
  GPIO_PIN20,
  GPIO_PIN26,
  GPIO_PIN13,
};

const int step_led[] = {
  GPIO_PIN5,
  GPIO_PIN23,
  GPIO_PIN25,
  GPIO_PIN7,
  GPIO_PIN16,
  GPIO_PIN21, //turning on when I send a MIDI note to GPIO17....
  GPIO_PIN19,
  GPIO_PIN6,
};

const int samp_button[]  = {
  GPIO_PIN11,
  GPIO_PIN9,
  GPIO_PIN22,
  GPIO_PIN27,
};

const int WRITE_BUTTON = 100;
const int REC_BUTTON = 100;

int *curr_delay;
int delay1[] = {
  500,
  500,
  500,
  500,
  500,
  500,
  500,
  500,
};

/*Function to ensure pins were put into arrays correctly*/
void test_print_pins(){
  printf("hello, world!\n");
  for (int i = 0; i < STEPS; i++){
    printf("Step Button %d: %d", i, step_button[i]);
    printf("    Step LED %d: %d\n", i, step_led[i]);
  }
  for (int i = 0; i < SAMPS; i++){
    printf("Sample Button %d: %d\n", i, samp_button[i]);
  }
  printf("Write Button: %d", WRITE_BUTTON);
  printf("    Record Button: %d\n", REC_BUTTON);
}

/*Calls and checks whether the gpio pins have been configured properly*/
void test_gpio_IOs(){
  for(int i = 0; i < STEPS; i++){
    printf("Step %d: %d", i, gpio_get_function(step_button[i]));
    printf("     StepLED %d: %d\n", i, gpio_get_function(step_led[i]));
    if (i < SAMPS){
      printf("Sample %d: %d\n", i, gpio_get_function(samp_button[i]));
    }
  }
  printf("Play Button: %d", gpio_get_function(WRITE_BUTTON));
  printf("    Record Button: %d\n", gpio_get_function(REC_BUTTON));
}

/*Continually reads every Button to check which are depressed*/
void test_button_presses(){
  while(1){
    for(int i = 0; i < STEPS; i++){
      if(!gpio_read(step_button[i])){
        printf("Step Button %d pressed\n", i);
      }
      if(i < SAMPS){
        if(!gpio_read(samp_button[i])){
          printf("Sample Button %d pressed\n", i);
        }
      }
    }
    if(!gpio_read(REC_BUTTON)){
      printf("Record Button Pressed\n");
    }
    if(!gpio_read(WRITE_BUTTON)){
      printf("Play Button Pressed\n");
    }
    timer_delay_ms(1);
  }
}

/*Makes one loop around all Step LEDs*/
void test_leds(){
  for(int i = 0; i < STEPS; i++){
    gpio_write(step_led[i],1);
    timer_delay(1);
    gpio_write(step_led[i],0);
  }
}

typedef struct {
  uint8_t channel;
  uint8_t note;
  uint8_t velocity;
} midi_msg_t;

midi_msg_t samples[SAMPS];

/*Initially populates the sample buttons with usable midi notes*/
void sample_init(){
  samples[0].channel = 1;  
  samples[0].note = 36; //kick
  samples[0].velocity = 125;

  samples[1].channel = 1;  
  samples[1].note = 44; //closed hat
  samples[1].velocity = 70;

  samples[2].channel = 1;  
  samples[2].note = 40; //snare
  samples[2].velocity = 125;

  samples[3].channel = 1;  
  samples[3].note = 37; //tom
  samples[3].velocity = 125;
}

/*Stores the beats at which each sample is played, lsb first:
  rhythm[1] = 0b01000001 means that the second sample
  will be played at the first and seventh steps*/
unsigned char rhythm[] = {
  0b10101010,
  0b11111111,
  0b01010101,
  0b00000001,
};

void set_delay(){
  curr_delay = delay1;
}

/*Sets all gpio pins attached to buttons and LEDs to do their assigned task*/
void gpio_IO_init(){
    for(int i = 0; i < STEPS; i++){
        gpio_set_input(step_button[i]);
        gpio_set_pullup(step_button[i]);
        gpio_set_output(step_led[i]);
        if (i < SAMPS){
            gpio_set_input(samp_button[i]);
            gpio_set_pullup(samp_button[i]);
        }
    }
    gpio_set_output(WRITE_BUTTON);
    gpio_set_output(REC_BUTTON);
}

void led_flash(){
  for(int i = 0; i < STEPS; i++){
    gpio_write(step_led[i], 1);
    timer_delay_ms(250);
  }
  for(int i = 0; i < STEPS; i++){
    gpio_write(step_led[i], 0);
    timer_delay_ms(250);
  }
}

/*Simple boolean to determine whether you are writing to steps or not, and for which sample*/
/*Use bitwise operations to determine which sample should be written to*/
unsigned char sample_write_mode = 0b00000000;
unsigned int global_step = 0;


void enable_step_interrupts(){
  for(int i = 0; i < STEPS; i++){
    gpio_enable_event_detection(step_button[i], GPIO_DETECT_FALLING_EDGE);
  }
}

void disable_step_interrupts(){
  for(int i = 0; i < STEPS; i++){
    gpio_disable_event_detection(step_button[i], GPIO_DETECT_FALLING_EDGE);
  }
}


void sample_interrupt(int sample){
  unsigned int curr_mode = (sample_write_mode & (1 << sample));
  //printf("Interrupted. Mode: %d\n", curr_mode);
  printf("sample write mode before: %d \n",sample_write_mode);
  if(!curr_mode){
    gpio_write(REC_BUTTON, 1);
    for(int i = 0; i < STEPS; i++){
      unsigned int beat = (rhythm[sample] & (1 << i));
      if(beat){
        beat = 1;
      }
      gpio_write(step_led[i], (beat ^ gpio_read(step_led[i])));
    }
    enable_step_interrupts();
    sample_write_mode |= (1 << sample);
  } else {
    gpio_write(REC_BUTTON, 0);
    for(int i = 0; i < STEPS; i++){
      gpio_write(step_led[i], 0);
    }
    gpio_write(step_led[global_step], 1);
    disable_step_interrupts();
    sample_write_mode &= ~(1<<sample);
  }
  printf("sample write mode after operation: %d \n",sample_write_mode);
}

void print_rhythm(int sample){
  printf("Sample %d Rhythm: 0b", sample);
  for(int i = 0; i < STEPS; i++){
    printf("%d", (rhythm[sample] >> STEPS-(i + 1) & 1));
  }
  printf("\n");
}

void step_interrupt(int step){ //is only called when a single sample is in write mode
  int i;
  for(i = 0; i < SAMPS; i++){
    if(sample_write_mode & (1 << i)){
      break;
    }
  }
  rhythm[i] ^= (1 << step);
  gpio_write(step_led[step], (!gpio_read(step_led[step])));
  print_rhythm(i);
}

int interrupts = 0;
unsigned int last_interrupt = 0;


void interrupt_handler(){
  //unsigned int curr_time = timer_get_ticks();
  unsigned int curr_time = timer_get_ticks();
  if (curr_time < (last_interrupt + 200000)){ 
    //if ((curr_time < (last_interrupt + 200000)) || gpio_read(samp_button[i])){
    return;
  }
  timer_delay_ms(3); //ignore (blocking) the erratic on/offs
  last_interrupt = curr_time;
  for(int i = 0; i < SAMPS; i++){
    if(gpio_check_event(samp_button[i])){
      gpio_clear_event(samp_button[i]);
      if(!gpio_read(samp_button[i])){
        sample_interrupt(i);
        interrupts++;
      }
    }
  }
  for(int i = 0; i < STEPS; i++){
    if(gpio_check_event(step_button[i])){
      gpio_clear_event(step_button[i]);
      if(!gpio_read(step_button[i])){
        step_interrupt(i);
        interrupts++;
      }
    }
  }
}

void configure_interrupts(){
  for(int i = 0; i < SAMPS; i++){
    gpio_enable_event_detection(samp_button[i], GPIO_DETECT_FALLING_EDGE);
    bool ok = interrupts_attach_handler(interrupt_handler);
    assert(ok);
  }

    interrupts_enable_source(INTERRUPTS_GPIO0);
    interrupts_enable_source(INTERRUPTS_GPIO1);
    interrupts_enable_source(INTERRUPTS_GPIO2);
    interrupts_enable_source(INTERRUPTS_GPIO3);
    interrupts_global_enable();

}


/*Sends all MIDI notes stored on the current step*/
void seek(int step){
  for(int i = 0; i < SAMPS; i++){
    if(rhythm[i] & (1 << step)){
      midi_note_on(samples[i].note,samples[i].velocity);
    }
  }
}

void cycle(){
  for (global_step = 0; global_step < STEPS; global_step++){
    int step_status = gpio_read(step_led[global_step]);
    gpio_write(step_led[global_step], !gpio_read(step_led[global_step]));
    seek(global_step);
    timer_delay_ms(curr_delay[global_step]);
    gpio_write(step_led[global_step], !gpio_read(step_led[global_step]));
  }
}

void main() {
    gpio_IO_init();
    sample_init();
    test_gpio_IOs();
    set_delay();
    midi_init();
    midi_test();
    configure_interrupts();
    led_flash();
    gpio_write(WRITE_BUTTON, 1);
    while(1){
      //print_rhythm(0);
      //print_rhythm(1);
      //print_rhythm(2);
      //print_rhythm(3);
      cycle();
    }
}
