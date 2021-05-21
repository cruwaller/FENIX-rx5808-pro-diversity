#include <stdint.h>
#include "settings_eeprom.h"
#include "state_expresslrs.h"
#include "ui.h"
#include "temperature.h"
#include "touchpad.h"
#include "comm_espnow.h"
#include "protocol_ExpressLRS.h"


static const char* unknown_value = "---";

static const char* elrs_lookuptable_rates_900[ExLRS_RATE_MAX] = {
    "200Hz", "100Hz", "50Hz",
};
static const char* elrs_lookuptable_rates_2400[ExLRS_RATE_ISM_MAX] = {
    "500Hz", "250Hz", "125Hz", "50Hz",
};
static const char* elrs_lookuptable_rates_2400_FLRC[1] = {
    "500Hz",
};
static const char* elrs_lookuptable_power[ExLRS_PWR_MAX] = {
    "dynamic", "10mW", "25mW", "50mW", "100mW", "250mW", "500mW", "1000mW", "2000mW",
};
static const char* elrs_lookuptable_tlm[ExLRS_TLM_RATIO_MAX] = {
    "OFF", "1/128", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2"
};

#define BOX_OFFSET_S    4
#define BOX_OFFSET_E    3
#define BOX_OFFSET      (BOX_OFFSET_S + BOX_OFFSET_E)

#define BASE_OFF_X      20
#define BASE_OFF_Y      20

#define LINE_SPACING    (2*Ui::CHAR_H)

#define GET_X(idx)      (BASE_OFF_X + ((idx)*Ui::CHAR_W))
#define GET_X2(idx)     (BASE_OFF_X + 20 + ((idx)*Ui::CHAR_W))

#define GET_Y(idx)      (BASE_OFF_Y + ((idx)*(LINE_SPACING)))

#define GET_X_BOX(idx)  (GET_X(idx) - BOX_OFFSET_S)
#define GET_Y_BOX(idx)  (GET_Y(idx) - BOX_OFFSET_S)

#define GET_X_BOX_END(I,H)  (GET_X_BOX(I) + GET_BOX_W(H))
#define GET_Y_BOX_END(I,H)  (GET_Y_BOX(I) + GET_BOX_H(H))

#define GET_BOX_W(c)    (((c) * Ui::CHAR_W) + BOX_OFFSET)
#define GET_BOX_H(c)    (((c) * Ui::CHAR_H) + BOX_OFFSET)


// Lines
enum {
    LINE_DOMAIN = 0,
    LINE_RATE,
    LINE_POWER,
    LINE_TLM,
    LINE_VTX,
    /* Empty line */
    LINE_CURRENT = LINE_VTX+2,
};


