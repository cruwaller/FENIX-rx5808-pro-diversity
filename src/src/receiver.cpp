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
    ReceiverId DMA_ATTR activeReceiver = ReceiverId::ALL; // Init to ALL to make sure the initial value is set correctly!
    uint8_t DMA_ATTR activeChannel;

    uint16_t DMA_ATTR rssiA = 0;
    uint32_t DMA_ATTR rssiARaw = 0;
    uint16_t DMA_ATTR rssiALast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint16_t DMA_ATTR rssiB = 0;
    uint32_t DMA_ATTR rssiBRaw = 0;
    uint16_t DMA_ATTR rssiBLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint32_t DMA_ATTR antennaAOnTime = 0;
    uint32_t DMA_ATTR antennaBOnTime = 0;

    static uint32_t DMA_ATTR previousSwitchTime = 0;

    static ReceiverId DMA_ATTR diversityTargetReceiver = activeReceiver;
    static Timer DMA_ATTR diversityHysteresisTimer = Timer(5); // default value and is replce by value stored in eeprom during setup

    static Timer DMA_ATTR rssiStableTimer = Timer(30); // default value and is replce by value stored in eeprom during setup
    static Timer DMA_ATTR rssiLogTimer = Timer(RECEIVER_LAST_DELAY);

    static bool DMA_ATTR hasRssiUpdated = false;

    void IRAM_ATTR setChannel(uint8_t channel, ReceiverId rcvr_id)
    {
        ReceiverSpi::setSynthRegisterB(Channels::getFrequency(channel), rcvr_id);

        rssiStableTimer.reset();
        activeChannel = channel;
        hasRssiUpdated = false;
    }

    void IRAM_ATTR setChannelByFreq(uint16_t freq)
    {
        setChannel(Channels::getClosestChannel(freq));
    }

    static void IRAM_ATTR setActiveReceiverSwitch(ReceiverId receiver)
    {
        if (receiver == activeReceiver)
            return;

        switch (receiver) {
            case ReceiverId::A:
                digitalWrite(PIN_RX_SWITCH, LOW);
                break;

            case ReceiverId::B:
                digitalWrite(PIN_RX_SWITCH, HIGH);
                break;

            default:
                return;
        }

        activeReceiver = receiver;
    }

    void IRAM_ATTR setDiversityMode(DiversityMode mode)
    {
        EepromSettings.diversityMode = mode;
        ReceiverId receiver = activeReceiver;

#if POWER_OFF_RX
        ReceiverSpi::rxStandby(Receiver::ReceiverId::ALL);
#endif

        switch (mode) {
            case Receiver::DiversityMode::ANTENNA_A:
                receiver = ReceiverId::A;
                break;

            case Receiver::DiversityMode::ANTENNA_B:
                receiver = ReceiverId::B;
                break;
            case Receiver::DiversityMode::DIVERSITY:
            default:
                receiver = ReceiverId::ALL;
                break;
        }

#if POWER_OFF_RX
        ReceiverSpi::rxWakeup(receiver);
#endif // POWER_OFF_RX

        setChannel(activeChannel, receiver);

        if (receiver != ReceiverId::ALL)
            setActiveReceiverSwitch(receiver);
    }

    bool IRAM_ATTR isRssiStable()
    {
        return rssiStableTimer.hasTicked();
    }

    bool IRAM_ATTR isRssiStableAndUpdated()
    {
        return isRssiStable() && hasRssiUpdated;
    }

    void IRAM_ATTR updateRssi()
    {
        uint32_t _rssiARaw = 0, _rssiBRaw = 0;
        uint8_t iter;

#define RSSI_READS 4 // 3;

        for (iter = 0; iter < RSSI_READS; iter++) {
            _rssiARaw += analogRead(PIN_RSSI_A);
            _rssiBRaw += analogRead(PIN_RSSI_B);
        }

        _rssiARaw /= RSSI_READS;
        _rssiBRaw /= RSSI_READS;

        rssiARaw = _rssiARaw;
        rssiBRaw = _rssiBRaw;

        if (StateMachine::currentState != StateMachine::State::SETTINGS_RSSI) {

            rssiA = constrain(
                map(
                    _rssiARaw,
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
                    _rssiBRaw,
                    EepromSettings.rssiBMin,
                    EepromSettings.rssiBMax,
                    0,
                    1000
                ),
                0,
                1000
            );
        }

        if (rssiLogTimer.hasTicked()) {
#if !HOME_SHOW_LAPTIMES
            for (uint8_t i = 0; i < RECEIVER_LAST_DATA_SIZE - 1; i++) {
                rssiALast[i] = rssiALast[i + 1];
                rssiBLast[i] = rssiBLast[i + 1];
            }
            rssiALast[RECEIVER_LAST_DATA_SIZE - 1] = rssiA;
            rssiBLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiB;
#endif // HOME_SHOW_LAPTIMES

            rssiLogTimer.reset();
            hasRssiUpdated = true;
        }
    }

    static void IRAM_ATTR switchDiversity()
    {
        ReceiverId nextReceiver = activeReceiver;
        ReceiverId currentBestReceiver = activeReceiver;
        int32_t rssiDiff = (int32_t)(rssiA - rssiB);

        if (abs(rssiDiff) >= EepromSettings.rssiHysteresis) {
            if (rssiDiff > 0) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiDiff < 0) {
                currentBestReceiver = ReceiverId::B;
            }

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

        setActiveReceiverSwitch(nextReceiver);
    }

    static void IRAM_ATTR updateAntenaOnTime()
    {
        uint32_t _sec_now = (millis() / 1000);
        uint32_t secs = _sec_now - previousSwitchTime;
        switch (activeReceiver) {
            case ReceiverId::A:
                antennaAOnTime += secs;
                break;
            case ReceiverId::B:
                antennaBOnTime += secs;
                break;
            default:
                break;
        }
        previousSwitchTime = _sec_now;
    }

    void setup()
    {
        diversityHysteresisTimer = Timer(EepromSettings.rssiHysteresisPeriod);
        rssiStableTimer = Timer(EepromSettings.rssiMinTuneTime);
        activeChannel = EepromSettings.startChannel;
        setDiversityMode(EepromSettings.diversityMode);
    }

    void IRAM_ATTR update()
    {
        if (isRssiStable()) {

            updateAntenaOnTime();

            updateRssi();

            /* Switch only if mode is selected */
            if (EepromSettings.diversityMode == Receiver::DiversityMode::DIVERSITY)
                switchDiversity();
        }

    }
}
