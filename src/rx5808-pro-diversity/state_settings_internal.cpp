
#include "state_settings_internal.h"
#include "settings_eeprom.h"

#include "state.h"
#include "ui.h"
#include "voltage.h"

int8_t selectedInternalMenuItem = 0;
bool showChangeInternalMenuOptions = false;

uint8_t factoryReset = 0;

uint8_t menuInternalItems = 7; // Number of items in settingsInternalMenu[]
char* settingsInternalMenu[]={ 
    "Factory Reset",
    "spectatorFreqScanStep",
    "spectatorFWHM",
    "rssiSeekTreshold",
    "rssiMinTuneTime",
    "rssiHysteresis",
    "rssiHysteresisPeriod",
    };

void StateMachine::SettingsInternalStateHandler::onEnter() {
    selectedInternalMenuItem = 0;
    factoryReset = 0;
    showChangeInternalMenuOptions = false;
//    Ui::clear();
}

void StateMachine::SettingsInternalStateHandler::onExit() {
}

void StateMachine::SettingsInternalStateHandler::onUpdate() {
//    Ui::needUpdate();
}

void StateMachine::SettingsInternalStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void StateMachine::SettingsInternalStateHandler::onUpdateDraw() {


    if (!showChangeInternalMenuOptions) {
      
        int8_t index;
    
//        Ui::clear();
//        Ui::setTextSize(1);
        
//        Ui::setCursor(0, 8);
        index = selectedInternalMenuItem-2;
        if (index < 0) {
          index += menuInternalItems;
        }
//        Ui::display.print( settingsInternalMenu[index] );
        
//            Ui::setCursor(6, 18);
            index = selectedInternalMenuItem-1;
            if (index < 0) {
              index += menuInternalItems;
            }
//            Ui::display.print( settingsInternalMenu[index] );
            
//                Ui::setCursor(12, 28);
//                Ui::display.setTextColor(BLACK, WHITE); // 'inverted' text
//                Ui::display.print( settingsInternalMenu[selectedInternalMenuItem] );
//                Ui::display.setTextColor(WHITE, BLACK);
            
//            Ui::setCursor(6, 38);    
            index = selectedInternalMenuItem+1;
            if (index > menuInternalItems-1) {
              index -= menuInternalItems;
            }
//            Ui::display.print( settingsInternalMenu[index] );
        
//        Ui::setCursor(0, 48);
        index = selectedInternalMenuItem+2;
        if (index > menuInternalItems-1) {
          index -= menuInternalItems;
        }
//        Ui::display.print( settingsInternalMenu[index] );
    }
    
    if (showChangeInternalMenuOptions) {
//        Ui::fillRect(15, 20, 96, 24, BLACK);
//        Ui::drawRoundRect(15, 20, 96, 24, 2, WHITE);
//        Ui::setCursor(29, 28);

        switch(selectedInternalMenuItem) {
          
            case 0:    // Factory Reset
              if (factoryReset == 0) {
//                  Ui::display.print(PSTR2("    No     "));
              } else if (factoryReset == 1) {
//                  Ui::display.print(PSTR2("       No  "));
              } else if (factoryReset == 2) {
//                  Ui::display.print(PSTR2("  No       "));
              } else if (factoryReset == 3) {
//                  Ui::display.print(PSTR2("    Yes     "));
              } else if (factoryReset == 4) {
//                  Ui::display.print(PSTR2("  No       "));
              } else if (factoryReset == 5) {
//                  Ui::display.print(PSTR2("       No  "));
              }
            break;
       
            case 1:    // spectatorFreqScanStep
//                Ui::display.print(EepromSettings.spectatorFreqScanStep);
            break;
            
            case 2:    // spectatorFWHM
//                Ui::display.print(EepromSettings.spectatorFWHM);
            break;
            
            case 3:    // rssiSeekTreshold
//                Ui::display.print(EepromSettings.rssiSeekTreshold);
            break;
            
            case 4:    // rssiMinTuneTime
//                Ui::display.print(EepromSettings.rssiMinTuneTime);
            break;
            
            case 5:    // rssiHysteresis
//                Ui::display.print(EepromSettings.rssiHysteresis);
            break;
            
            case 6:    // rssiHysteresisPeriod
//                Ui::display.print(EepromSettings.rssiHysteresisPeriod);
            break;
            
            case 7:    // 
            break;
            
        }    
    }
        
//    Ui::needDisplay();

}