void StateMachine::ExLRSStateHandler::onUpdate(TouchPad::TouchData const &touch)
{
    const char * param_str;
    int16_t const cursor_x = touch.cursorX;
    int16_t const cursor_y = touch.cursorY;
    uint8_t const tapAction = touch.buttonPrimary;
    uint8_t region = expresslrs_params_get_region();
    uint8_t const has_dual = (region & ExLRS_RF_MODE_DUAL);
    uint8_t param_value;
    uint8_t max_pwr = ExLRS_PWR_100mW;

    region &= ExLRS_RF_MODE_MASK;

    if (drawHeader(cursor_x, cursor_y, tapAction))
        return;

#define SELECT_OFFSET  17

    // Frequency domain
    if (has_dual) {
        Ui::display.setCursor(GET_X(0), GET_Y(LINE_DOMAIN));
        Ui::display.print("Domain:");
        Ui::display.setCursor(GET_X(SELECT_OFFSET), GET_Y(LINE_DOMAIN));
        Ui::display.print("900   2400  FLRC");
    }

    // Rate
    Ui::display.setCursor(GET_X(0), GET_Y(LINE_RATE));
    Ui::display.print("Rate (Hz):");
    Ui::display.setCursor(GET_X(SELECT_OFFSET), GET_Y(LINE_RATE));
    if (ExLRS_RF_MODE_2400_ISM_FLRC == region)
        Ui::display.print("500");
    else if (ExLRS_RF_MODE_2400_ISM <= region)
        Ui::display.print("500   250   125");
    else
        Ui::display.print("200   100   50");

    // RF Power
    (void)expresslrs_params_get_power(&max_pwr); // get just max
    Ui::display.setCursor(GET_X(0), GET_Y(LINE_POWER));
    Ui::display.print("Power (mW):");
    Ui::display.setCursor(GET_X(SELECT_OFFSET), GET_Y(LINE_POWER));
    Ui::display.print("10    25    50");
    if (ExLRS_PWR_50mW < max_pwr)
        Ui::display.print("    100");

    // TLM Rate
    Ui::display.setCursor(GET_X(0), GET_Y(LINE_TLM));
    Ui::display.print("Telemetry:");
    Ui::display.setCursor(GET_X(SELECT_OFFSET), GET_Y(LINE_TLM));
    Ui::display.print("On    Off");

    // Set VTX channel
    Ui::display.setCursor(GET_X(0), GET_Y(LINE_VTX));
    Ui::display.print("VTX channel:");
    Ui::display.setCursor(GET_X(SELECT_OFFSET), GET_Y(LINE_VTX));
    Ui::display.print("SEND");

    // Draw Mode box
    if (has_dual && cursor_y > GET_Y_BOX(LINE_DOMAIN) && cursor_y < GET_Y_BOX_END(LINE_DOMAIN, 1))
    {
        uint8_t next_domain = ExLRS_RF_MODE_INVALID;
        if ( // 900MHz
            cursor_x > GET_X_BOX(SELECT_OFFSET) && cursor_x < GET_X_BOX_END(SELECT_OFFSET, 3))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET), GET_Y_BOX(LINE_DOMAIN), GET_BOX_W(3), GET_BOX_H(1), 100);
            next_domain = ExLRS_RF_MODE_868_EU;
        }
        else if ( // 2.4GHz LoRa
            cursor_x > GET_X_BOX(SELECT_OFFSET+6) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+6, 4))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET+6), GET_Y_BOX(LINE_DOMAIN), GET_BOX_W(4), GET_BOX_H(1), 100);
            next_domain = ExLRS_RF_MODE_2400_ISM_500;
        }
        else if ( // 2.4GHz FLRC
            cursor_x > GET_X_BOX(SELECT_OFFSET+12) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+12, 4))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET+12), GET_Y_BOX(LINE_DOMAIN), GET_BOX_W(4), GET_BOX_H(1), 100);
            next_domain = ExLRS_RF_MODE_2400_ISM_FLRC;
        }
        if (tapAction && next_domain != ExLRS_RF_MODE_INVALID)
            expresslrs_domain_send(next_domain);
    }
    // Draw Mode box
    else if (cursor_y > GET_Y_BOX(LINE_RATE) && cursor_y < GET_Y_BOX_END(LINE_RATE, 1))
    {
        uint8_t next_rate = ExLRS_RATE_INVALID;
        if (cursor_x > GET_X_BOX(SELECT_OFFSET) && cursor_x < GET_X_BOX_END(SELECT_OFFSET, 3))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET), GET_Y_BOX(LINE_RATE), GET_BOX_W(3), GET_BOX_H(1), 100);
            next_rate = (ExLRS_RF_MODE_2400_ISM <= region) ? (uint8_t)ExLRS_RATE_ISM_500 : (uint8_t)ExLRS_RATE_200;
        }
        else if (region <= ExLRS_RF_MODE_2400_ISM_500)
        {
            if (cursor_x > GET_X_BOX(SELECT_OFFSET+6) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+6, 3))
            {
                Ui::display.rect(GET_X_BOX(SELECT_OFFSET+6), GET_Y_BOX(LINE_RATE), GET_BOX_W(3), GET_BOX_H(1), 100);
                next_rate = (ExLRS_RF_MODE_2400_ISM <= region) ? (uint8_t)ExLRS_RATE_ISM_250 : (uint8_t)ExLRS_RATE_100;
            }
            else if (cursor_x > GET_X_BOX(SELECT_OFFSET+12) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+12, 3))
            {
                Ui::display.rect(GET_X_BOX(SELECT_OFFSET+12), GET_Y_BOX(LINE_RATE), GET_BOX_W(3), GET_BOX_H(1), 100);
                next_rate = (ExLRS_RF_MODE_2400_ISM <= region) ? (uint8_t)ExLRS_RATE_ISM_125 : (uint8_t)ExLRS_RATE_50;
            }
        }
        if (tapAction && next_rate != ExLRS_RATE_INVALID)
            expresslrs_rate_send(next_rate);
    }
    // Draw RF Power box
    else if (cursor_y > GET_Y_BOX(LINE_POWER) && cursor_y < GET_Y_BOX_END(LINE_POWER, 1))
    {
        if ( // 10mW
            cursor_x > GET_X_BOX(SELECT_OFFSET) && cursor_x < GET_X_BOX_END(SELECT_OFFSET, 2))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET), GET_Y_BOX(LINE_POWER), GET_BOX_W(2), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_10mW);
        }
        else if ( // 25mW
            cursor_x > GET_X_BOX(SELECT_OFFSET+6) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+6, 2))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET+6), GET_Y_BOX(LINE_POWER), GET_BOX_W(2), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_25mW);
        }
        else if ( // 50mW
            cursor_x > GET_X_BOX(SELECT_OFFSET+12) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+12, 2))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET+12), GET_Y_BOX(LINE_POWER), GET_BOX_W(2), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_50mW);
        }
        else if ( // 100mW
            ExLRS_PWR_50mW < max_pwr &&
            cursor_x > GET_X_BOX(SELECT_OFFSET+18) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+18, 3))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET+18), GET_Y_BOX(LINE_POWER), GET_BOX_W(3), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_power_send(ExLRS_PWR_100mW);
        }
    }
    // Draw TLM box
    else if (cursor_y > GET_Y_BOX(LINE_TLM) && cursor_y < GET_Y_BOX_END(LINE_TLM, 1))
    {
        if ( // On
            cursor_x > GET_X_BOX(SELECT_OFFSET) && cursor_x < GET_X_BOX_END(SELECT_OFFSET, 2))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET), GET_Y_BOX(LINE_TLM), GET_BOX_W(2), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_tlm_send(ExLRS_TLM_ON);
        }
        else if ( // Off
            cursor_x > GET_X_BOX(SELECT_OFFSET+6) && cursor_x < GET_X_BOX_END(SELECT_OFFSET+6, 3))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET+6), GET_Y_BOX(LINE_TLM), GET_BOX_W(3), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_tlm_send(ExLRS_TLM_OFF);
        }
    }
    // Draw VTX SEND box
    else if (cursor_y > GET_Y_BOX(LINE_VTX) && cursor_y < GET_Y_BOX_END(LINE_VTX, 1))
    {
        if (cursor_x > GET_X_BOX(SELECT_OFFSET) && cursor_x < GET_X_BOX_END(SELECT_OFFSET, 4))
        {
            Ui::display.rect(GET_X_BOX(SELECT_OFFSET), GET_Y_BOX(LINE_VTX), GET_BOX_W(4), GET_BOX_H(1), 100);
            if (tapAction)
                expresslrs_vtx_freq_send(Channels::getFrequency(Receiver::activeChannel));
        }
    }


    /*************************************/
    // Print current settings
    uint32_t off_y = GET_Y(LINE_CURRENT);

    Ui::display.setCursor(GET_X(0), off_y);
    Ui::display.print("== Current settings ==");

