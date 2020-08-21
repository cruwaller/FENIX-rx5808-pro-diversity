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
    uint8_t otaUpdateRequested;

    Receiver::DiversityMode diversityMode;

    uint8_t startChannel;
    int lastKnownMenuItem; // TODO: Useless, remove this later
    StateMachine::State lastKnownState;

    uint8_t beepEnabled;

    uint16_t rssiAMin;
    uint16_t rssiAMax;
    uint16_t rssiBMin;
    uint16_t rssiBMax;
    uint16_t rssiCMin;
    uint16_t rssiCMax;
    uint16_t rssiDMin;
    uint16_t rssiDMax;

    uint8_t vbatScale;
    uint8_t vbatWarning;
    uint8_t vbatCritical;

    uint8_t favouriteChannels[8];
    int spectatorChannels[8];

    uint8_t quadversity;
    uint8_t buttonBeep;

    // Internal settings
    uint8_t spectatorFreqScanStep;
    uint8_t spectatorFWHM;
    uint16_t rssiSeekTreshold;
    uint16_t rssiMinTuneTime;
    uint8_t rssiHysteresis;
    uint16_t rssiHysteresisPeriod;

    void setup();
    void update();

    void load();
    void save();
    void markDirty();

    void initDefaults();
};


extern struct EepromSettings DMA_ATTR EepromSettings;


#endif
