#include <stdint.h>
#include "state_chorus.h"
#include "ui.h"
#include "touchpad.h"
#include "protocol_chorus.h"
#include "lap_times.h"

#define RETURN_WHEN_RDY 1

/* TODO: Store these settings into EEPROM if not get from Chorus32 at boot */
static uint8_t DRAM_ATTR consecutives = 5;

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
    int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;
    uint8_t iter, range_valid_node, range_valid_lap, range_valid_consec;

    if (drawHeader())
        return;

    Ui::display.setCursor(UI_GET_MID_X(31), 12);
    Ui::display.print("== Chorus32 Laptimer control =="); // 31 chars

#define X_BASE_OFFSET   (20)
#define Y_BASE_OFFSET   (30)
#define X_OFFSET_VALUE  (X_BASE_OFFSET + (10 * Ui::CHAR_W))

    /************** NODE INDEX ***************/

#define NODE_X          (X_BASE_OFFSET)
#define NODE_Y          (Y_BASE_OFFSET)
// 'NODE IDX: -  0  +
#define NODE_X_OFFSET_DEC    (X_OFFSET_VALUE)
#define NODE_X_OFFSET_CNT    (NODE_X_OFFSET_DEC + 3*Ui::CHAR_W)
#define NODE_X_OFFSET_INC    (NODE_X_OFFSET_CNT + 3*Ui::CHAR_W)

    Ui::display.setCursor(NODE_X, NODE_Y);
    Ui::display.print("NODE IDX:");
    range_valid_node = (cursor_y > AREA_Y_START(NODE_Y) && cursor_y < AREA_Y_END(NODE_Y, 1));

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

#if RETURN_WHEN_RDY
        return; // No need to draw rest
#endif
    }

    /************** MIN LAP TIME ***************/

#define LAP_MIN_X               (X_BASE_OFFSET)
#define LAP_MIN_Y               (NODE_Y + 2*Ui::CHAR_H)
// 'MIN LAP:  - 10s +
#define LAP_MIN_X_OFFSET_DEC    (X_OFFSET_VALUE)
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

#if RETURN_WHEN_RDY
        return; // No need to draw rest
#endif
    }

    /*********** CONSECUTIVE LAPS *************/
