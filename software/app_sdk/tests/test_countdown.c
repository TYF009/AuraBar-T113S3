#include "countdown.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    countdown timer;
    countdown_start(&timer,60000,59000);
    assert(countdown_update(&timer,60000)==59000);
    assert(countdown_update(&timer,118999)==1);
    assert(countdown_update(&timer,119000)==0);
    countdown_start(&timer,60000,59000);
    countdown_running(&timer,false,89000);
    assert(countdown_update(&timer,299000)==30000);
    countdown_running(&timer,true,299000);
    assert(countdown_update(&timer,328999)==1);
    assert(countdown_update(&timer,330000)==0);
    countdown_start(&timer,0,400000);
    assert(!timer.running);
    puts("countdown: exact start, pause/resume and delayed ticks passed");
}
