#include <stdint.h>
#include "state_chorus.h"
#include "ui.h"
#include "touchpad.h"
#include "protocol_chorus.h"


void StateMachine::ChorusStateHandler::onEnter()
{
}

void StateMachine::ChorusStateHandler::onUpdate()
{
    if (TouchPad::touchData.buttonPrimary) {
        TouchPad::touchData.buttonPrimary = false;
        this->doTapAction();
    }
    onUpdateDraw();
}

void StateMachine::ChorusStateHandler::onInitialDraw()
{
    onUpdateDraw();
}

void StateMachine::ChorusStateHandler::onUpdateDraw()
{
    drawHeader();

    Ui::display.setTextColor(100);
    Ui::display.setCursor(10, 12);
    Ui::display.print("Chorus Laptimer control!");

    Ui::display.setCursor(30, 30);
    Ui::display.print("START");

    Ui::display.setCursor(130, 30);
    Ui::display.print("STOP");

    // Draw selection box
    if (TouchPad::touchData.cursorY > (30 - 4) && TouchPad::touchData.cursorY < (30 + 8 + 3))
    {
        if (TouchPad::touchData.cursorX > (30 - 4) && TouchPad::touchData.cursorX < (30 + 5 * 8 + 3))
        {
            Ui::display.rect(26, 26, 4+3+5*8, 15, 100);
        }
        else if (TouchPad::touchData.cursorX > (130-4) && TouchPad::touchData.cursorX < (130 + 4 * 8 + 3))
        {
            Ui::display.rect(126, 26, 4+3+4*8, 15, 100);
        }
    }
}

void StateMachine::ChorusStateHandler::doTapAction()
{
    if (TouchPad::touchData.cursorX > 314 && TouchPad::touchData.cursorY < 8)
    { // Menu
        StateMachine::switchState(StateMachine::State::MENU);
    }
    else if (TouchPad::touchData.cursorY > (30 - 4) && TouchPad::touchData.cursorY < (30 + 8 + 3))
    {
        if (TouchPad::touchData.cursorX > (30 - 4) && TouchPad::touchData.cursorX < (30 + 5 * 8 + 3))
        {
            chorus_race_start();
        }
        else if (TouchPad::touchData.cursorX > (126) && TouchPad::touchData.cursorX < (130 + 4 * 8 + 3))
        {
            chorus_race_end();
        }
    }
}
