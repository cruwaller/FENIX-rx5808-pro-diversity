#include <Arduino.h>
        
namespace Temperature {
  
    float temperature = 0;
      
    void update() {
      
        adc_reg_map *regs = ADC1->regs;
        regs->CR2 |= ADC_CR2_TSVREFE;    
        regs->SMPR1 |=  (0b111 << 18);  // sample rate temperature
        regs->SMPR1 |=  (0b111 << 21);  // sample rate vrefint
        adc_calibrate(ADC1);
        
        float vdd = 1.20 * 4096.0 / adc_read(ADC1, 17);
        float currentTemp = (1.43 - (vdd / 4096.0 * adc_read(ADC1, 16))) / 0.0043 + 25.0;

        if (temperature == 0) {
            temperature = currentTemp;
        } else {
            temperature = currentTemp*0.001 + temperature*0.999;
        }
    }
}
