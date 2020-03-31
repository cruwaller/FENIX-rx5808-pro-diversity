#include <stdint.h>
#include "state_expresslrs.h"
#include "ui.h"
#include "touchpad.h"
    
void StateMachine::ExLRSStateHandler::onEnter() {    
}

void StateMachine::ExLRSStateHandler::onUpdate() {
    onUpdateDraw();
    doTapAction();
}

void StateMachine::ExLRSStateHandler::doTapAction() {
}

void StateMachine::ExLRSStateHandler::onInitialDraw() {
    onUpdateDraw();
}

void StateMachine::ExLRSStateHandler::onUpdateDraw() {    
    Ui::display.setTextColor(100);
    Ui::display.setCursor( 40, 40);
    Ui::display.print("ExLRSStateHandler");
    Ui::display.setCursor( 40, 50);
}