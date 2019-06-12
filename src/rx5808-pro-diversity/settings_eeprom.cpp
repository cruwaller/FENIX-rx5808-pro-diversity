#include <string.h>
#include "EEPROM.h"

#include "settings.h"
#include "settings_eeprom.h"
#include "timer.h"
#include "ui.h"

static Timer saveTimer = Timer(EEPROM_SAVE_TIME);
static bool isDirty = false;

struct EepromSettings EepromSettings;

void EepromSettings::setup() {
    this->load();
}

void EepromSettings::update() {
    if (isDirty && saveTimer.hasTicked() && !Ui::isTvOn) {
        isDirty = false;
        saveTimer.reset();
        this->save();
    }
}

void EepromSettings::load() {
    EEPROM.get(0, *this);
    
    if (this->versionNumber != VERSION_NUMBER)
        this->initDefaults();
}

void EepromSettings::save() {
    EEPROM.put(0, *this);
    EEPROM.commit();
}

void EepromSettings::markDirty() {
    isDirty = true;
}

void EepromSettings::initDefaults() {
    memcpy_P(this, &EepromDefaults, sizeof(EepromDefaults));
    this->save();
}
