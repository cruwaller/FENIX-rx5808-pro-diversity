#include <Arduino.h>

#include "settings.h"
#include "settings_eeprom.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "channels.h"
#include "state.h"
#include "ui.h"

#include "timer.h"

namespace Receiver {
    ReceiverId activeReceiver = ReceiverId::A;
    uint8_t activeChannel = EepromSettings.startChannel;

    uint16_t  rssiA = 0;
    uint32_t rssiARaw = 0;
    uint16_t  rssiALast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint16_t  rssiB = 0;
    uint32_t rssiBRaw = 0;
    uint16_t  rssiBLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
    uint16_t  rssiC = 0;
    uint32_t rssiCRaw = 0;
    uint16_t  rssiCLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint16_t  rssiD = 0;
    uint32_t rssiDRaw = 0;
    uint16_t  rssiDLast[RECEIVER_LAST_DATA_SIZE] = { 0 };
#endif
    uint16_t rssiBandScanData[CHANNELS_SIZE] = { 0 };

    uint16_t previousSwitchTime = 0;
    uint16_t antennaAOnTime = 0;
    uint16_t antennaBOnTime = 0;
    uint16_t antennaCOnTime = 0;
    uint16_t antennaDOnTime = 0;

    ReceiverId diversityTargetReceiver = activeReceiver;
    static Timer diversityHysteresisTimer = Timer(5); // default value and is replce by value stored in eeprom during setup

    static Timer rssiStableTimer = Timer(30); // default value and is replce by value stored in eeprom during setup
    static Timer rssiLogTimer = Timer(RECEIVER_LAST_DELAY);

    bool hasRssiUpdated = false;

    void setChannel(uint8_t channel)
    {
        ReceiverSpi::setSynthRegisterB(Channels::getSynthRegisterB(channel));

        rssiStableTimer.reset();
        activeChannel = channel;

        hasRssiUpdated = false;
    }

    void setChannelByFreq(uint16_t freq)
    {
        ReceiverSpi::setSynthRegisterB(Channels::getSynthRegisterBFreq(freq));

        rssiStableTimer.reset();
//        activeChannel = channel;

        hasRssiUpdated = false;
    }

    void setActiveReceiver(ReceiverId receiver) {

        switch (EepromSettings.diversityMode) {
            case Receiver::DiversityMode::ANTENNA_A:
                receiver = ReceiverId::A;
                digitalWrite(PIN_RX_SWITCH, LOW);
                break;

            case Receiver::DiversityMode::ANTENNA_B:
                receiver = ReceiverId::B;
                digitalWrite(PIN_RX_SWITCH, HIGH);
                break;

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            case Receiver::DiversityMode::ANTENNA_C:
                receiver = ReceiverId::C;
                break;

            case Receiver::DiversityMode::ANTENNA_D:
                receiver = ReceiverId::D;
                break;
#endif
            case Receiver::DiversityMode::DIVERSITY:
                if (receiver == ReceiverId::A) {
                    digitalWrite(PIN_RX_SWITCH, LOW);
                } else if (receiver == ReceiverId::B) {
                    digitalWrite(PIN_RX_SWITCH, HIGH);
                }
                break;

            case Receiver::DiversityMode::QUADVERSITY:
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
                digitalWrite(PIN_LED_A, receiver == ReceiverId::A);
                digitalWrite(PIN_LED_B, receiver == ReceiverId::B);
                digitalWrite(PIN_LED_C, receiver == ReceiverId::C);
                digitalWrite(PIN_LED_D, receiver == ReceiverId::D);
                break;
#endif
            default:
                break;
        }

        activeReceiver = receiver;
    }

    bool isRssiStable() {
        return rssiStableTimer.hasTicked();
    }

    void updateRssi() {

        uint8_t const RSSI_READS = 4; // 3; //15;

        rssiARaw = rssiBRaw = 0;
        for (uint8_t i = 0; i < RSSI_READS; i++) {
            rssiARaw += analogRead(PIN_RSSI_A);
            rssiBRaw += analogRead(PIN_RSSI_B);
        }
        rssiARaw /= RSSI_READS;
        rssiBRaw /= RSSI_READS;

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
        if (EepromSettings.quadversity) {
            rssiCRaw = rssiDRaw = 0;
            for (uint8_t i = 0; i < RSSI_READS; i++) {
                rssiCRaw += analogRead(PIN_RSSI_C);
                rssiDRaw += analogRead(PIN_RSSI_D);
            }
            rssiCRaw /= RSSI_READS;
            rssiDRaw /= RSSI_READS;
        }
#endif

        if (StateMachine::currentState != StateMachine::State::SETTINGS_RSSI) {

            rssiA = constrain(
                map(
                    rssiARaw,
                    EepromSettings.rssiAMin,
                    EepromSettings.rssiAMax,
                    0,
                    1000
                ),
                0,
                1000
            );

            rssiB = constrain(
                map(
                    rssiBRaw,
                    EepromSettings.rssiBMin,
                    EepromSettings.rssiBMax,
                    0,
                    1000
                ),
                0,
                1000
            );

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            rssiC = constrain(
                map(
                    rssiCRaw,
                    EepromSettings.rssiCMin,
                    EepromSettings.rssiCMax,
                    0,
                    1000
                ),
                0,
                1000
            );
            rssiD = constrain(
                map(
                    rssiDRaw,
                    EepromSettings.rssiDMin,
                    EepromSettings.rssiDMax,
                    0,
                    1000
                ),
                0,
                1000
            );
#endif
        }

        if (rssiLogTimer.hasTicked()) {
            for (uint8_t i = 0; i < RECEIVER_LAST_DATA_SIZE - 1; i++) {
                rssiALast[i] = rssiALast[i + 1];
                rssiBLast[i] = rssiBLast[i + 1];
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
                if (EepromSettings.quadversity) {
                    rssiCLast[i] = rssiCLast[i + 1];
                    rssiDLast[i] = rssiDLast[i + 1];
                }
#endif
            }

            rssiALast[RECEIVER_LAST_DATA_SIZE - 1] = rssiA;
            rssiBLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiB;
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            if (EepromSettings.quadversity) {
                rssiCLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiC;
                rssiDLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiD;
            }
#endif
            rssiLogTimer.reset();
            hasRssiUpdated = true;
        }

    }

