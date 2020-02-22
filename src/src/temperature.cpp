#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

namespace Temperature {

    float getTemperature() {

        return ( temprature_sens_read() - 32 ) / 1.8;
        
    }
}
