
#include "state_settings.h"
#include "settings_eeprom.h"

#include "state.h"
#include "ui.h"
#include "voltage.h"

int8_t selectedMenuItem = 0;
bool showChangeMenuOptions = false;

uint8_t menuItems = 13; // Number of items in settingsMenu[]
char* settingsMenu[]={ 
    "Quadversity",
    "Home Page", //defaut, simple, stats
    "Voltage Scale", 
    "Warning Voltage",
    "Save Screen", // Off, option 1, 2, 3
    "Use OLED",
    "Fast Boot",
    "About",
    "Advanced Settings",
    "Button Beep",
    "Draw Logo",
    "Invert Display",
    "Rotate OLED",
    };

void StateMachine::SettingsStateHandler::onEnter() {
    selectedMenuItem = 0;
    showChangeMenuOptions = false;
//    Ui::clear();
}

void StateMachine::SettingsStateHandler::onExit() {
}

void StateMachine::SettingsStateHandler::onUpdate() {
//    Ui::needUpdate();
}

void StateMachine::SettingsStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void StateMachine::SettingsStateHandler::onUpdateDraw() {


    if (!showChangeMenuOptions) {
      
        int8_t index;
    
//        Ui::clear();
//        Ui::setTextSize(1);
        
//        Ui::setCursor(0, 8);
        index = selectedMenuItem-2;
        if (index < 0) {
          index += menuItems;
        }
//        Ui::display.print( settingsMenu[index] );
        
//            Ui::setCursor(6, 18);
            index = selectedMenuItem-1;
            if (index < 0) {
              index += menuItems;
            }
//            Ui::display.print( settingsMenu[index] );
            
//                Ui::setCursor(12, 28);
//                Ui::display.setTextColor(BLACK, WHITE); // 'inverted' text
//                Ui::display.print( settingsMenu[selectedMenuItem] );
//                Ui::display.setTextColor(WHITE, BLACK);
            
//            Ui::setCursor(6, 38);    
            index = selectedMenuItem+1;
            if (index > menuItems-1) {
              index -= menuItems;
            }
//            Ui::display.print( settingsMenu[index] );
        
//        Ui::setCursor(0, 48);
        index = selectedMenuItem+2;
        if (index > menuItems-1) {
          index -= menuItems;
        }
//        Ui::display.print( settingsMenu[index] );
    }
    
    if (showChangeMenuOptions) {
//        Ui::fillRect(15, 20, 96, 24, BLACK);
//        Ui::drawRoundRect(15, 20, 96, 24, 2, WHITE);
//        Ui::setCursor(29, 28);

        switch(selectedMenuItem) {
          
            case 0:    // Diversity
                  #ifdef FENIX_QUADVERSITY
                      if (EepromSettings.quadversity) {
//                          Ui::display.print(PSTR2("    On     "));
                      } else {
//                          Ui::display.print(PSTR2("    Off    "));
                      }
                      break;
                  #else
//                      Ui::display.print(PSTR2("    NA     "));
                  #endif
            break;
            
            case 1:    // Home Page
//              if (EepromSettings.selectedHomePage == 0) {
////                  Ui::display.print(PSTR2("  Default  "));
//              }
//              if (EepromSettings.selectedHomePage == 1) {
////                  Ui::display.print(PSTR2("  Simple   "));
//              }
//              if (EepromSettings.selectedHomePage == 2) {
////                  Ui::display.print(PSTR2("  Stats    "));
//              }
            break;
            
            case 2:    // Voltage Scale
                  #ifdef FENIX_QUADVERSITY
//                      Ui::display.print(PSTR2("  "));
//                      Ui::display.print(Voltage::voltage);   
//                      Ui::display.print(PSTR2("."));        
//                      Ui::display.print(Voltage::voltageDec);
//                      Ui::display.print(PSTR2(" V"));
                  #else
//                      Ui::display.print(PSTR2("    NA     "));
                  #endif
            break;
            
            case 3:    // Volt Warning
                  #ifdef FENIX_QUADVERSITY
//                      Ui::display.print(PSTR2("  "));
//                      Ui::display.print(EepromSettings.vbatWarning / 10);
//                      Ui::display.print(PSTR2("."));
//                      Ui::display.print(EepromSettings.vbatWarning % 10);
//                      Ui::display.print(PSTR2(" V"));
                  #else
//                      Ui::display.print(PSTR2("    NA     "));
                  #endif
            break;
            
            case 4:    // Save Screen
//              if (EepromSettings.saveScreenOn == 0) {
////                  Ui::display.print(PSTR2("    Off    "));
//              }
//              if (EepromSettings.saveScreenOn == 1) {
////                  Ui::display.print(PSTR2("  Default  "));
//              }
//              if (EepromSettings.saveScreenOn == 2) {
////                  Ui::display.print(PSTR2("Betaflight "));
//              }
//              if (EepromSettings.saveScreenOn == 3) {
////                  Ui::display.print(PSTR2(" RotorRiot "));
//              }
//              if (EepromSettings.saveScreenOn == 4) {
////                  Ui::display.print(PSTR2("  Random   "));
//              }
//              if (EepromSettings.saveScreenOn == 5) {
////                  Ui::display.print(PSTR2("  Custom   "));
//              }
            break;
            
            case 5:    // Use OLED Screen
//              if (EepromSettings.useOledScreen) {
////                  Ui::display.print(PSTR2("    Yes     "));
//              } else {
////                  Ui::display.print(PSTR2("    No    "));
//              }
            break;
            
            case 6:    // Use Fast Boot
//              if (EepromSettings.useFastBoot) {
////                  Ui::display.print(PSTR2("    On     "));
//              } else {
////                  Ui::display.print(PSTR2("    Off    "));
//              }
            break;
            
            case 7:    // Version
//              Ui::display.print(PSTR2("Version v0.2"));
            break;
            
            case 8:    // Adv Settings
              StateMachine::switchState(StateMachine::State::SETTINGS_INTERNAL);
            break;
            
            case 9:    // Button Beep
              if (EepromSettings.buttonBeep) {
//                  Ui::display.print(PSTR2("    On     "));
              } else {
//                  Ui::display.print(PSTR2("    Off    "));
              }
            break;
            
            case 10:    // CUSTOMLOGO
              #ifdef FENIX_QUADVERSITY
//                  StateMachine::switchState(StateMachine::State::CUSTOMLOGO);
              #endif
            break;
            
            case 11:    // Invert Display
//              if (EepromSettings.invertDisplay) {
////                  Ui::display.print(PSTR2("    On     "));
//              } else {
////                  Ui::display.print(PSTR2("    Off    "));
//              }
            break;
            
            case 12:    // Rotate OLED
//              if (EepromSettings.rotateOled) {
////                  Ui::display.print(PSTR2("  Rotated  "));
//              } else {
////                  Ui::display.print(PSTR2("  Normal   "));
//              }
            break;
            
        }    
    }
        
//    Ui::needDisplay();

}

