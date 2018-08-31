#include <Arduino.h>
#include <avr/pgmspace.h>

#include "voltage.h"
#include "settings_eeprom.h"
#include "settings.h"
#include "pstr_helper.h"
#include "timer.h"
#include "ui.h"
        
namespace Voltage {
  
    Timer voltageBeeper = Timer(ALARM_EVERY_SEC);
  
    uint16_t voltageRaw = 0;  
    uint8_t RSSI_READS = 15;  
    uint8_t voltage;
    uint8_t voltageDec;
    uint16_t prevVoltageRaw = 0;
      
    void update() {

        #ifdef FENIX_QUADVERSITY
      
            for (uint8_t i = 0; i < RSSI_READS; i++) {                       
                voltageRaw += analogRead(PIN_VBAT);
            }
            voltageRaw /= RSSI_READS;

            if (prevVoltageRaw == 0) prevVoltageRaw = voltageRaw;
      
            voltageRaw = voltageRaw/10 + 9*prevVoltageRaw/10;
            prevVoltageRaw = voltageRaw;
              
            voltage = voltageRaw / EepromSettings.vbatScale;
            voltageDec = 
                map(
                    voltageRaw % EepromSettings.vbatScale,
                    0,
                    VBAT_SCALE,
                    0,
                    10
            );
      
            if ( (voltage*10 + voltageDec) < EepromSettings.vbatWarning && voltageBeeper.hasTicked()) {
              Ui::beep();
              voltageBeeper.reset();
            }
        #endif
    }
}
