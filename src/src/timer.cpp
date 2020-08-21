#include <Arduino.h>
#include "timer.h"


Timer::Timer(uint16_t delay) {
    delay = delay;
    reset();
}

const bool ICACHE_RAM_ATTR Timer::hasTicked() {
    if (!ticked)
        ticked = !!(0 >= (int32_t)(nextTick - millis()));
    return ticked;
}

void ICACHE_RAM_ATTR Timer::reset() {
    nextTick = millis() + delay;
    ticked = false;
}
