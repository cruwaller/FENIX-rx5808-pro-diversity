#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

namespace Temperature {
  
//    float temperature = 0;

//    void setup() {
//        adc_reg_map *regs = ADC1->regs;
//        regs->CR2 |= ADC_CR2_TSVREFE;    
//        regs->SMPR1 = (0b111 << (3 * 6));
//    }
      
//    void update() {
//      
//    }


//    uint8_t temprature_sens_read();

    float getTemperature() {

        return ( temprature_sens_read() - 32 ) / 1.8;

//        uint16_t result = adc_read(ADC1, 16);        
//        float Vsense = (3300.0*result)/4096;        
//        float currentTemp = ((1430.0-Vsense)/4.3) + 25.0; 
//
//        if (temperature == 0) {
//            temperature = currentTemp;
//        } else {
//            temperature = currentTemp*0.001 + temperature*0.999;
//        }
    }
}