#define OFFSET_KEY      GET_X(3)
#define OFFSET_VALUE    GET_X(3+20)
    off_y += 12;

    Ui::display.setCursor(OFFSET_KEY, off_y);
    Ui::display.print("Domain:");
    Ui::display.setCursor(OFFSET_VALUE, off_y);
    switch (region) {
        case ExLRS_RF_MODE_915_AU_FCC:
            Ui::display.print("915MHz LoRa");
            break;
        case ExLRS_RF_MODE_868_EU:
            Ui::display.print("868MHz LoRa");
            break;
        case ExLRS_RF_MODE_433_AU_EU:
            Ui::display.print("433MHz LoRa");
            break;
        case ExLRS_RF_MODE_2400_ISM:
        case ExLRS_RF_MODE_2400_ISM_500:
            Ui::display.print("2.4GHz LoRa");
            break;
        case ExLRS_RF_MODE_2400_ISM_FLRC:
            Ui::display.print("2.4GHz FLRC");
            break;
        default:
            Ui::display.print(unknown_value);
            break;
    };

    off_y += 10;
    Ui::display.setCursor(OFFSET_KEY, off_y);
    Ui::display.print("Rate:");
    Ui::display.setCursor(OFFSET_VALUE, off_y);
    param_str = unknown_value;
    param_value = expresslrs_params_get_rate();

    if (ExLRS_RF_MODE_2400_ISM_FLRC == region) {
        if (param_value < ARRAY_SIZE(elrs_lookuptable_rates_2400_FLRC))
            param_str = elrs_lookuptable_rates_2400_FLRC[param_value];
    } else if (ExLRS_RF_MODE_2400_ISM < region) {
        if (param_value < ARRAY_SIZE(elrs_lookuptable_rates_2400))
            param_str = elrs_lookuptable_rates_2400[param_value];
    } else {
        if (param_value <= ARRAY_SIZE(elrs_lookuptable_rates_900))
            param_str = elrs_lookuptable_rates_900[param_value];
    }
    Ui::display.print(param_str);

    off_y += 10;
    Ui::display.setCursor(OFFSET_KEY, off_y);
    Ui::display.print("Power:");
    Ui::display.setCursor(OFFSET_VALUE, off_y);
    param_str = unknown_value;
    param_value = expresslrs_params_get_power(NULL);
    if (param_value < ARRAY_SIZE(elrs_lookuptable_power))
        param_str = elrs_lookuptable_power[param_value];
    Ui::display.print(param_str);

    off_y += 10;
    Ui::display.setCursor(OFFSET_KEY, off_y);
    Ui::display.print("Telemetry:");
    Ui::display.setCursor(OFFSET_VALUE, off_y);
    param_str = unknown_value;
    param_value = expresslrs_params_get_tlm();
    if (param_value < ARRAY_SIZE(elrs_lookuptable_tlm))
        param_str = elrs_lookuptable_tlm[param_value];
    Ui::display.print(param_str);
}
