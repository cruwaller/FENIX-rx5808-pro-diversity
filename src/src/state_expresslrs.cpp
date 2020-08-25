#include <stdint.h>
#include "settings_eeprom.h"
#include "state_expresslrs.h"
#include "ui.h"
#include "temperature.h"
#include "touchpad.h"
#include "comm_espnow.h"
#include "protocol_ExpressLRS.h"


void StateMachine::ExLRSStateHandler::onEnter()
{
    //onUpdateDraw(false);
}


void StateMachine::ExLRSStateHandler::onUpdate()
{
    onUpdateDraw(TouchPad::touchData.buttonPrimary);
}


void StateMachine::ExLRSStateHandler::onUpdateDraw(uint8_t tapAction)
{
    uint32_t off_x = 20, off_x2 = off_x + 17 * Ui::CHAR_W, off_y = 20;
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t region = expresslrs_params_get_region();

    drawHeader();

    Ui::display.setTextColor(100);

    // Mode
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Mode (Hz):");
    Ui::display.setCursor(off_x2, off_y);
    if (region == 3)
        Ui::display.print("50    125    250");
    else
        Ui::display.print("50    100    200");
    off_y += 20;

    // RF Power
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Power (mW):");
    Ui::display.setCursor(off_x2, off_y);
    Ui::display.print("25    50     100");
    off_y += 20;

    // TLM Rate
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Telemetry:");
    Ui::display.setCursor(off_x2, off_y);
    Ui::display.print("On    Off");
    off_y += 20;

    // Set VTX channel
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("VTX channel:");
    Ui::display.setCursor(off_x2, off_y);
    Ui::display.print("SEND");
    off_y += 20;


    /*************************************/
    // Print current settings
    off_y += 20;

    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("== Current settings ==");

    off_y += 12;
    off_x = 40;
    off_x2 = off_x + 100;

    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Frequency:");
    Ui::display.setCursor(off_x2, off_y);
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
            Ui::display.print("2.4GHz");
            break;
        default:
            Ui::display.print("---");
            break;
    };

    off_y += 10;
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Rate:");
    Ui::display.setCursor(off_x2, off_y);
    switch (expresslrs_params_get_rate()) {
        case 0:
            Ui::display.print((region == 3) ? "250Hz" : "200Hz");
            break;
        case 1:
            Ui::display.print((region == 3) ? "125Hz" : "100Hz");
            break;
        case 2:
            Ui::display.print("50Hz");
            break;
        default:
            Ui::display.print("---");
            break;
    };

    off_y += 10;
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Power:");
    Ui::display.setCursor(off_x2, off_y);
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
        case 4:
            Ui::display.print("100mW");
            break;
        case 5:
            Ui::display.print("250mW");
            break;
        case 6:
            Ui::display.print("500mW");
            break;
        case 7:
            Ui::display.print("1000mW");
            break;
        case 8:
            Ui::display.print("2000mW");
            break;
        default:
            Ui::display.print("---");
            break;
    };

    off_y += 10;
    Ui::display.setCursor(off_x, off_y);
    Ui::display.print("Telemetry:");
    Ui::display.setCursor(off_x2, off_y);
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
        case 4:
            Ui::display.print("1/16");
            break;
        case 5:
            Ui::display.print("1/8");
            break;
        case 6:
            Ui::display.print("1/4");
            break;
        case 7:
            Ui::display.print("1/2");
            break;
        default:
            Ui::display.print("---");
            break;
    };


    // Draw Mode box
    if (cursor_y > 16 && cursor_y < 31)
    {
        if ( // 50
            cursor_x > (20 + 17 * 8 - 4) && cursor_x < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 16, 23, 15, 100);
            if (tapAction)
                expresslrs_rate_send(ExLRS_50Hz);
        }
        else if ( // 100
            cursor_x > (20 + 23 * 8 - 4) && cursor_x < (20 + 26 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 16, 31, 15, 100);
            if (tapAction)
                expresslrs_rate_send(ExLRS_100Hz);
        }
        else if ( // 200
            cursor_x > (20 + 30 * 8 - 4) && cursor_x < (20 + 33 * 8 + 3))
        {
            Ui::display.rect(20 + 30 * 8 - 4, 16, 31, 15, 100);
            if (tapAction)
                expresslrs_rate_send(ExLRS_200Hz);
        }
    }
    // Draw RF Power box
    else if (cursor_y > 36 && cursor_y < 51)
    {
        if ( // 25mW
            cursor_x > (20 + 17 * 8 - 4) && cursor_x < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 36, 23, 15, 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_25mW);
        }
        else if ( // 50mW
            cursor_x > (20 + 23 * 8 - 4) && cursor_x < (20 + 25 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 36, 23, 15, 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_50mW);
        }
        else if ( // 100mW
            cursor_x > (20 + 30 * 8 - 4) && cursor_x < (20 + 33 * 8 + 3))
        {
            Ui::display.rect(20 + 30 * 8 - 4, 36, 31, 15, 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_100mW);
        }
    }
    // Draw TLM box
    else if (cursor_y > 56 && cursor_y < 71)
    {
        if ( // On
            cursor_x > (20 + 17 * 8 - 4) && cursor_x < (20 + 19 * 8 + 3))
        {
            Ui::display.rect(20 + 17 * 8 - 4, 56, 23, 15, 100);
            if (tapAction)
                expresslrs_tlm_send(ExLRS_TLM_ON);
        }
        else if ( // Off
            cursor_x > (20 + 23 * 8 - 4) && cursor_x < (20 + 26 * 8 + 3))
        {
            Ui::display.rect(20 + 23 * 8 - 4, 56, 31, 15, 100);
            if (tapAction)
                expresslrs_tlm_send(ExLRS_TLM_OFF);
        }
    }
    // Draw VTX SEND box
    else if (cursor_y > 76 && cursor_y < 91)
    {
        if (cursor_x > (20 + 17 * 8 - 4) && cursor_x < (20 + 21 * 8 + 3))
        {
            Ui::display.rect((20 + 17 * 8 - 4), 76, (4 + 4 * 8 + 3), 15, 100);
            if (tapAction)
                expresslrs_vtx_freq_send(Channels::getFrequency(Receiver::activeChannel));
        }
    }
}
