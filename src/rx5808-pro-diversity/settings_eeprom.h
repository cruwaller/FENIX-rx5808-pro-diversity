#ifndef SETTINGS_EEPROM_H
#define SETTINGS_EEPROM_H


#include <stdint.h>

#include "settings.h"
#include "settings_internal.h"
#include "receiver.h"
#include "state.h"


struct EepromSettings {
    uint8_t versionNumber;
    
    uint8_t isCalibrated;
    
    uint8_t startChannel;
    int lastKnownMenuItem;
    StateMachine::State lastKnownState;

    uint8_t beepEnabled;

    uint8_t searchManual;
    uint8_t searchOrderByChannel;
    
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
    uint8_t saveScreenOn;
    uint8_t useOledScreen;
    uint8_t useFastBoot;
    uint8_t selectedHomePage;
    uint8_t buttonBeep;
    uint8_t invertDisplay;
    uint8_t rotateOled;

    // Internal settings
    uint8_t spectatorFreqScanStep;
    uint8_t spectatorFWHM;
    uint8_t rssiSeekTreshold;
    uint16_t rssiMinTuneTime;
    uint8_t rssiHysteresis;
    uint16_t rssiHysteresisPeriod;

//    #ifndef EEPROM_AT24C02
//       unsigned char customLogo[128*64/8];
//    #endif
    
    void update();

    void load();
    void save();
    void markDirty();

    void initDefaults();
};


const struct {
    uint8_t versionNumber = VERSION_NUMBER;
    
    uint8_t isCalibrated = false;
    
    uint8_t startChannel = 0;
    int lastKnownMenuItem = 0;
    StateMachine::State lastKnownState = StateMachine::State::HOME;

    uint8_t beepEnabled = true;

    uint8_t searchManual = false;
    uint8_t searchOrderByChannel = false;
    
    uint16_t rssiAMin = RSSI_MIN_VAL;
    uint16_t rssiAMax = RSSI_MAX_VAL;
    uint16_t rssiBMin = RSSI_MIN_VAL;
    uint16_t rssiBMax = RSSI_MAX_VAL;
    uint16_t rssiCMin = RSSI_MIN_VAL;
    uint16_t rssiCMax = RSSI_MAX_VAL;
    uint16_t rssiDMin = RSSI_MIN_VAL;
    uint16_t rssiDMax = RSSI_MAX_VAL;

    uint8_t vbatScale = VBAT_SCALE;
    uint8_t vbatWarning = WARNING_VOLTAGE;
    uint8_t vbatCritical = CRITICAL_VOLTAGE;
    
    uint8_t favouriteChannels[8] = {32, 33, 34, 35, 36, 37, 38, 39}; // Race band
    int spectatorChannels[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    uint8_t quadversity = false;
    uint8_t saveScreenOn = 1;
    uint8_t useOledScreen = false;
    uint8_t useFastBoot = true;
    uint8_t selectedHomePage = 0;
    uint8_t buttonBeep = true;
    uint8_t invertDisplay = false;
    uint8_t rotateOled = false;

    // Internal settings
    uint8_t spectatorFreqScanStep = 5;
    uint8_t spectatorFWHM = 20;
    uint8_t rssiSeekTreshold = 50;
    uint16_t rssiMinTuneTime = 30;
    uint8_t rssiHysteresis = 2;
    uint16_t rssiHysteresisPeriod = 5;

//    #ifndef EEPROM_AT24C02
//        unsigned char customLogo[128*64/8] = {0};
//    #endif
} EepromDefaults;


extern EepromSettings EepromSettings;


#endif
