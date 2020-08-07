#include <Arduino.h>
#include "timer.h"


Timer::Timer(uint16_t delay) {
    this->delay = delay;
    this->nextTick = millis() + this->delay;
    this->ticked = false;
}

const bool Timer::hasTicked() {
    if (this->ticked)
        return true;

    int32_t _ticked = this->nextTick - millis();
    if (_ticked <= 0) {
        this->ticked = true;
        return true;
    }
    return false;
}

void Timer::reset() {
    this->nextTick = millis() + this->delay;
    this->ticked = false;
}