    void switchDiversity() {
        int32_t rssiDiff = 0;
        ReceiverId nextReceiver = activeReceiver;
        ReceiverId currentBestReceiver = activeReceiver;

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
        if (EepromSettings.quadversity) {
            int32_t rssiMax = max(max(rssiA, rssiB), max(rssiC, rssiD));

            // Find which Rx has the highest RSSI.
            if (rssiA == rssiMax) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiB == rssiMax) {
                currentBestReceiver = ReceiverId::B;
            } else if (rssiC == rssiMax) {
                currentBestReceiver = ReceiverId::C;
            } else if (rssiD == rssiMax) {
                currentBestReceiver = ReceiverId::D;
            }

            // Difference against currently active Rx.
            if (ReceiverId::A == activeReceiver) {
                rssiDiff = rssiMax - rssiA;
            } else if (ReceiverId::B == activeReceiver) {
                rssiDiff = rssiMax - rssiB;
            } else if (ReceiverId::C == activeReceiver) {
                rssiDiff = rssiMax - rssiC;
            } else if (ReceiverId::D == activeReceiver) {
                rssiDiff = rssiMax - rssiD;
            }

            if (abs(rssiDiff) >= EepromSettings.rssiHysteresis) {
                if (currentBestReceiver == diversityTargetReceiver) {
                    if (diversityHysteresisTimer.hasTicked()) {
                        nextReceiver = diversityTargetReceiver;
                    }
                } else {
                    diversityTargetReceiver = currentBestReceiver;
                    diversityHysteresisTimer.reset();
                }
            } else {
                diversityHysteresisTimer.reset();
            }
        }
        else
#endif // PIN_RSSI_C && PIN_RSSI_D
        {
            rssiDiff = (int32_t)(rssiA - rssiB);

            if (rssiDiff > 0) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiDiff < 0) {
                currentBestReceiver = ReceiverId::B;
            } else {
                currentBestReceiver = activeReceiver;
            }

            if (abs(rssiDiff) >= EepromSettings.rssiHysteresis) {
                if (currentBestReceiver == diversityTargetReceiver) {
                    if (diversityHysteresisTimer.hasTicked()) {
                        nextReceiver = diversityTargetReceiver;
                    }
                } else {
                    diversityTargetReceiver = currentBestReceiver;
                    diversityHysteresisTimer.reset();
                }
            } else {
                diversityHysteresisTimer.reset();
            }
        }

        setActiveReceiver(nextReceiver);
    }

    void updateAntenaOnTime() {
        uint32_t ms = millis() / 1000;
        if (ReceiverId::A == activeReceiver) {
            antennaAOnTime += ms - previousSwitchTime;
        }
        else if (ReceiverId::B == activeReceiver) {
            antennaBOnTime += ms - previousSwitchTime;
        }
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
        if (EepromSettings.quadversity) {
            if (ReceiverId::C == activeReceiver) {
                antennaCOnTime += ms - previousSwitchTime;
            }
            else if (ReceiverId::D == activeReceiver) {
                antennaDOnTime += ms - previousSwitchTime;
            }
        }
#endif
        previousSwitchTime = ms;
    }

    void setup() {
#ifdef DISABLE_AUDIO
        ReceiverSpi::setPowerDownRegister(0b00010000110111110011);
#endif
        setChannel(EepromSettings.startChannel);
        setActiveReceiver(ReceiverId::A);
        diversityHysteresisTimer = Timer(EepromSettings.rssiHysteresisPeriod);
        rssiStableTimer = Timer(EepromSettings.rssiMinTuneTime);
    }

    void update() {

        if (rssiStableTimer.hasTicked()) {

            updateAntenaOnTime();

            updateRssi();

            switchDiversity();
        }

    }
}
