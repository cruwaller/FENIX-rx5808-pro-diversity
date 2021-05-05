#include <Arduino.h>
#include "voltage.h"
#include "settings.h"
#include "task_prios.h"



namespace Voltage
{

static volatile uint8_t voltage;
static volatile uint8_t voltageDec;

#ifdef PIN_VBAT
static uint16_t prevVoltageRaw = 0;

static void voltage_task(void*)
{
    uint32_t voltageRaw;

    while (1) {
        voltageRaw = 0;

        for (uint8_t i = 0; i < 8; i++)
        {
            voltageRaw += analogRead(PIN_VBAT);
        }
        voltageRaw /= 8;

        if (prevVoltageRaw == 0)
            prevVoltageRaw = voltageRaw;

        voltageRaw = (voltageRaw / 10) + ((9 * prevVoltageRaw) / 10);
        prevVoltageRaw = voltageRaw;

        voltageRaw /= VBAT_SCALE;

        voltage = voltageRaw / 10;
        voltageDec = voltageRaw % 10;

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
#endif

void setup()
{
#ifdef PIN_VBAT
    xTaskCreatePinnedToCore(voltage_task, "volt",
        512, NULL, TASK_PRIO_VOLT, NULL, 1);
#endif
}

uint8_t get_mant()
{
    return voltage;
}

uint8_t get_dec()
{
    return voltageDec;
}

} // namespace Voltage
