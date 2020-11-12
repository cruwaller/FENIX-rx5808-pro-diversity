#ifndef SETTINGS_EEPROM_H
#define SETTINGS_EEPROM_H


#include <stdint.h>

#include "settings.h"
#include "state.h"
#include "receiver.h"
#include <esp_attr.h>


struct EepromSettings {
    uint8_t versionNumber;
    uint8_t isCalibrated;
    uint8_t startChannel;
    uint8_t dummy;

    Receiver::DiversityMode diversityMode;
    StateMachine::State lastKnownState;

    uint16_t rssiAMin;
    uint16_t rssiAMax;
    uint16_t rssiBMin;
    uint16_t rssiBMax;

    // Internal settings
    uint16_t rssiSeekTreshold;
    uint16_t rssiMinTuneTime;
    uint16_t rssiHysteresis;
    uint16_t rssiHysteresisPeriod;

    // Chorus32 settings
    uint8_t consecutives;

    void setup();
    void update();

    void load();
    void save();
    void markDirty();

    void initDefaults();
};


extern struct EepromSettings DMA_ATTR EepromSettings;


#endif
