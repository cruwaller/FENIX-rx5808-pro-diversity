#ifndef VOLTAGE_H
#define VOLTAGE_H

#include <stdint.h>


namespace Voltage {
    void setup();
    void update();
    uint8_t get_mant();
    uint8_t get_dec();
}


#endif
