//#include "state.h"
//#include "state_menu.h"
//#include "ui.h"
//#include "ui_menu.h"
//#include "buttons.h"
//#include "bitmaps.h"
//#include "settings_eeprom.h"
//
//static void homeMenuHandler();
//static void searchMenuHandler();
//static void bandScannerMenuHandler();
//static void spectatorMenuHandler();
//static void settingsMenuHandler();
//static void favouritesMenuHandler();
//static void finderMenuHandler();
//static void calibrationMenuHandler();
//
//void StateMachine::MenuStateHandler::onEnter() {
//    this->menu.reset();
//    this->menu.addItem(PSTR("Home"), homeIcon, homeMenuHandler);
//    this->menu.addItem(PSTR("Favourites"), starIcon, favouritesMenuHandler);  
//    this->menu.addItem(PSTR("Search"), searchIcon, searchMenuHandler);
//    this->menu.addItem(PSTR("Spectrum"), bandScanIcon, bandScannerMenuHandler);
//    this->menu.addItem(PSTR("Spectator"), spectatorIcon, spectatorMenuHandler);
//    this->menu.addItem(PSTR("Finder"), finderIcon, finderMenuHandler);
//    this->menu.addItem(PSTR("Calibrate"), calibrationIcon, calibrationMenuHandler);
//    this->menu.addItem(PSTR("Settings"), settingsIcon, settingsMenuHandler);
//}
//  
//void StateMachine::MenuStateHandler::onButtonChange(
//    Button button,
//    Buttons::PressType pressType
//) {
//    if (pressType != Buttons::PressType::SHORT)
//        return;
//
//    switch (button) {
//        case Button::UP_PRESSED:
//            this->menu.selectPreviousItem();
//            Ui::needUpdate();
//            break;
//
//        case Button::DOWN_PRESSED:
//            this->menu.selectNextItem();
//            Ui::needUpdate();
//            break;
//
//        case Button::MODE_PRESSED:
//            this->menu.activateItem();
//            break;
//    }
//}
//
//
//static void homeMenuHandler() {
//    switch(EepromSettings.selectedHomePage) {
//        case 0:
//          StateMachine::switchState(StateMachine::State::HOME);
//        break;
//        case 1:
////          StateMachine::switchState(StateMachine::State::HOME_SIMPLE);
//          StateMachine::switchState(StateMachine::State::HOME);
//        break;
//        case 2:
////          StateMachine::switchState(StateMachine::State::HOME_STATS);
//          StateMachine::switchState(StateMachine::State::HOME);
//        break;
//    }  
//}
//
////static void searchMenuHandler() {
////    StateMachine::switchState(StateMachine::State::SEARCH);
////};
////
////static void bandScannerMenuHandler() {
////    StateMachine::switchState(StateMachine::State::BANDSCAN);
////};
////
////static void spectatorMenuHandler() {
////    StateMachine::switchState(StateMachine::State::SPECTATOR);
////};
//
//static void settingsMenuHandler() {
//    StateMachine::switchState(StateMachine::State::SETTINGS);
//};
//
////static void favouritesMenuHandler() {
////    StateMachine::switchState(StateMachine::State::FAVOURITES);
////};
////
////static void finderMenuHandler() {
////    StateMachine::switchState(StateMachine::State::FINDER);
////};
//
//static void calibrationMenuHandler() {
//    StateMachine::switchState(StateMachine::State::SETTINGS_RSSI);
//};

