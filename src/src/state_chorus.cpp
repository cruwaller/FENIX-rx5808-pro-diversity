#include <stdint.h>
#include "state_chorus.h"
#include "ui.h"
#include "touchpad.h"
#include "protocol_chorus.h"
#include "lap_times.h"


void StateMachine::ChorusStateHandler::onEnter()
{
}

void StateMachine::ChorusStateHandler::onUpdate()
{
    onUpdateDraw(TouchPad::touchData.buttonPrimary);
    TouchPad::touchData.buttonPrimary = false;
    /*if (TouchPad::touchData.buttonPrimary) {
        TouchPad::touchData.buttonPrimary = false;
        this->doTapAction();
    }*/
}

void StateMachine::ChorusStateHandler::onInitialDraw()
{
    onUpdateDraw(false);
}

void StateMachine::ChorusStateHandler::onUpdateDraw(uint8_t tapAction)
{
    uint32_t x_off;
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t iter, range_valid_node, range_valid_lap;
    drawHeader();

    Ui::display.setTextColor(WHITE);
    Ui::display.setCursor(UI_GET_MID_X(31), 12);
    Ui::display.print("== Chorus32 Laptimer control =="); // 31 chars

#define X_BASE_OFF      (30)

#define NODE_X          (X_BASE_OFF)
#define NODE_Y          (30)
#define NODE_X_OFFSET   (10 * Ui::CHAR_W)

    x_off = NODE_X;
    Ui::display.setCursor(x_off, NODE_Y);
    Ui::display.print("NODE IDX:");
    x_off += NODE_X_OFFSET;
    range_valid_node = (cursor_y > AREA_Y_START(NODE_Y) && cursor_y < AREA_Y_END(NODE_Y, 1));
    for (iter = 0; iter < MAX_NODES; iter++, x_off+=(2*Ui::CHAR_W)) {
        Ui::display.setCursor(x_off, NODE_Y);
        Ui::display.print(iter, 10, 1, (iter == lap_times_nodeidx_get())); // invert current

        if (range_valid_node &&
            cursor_x > AREA_X_START(x_off) && cursor_x < AREA_X_END(x_off, 1))
        {
            Ui::display.rect(AREA_X_START(x_off), AREA_Y_START(NODE_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction) {
                lap_times_nodeidx_set(iter);
            }
        }
    }

#define LAP_MIN_X               (X_BASE_OFF)
#define LAP_MIN_Y               (40)
// 'MIN LAP:  [-]  10s  [+]
#define LAP_MIN_X_OFFSET_DEC    (LAP_MIN_X + NODE_X_OFFSET)
#define LAP_MIN_X_OFFSET_TIME   (LAP_MIN_X_OFFSET_DEC + 5*Ui::CHAR_W)
#define LAP_MIN_X_OFFSET_INC    (LAP_MIN_X_OFFSET_TIME + 5*Ui::CHAR_W)

    Ui::display.setCursor(LAP_MIN_X, LAP_MIN_Y);
    Ui::display.print("MIN LAP:");
    range_valid_lap = (cursor_y > AREA_Y_START(LAP_MIN_Y) && cursor_y < AREA_Y_END(LAP_MIN_Y, 1));

    // Dec min lap
    Ui::display.setCursor(LAP_MIN_X_OFFSET_DEC, LAP_MIN_Y);
    Ui::display.print("[-]");

    // Min lap time
    Ui::display.setCursor(LAP_MIN_X_OFFSET_TIME, LAP_MIN_Y);
    Ui::display.print(chorus_race_lap_time_min(), 10, 2); // Base 10, 2 digits
    Ui::display.print("s");

    // Inc min lap
    Ui::display.setCursor(LAP_MIN_X_OFFSET_INC, LAP_MIN_Y);
    Ui::display.print("[+]");

    if (!range_valid_node && range_valid_lap) {
        if ((cursor_x > (LAP_MIN_X_OFFSET_DEC - SPACE_BEF)) &&
            (cursor_x < (LAP_MIN_X_OFFSET_DEC + 3 * Ui::CHAR_W + SPACE_AFT))) {
            // Decrease
            Ui::display.rect(AREA_X_START(LAP_MIN_X_OFFSET_DEC), AREA_Y_START(LAP_MIN_Y),
                             AREA_X_LEN(3), AREA_Y_LEN(1), WHITE);
            if (tapAction)
                chorus_race_lap_time_min_change(-1);
        } else if ((cursor_x > (LAP_MIN_X_OFFSET_INC - SPACE_BEF)) &&
                   (cursor_x < (LAP_MIN_X_OFFSET_INC + 3 * Ui::CHAR_W + SPACE_AFT))) {
            // Increase
            Ui::display.rect(AREA_X_START(LAP_MIN_X_OFFSET_INC), AREA_Y_START(LAP_MIN_Y),
                             AREA_X_LEN(3), AREA_Y_LEN(1), WHITE);
            if (tapAction)
                chorus_race_lap_time_min_change(1);
        }
    }


#define START_BTN_X (X_BASE_OFF)
#define START_BTN_Y (60)
    Ui::display.setCursor(START_BTN_X, START_BTN_Y);
    if (chorus_race_is_start())
        Ui::display.print("STOP");
    else
        Ui::display.print("START");

#define READ_BTN_X (X_BASE_OFF + NODE_X_OFFSET)
#define READ_BTN_Y START_BTN_Y
    Ui::display.setCursor(READ_BTN_X, READ_BTN_Y);
    Ui::display.print("GET LAPS");


#define RET_CURSOR_X (Ui::XRES - 6 * Ui::CHAR_W)
#define RET_CURSOR_Y (20)
    Ui::display.setCursor(RET_CURSOR_X, RET_CURSOR_Y);
    Ui::display.print("[ X ]");

    if (range_valid_node || range_valid_lap)
        return;

    // Draw selection boxes and handle touches

    if (cursor_x > 314 && cursor_y < 8)
    {
        // Menu
        if (tapAction)
            StateMachine::switchState(StateMachine::State::MENU);
    }
    else if (cursor_y > AREA_Y_START(START_BTN_Y) && cursor_y < AREA_Y_END(START_BTN_Y, 1) &&
             cursor_x > AREA_X_START(START_BTN_X) && cursor_x < AREA_X_END(START_BTN_X, 5))
    {
        // START/STOP
        Ui::display.rect(AREA_X_START(START_BTN_X), AREA_Y_START(START_BTN_Y),
                         AREA_X_LEN(5), AREA_Y_LEN(1), WHITE);
        if (tapAction) {
            if (chorus_race_is_start())
                chorus_race_end();
            else
                chorus_race_start();
        }
    }
    else if (cursor_y > AREA_Y_START(READ_BTN_Y) && cursor_y < AREA_Y_END(READ_BTN_Y, 1) &&
             cursor_x > (READ_BTN_X - SPACE_BEF) && cursor_x < AREA_X_END(READ_BTN_X, 8))
    {
        // GET LAPS
        Ui::display.rect(AREA_X_START(READ_BTN_X), AREA_X_START(READ_BTN_Y),
                         AREA_X_LEN(8), AREA_Y_LEN(1), WHITE);
        if (tapAction)
            chorus_race_laps_get();
    }
    else if (cursor_y > AREA_Y_START(RET_CURSOR_Y) && cursor_y < AREA_Y_END(RET_CURSOR_Y, 1) &&
             cursor_x > (RET_CURSOR_X - SPACE_BEF) && cursor_x < AREA_X_END(RET_CURSOR_X, 5))
    {
        Ui::display.rect(AREA_X_START(RET_CURSOR_X), AREA_X_START(RET_CURSOR_Y),
                         AREA_X_LEN(5), AREA_Y_LEN(1), WHITE);
        if (tapAction)
            StateMachine::switchState(StateMachine::lastState);
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
    else if (cursor_y > (READ_BTN_Y - SPACE_BEF) && cursor_y < (READ_BTN_Y + Ui::CHAR_H + SPACE_AFT) &&
             cursor_x > (READ_BTN_X - SPACE_BEF) && cursor_x < (READ_BTN_X + 8 * Ui::CHAR_W + SPACE_AFT))
    {
        // GET LAPS
        chorus_race_laps_get();
    }
    else if (cursor_y > (RET_CURSOR_Y - SPACE_BEF) && cursor_y < (RET_CURSOR_Y + Ui::CHAR_H + SPACE_AFT) &&
             cursor_x > (RET_CURSOR_X - SPACE_BEF) && cursor_x < (RET_CURSOR_X + 5 * Ui::CHAR_W + SPACE_AFT))
    {
        StateMachine::switchState(StateMachine::lastState);
    }
    else if (cursor_y > (LAP_MIN_Y - SPACE_BEF) && cursor_y < (LAP_MIN_Y + Ui::CHAR_H + SPACE_AFT))
    {
        // Minimum lap time
        if ((cursor_x > (LAP_MIN_X_OFFSET_DEC - SPACE_BEF)) &&
            (cursor_x < (LAP_MIN_X_OFFSET_DEC + 3 * Ui::CHAR_W + SPACE_AFT))) {
            // Decrease
            chorus_race_lap_time_min_change(-1);
        } else if ((cursor_x > (LAP_MIN_X_OFFSET_INC - SPACE_BEF)) &&
                   (cursor_x < (LAP_MIN_X_OFFSET_INC + 3 * Ui::CHAR_W + SPACE_AFT))) {
            // Increase
            chorus_race_lap_time_min_change(1);
        }
    }
    else if (cursor_y > (NODE_Y - SPACE_BEF) && cursor_y < (NODE_Y + Ui::CHAR_H + SPACE_AFT))
    {
        x_off = NODE_X + NODE_X_OFFSET;
        for (iter = 0; iter < MAX_NODES; iter++, x_off+=(2*Ui::CHAR_W)) {
            if (cursor_x > (x_off - SPACE_BEF) && cursor_x < (x_off + Ui::CHAR_W + SPACE_AFT))
            {
                lap_times_nodeidx_set(iter);
                break;
            }
        }
    }
}
