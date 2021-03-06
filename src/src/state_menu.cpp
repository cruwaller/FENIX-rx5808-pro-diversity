#include "state_menu.h"
#include "ui.h"
#include "touchpad.h"
#include "CompositeGraphics.h"
#include "settings_eeprom.h"
#include "settings.h"

#include "icons.h"

#include "WebUpdater.h"

#define INFO_TXT_Y_POS (200U)

Image<CompositeGraphics> iconHome(home::xres, home::yres, home::pixels);
Image<CompositeGraphics> iconExLRS(exlrs::xres, exlrs::yres, exlrs::pixels);
Image<CompositeGraphics> iconCalibrate(calibrate::xres, calibrate::yres, calibrate::pixels);
Image<CompositeGraphics> iconUpdate(update::xres, update::yres, update::pixels);
Image<CompositeGraphics> iconBookmark(bookmark::xres, bookmark::yres, bookmark::pixels);
Image<CompositeGraphics> iconChorus(chorus::xres, chorus::yres, chorus::pixels);

void StateMachine::MenuStateHandler::onEnter()
{
   //onUpdateDraw(false);
}


void StateMachine::MenuStateHandler::onUpdate()
{
   onUpdateDraw(TouchPad::touchData.buttonPrimary);
}


void StateMachine::MenuStateHandler::onUpdateDraw(uint8_t tapAction)
{
    if (drawHeader())
        return;

   // 1st row
   iconHome.draw(Ui::display, 47, 57);          // Home
   iconExLRS.draw(Ui::display, 47+60, 57);      // ExpressLRS
   iconChorus.draw(Ui::display, 47+120, 57);    // Chorus
   iconBookmark.draw(Ui::display, 47+180, 57);
    // 2nd row
   iconBookmark.draw(Ui::display, 47, 117);
   iconBookmark.draw(Ui::display, 47+60, 117);
   iconCalibrate.draw(Ui::display, 47+120, 117); // Calibration
   iconUpdate.draw(Ui::display, 47+180, 117);    // OTA update

   if ( // Home
      TouchPad::touchData.cursorX > 47  && TouchPad::touchData.cursorX < 47+50 &&
      TouchPad::touchData.cursorY > 57 && TouchPad::touchData.cursorY < 107
   )
   {
      Ui::display.rect(47-5, 57-6, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(11), INFO_TXT_Y_POS);
      Ui::display.print("Home Screen");
      if (tapAction)
         StateMachine::switchState(StateMachine::State::HOME);
   }
   else if ( // ExpressLRS Settings
      TouchPad::touchData.cursorX > 47+60  && TouchPad::touchData.cursorX < 47+60+50 &&
      TouchPad::touchData.cursorY > 57 && TouchPad::touchData.cursorY < 107
   )
   {
      Ui::display.rect(47+60-5, 57-6, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(19), INFO_TXT_Y_POS);
      Ui::display.print("ExpressLRS Settings");
      if (tapAction)
         StateMachine::switchState(StateMachine::State::EXPRESSLRS);
   }
   else if ( // Chorus settings
      TouchPad::touchData.cursorX > 47+120  && TouchPad::touchData.cursorX < 47+120+50 &&
      TouchPad::touchData.cursorY > 57 && TouchPad::touchData.cursorY < 107
   )
   {
      Ui::display.rect(47+120-5, 57-6, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(14), INFO_TXT_Y_POS);
      Ui::display.print("Chorus control");
      if (tapAction)
         StateMachine::switchState(StateMachine::State::CHORUS);
   }
   else if ( // item 4
      TouchPad::touchData.cursorX > 47+180  && TouchPad::touchData.cursorX < 47+180+50 &&
      TouchPad::touchData.cursorY > 57 && TouchPad::touchData.cursorY < 107
   )
   {
      Ui::display.rect(47+180-5, 57-6, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(6), INFO_TXT_Y_POS);
      Ui::display.print("Menu 4");
      //if (tapAction)
      //   StateMachine::switchState(StateMachine::State::);
   }
   else if ( // item 5
      TouchPad::touchData.cursorX > 47  && TouchPad::touchData.cursorX < 47+50 &&
      TouchPad::touchData.cursorY > 117 && TouchPad::touchData.cursorY < 167
   )
   {
      Ui::display.rect(47-5, 117-5, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(6), INFO_TXT_Y_POS);
      Ui::display.print("Menu 5");
      //if (tapAction)
      //   StateMachine::switchState(StateMachine::State::);
   }
   else if ( // item 6
      TouchPad::touchData.cursorX > 47+60  && TouchPad::touchData.cursorX < 47+60+50 &&
      TouchPad::touchData.cursorY > 117 && TouchPad::touchData.cursorY < 167
   )
   {
      Ui::display.rect(47+60-5, 117-5, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(6), INFO_TXT_Y_POS);
      Ui::display.print("Menu 6");
      //if (tapAction)
      //   StateMachine::switchState(StateMachine::State::);
   }
   else if ( // Receiver Calibration
      TouchPad::touchData.cursorX > 47+120  && TouchPad::touchData.cursorX < 47+120+50 &&
      TouchPad::touchData.cursorY > 117 && TouchPad::touchData.cursorY < 167
   )
   {
      Ui::display.rect(47+120-5, 117-5, 60, 60, WHITE);
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
      TouchPad::touchData.cursorX > 47+180  && TouchPad::touchData.cursorX < 47+180+50 &&
      TouchPad::touchData.cursorY > 117 && TouchPad::touchData.cursorY < 167
   )
   {
      Ui::display.rect(47+180-5, 117-5, 60, 60, WHITE);
      Ui::display.setCursor(UI_GET_MID_X(11), (INFO_TXT_Y_POS - Ui::CHAR_H));
      Ui::display.print("WiFi Update");
      Ui::display.setCursor( 50, INFO_TXT_Y_POS);
      Ui::display.print("SSID: ");
      Ui::display.print(STASSID);
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
