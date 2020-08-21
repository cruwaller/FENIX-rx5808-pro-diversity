#include <string.h>
#include "EEPROM.h"

#include "settings.h"
#include "settings_eeprom.h"
#include "timer.h"
#include "ui.h"


const struct EepromSettings EepromDefaults = {
    .versionNumber = VERSION_NUMBER,

    .isCalibrated = false,
    .otaUpdateRequested = false,

    .diversityMode = Receiver::DiversityMode::DIVERSITY,

    .startChannel = 27,
    .lastKnownMenuItem = 0,
    .lastKnownState = StateMachine::State::HOME,

    .beepEnabled = true,

    .rssiAMin = RSSI_MIN_VAL,
    .rssiAMax = RSSI_MAX_VAL,
    .rssiBMin = RSSI_MIN_VAL,
    .rssiBMax = RSSI_MAX_VAL,
    .rssiCMin = RSSI_MIN_VAL,
    .rssiCMax = RSSI_MAX_VAL,
    .rssiDMin = RSSI_MIN_VAL,
    .rssiDMax = RSSI_MAX_VAL,

    .vbatScale = VBAT_SCALE,
    .vbatWarning = WARNING_VOLTAGE,
    .vbatCritical = CRITICAL_VOLTAGE,

    .favouriteChannels = {32, 33, 34, 35, 36, 37, 38, 39}, // Race band
    .spectatorChannels = {-1, -1, -1, -1, -1, -1, -1, -1},

    .quadversity = false,
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
static bool DMA_ATTR isDirty = false;

struct EepromSettings DMA_ATTR EepromSettings;

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
