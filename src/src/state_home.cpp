#include <Arduino.h>
#include <WString.h>

#include "settings_eeprom.h"
#include "settings.h"

#include "state_home.h"

#include "receiver.h"
#include "receiver_spi.h"
#include "channels.h"
#include "state.h"
#include "ui.h"
#include "temperature.h"
#include "voltage.h"
#include "touchpad.h"
#include "protocol_ExpressLRS.h"
#include "protocol_chorus.h"
#include "lap_times.h"


// For scalling graphics accross screen
#if (CHANNELS_SIZE == 72)
    #define CHANNELS_SIZE_DIVIDER 4
#elif (CHANNELS_SIZE == 48)
    #define CHANNELS_SIZE_DIVIDER 6
#endif

static uint16_t DMA_ATTR rssiBandScanData[CHANNELS_SIZE];

using StateMachine::HomeStateHandler;


void HomeStateHandler::onEnter()
{
    displayActiveChannel = Receiver::activeChannel;
    //this->onUpdateDraw(false);
}


void HomeStateHandler::onUpdate()
{
    this->onUpdateDraw(TouchPad::touchData.buttonPrimary);
}


void HomeStateHandler::onUpdateDraw(uint8_t tapAction)
{
    uint32_t sec_now;
    uint32_t x_off, y_off;
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t iter;

    if (isInBandScanRegion()) {
        bandScanUpdate();
        wasInBandScanRegion = true;
    } else {
        if (wasInBandScanRegion) {
            Receiver::setChannel(displayActiveChannel);
        }
        displayActiveChannel = Receiver::activeChannel;
        wasInBandScanRegion = false;
    }

    if (drawHeader())
        return;

    /*************************************************/
    /*********      PRINT HOME      ******************/

    // Handle channel change before drawing
    if (tapAction) {
        if (cursor_x >= 0 && cursor_x < 61 &&
            cursor_y > 8  && cursor_y < 54) {
            this->setChannel(8); // Up band
        }
        else if (cursor_x >= 0 && cursor_x < 61 &&
                 cursor_y > 54 && cursor_y < 99) {
            this->setChannel(-8); // Down band
        }
        else if (cursor_x > 61 && cursor_x < 122 &&
                 cursor_y > 8  && cursor_y < 54) {
            this->setChannel(1); // Up channel
        }
        else if (cursor_x > 61 && cursor_x < 122 &&
                 cursor_y > 54 && cursor_y < 99) {
            this->setChannel(-1); // Down channel
        }
    }

    // Display Band and Channel
    Ui::display.setCursor( 2, 15);
    Ui::display.printLarge(Channels::getName(displayActiveChannel), 8, 12);

    // Display Frequency
    Ui::display.setCursor( 0, 105);
    Ui::display.printLarge(Channels::getFrequency(displayActiveChannel), 4, 3);

    // Channel labels
    x_off = 130;
    Ui::display.setCursor( x_off, 12 + 28*0 + 3);
    Ui::display.printLarge("A", 2, 2);
    Ui::display.setCursor( x_off, 12 + 28*2 + 3);
    Ui::display.printLarge("B", 2, 2);

    // Channel selected square
    x_off -= 2;
    //y_off = 10 + 3 + 28 * (Receiver::activeReceiver == Receiver::ReceiverId::B) ? 2 : 0;
    //Ui::display.rect(x_off, y_off, 19, 19, WHITE);
    if (Receiver::activeReceiver == Receiver::ReceiverId::A) {
        Ui::display.rect(128, (10 + 28*0 + 3), 19, 19, 100);
    } else if (Receiver::activeReceiver == Receiver::ReceiverId::B) {
        Ui::display.rect(128, (10 + 28*2 + 3), 18, 18, 100);
    }

    // On percentage
    sec_now = millis() / 1000;
    Ui::display.setCursor(x_off, 30 + 28*0 + 3);
    Ui::display.print( (100.0 * Receiver::antennaAOnTime) / (sec_now) );
    Ui::display.print("%");
    Ui::display.setCursor(x_off, 32 + 28*2 + 3);
    Ui::display.print( (100.0 * Receiver::antennaBOnTime) / (sec_now) );
    Ui::display.print("%");

    //x_off += 2 + 3 + 4 * Ui::CHAR_W + 4;

    // Send frequency to all peers
#define SEND_Y_OFF 43
#define SEND_X_OFF (130 + 3 * Ui::CHAR_W + 4)
    y_off = SEND_Y_OFF;
    x_off = SEND_X_OFF;
    Ui::display.setCursor( x_off, y_off + Ui::CHAR_H * 0); // 130 - 11 - 4*8 / 2
    Ui::display.print("S");
    Ui::display.setCursor( x_off, y_off + Ui::CHAR_H * 1);
    Ui::display.print("E");
    Ui::display.setCursor( x_off, y_off + Ui::CHAR_H * 2);
    Ui::display.print("N");
    Ui::display.setCursor( x_off, y_off + Ui::CHAR_H * 3);
    Ui::display.print("D");
    // Draw selection box over SEND
    if (cursor_y > (y_off - 4) && cursor_y < (y_off + 4 * Ui::CHAR_H + 4 + 3) &&
        cursor_x > (x_off - 4) && cursor_x < (x_off + Ui::CHAR_W + 3))
    {
        Ui::display.rect(x_off - 4, y_off - 4,
                         (4 + 3 + Ui::CHAR_W), (3 + 4 + 4 * Ui::CHAR_H), WHITE);
        if (tapAction) {
            expresslrs_vtx_freq_send(Channels::getFrequency(Receiver::activeChannel));
        }
    }


#if HOME_SHOW_LAPTIMES
#define LAPTIMES_X_POS 188 // max is 200
    //x_off = Ui::XRES - 16 * Ui::CHAR_W;
    y_off = 12;
    char tmp_buff[16]; // '11) 00:00.000\n' => 13 chars
    uint8_t fastest = 0, num_laps = lapt_time_race_num_laps();

    Ui::display.setCursor(LAPTIMES_X_POS, y_off);
    Ui::display.print("LAP TIMES "); // "LAP TIMES [  1]", 15chars
    snprintf(tmp_buff, sizeof(tmp_buff), "[%3u]\n", lapt_time_race_idx_get());
    Ui::display.print(tmp_buff, chorus_race_is_start()); // inverted if race is started

    // Draw selection box
    if (cursor_y > (y_off - 4) && cursor_y < (y_off + Ui::CHAR_H + 3) &&
        cursor_x > (LAPTIMES_X_POS - 4) && cursor_x < (LAPTIMES_X_POS + 15 * Ui::CHAR_W + 3))
    {
        Ui::display.rect(LAPTIMES_X_POS-4, 12-4, 4+3+15*8, 15, WHITE);
        if (tapAction) {
            StateMachine::switchState(StateMachine::State::CHORUS);
            return; // No need to draw reset
        }
    }

    y_off += 9;
    lap_time_t lap_time;
    for (iter = 1; iter <= num_laps; iter++, y_off += 9) {
        lap_time = lapt_time_laptime_get(iter, fastest); // time in ms
        if (*((uint32_t*)&lap_time) == 0)
            break;

        Ui::display.setCursor(LAPTIMES_X_POS, y_off);

        snprintf(tmp_buff, sizeof(tmp_buff), "%2u) %02u:%02u.%03u\n",
                 iter, lap_time.m, lap_time.s, lap_time.ms);
        Ui::display.print(tmp_buff, fastest); // invert colors if fastest
    }

#else // !HOME_SHOW_LAPTIMES
    // Draw RSSI Plots
    Ui::display.rect(195, 12 + 28*0 + 3, 64*2, 28*2-1, WHITE);
    Ui::display.rect(195, 12 + 28*2 + 3, 64*2, 28*2-1, WHITE);
    for (uint8_t i=0; i < RECEIVER_LAST_DATA_SIZE-1; i++) {
        Ui::display.line(195+1*i, (12 + 28*2)-Receiver::rssiALast[i]/20, 195+1*(i+1), (12 + 28*2)-Receiver::rssiALast[i+1]/20, WHITE);
        Ui::display.line(195+1*i, (12 + 28*4)-Receiver::rssiBLast[i]/20, 195+1*(i+1), (12 + 28*4)-Receiver::rssiBLast[i+1]/20, WHITE);
    }
#endif // HOME_SHOW_LAPTIMES

    // ================== Plot RSSI Spectrum ==================
    constexpr uint16_t y_min = Ui::YRES - 10;
    constexpr uint16_t x_min = 18;
    uint32_t rssi, rssi_h;

    // Frame
    Ui::display.line(0, (Ui::YRES - 11), (Ui::XRES - 1), (Ui::YRES - 11), WHITE);
    // min freq
    Ui::display.setCursor( 1, (Ui::YRES - 9)); // Y=215
    Ui::display.print(Channels::getFrequency(Channels::getOrderedIndex(0)));
    // max freq
    Ui::display.setCursor( 290, (Ui::YRES - 9)); // Y=215
    Ui::display.print(Channels::getFrequency(Channels::getOrderedIndex(CHANNELS_SIZE-1)));
    // Freq based on cursor position
    if (HomeStateHandler::isInBandScanRegion()) {
        // Marker
        uint16_t markerX = Channels::getOrderedIndexFromIndex(Receiver::activeChannel);
        markerX *= CHANNELS_SIZE_DIVIDER;
        markerX += x_min;
        Ui::display.fillRect((markerX + 1), (y_min + 1), 4, 4, WHITE);

        // RSSI bars
        for (iter = 0; iter < CHANNELS_SIZE; iter++) {
            rssi = rssiBandScanData[iter];
            rssi_h = constrain((rssi * 8) / 100, 0, y_min);
            Ui::display.fillRect(x_min + (CHANNELS_SIZE_DIVIDER * iter),
                                y_min - rssi_h,
                                CHANNELS_SIZE_DIVIDER,
                                rssi_h,
                                rssi / 10);
        }

        if ((cursor_x > x_min) && (cursor_x < (Ui::XRES - x_min))) {
            // reuse markerX for channel index
            markerX = Channels::getOrderedIndex((cursor_x - x_min) / CHANNELS_SIZE_DIVIDER);
            Ui::display.fillRect( cursor_x - 33, cursor_y - 17, 33, 17, 10);
            Ui::display.setCursor( (cursor_x - (4 * Ui::CHAR_W)), (cursor_y - (2 * Ui::CHAR_H)) );
            Ui::display.print(Channels::getName(markerX));
            Ui::display.setCursor( (cursor_x - (4 * Ui::CHAR_W)), (cursor_y - Ui::CHAR_H) );
            Ui::display.print(Channels::getFrequency(markerX));

            if (tapAction) {
                setChannel(0, markerX);
            }
        }
    }
}


