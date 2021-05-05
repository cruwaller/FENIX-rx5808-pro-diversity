#include "state_menu.h"
#include "ui.h"
#include "touchpad.h"
#include "CompositeGraphics.h"
#include "Image.h"
#include "settings_eeprom.h"
#include "settings.h"

#include "icons.h"

#include "WebUpdater.h"

#include <WiFi.h>

#define INFO_TXT_Y_POS (200U)

Image<CompositeGraphics> iconHome(home::xres, home::yres, home::pixels);
Image<CompositeGraphics> iconExLRS(exlrs::xres, exlrs::yres, exlrs::pixels);
Image<CompositeGraphics> iconCalibrate(calibrate::xres, calibrate::yres, calibrate::pixels);
Image<CompositeGraphics> iconUpdate(update::xres, update::yres, update::pixels);
Image<CompositeGraphics> iconBookmark(bookmark::xres, bookmark::yres, bookmark::pixels);
Image<CompositeGraphics> iconChorus(chorus::xres, chorus::yres, chorus::pixels);

#define OFFSET 10

#define X_BASE 47
#define Y_BASE 57

/* Icon size is 50x50 */
#define X_PIC (50 + OFFSET)
#define Y_PIC (50 + OFFSET)

#define GET_X(c) (X_BASE + ((c) * X_PIC))
#define GET_Y(c) (Y_BASE + ((c) * Y_PIC))

#define GET_X_END(c) (GET_X((c)+1) - OFFSET)
#define GET_Y_END(c) (GET_Y((c)+1) - OFFSET)

#define GET_X_RECT(c) (GET_X(c) - (OFFSET/2))
#define GET_Y_RECT(c) (GET_Y(c) - (OFFSET/2))

void StateMachine::MenuStateHandler::onUpdate(TouchPad::TouchData const &touch)
{
    int16_t const cursor_x = touch.cursorX;
    int16_t const cursor_y = touch.cursorY;
    uint8_t const tapAction = touch.buttonPrimary;

    if (drawHeader(cursor_x, cursor_y, tapAction))
        return;

   // 1st row
   iconHome.draw(Ui::display, GET_X(0), GET_Y(0));    // Home
   iconExLRS.draw(Ui::display, GET_X(1), GET_Y(0));   // ExpressLRS
   iconChorus.draw(Ui::display, GET_X(2), GET_Y(0));  // Chorus
   //iconBookmark.draw(Ui::display, GET_X(3), GET_Y(0));
    // 2nd row
   //iconBookmark.draw(Ui::display, GET_X(0), GET_Y(1));
   //iconBookmark.draw(Ui::display, GET_X(1), GET_Y(1));
   iconCalibrate.draw(Ui::display, GET_X(2), GET_Y(1)); // Calibration
   iconUpdate.draw(Ui::display, GET_X(3), GET_Y(1));    // OTA update

   // Check 1st row
   if (cursor_y > GET_Y(0) && cursor_y < GET_Y_END(0)) {

      if ( // Home
         cursor_x > GET_X(0)  && cursor_x < GET_X_END(0))
      {
         Ui::display.rect(GET_X_RECT(0), GET_Y_RECT(0), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(11), INFO_TXT_Y_POS);
         Ui::display.print("Home Screen");
         if (tapAction)
            StateMachine::switchState(StateMachine::State::HOME);
      }
      else if ( // ExpressLRS Settings
         cursor_x > GET_X(1)  && cursor_x < GET_X_END(1))
      {
         Ui::display.rect(GET_X_RECT(1), GET_Y_RECT(0), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(19), INFO_TXT_Y_POS);
         Ui::display.print("ExpressLRS Settings");
         if (tapAction)
            StateMachine::switchState(StateMachine::State::EXPRESSLRS);
      }
      else if ( // Chorus settings
         cursor_x > GET_X(2)  && cursor_x < GET_X_END(2))
      {
         Ui::display.rect(GET_X_RECT(2), GET_Y_RECT(0), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(14), INFO_TXT_Y_POS);
         Ui::display.print("Chorus control");
         if (tapAction)
            StateMachine::switchState(StateMachine::State::CHORUS);
      }
#if 0
      else if ( // item 4
         cursor_x > GET_X(3)  && cursor_x < GET_X_END(3))
      {
         Ui::display.rect(GET_X_RECT(3), GET_Y_RECT(0), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(6), INFO_TXT_Y_POS);
         Ui::display.print("Menu 4");
         //if (tapAction)
         //   StateMachine::switchState(StateMachine::State::);
      }
#endif
   } else if (cursor_y > GET_Y(1) && cursor_y < GET_Y_END(1)) {

#if 0
      if ( // item 5
         cursor_x > GET_X(0)  && cursor_x < GET_X_END(0))
      {
         Ui::display.rect(GET_X_RECT(0), GET_Y_RECT(1), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(6), INFO_TXT_Y_POS);
         Ui::display.print("Menu 5");
         //if (tapAction)
         //   StateMachine::switchState(StateMachine::State::);
      }
      else if ( // item 6
         cursor_x > GET_X(1)  && cursor_x < GET_X_END(1))
      {
         Ui::display.rect(GET_X_RECT(1), GET_Y_RECT(1), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(6), INFO_TXT_Y_POS);
         Ui::display.print("Menu 6");
         //if (tapAction)
         //   StateMachine::switchState(StateMachine::State::);
      }
      else
#endif
      if ( // Receiver Calibration
         cursor_x > GET_X(2)  && cursor_x < GET_X_END(2))
      {
         Ui::display.rect(GET_X_RECT(2), GET_Y_RECT(1), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(20), INFO_TXT_Y_POS);
         Ui::display.print("Receiver Calibration");
         if (tapAction) {
            //EepromSettings.initDefaults();
            EepromSettings.isCalibrated = false;
            EepromSettings.save();
            ESP.restart();
         }
      }
      else if ( // WiFi OTA Update
         cursor_x > GET_X(3)  && cursor_x < GET_X_END(3))
      {
         Ui::display.rect(GET_X_RECT(3), GET_Y_RECT(1), X_PIC, Y_PIC, WHITE);
         Ui::display.setCursor(UI_GET_MID_X(11), (INFO_TXT_Y_POS - Ui::CHAR_H));
         Ui::display.print("WiFi Update");
         Ui::display.setCursor( 50, INFO_TXT_Y_POS);
         Ui::display.print("SSID: ");
         Ui::display.print(WIFI_AP_SSID);
         Ui::display.setCursor( 50, (INFO_TXT_Y_POS + Ui::CHAR_H));
         Ui::display.print("IP:   192.168.4.1");
         if (tapAction) {
            BeginWebUpdate(); // Start updater

            uint32_t previousLEDTime = 0, now;
            // ... and handle client requests
            while (1) {
               HandleWebUpdate();
               now = millis();
               if (100u <= (now - previousLEDTime)) {
                     digitalWrite(PIN_RX_SWITCH, !digitalRead(PIN_RX_SWITCH));
                     previousLEDTime = now;
               }
               yield();
            }
         }
      }
   }
}