//void StateMachine::SettingsInternalStateHandler::onButtonChange(
//    Button button,
//    Buttons::PressType pressType
//)  {
//  
//  if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::MODE_PRESSED
//     ) {
//          if (showChangeInternalMenuOptions) {
//              showChangeInternalMenuOptions = false;
//              if (factoryReset == 3) {
//                EepromSettings.initDefaults(); 
//              }
//              EepromSettings.save();    
//              delay(250); // wait for eeprom to finish writing
//              nvic_sys_reset();
//          } else {
//              showChangeInternalMenuOptions = true;        
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
//          if (!showChangeInternalMenuOptions) {
//              selectedInternalMenuItem--;
//              if (selectedInternalMenuItem < 0)
//                selectedInternalMenuItem += menuInternalItems;
//          }    
//          if (showChangeInternalMenuOptions) {   
//    
//            switch(selectedInternalMenuItem) {
//                
//                case (0):    // Factry Reset 
//                    factoryReset--;
//                    if (factoryReset > 5) {
//                      factoryReset = 5;
//                    }                         
//                break;
//           
//                case 1:    // spectatorFreqScanStep
//                    EepromSettings.spectatorFreqScanStep--;
//                break;
//                
//                case 2:    // spectatorFWHM
//                    EepromSettings.spectatorFWHM--;
//                break;
//                
//                case 3:    // rssiSeekTreshold
//                    EepromSettings.rssiSeekTreshold--;
//                break;
//                
//                case 4:    // rssiMinTuneTime
//                    EepromSettings.rssiMinTuneTime--;
//                break;
//                
//                case 5:    // rssiHysteresis
//                    EepromSettings.rssiHysteresis--;
//                break;
//                
//                case 6:    // rssiHysteresisPeriod
//                    EepromSettings.rssiHysteresisPeriod--;
//                break;
//
//                case (7):    // 
//                break;
//                
//            }  
//          }
//        }
//  else if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::DOWN_PRESSED
//     ) {
//          if (!showChangeInternalMenuOptions) {
//              selectedInternalMenuItem++;
//              if (selectedInternalMenuItem > menuInternalItems-1)
//                selectedInternalMenuItem -= menuInternalItems;
//          } 
//          if (showChangeInternalMenuOptions) {   
//    
//            switch(selectedInternalMenuItem) {
//                
//                case (0):    // Factry Reset 
//                    factoryReset++;
//                    if (factoryReset > 5) {
//                      factoryReset = 0;
//                    }     
//                break;
//           
//                case 1:    // spectatorFreqScanStep
//                    EepromSettings.spectatorFreqScanStep++;
//                break;
//                
//                case 2:    // spectatorFWHM
//                    EepromSettings.spectatorFWHM++;
//                break;
//                
//                case 3:    // rssiSeekTreshold
//                    EepromSettings.rssiSeekTreshold++;
//                break;
//                
//                case 4:    // rssiMinTuneTime
//                    EepromSettings.rssiMinTuneTime++;
//                break;
//                
//                case 5:    // rssiHysteresis
//                    EepromSettings.rssiHysteresis++;
//                break;
//                
//                case 6:    // rssiHysteresisPeriod
//                    EepromSettings.rssiHysteresisPeriod++;
//                break;
//
//                case (7):    // 
//                break;
//                
//            }
//          }
//        }
//
//}