void HomeStateHandler::setChannel(int channelIncrement, int setChannel)
{
    uint8_t activeChannel = (setChannel < 0) ? Receiver::activeChannel : setChannel;

    int band = activeChannel / 8;
    int channel = activeChannel % 8;

    if (channelIncrement == 8) {
      band = band + 1;
    } else if (channelIncrement == -8) {
      band = band - 1;
    } else if (channelIncrement == 1 ) {
      channel = channel + 1;
      if (channel > 7) {
        channel = 0;
      }
    } else if (channelIncrement == -1 ) {
      channel = channel - 1;
      if (channel < 0) {
        channel = 7;
      }
    }

    int newChannelIndex = band * 8 + channel;

    // Check wraparounds
    if (newChannelIndex >= CHANNELS_SIZE) {
      newChannelIndex = newChannelIndex - CHANNELS_SIZE;
    } else if (newChannelIndex < 0) {
      newChannelIndex = newChannelIndex + CHANNELS_SIZE;
    }

    Receiver::setChannel(newChannelIndex);
    EepromSettings.startChannel = newChannelIndex;
    EepromSettings.markDirty();
    centred = false;

    displayActiveChannel = Receiver::activeChannel;
}

// Frequency 'Centring' function.
// The function walks up and then down from the currently Rx frequency
// in 1 MHz steps until RSSI < threshold.  The Rx is then set to the
// centre of these 2 frequencies.
void HomeStateHandler::centreFrequency()
{
    uint16_t activeChannelFreq = Channels::getFrequency(Receiver::activeChannel);
    uint16_t centerFreq = Channels::getCenterFreq(activeChannelFreq);
    Receiver::setChannel(Channels::getClosestChannel(centerFreq));

    wasInBandScanRegion = false;
}

bool HomeStateHandler::isInBandScanRegion()
{
    if (TouchPad::touchData.cursorY > 130 ) {
        return true;
    } else {
        return false;
    }
}

void HomeStateHandler::bandScanUpdate()
{
    Ui::UiTimeOut.reset();

    if (!wasInBandScanRegion) {
        orderedChanelIndex = Channels::getOrderedIndexFromIndex(displayActiveChannel); // Start from currently selected channel to prevent initial spike artifact.
    }

    if (Receiver::isRssiStableAndUpdated()) {

        rssiBandScanData[orderedChanelIndex] = max(Receiver::rssiA, Receiver::rssiB);

        orderedChanelIndex = orderedChanelIndex + 1;
        if (orderedChanelIndex == CHANNELS_SIZE) {
            orderedChanelIndex = 0;
        }
        Receiver::setChannel(Channels::getOrderedIndex(orderedChanelIndex));
    }
}
