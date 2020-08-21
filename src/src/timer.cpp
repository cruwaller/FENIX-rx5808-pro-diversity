#include <Arduino.h>
#include "timer.h"


Timer::Timer(uint16_t delay) {
    this->delay = delay;
    reset();
}

const bool ICACHE_RAM_ATTR Timer::hasTicked() {
    if (!this->ticked)
        this->ticked = (0 >= (int32_t)(this->nextTick - millis()));
    return this->ticked;
}

void ICACHE_RAM_ATTR Timer::reset() {
    this->nextTick = millis() + this->delay;
    this->ticked = false;
}
