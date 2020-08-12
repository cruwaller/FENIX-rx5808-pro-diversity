#include <stdint.h>
#include "settings_eeprom.h"
#include "state_expresslrs.h"
#include "ui.h"
#include "temperature.h"
#include "touchpad.h"
#include "comm_espnow.h"
#include "ExpressLRS_Protocol.h"


void StateMachine::ExLRSStateHandler::onEnter()
{
}

void StateMachine::ExLRSStateHandler::onUpdate()
{
    if (TouchPad::touchData.buttonPrimary) {
        TouchPad::touchData.buttonPrimary = false;
        this->doTapAction();
    }
    onUpdateDraw();
}

void StateMachine::ExLRSStateHandler::onInitialDraw()
{
    onUpdateDraw();
}

void StateMachine::ExLRSStateHandler::onUpdateDraw()
{
    uint8_t ism = 0;
#if 0
#ifdef USE_TEMPERATURE_MONITORING
    Ui::display.setCursor(205, 0);

    // Temperature // Doesnt currently work within ESP32 Arduino.
    Ui::display.print(Temperature::getTemperature());
    Ui::display.print("C");

    Ui::display.print(" / ");
#else
    Ui::display.setCursor( 221+4*8, 0);
#endif

    // On Time
    uint32_t sec_now = millis() / 1000;
    uint8_t hours = sec_now / 60 / 60;
    uint8_t mins = sec_now / 60 - hours * 60 * 60;
    uint8_t secs = sec_now - hours * 60 * 60 - mins * 60;
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
#else
    drawHeader();
#endif
    Ui::display.setCursor(20, 10);
    Ui::display.print("Regulatory domain ");
    switch (expresslrs_params_get_region()) {
        case 0:
            Ui::display.print("915MHz");
            break;
        case 1:
            Ui::display.print("868MHz");
            break;
        case 2:
            Ui::display.print("433MHz");
            break;
        case 3:
            Ui::display.print("ISM 2400");
            ism = 1;
            break;
        default:
            Ui::display.print("UNKNOWN");
            break;
    };

    // Mode
    Ui::display.setCursor(20, 20);
    Ui::display.print("Mode (Hz):       50    100    200");
    Ui::display.setCursor(50, 30);
    Ui::display.print("current: ");
    switch (expresslrs_params_get_rate()) {
        case 0:
            Ui::display.print(ism ? "250Hz" : "200Hz");
            break;
        case 1:
            Ui::display.print(ism ? "125Hz" : "100Hz");
            break;
        case 2:
            Ui::display.print("50Hz");
            break;
        default:
            Ui::display.print("---");
            break;
    };

    // RF Power
    Ui::display.setCursor(20, 40);
    Ui::display.print("RF Power (mW):   50    200    1000");
    Ui::display.setCursor(50, 50);
    Ui::display.print("current: ");
    switch (expresslrs_params_get_power()) {
        case 0:
            Ui::display.print("dynamic");
            break;
        case 1:
            Ui::display.print("10mW");
            break;
        case 2:
            Ui::display.print("25mW");
            break;
        case 3:
            Ui::display.print("50mW");
            break;
        default:
            Ui::display.print("---");
            break;
    };

    // TLM Rate
    Ui::display.setCursor(20, 60);
    Ui::display.print("TLM:             On    Off");
    Ui::display.setCursor(50, 70);
    Ui::display.print("current: ");
    switch (expresslrs_params_get_tlm()) {
        case 0:
            Ui::display.print("OFF");
            break;
        case 1:
            Ui::display.print("1/128");
            break;
        case 2:
            Ui::display.print("1/64");
            break;
        case 3:
            Ui::display.print("1/32");
            break;
        default:
            Ui::display.print("---");
            break;
    };

    // Set VTX channel
    Ui::display.setCursor(20, 80);
    Ui::display.print("VTX channel:     SEND");

    // Draw Mode box
    if (TouchPad::touchData.cursorY > 16 && TouchPad::touchData.cursorY < 31)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 16, 23, 15, 100);
        }
        else if ( // 100
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 16, 31, 15, 100);
        }
        else if ( // 200
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 33 * 8 + 3))
        {
            Ui::display.rect(20 + 30 * 8 - 4, 16, 31, 15, 100);
        }
    }
    // Draw RF Power box
    else if (TouchPad::touchData.cursorY > 36 && TouchPad::touchData.cursorY < 51)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 36, 23, 15, 100);
        }
        else if ( // 200
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 27 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 36, 31, 15, 100);
        }
        else if ( // 1000
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 34 * 8 + 3))
        {
            Ui::display.rect(20 + 30 * 8 - 4, 36, 39, 15, 100);
        }
    }
    // Draw TLM box
    else if (TouchPad::touchData.cursorY > 56 && TouchPad::touchData.cursorY < 71)
    {
        if ( // On
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 56, 23, 15, 100);
        }
        else if ( // Off
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 56, 31, 15, 100);
        }
    }
    // Draw VTX SEND box
    else if (TouchPad::touchData.cursorY > 76 && TouchPad::touchData.cursorY < 91)
    {
        if (TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 21 * 8 + 3))
        {
            Ui::display.rect((20 + 17 * 8 - 4), 76, (4 + 4 * 8 + 3), 15, 100);
        }
    }
}

void StateMachine::ExLRSStateHandler::doTapAction()
{
    if ( // Menu
        TouchPad::touchData.cursorX > 314 && TouchPad::touchData.cursorY < 8)
    {
        // EepromSettings.save(); // No changes done to EEPROM, NOT NEEDED!
        StateMachine::switchState(StateMachine::State::MENU);
    }
    else if ( // mode
        TouchPad::touchData.cursorY > 16 && TouchPad::touchData.cursorY < 31)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            expresslrs_rate_send(ExLRS_50Hz);
        }
        else if ( // 100
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            expresslrs_rate_send(ExLRS_100Hz);
        }
        else if ( // 200
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 33 * 8 + 3))
        {
            expresslrs_rate_send(ExLRS_200Hz);
        }
    }
    else if ( // power
        TouchPad::touchData.cursorY > 36 && TouchPad::touchData.cursorY < 51)
    {
        if ( // 50
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            expresslrs_power_send(50);
        }
        else if ( // 200
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 27 * 8 + 3))
        {
            expresslrs_power_send(200);
        }
        else if ( // 1000
            TouchPad::touchData.cursorX > (20 + 30 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 34 * 8 + 3))
        {
            expresslrs_power_send(1000);
        }
    }
    else if ( // TLM
        TouchPad::touchData.cursorY > 56 && TouchPad::touchData.cursorY < 71)
    {
        if ( // On
            TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 19 * 8 + 3))
        {
            expresslrs_tlm_send(1);
        }
        else if ( // Off
            TouchPad::touchData.cursorX > (20 + 23 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 26 * 8 + 3))
        {
            expresslrs_tlm_send(0);
        }
    }
    // VTX SEND
    else if (TouchPad::touchData.cursorY > 76 && TouchPad::touchData.cursorY < 91)
    {
        if (TouchPad::touchData.cursorX > (20 + 17 * 8 - 4) && TouchPad::touchData.cursorX < (20 + 21 * 8 + 3))
        {
            expresslrs_vtx_channel_send(Receiver::activeChannel);
        }
    }
}
