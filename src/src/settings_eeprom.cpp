#include <string.h>
#include "EEPROM.h"

#include "settings.h"
#include "settings_eeprom.h"
#include "timer.h"
#include "ui.h"
#include <esp_attr.h>

const struct EepromSettings EepromDefaults = {
    .versionNumber = VERSION_NUMBER,

    .isCalibrated = false,
    .otaUpdateRequested = false,

    .diversityMode = Receiver::DiversityMode::DIVERSITY,

    .startChannel = 27,
    .lastKnownMenuItem = 0, // remove
    .lastKnownState = StateMachine::State::HOME, // remove

    .beepEnabled = true, // remove

    .rssiAMin = RSSI_MIN_VAL,
    .rssiAMax = RSSI_MAX_VAL,
    .rssiBMin = RSSI_MIN_VAL,
    .rssiBMax = RSSI_MAX_VAL,
    .rssiCMin = RSSI_MIN_VAL, // remove
    .rssiCMax = RSSI_MAX_VAL, // remove
    .rssiDMin = RSSI_MIN_VAL, // remove
    .rssiDMax = RSSI_MAX_VAL, // remove

    .vbatScale = VBAT_SCALE,
    .vbatWarning = WARNING_VOLTAGE,
    .vbatCritical = CRITICAL_VOLTAGE,

    .favouriteChannels = {32, 33, 34, 35, 36, 37, 38, 39}, // Race band,  // remove
    .spectatorChannels = {-1, -1, -1, -1, -1, -1, -1, -1}, // remove

    .quadversity = false, // remove
    .buttonBeep = true,

    // Internal settings
    .spectatorFreqScanStep = 5,
    .spectatorFWHM = 20,
    .rssiSeekTreshold = 500,
    .rssiMinTuneTime = 30, // ms
    .rssiHysteresis = 0,
    .rssiHysteresisPeriod = 0, // ms
};



static Timer DMA_ATTR saveTimer = Timer(EEPROM_SAVE_TIME);
static bool DMA_ATTR isDirty;

struct EepromSettings DMA_ATTR EepromSettings;

void EepromSettings::setup() {
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
    memcpy_P(this, &EepromDefaults, sizeof(EepromDefaults));
    this->save();
}