//void StateMachine::SettingsStateHandler::onButtonChange(
//    Button button,
//    Buttons::PressType pressType
//)  {
//  
//  if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::MODE_PRESSED
//     ) {
//          if (showChangeMenuOptions) {
//              showChangeMenuOptions = false;
//              EepromSettings.markDirty();    
//          } else {
//              showChangeMenuOptions = true;        
//          }
//        }
//  else if (
//      pressType == Buttons::PressType::LONG &&
//      button == Button::MODE_PRESSED
//     ) {
//        }
//  else if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::UP_PRESSED
//     ) {
//          if (!showChangeMenuOptions) {
//              selectedMenuItem--;
//              if (selectedMenuItem < 0)
//                selectedMenuItem += menuItems;
//          }    
//          if (showChangeMenuOptions) {   
//    
//            switch(selectedMenuItem) {
//                
//                case (0):    // Diversity
//                  #ifdef FENIX_QUADVERSITY
//                    EepromSettings.quadversity = !EepromSettings.quadversity;
//                    EepromSettings.isCalibrated = !EepromSettings.isCalibrated;
//                  #endif                    
//                break;
//                
//                case (1):    // Home Page
//                    EepromSettings.selectedHomePage--;
//                    if (EepromSettings.selectedHomePage == 255) {
//                      EepromSettings.selectedHomePage = 2;
//                    }                      
//                break;
//                
//                case (2):    // Voltage Scale
//                  EepromSettings.vbatScale++;                 
//                break;
//                
//                case (3):    // Volt Warning
//                  EepromSettings.vbatWarning--;                 
//                break;
//                
//                case (4):    // Save Screen
//                  EepromSettings.saveScreenOn--; 
//                  if (EepromSettings.saveScreenOn == 255) {
//                    #ifdef FENIX_QUADVERSITY
//                        EepromSettings.saveScreenOn = 5;
//                    #else
//                        EepromSettings.saveScreenOn = 4;
//                    #endif
//                  }                        
//                break;
//                
//                case (5):    // Use OLED Screen
//                  EepromSettings.useOledScreen = !EepromSettings.useOledScreen;                
//                break;
//                
//                case (6):    // Use Fast Boot
//                  EepromSettings.useFastBoot = !EepromSettings.useFastBoot;               
//                break;
//
//                case (7):    // About
//                break;
//
//                case (8):    // Adv Settings
//                break;
//                
//                case (9):    // Use Fast Boot
//                  EepromSettings.buttonBeep = !EepromSettings.buttonBeep;               
//                break;
//                
//                case (10):    // CUSTOMLOGO
//                  EepromSettings.buttonBeep = !EepromSettings.buttonBeep;               
//                break;
//                
//                case (11):    // Invert Display
//                  EepromSettings.invertDisplay = !EepromSettings.invertDisplay;               
//                break;
//                
//                case (12):    // Rotate OLED
//                  EepromSettings.rotateOled = !EepromSettings.rotateOled;               
//                break;
//                
//            }  
//          }
//        }
//  else if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::DOWN_PRESSED
//     ) {
//          if (!showChangeMenuOptions) {
//              selectedMenuItem++;
//              if (selectedMenuItem > menuItems-1)
//                selectedMenuItem -= menuItems;
//          } 
//          if (showChangeMenuOptions) {   
//    
//            switch(selectedMenuItem) {
//                
//                case (0):    // Diversity
//                    #ifdef FENIX_QUADVERSITY
//                      EepromSettings.quadversity = !EepromSettings.quadversity;
//                      EepromSettings.isCalibrated = !EepromSettings.isCalibrated;
//                    #endif
//                break;
//                
//                case (1):    // Home Page
//                    EepromSettings.selectedHomePage++;
//                    if (EepromSettings.selectedHomePage == 3) {
//                      EepromSettings.selectedHomePage = 0;
//                    }      
//                break;
//                
//                case (2):    // Voltage Scale
//                    EepromSettings.vbatScale--;
//                break;
//                
//                case (3):    // Volt Warning
//                    EepromSettings.vbatWarning++;
//                break;
//                
//                case (4):    // Save Screen
//                    EepromSettings.saveScreenOn++;
//                    #ifdef FENIX_QUADVERSITY
//                        if (EepromSettings.saveScreenOn == 6) {
//                    #else
//                        if (EepromSettings.saveScreenOn == 5) {
//                    #endif
//                      EepromSettings.saveScreenOn = 0;
//                    }                
//                break;
//                
//                case (5):    // Use OLED Screen
//                    EepromSettings.useOledScreen = !EepromSettings.useOledScreen; 
//                break;
//                
//                case (6):    // Use Fast Boot
//                    EepromSettings.useFastBoot = !EepromSettings.useFastBoot; 
//                break;
//
//                case (7):    // About
//                break;
//
//                case (8):    // Adv Settings
//                break;
//                
//                case (9):    // Use Fast Boot
//                  EepromSettings.buttonBeep = !EepromSettings.buttonBeep;               
//                break;
//                
//                case (10):    // CUSTOMLOGO
//                  EepromSettings.buttonBeep = !EepromSettings.buttonBeep;               
//                break;
//                
//                case (11):    // Invert Display
//                  EepromSettings.invertDisplay = !EepromSettings.invertDisplay;               
//                break;
//                
//                case (12):    // Rotate OLED
//                  EepromSettings.rotateOled = !EepromSettings.rotateOled;               
//                break;
//                
//            }
//          }
//        }
//
//}
