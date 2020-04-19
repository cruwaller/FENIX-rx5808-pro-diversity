#include <stdint.h>
#include "settings_eeprom.h"
#include "state_expresslrs.h"
#include "ui.h"
#include "temperature.h"
#include "touchpad.h"
#include "ExpressLRS_Protocol.h"

extern void sendToExLRS(uint16_t function, uint16_t payloadSize, const uint8_t *payload);

void StateMachine::ExLRSStateHandler::onEnter()
{
}

void StateMachine::ExLRSStateHandler::onUpdate()
{
}

void StateMachine::ExLRSStateHandler::onInitialDraw()
{
    onUpdateDraw();
}

void StateMachine::ExLRSStateHandler::onUpdateDraw()
{

    if (TouchPad::touchData.buttonPrimary)
    {
        TouchPad::touchData.buttonPrimary = false;
        this->doTapAction();
    }

    Ui::display.setCursor(205, 0);

    // Temperature // Doesnt currently work within ESP32 Arduino.
    Ui::display.print(Temperature::getTemperature());
    Ui::display.print("C");

    Ui::display.print(" / ");

    // On Time
    uint8_t hours = millis() / 1000 / 60 / 60;
    uint8_t mins = millis() / 1000 / 60 - hours * 60 * 60;
    uint8_t secs = millis() / 1000 - hours * 60 * 60 - mins * 60;
    Ui::display.print(hours);
    Ui::display.print(":");
    if (mins < 10)
    {
        Ui::display.print("0");
    }
    Ui::display.print(mins);
    Ui::display.print(":");
    if (secs < 10)
    {
        Ui::display.print("0");
    }
    Ui::display.print(secs);

    // Menu Icon
    Ui::display.line(315, 1, 322, 1, 100);
    Ui::display.line(315, 4, 322, 4, 100);
    Ui::display.line(315, 7, 322, 7, 100);

    // Horixontal line
    Ui::display.line(0, 9, SCREEN_WIDTH, 9, 100);

    // Mode
    Ui::display.setCursor(20, 20);
    //                 123456789012345678901234567890123
    Ui::display.print("Mode (Hz):       50    100    200");

    // RF Power
    Ui::display.setCursor(20, 40);
    Ui::display.print("RF Power (mW):   50    200    1000");

    // TLM Rate
    Ui::display.setCursor(20, 60);
    Ui::display.print("TLM:             On    Off");

    // Draw Mode box
    if (
        TouchPad::touchData.cursorY > 16 && TouchPad::touchData.cursorY < 31)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 16, 23, 15, 100);
        }
        if ( // 100
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 16, 31, 15, 100);
        }
        if ( // 200
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 33 * 8 + 3))
        {
            Ui::display.rect(20 + 30 * 8 - 4, 16, 31, 15, 100);
        }
    }
    // Draw RF Power box
    if (
        TouchPad::touchData.cursorY > 36 && TouchPad::touchData.cursorY < 51)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 36, 23, 15, 100);
        }
        if ( // 200
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 27 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 36, 31, 15, 100);
        }
        if ( // 1000
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 34 * 8 + 3))
        {
            Ui::display.rect(20 + 30 * 8 - 4, 36, 39, 15, 100);
        }
    }
    // Draw TLM box
    if (
        TouchPad::touchData.cursorY > 56 && TouchPad::touchData.cursorY < 71)
    {
        if ( // On
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 56, 23, 15, 100);
        }
        if ( // Off
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 56, 31, 15, 100);
        }
    }
}

void StateMachine::ExLRSStateHandler::doTapAction()
{
    if ( // Menu
        TouchPad::touchData.cursorX > 314 && TouchPad::touchData.cursorY < 8)
    {
        EepromSettings.save();
        StateMachine::switchState(StateMachine::State::MENU);
    }

    if ( // mode
        TouchPad::touchData.cursorY > 16 && TouchPad::touchData.cursorY < 31)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            uint8_t payload[2] = {0};
            payload[0] = ExLRS_MODE;
            payload[1] = ExLRS_50Hz;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
        if ( // 100
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            uint8_t payload[2] = {0};
            payload[0] = ExLRS_MODE;
            payload[1] = ExLRS_100Hz;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
        if ( // 200
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 33 * 8 + 3))
        {
            uint8_t payload[2] = {0};
            payload[0] = ExLRS_MODE;
            payload[1] = ExLRS_200Hz;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
    }
    
    if ( // power
        TouchPad::touchData.cursorY > 36 && TouchPad::touchData.cursorY < 51)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            uint8_t payload[3] = {0};
            payload[0] = ExLRS_TX_POWER;
            payload[1] = 50 & 0xff;
            payload[2] = (50 >> 8) & 0xff;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
        if ( // 200
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 27 * 8 + 3))
        {
            uint8_t payload[3] = {0};
            payload[0] = ExLRS_TX_POWER;
            payload[1] = 200 & 0xff;
            payload[2] = (200 >> 8) & 0xff;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
        if ( // 1000
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 34 * 8 + 3))
        {
            uint8_t payload[3] = {0};
            payload[0] = ExLRS_TX_POWER;
            payload[1] = 1000 & 0xff;
            payload[2] = (1000 >> 8) & 0xff;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
    }
    
    if ( // TLM
        TouchPad::touchData.cursorY > 56 && TouchPad::touchData.cursorY < 71)
    {
        if ( // On
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            uint8_t payload[2] = {0};
            payload[0] = ExLRS_TLM;
            payload[1] = ExLRS_TLM_ON;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
        if ( // Off
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            uint8_t payload[2] = {0};
            payload[0] = ExLRS_TLM;
            payload[1] = ExLRS_TLM_OFF;

            sendToExLRS(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
        }
    }
}