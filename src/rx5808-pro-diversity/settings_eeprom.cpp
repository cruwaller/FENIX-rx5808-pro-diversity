#include <string.h>
#include "extEEPROM.h"
#include <Wire.h>

#include "settings.h"
#include "settings_internal.h"
#include "settings_eeprom.h"

#include "timer.h"


static Timer saveTimer = Timer(EEPROM_SAVE_TIME);
static bool isDirty = false;

#ifdef EEPROM_AT24C02
    extEEPROM EEPROM(kbits_2, 1, 1);
#endif
#ifdef EEPROM_AT24C16
    extEEPROM EEPROM(kbits_16, 1, 8);
#endif
#ifdef EEPROM_AT24C32
    extEEPROM EEPROM(kbits_32, 1, 32);
#endif
#ifdef EEPROM_AT24C64
    extEEPROM EEPROM(kbits_64, 1, 32);
#endif

TwoWire Wire2(2);
byte i2cStat = EEPROM.begin(extEEPROM::twiClock100kHz,  &Wire2); 

struct EepromSettings EepromSettings;


void EepromSettings::update() {
    if (isDirty) {
        if (saveTimer.hasTicked()) {
            isDirty = false;
            saveTimer.reset();

            this->save();
        }
    }
}

void EepromSettings::load() {
  EEPROM.read(0, (byte *)this, sizeof(EepromSettings));

    if (this->versionNumber != VERSION_NUMBER)
        this->initDefaults();
}

void EepromSettings::save() {
  byte i2cStat = EEPROM.write(0, (byte *)this, sizeof(EepromSettings));
}

void EepromSettings::markDirty() {
    isDirty = true;
}

void EepromSettings::initDefaults() {
    memcpy_P(this, &EepromDefaults, sizeof(EepromDefaults));
    this->save();
}
