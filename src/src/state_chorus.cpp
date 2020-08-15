#include <stdint.h>
#include "state_chorus.h"
#include "ui.h"
#include "touchpad.h"
#include "protocol_chorus.h"
#include "lap_times.h"

#define SPACE_BEF 4
#define SPACE_AFT 3


void StateMachine::ChorusStateHandler::onEnter()
{
}

void StateMachine::ChorusStateHandler::onUpdate()
{
    onUpdateDraw();
    if (TouchPad::touchData.buttonPrimary) {
        TouchPad::touchData.buttonPrimary = false;
        this->doTapAction();
    }
}

void StateMachine::ChorusStateHandler::onInitialDraw()
{
    onUpdateDraw();
}

void StateMachine::ChorusStateHandler::onUpdateDraw()
{
    uint32_t x_off;
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t iter, range_valid;
    drawHeader();

    Ui::display.setTextColor(WHITE);
    Ui::display.setCursor(UI_GET_MID_X(31), 12);
    Ui::display.print("== Chorus32 Laptimer control =="); // 31 chars

#define NODE_X (30)
#define NODE_Y (30)
    x_off = NODE_X;
    Ui::display.setCursor(x_off, NODE_Y);
    Ui::display.print("NODE:");
    x_off += 7 * Ui::CHAR_W;
    range_valid = (cursor_y > (NODE_Y - SPACE_BEF) && cursor_y < (NODE_Y + Ui::CHAR_H + SPACE_AFT));
    for (iter = 0; iter < MAX_NODES; iter++, x_off+=(2*Ui::CHAR_W)) {
        Ui::display.setCursor(x_off, x_off);
        Ui::display.print(iter, (iter == lap_times_nodeidx_get())); // invert current

        if (range_valid &&
            cursor_x > (x_off - SPACE_BEF) && cursor_x < (x_off + Ui::CHAR_W + SPACE_AFT))
        {
            // START/STOP
            Ui::display.rect((x_off - SPACE_BEF),
                             (NODE_Y - SPACE_BEF),
                             (SPACE_BEF + SPACE_AFT + Ui::CHAR_W),
                             (SPACE_BEF + SPACE_AFT + Ui::CHAR_H), WHITE);
        }
    }

#define START_BTN_X (30)
#define START_BTN_Y (60)
    Ui::display.setCursor(START_BTN_X, START_BTN_Y);
    if (chorus_race_is_start())
        Ui::display.print("STOP");
    else
        Ui::display.print("START");

#define RET_CURSOR_X (Ui::XRES - 6 * Ui::CHAR_W)
#define RET_CURSOR_Y (20)
    Ui::display.setCursor(RET_CURSOR_X, RET_CURSOR_Y);
    Ui::display.print("[ X ]");

    // Draw selection box
    if (cursor_y > (START_BTN_Y - SPACE_BEF) && cursor_y < (START_BTN_Y + Ui::CHAR_H + SPACE_AFT) &&
        cursor_x > (START_BTN_X - SPACE_BEF) && cursor_x < (START_BTN_X + 5 * Ui::CHAR_W + SPACE_AFT))
    {
        // START/STOP
        Ui::display.rect((START_BTN_X - SPACE_BEF),
                         (START_BTN_Y - SPACE_BEF),
                         (SPACE_BEF + SPACE_AFT + 5 * Ui::CHAR_W),
                         (SPACE_BEF + SPACE_AFT + Ui::CHAR_H), WHITE);
    }
    else if (cursor_y > (RET_CURSOR_Y - SPACE_BEF) && cursor_y < (RET_CURSOR_Y + Ui::CHAR_H + SPACE_AFT) &&
             cursor_x > (RET_CURSOR_X - SPACE_BEF) && cursor_x < (RET_CURSOR_X + 5 * Ui::CHAR_W + SPACE_AFT))
    {
        Ui::display.rect((RET_CURSOR_X - SPACE_BEF),
                         (RET_CURSOR_Y - SPACE_BEF),
                         (SPACE_BEF + SPACE_AFT + 5 * Ui::CHAR_W),
                         (SPACE_BEF + SPACE_AFT + Ui::CHAR_H), WHITE);
    }
}

void StateMachine::ChorusStateHandler::doTapAction()
{
    uint32_t x_off;
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t iter;

    if (cursor_x > 314 && cursor_y < 8)
    { // Menu
        StateMachine::switchState(StateMachine::State::MENU);
    }
    else if (cursor_y > (START_BTN_Y - SPACE_BEF) && cursor_y < (START_BTN_Y + Ui::CHAR_H + SPACE_AFT) &&
             cursor_x > (START_BTN_X - SPACE_BEF) && cursor_x < (START_BTN_X + 5 * Ui::CHAR_W + SPACE_AFT))
    {
        if (chorus_race_is_start())
            chorus_race_end();
        else
            chorus_race_start();
    }
    else if (cursor_y > (RET_CURSOR_Y - SPACE_BEF) && cursor_y < (RET_CURSOR_Y + Ui::CHAR_H + SPACE_AFT) &&
             cursor_x > (RET_CURSOR_X - SPACE_BEF) && cursor_x < (RET_CURSOR_X + 5 * Ui::CHAR_W + SPACE_AFT))
    {
        StateMachine::switchState(StateMachine::lastState);
    }
    else if (cursor_y > (NODE_Y - SPACE_BEF) && cursor_y < (NODE_Y + Ui::CHAR_H + SPACE_AFT))
    {
        x_off = NODE_X + 7 * Ui::CHAR_W;
        for (iter = 0; iter < MAX_NODES; iter++, x_off+=(2*Ui::CHAR_W)) {
            if (cursor_x > (x_off - SPACE_BEF) && cursor_x < (x_off + Ui::CHAR_W + SPACE_AFT))
            {
                lap_times_nodeidx_set(iter);
                break;
            }
        }
    }
}
