#include <string.h>
#include <EEPROM.h>

#include "settings.h"
#include "settings_eeprom.h"
#include "timer.h"
#include "ui.h"
#include <esp_attr.h>

const struct EepromSettings EepromDefaults = {
    .versionNumber = VERSION_NUMBER,
    .isCalibrated = false,
    .startChannel = 27,
    .dummy = 0,

    .diversityMode = Receiver::DiversityMode::DIVERSITY,

    .lastKnownState = StateMachine::State::HOME, // remove

    .rssiAMin = RSSI_MIN_VAL,
    .rssiAMax = RSSI_MAX_VAL,
    .rssiBMin = RSSI_MIN_VAL,
    .rssiBMax = RSSI_MAX_VAL,

    // Internal settings
    .rssiSeekTreshold = 500,
    .rssiMinTuneTime = 30, // ms
    .rssiHysteresis = 0,
    .rssiHysteresisPeriod = 0, // ms

    // Chorus32 settings
    .consecutives = 5,
};



static Timer DMA_ATTR saveTimer = Timer(EEPROM_SAVE_TIME);
static bool DMA_ATTR isDirty;

struct EepromSettings DMA_ATTR EepromSettings;

void EepromSettings::setup() {
    EEPROM.begin(2048); // Change size to match sizeof(*this)
    this->load();
}

void EepromSettings::update() {
    if (isDirty && saveTimer.hasTicked() && !Ui::isTvOn) {
        saveTimer.reset();
        this->save();
    }
}

void EepromSettings::load() {
    EEPROM.get(0, *this);

    if (this->versionNumber != VERSION_NUMBER)
        this->initDefaults();
    isDirty = false;
}

void EepromSettings::save() {
    EEPROM.put(0, *this);
    EEPROM.commit();
    isDirty = false;
}

void EepromSettings::markDirty() {
    isDirty = true;
}

void EepromSettings::initDefaults() {
    memcpy(this, &EepromDefaults, sizeof(EepromDefaults));
    this->save();
}