#define CONSECUTIVE_X               (X_BASE_OFFSET)
#define CONSECUTIVE_Y               (LAP_MIN_Y + 2*Ui::CHAR_H)
// 'CONSECUT: -  0  +
#define CONSECUTIVE_X_OFFSET_DEC    (X_OFFSET_VALUE)
#define CONSECUTIVE_X_OFFSET_TIME   (CONSECUTIVE_X_OFFSET_DEC + 2*Ui::CHAR_W)
#define CONSECUTIVE_X_OFFSET_INC    (CONSECUTIVE_X_OFFSET_TIME + 4*Ui::CHAR_W)

    Ui::display.setCursor(CONSECUTIVE_X, CONSECUTIVE_Y);
    Ui::display.print("CONSECUT:");
    range_valid_consec = (cursor_y > AREA_Y_START(CONSECUTIVE_Y) &&
                          cursor_y < AREA_Y_END(CONSECUTIVE_Y, 1));

    // Dec min lap
    Ui::display.setCursor(CONSECUTIVE_X_OFFSET_DEC, CONSECUTIVE_Y);
    Ui::display.print("-");

    // Min lap time
    Ui::display.setCursor(CONSECUTIVE_X_OFFSET_TIME, CONSECUTIVE_Y);
    Ui::display.print(consecutives, 10, 2); // Base 10, 2 digits

    // Inc min lap
    Ui::display.setCursor(CONSECUTIVE_X_OFFSET_INC, CONSECUTIVE_Y);
    Ui::display.print("+");

    if (range_valid_consec && !range_valid_node && !range_valid_lap) {
        if ((cursor_x > AREA_X_START(CONSECUTIVE_X_OFFSET_DEC)) &&
            (cursor_x < AREA_X_END(CONSECUTIVE_X_OFFSET_DEC, 1))) {
            // Decrease
            Ui::display.rect(AREA_X_START(CONSECUTIVE_X_OFFSET_DEC), AREA_Y_START(CONSECUTIVE_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction && (1 < consecutives))
                consecutives--;
        } else if ((cursor_x > AREA_X_START(CONSECUTIVE_X_OFFSET_INC)) &&
                   (cursor_x < AREA_X_END(CONSECUTIVE_X_OFFSET_INC, 1))) {
            // Increase
            Ui::display.rect(AREA_X_START(CONSECUTIVE_X_OFFSET_INC), AREA_Y_START(CONSECUTIVE_Y),
                             AREA_X_LEN(1), AREA_Y_LEN(1), WHITE);
            if (tapAction && (99 > consecutives))
                consecutives++;
        }

#if RETURN_WHEN_RDY
        return; // No need to draw rest
#endif
    }

    /**************** BUTTONS *****************/

#define START_BTN_X (X_BASE_OFFSET)
#define START_BTN_Y (CONSECUTIVE_Y + 2*Ui::CHAR_H)
    Ui::display.setCursor(START_BTN_X, START_BTN_Y);
    if (chorus_race_is_started())
        Ui::display.print("STOP");
    else
        Ui::display.print("START");

#define READ_BTN_X (X_OFFSET_VALUE)
#define READ_BTN_Y START_BTN_Y
    Ui::display.setCursor(READ_BTN_X, READ_BTN_Y);
    Ui::display.print("GET LAPS");

#define RET_CURSOR_X (X_BASE_OFFSET)
#define RET_CURSOR_Y (START_BTN_Y + 2*Ui::CHAR_H)
    Ui::display.setCursor(RET_CURSOR_X, RET_CURSOR_Y);
    Ui::display.print("[ X ]");

    /**************** LAP TIMES *****************/

//#define LAPTIMES_X_POS 188 // max is 200
#define LAPTIMES_X_POS (X_BASE_OFFSET + (20 * Ui::CHAR_W)) // = 180
#define LAPTIMES_OFFSET (Ui::CHAR_H + 1U)

    uint32_t y_off = Y_BASE_OFFSET;
    char tmp_buff[16]; // '11) 00:00.000\n' => 13 chars
    uint8_t fastest = 0, num_laps = lapt_time_race_num_laps();

    Ui::display.setCursor(LAPTIMES_X_POS, y_off);
    Ui::display.print("LAP TIMES "); // "LAP TIMES [  1]", 15chars
    snprintf(tmp_buff, sizeof(tmp_buff), "[%3u]\n", lapt_time_race_idx_get());
    Ui::display.print(tmp_buff, chorus_race_is_started()); // inverted if race is started

    y_off += LAPTIMES_OFFSET;
    lap_time_t lap_time;
    for (iter = FIRST_LAP_IDX; iter <= num_laps; iter++, y_off += LAPTIMES_OFFSET) {
        lap_time = lapt_time_laptime_get(iter, fastest); // time in ms
        if (*((uint32_t*)&lap_time) == 0)
            break;

        Ui::display.setCursor(LAPTIMES_X_POS, y_off);

        snprintf(tmp_buff, sizeof(tmp_buff), "%2u) %02u:%02u.%03u\n",
                 (iter-1), lap_time.m, lap_time.s, lap_time.ms);
        Ui::display.print(tmp_buff, fastest); // invert colors if fastest
    }

    /****** SHOW BEST CONSECUTIVE ***/
#define BEST_AVG_X   (X_BASE_OFFSET)
#define BEST_AVG_X2  (BEST_AVG_X + 2*Ui::CHAR_W)
#define BEST_AVG_Y0  (Ui::YRES - 4*Ui::CHAR_H)
#define BEST_AVG_Y1  (Ui::YRES - 3*Ui::CHAR_H)
#define BEST_AVG_Y2  (Ui::YRES - 2*Ui::CHAR_H)

    // 'AVG : 00:00:1234'   = 16 chars
    // 'LAPS: 00...99'      = 13 chars
    if (1 < consecutives) {
        // fastest is reused for start lap index
        lap_time = lapt_time_best_consecutives_get(consecutives, fastest);

        Ui::display.setCursor(BEST_AVG_X, BEST_AVG_Y0);
        Ui::display.print("= = =  BEST  = = ="); // 18 chars
        Ui::display.setCursor(BEST_AVG_X2, BEST_AVG_Y1);
        Ui::display.print("LAPS: ");
        snprintf(tmp_buff, sizeof(tmp_buff), "%2u...%2u\n",
                 fastest, (fastest + consecutives - 1));
        Ui::display.print(tmp_buff);
        Ui::display.setCursor(BEST_AVG_X2, BEST_AVG_Y2);
        Ui::display.print("AVG : ");
        snprintf(tmp_buff, sizeof(tmp_buff), "%02u:%02u.%03u\n",
                 lap_time.m, lap_time.s, lap_time.ms);
        Ui::display.print(tmp_buff);
    }

    /**************** TOUCHES *****************/

#if !RETURN_WHEN_RDY
    if (range_valid_node || range_valid_lap || range_valid_consec)
        return;
#endif

    // Draw selection boxes and handle touches

    if (cursor_y > AREA_Y_START(START_BTN_Y) && cursor_y < AREA_Y_END(START_BTN_Y, 1) &&
        cursor_x > AREA_X_START(START_BTN_X) && cursor_x < AREA_X_END(START_BTN_X, 5))
    {
        // START/STOP
        Ui::display.rect(AREA_X_START(START_BTN_X), AREA_Y_START(START_BTN_Y),
                         AREA_X_LEN(5), AREA_Y_LEN(1), WHITE);
        if (tapAction) {
            if (chorus_race_is_started())
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
