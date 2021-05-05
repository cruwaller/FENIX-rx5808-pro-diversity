#include <stddef.h>

#include "state.h"

#include "state_home.h"
#include "state_settings.h"
#include "state_settings_internal.h"
#include "state_settings_rssi.h"
#include "state_menu.h"
#include "state_expresslrs.h"
#include "state_chorus.h"

#include "ui.h"
#include "settings_eeprom.h"
#include "timer.h"
#include "temperature.h"
#include "voltage.h"
#include "cursor.h"

//void *operator new(size_t size, void *ptr){
//  return ptr;
//}

#define MAX(a, b) (a > b ? a : b)

#define STATE_BUFFER_SIZE \
    MAX(sizeof(HomeStateHandler), \
    MAX(sizeof(SettingsStateHandler), \
    MAX(sizeof(SettingsInternalStateHandler), \
    MAX(sizeof(SettingsRssiStateHandler), \
    MAX(sizeof(MenuStateHandler), \
        sizeof(ExLRSStateHandler) \
    )))))
;

namespace StateMachine {

    static StateHandler *getStateHandler(State stateType);
    static void drawCursor(TouchPad::TouchData const &touch);

    static uint8_t DMA_ATTR stateBuffer[STATE_BUFFER_SIZE];
    static StateHandler* DMA_ATTR currentHandler = nullptr;
    State DMA_ATTR currentState = State::BOOT;
    State DMA_ATTR lastState = currentState;


    void setup()
    {
    }

    void IRAM_ATTR update()
    {
        TouchPad::TouchData const touch = TouchPad::get();
        Ui::reset();
        if (currentHandler != nullptr) {
            currentHandler->onUpdate(touch);
        }
        drawCursor(touch);
        Ui::draw(); // draw OSD
    }

    void IRAM_ATTR switchState(State newState)
    {
        if (currentHandler != nullptr) {
            currentHandler->onExit();
        }

        lastState = currentState;
        currentState = newState;
        currentHandler = getStateHandler(newState);

        if (currentHandler != nullptr) {
            currentHandler->onEnter();
        }
    }

    static StateHandler *getStateHandler(State state)
    {
        #define STATE_FACTORY(s, c) \
            case s: \
                return new (&stateBuffer) c(); \
                break;

        switch (state) {
            STATE_FACTORY(State::HOME, HomeStateHandler);
            STATE_FACTORY(State::SETTINGS, SettingsStateHandler);
            STATE_FACTORY(State::SETTINGS_INTERNAL, SettingsInternalStateHandler);
            STATE_FACTORY(State::SETTINGS_RSSI, SettingsRssiStateHandler);
            STATE_FACTORY(State::MENU, MenuStateHandler);
            STATE_FACTORY(State::EXPRESSLRS, ExLRSStateHandler);
            STATE_FACTORY(State::CHORUS, ChorusStateHandler);

            default:
                return nullptr;
        }

        #undef STATE_FACTORY
    }

    static void IRAM_ATTR drawCursor(TouchPad::TouchData const &touch)
    {
        uint8_t i = 0, py, px, pixelValue;
        for (py = 0; py < Cursor::yres; py++) {
            for (px = 0; px < Cursor::xres; px++) {
                pixelValue = Cursor::pixels[i++];
                if (pixelValue == 255) {
                    Ui::display.dot(px + touch.cursorX, py + touch.cursorY, WHITE);
                } else if (pixelValue == 1) {
                    Ui::display.dot(px + touch.cursorX, py + touch.cursorY, BLACK);
                }
            }
        }
    }

    uint8_t IRAM_ATTR StateHandler::drawHeader(int16_t const cursorX, int16_t const cursorY, uint8_t const tap)
    {
        uint32_t x_off = Ui::CHAR_W;

        /*************************************************/
        /*********     PRINT HEADER     ******************/

        // Mode
        Ui::display.setTextColor(WHITE);
        Ui::display.setCursor(x_off, 0);
        Ui::display.print("Mode: "); // 6
        switch (EepromSettings.diversityMode) {
            case Receiver::DiversityMode::ANTENNA_A: {
                Ui::display.print("Antenna A");
                break;
            }
            case Receiver::DiversityMode::ANTENNA_B: {
                Ui::display.print("Antenna B");
                break;
            }
            case Receiver::DiversityMode::DIVERSITY: {
                Ui::display.print("Diversity");
                break;
            }
            default:
                Ui::display.print("FAIL");
                break;
        }
        x_off = 20 * Ui::CHAR_W;

        // Voltage
#ifdef USE_VOLTAGE_MONITORING
        uint8_t voltage = Voltage::get();
        if (voltage <= 9)
            x_off += Ui::CHAR_W;
        Ui::display.setCursor(x_off, 0);
        Ui::display.print(voltage);
        Ui::display.print(".");
        Ui::display.print(Voltage::get_dec());
        Ui::display.print("V");
#endif
        x_off += 6 * Ui::CHAR_W;

        // Temperature // Doesnt currently work within ESP32 Arduino.
#ifdef USE_TEMPERATURE_MONITORING
        Ui::display.setCursor(x_off, 0);
        Ui::display.print(Temperature::getTemperature());
        Ui::display.print("C");
#endif
        x_off += 4 * Ui::CHAR_W;

        // On Time. format: "00:00:00", 8 chars
#ifdef USE_PRINT_ON_TIME
        uint32_t sec_now = millis() / 1000;
        uint8_t hours = sec_now / 60 / 60;
        uint8_t mins  = sec_now / 60 - hours * 60 * 60;
        uint8_t secs  = sec_now - hours * 60 * 60 - mins * 60;
        Ui::display.setCursor(x_off, 0);
        Ui::display.print(hours);
        Ui::display.print(":");
        if (mins < 10) {
            Ui::display.print("0");
        }
        Ui::display.print(mins);
        Ui::display.print(":");
        if (secs < 10) {
            Ui::display.print("0");
        }
        Ui::display.print(secs);
#endif

        // Menu Icon
        Ui::display.line( 315, 1, 322, 1, 100);
        Ui::display.line( 315, 4, 322, 4, 100);
        Ui::display.line( 315, 7, 322, 7, 100);

        // Horixontal line
        Ui::display.line( 0, 9, Ui::XRES, 9, WHITE);

        // Check touch
        if (tap && cursorY < 8) {
            if (cursorX > 314) {
                // Menu
                if (StateMachine::currentState == StateMachine::State::MENU)
                    StateMachine::switchState(StateMachine::State::HOME);
                else
                    StateMachine::switchState(StateMachine::State::MENU);
                return 1;

            } else if (cursorX < 130) {
                // Change mode
                switch ( EepromSettings.diversityMode )
                {
                    case Receiver::DiversityMode::ANTENNA_A:
                        setDiversityMode(Receiver::DiversityMode::ANTENNA_B);
                        break;
                    case Receiver::DiversityMode::ANTENNA_B:
                        setDiversityMode(Receiver::DiversityMode::DIVERSITY);
                        break;
                    case Receiver::DiversityMode::DIVERSITY:
                        setDiversityMode(Receiver::DiversityMode::ANTENNA_A);
                        break;
                    default:
                        break;
                }

                EepromSettings.markDirty();
            }
        }

        return 0;
    }
}
