#include <Arduino.h>

#include "voltage.h"
#include "settings_eeprom.h"
#include "settings.h"
#include "timer.h"
#include "ui.h"

namespace Voltage
{

uint16_t voltageRaw = 0;
uint8_t voltage;
uint8_t voltageDec;
uint16_t prevVoltageRaw = 0;

void update()
{
    voltageRaw = 0;
#ifdef PIN_VBAT
    for (uint8_t i = 0; i < 8; i++)
    {
        voltageRaw += analogRead(PIN_VBAT);
    }
    voltageRaw /= 8;
#endif

    if (prevVoltageRaw == 0)
        prevVoltageRaw = voltageRaw;

    voltageRaw = voltageRaw / 10 + 9 * prevVoltageRaw / 10;
    prevVoltageRaw = voltageRaw;

    voltageRaw /= VBAT_SCALE;

    voltage = voltageRaw / 10;
    voltageDec = voltageRaw % 10;
}
} // namespace Voltage
