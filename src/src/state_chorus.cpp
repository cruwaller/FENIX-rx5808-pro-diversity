#include <stdint.h>
#include "state_chorus.h"
#include "ui.h"
#include "touchpad.h"
#include "protocol_chorus.h"
#include "lap_times.h"


void StateMachine::ChorusStateHandler::onEnter()
{
    //onUpdateDraw(false);
}


void StateMachine::ChorusStateHandler::onUpdate()
{
    onUpdateDraw(TouchPad::touchData.buttonPrimary);
}


void StateMachine::ChorusStateHandler::onUpdateDraw(uint8_t tapAction)
{
    uint32_t x_off;
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t iter, range_valid_node, range_valid_lap;

    if (drawHeader())
        return;

    Ui::display.setCursor(UI_GET_MID_X(31), 12);
    Ui::display.print("== Chorus32 Laptimer control =="); // 31 chars

#define X_BASE_OFF      (20)

#define NODE_X          (X_BASE_OFF)
#define NODE_Y          (30)
#define NODE_X_OFFSET   (10 * Ui::CHAR_W)

    x_off = NODE_X;
    Ui::display.setCursor(x_off, NODE_Y);
    Ui::display.print("NODE IDX:");
    x_off += NODE_X_OFFSET;
    range_valid_node = (cursor_y > AREA_Y_START(NODE_Y) && cursor_y < AREA_Y_END(NODE_Y, 1));
#if 0
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
#else
    (void)iter;
// 'NODE IDX: -  0  +
#define NODE_X_OFFSET_DEC    (NODE_X + NODE_X_OFFSET)
#define NODE_X_OFFSET_CNT    (NODE_X_OFFSET_DEC + 3*Ui::CHAR_W)
#define NODE_X_OFFSET_INC    (NODE_X_OFFSET_CNT + 3*Ui::CHAR_W)
    // Dec node
    Ui::display.setCursor(NODE_X_OFFSET_DEC, NODE_Y);
    Ui::display.print("-");
    // Node index
    Ui::display.setCursor(NODE_X_OFFSET_CNT, NODE_Y);
    Ui::display.print(lap_times_nodeidx_get(), 10, 1); // Base 10, 1 digits
    // Inc node
    Ui::display.setCursor(NODE_X_OFFSET_INC, NODE_Y);
    Ui::display.print("+");

    if (range_valid_node) {
        if ((cursor_x > AREA_X_START(NODE_X_OFFSET_DEC)) &&
            (cursor_x < AREA_X_END(NODE_X_OFFSET_DEC, 1))) {
            // Decrease
            Ui::display.rect(AREA_X_START(NODE_X_OFFSET_DEC), AREA_Y_START(NODE_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction)
                lap_times_nodeidx_roll(-1);
        } else if ((cursor_x > AREA_X_START(NODE_X_OFFSET_INC)) &&
                   (cursor_x < AREA_X_END(NODE_X_OFFSET_INC, 1))) {
            // Increase
            Ui::display.rect(AREA_X_START(NODE_X_OFFSET_INC), AREA_Y_START(NODE_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction)
                lap_times_nodeidx_roll(1);
        }
    }

#endif

#define LAP_MIN_X               (X_BASE_OFF)
#define LAP_MIN_Y               (NODE_Y + 2*Ui::CHAR_H)
// 'MIN LAP:  - 10s +
#define LAP_MIN_X_OFFSET_DEC    (LAP_MIN_X + NODE_X_OFFSET)
#define LAP_MIN_X_OFFSET_TIME   (LAP_MIN_X_OFFSET_DEC + 2*Ui::CHAR_W)
#define LAP_MIN_X_OFFSET_INC    (LAP_MIN_X_OFFSET_TIME + 4*Ui::CHAR_W)

    Ui::display.setCursor(LAP_MIN_X, LAP_MIN_Y);
    Ui::display.print("MIN LAP:");
    range_valid_lap = (cursor_y > AREA_Y_START(LAP_MIN_Y) && cursor_y < AREA_Y_END(LAP_MIN_Y, 1));

    // Dec min lap
    Ui::display.setCursor(LAP_MIN_X_OFFSET_DEC, LAP_MIN_Y);
    Ui::display.print("-");

    // Min lap time
    Ui::display.setCursor(LAP_MIN_X_OFFSET_TIME, LAP_MIN_Y);
    Ui::display.print(chorus_race_lap_time_min(), 10, 2); // Base 10, 2 digits
    Ui::display.print("s");

    // Inc min lap
    Ui::display.setCursor(LAP_MIN_X_OFFSET_INC, LAP_MIN_Y);
    Ui::display.print("+");

    if (!range_valid_node && range_valid_lap) {
        if ((cursor_x > AREA_X_START(LAP_MIN_X_OFFSET_DEC)) &&
            (cursor_x < AREA_X_END(LAP_MIN_X_OFFSET_DEC, 1))) {
            // Decrease
            Ui::display.rect(AREA_X_START(LAP_MIN_X_OFFSET_DEC), AREA_Y_START(LAP_MIN_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction)
                chorus_race_lap_time_min_change(-1);
        } else if ((cursor_x > AREA_X_START(LAP_MIN_X_OFFSET_INC)) &&
                   (cursor_x < AREA_X_END(LAP_MIN_X_OFFSET_INC, 1))) {
            // Increase
            Ui::display.rect(AREA_X_START(LAP_MIN_X_OFFSET_INC), AREA_Y_START(LAP_MIN_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction)
                chorus_race_lap_time_min_change(1);
        }
    }


#define START_BTN_X (X_BASE_OFF)
#define START_BTN_Y (LAP_MIN_Y + 2*Ui::CHAR_H)
    Ui::display.setCursor(START_BTN_X, START_BTN_Y);
    if (chorus_race_is_start())
        Ui::display.print("STOP");
    else
        Ui::display.print("START");

#define READ_BTN_X (X_BASE_OFF + NODE_X_OFFSET)
#define READ_BTN_Y START_BTN_Y
    Ui::display.setCursor(READ_BTN_X, READ_BTN_Y);
    Ui::display.print("GET LAPS");


//#define RET_CURSOR_X (Ui::XRES - 6 * Ui::CHAR_W)
//#define RET_CURSOR_Y (20)
#define RET_CURSOR_X (X_BASE_OFF)
#define RET_CURSOR_Y (START_BTN_Y + 2*Ui::CHAR_H)
    Ui::display.setCursor(RET_CURSOR_X, RET_CURSOR_Y);
    Ui::display.print("[ X ]");

    if (range_valid_node || range_valid_lap)
        return;

    // Draw selection boxes and handle touches

    if (cursor_y > AREA_Y_START(START_BTN_Y) && cursor_y < AREA_Y_END(START_BTN_Y, 1) &&
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
