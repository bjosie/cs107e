#include "timer.h"
#include "assert.h"

void timer_init(void) {
}

unsigned int *TIMER_CLO = (unsigned int *)0x20003004;

unsigned int timer_get_ticks(void) {
    unsigned int currTicks = *TIMER_CLO;
    if (currTicks > 0){
        return currTicks;
    } else {
         assert(1==0);
    }
}

void timer_delay_us(unsigned int usecs) {
    //unsigned int start = timer_get_ticks();
    unsigned int currTime = timer_get_ticks();
    unsigned int finish = currTime + usecs;
    while (currTime < finish) {
         currTime = timer_get_ticks();
    }
}

void timer_delay_ms(unsigned int msecs) {
    timer_delay_us(1000*msecs);
}

void timer_delay(unsigned int secs) {
    timer_delay_us(1000000*secs);
}