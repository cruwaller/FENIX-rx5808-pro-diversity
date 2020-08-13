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
    Ui::display.setCursor(UI_GET_MID_X(31), 12);
    Ui::display.print("== Chorus32 Laptimer control =="); // 31 chars

    Ui::display.setCursor(30, 30);
    if (chorus_race_is_start())
        Ui::display.print("STOP");
    else
        Ui::display.print("START");

#define RET_CURSOR_X (Ui::XRES - 6*Ui::CHAR_W)
#define RET_CURSOR_Y (20)
    Ui::display.setCursor(RET_CURSOR_X, RET_CURSOR_Y);
    Ui::display.print("[ X ]");

    // Draw selection box
    if (TouchPad::touchData.cursorY > (30 - 4) && TouchPad::touchData.cursorY < (30 + Ui::CHAR_H + 3) &&
        TouchPad::touchData.cursorX > (30 - 4) && TouchPad::touchData.cursorX < (30 + 5 * Ui::CHAR_W + 3))
    {
        // START/STOP
        Ui::display.rect(26, 26, (4 + 3 + 5 * Ui::CHAR_W), (4 + 3 + Ui::CHAR_H), 100);
    }
    else if (TouchPad::touchData.cursorY > (RET_CURSOR_Y - 4) && TouchPad::touchData.cursorY < (RET_CURSOR_Y + Ui::CHAR_H + 3) &&
             TouchPad::touchData.cursorX > (RET_CURSOR_X - 4) && TouchPad::touchData.cursorX < (RET_CURSOR_X + 5 * Ui::CHAR_W + 3))
    {
        Ui::display.rect((RET_CURSOR_X - 4), (RET_CURSOR_Y - 4), (4 + 3 + 5 * Ui::CHAR_W), (4 + 3 + Ui::CHAR_H), 100);
    }
}

void StateMachine::ChorusStateHandler::doTapAction()
{
    if (TouchPad::touchData.cursorX > 314 && TouchPad::touchData.cursorY < 8)
    { // Menu
        StateMachine::switchState(StateMachine::State::MENU);
    }
    else if (TouchPad::touchData.cursorY > (30 - 4) && TouchPad::touchData.cursorY < (30 + Ui::CHAR_H + 3) &&
             TouchPad::touchData.cursorX > (30 - 4) && TouchPad::touchData.cursorX < (30 + 5 * Ui::CHAR_W + 3))
    {
        if (chorus_race_is_start())
            chorus_race_end();
        else
            chorus_race_start();
    }
    else if (TouchPad::touchData.cursorY > (RET_CURSOR_Y - 4) && TouchPad::touchData.cursorY < (RET_CURSOR_Y + Ui::CHAR_H + 3) &&
             TouchPad::touchData.cursorX > (RET_CURSOR_X - 4) && TouchPad::touchData.cursorX < (RET_CURSOR_X + 5 * Ui::CHAR_W + 3))
    {
        StateMachine::switchState(StateMachine::lastState);
    }
}
