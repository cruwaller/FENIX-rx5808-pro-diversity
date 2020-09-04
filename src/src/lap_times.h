#ifndef _LAP_TIMES_H_
#define _LAP_TIMES_H_

#include <stdint.h>

#define MAX_LAP_TIMES   32
#define MAX_NODES       6

// [0] !! NA !!
// [1] start time (from start of the race to pass gate 1st time)
// [2] first lap time
#define SKIP_INIT     1 // Skip time to pass gate first time (start of the lap 1)
#define FIRST_LAP_IDX (1U+SKIP_INIT)


enum {
  ESPNOW_TYPE_INVALID,
  ESPNOW_TYPE_RACE_START,
  ESPNOW_TYPE_RACE_STOP,
  ESPNOW_TYPE_LAP_TIME,
};

typedef struct {
  uint32_t lap_time;
  uint16_t race_id;
  uint8_t lap;
  uint8_t node;
  uint8_t type;
} esp_now_send_lap_s;

typedef struct laptime {
  uint16_t ms;
  uint8_t s;
  uint8_t m;
} lap_time_t;

void lap_times_nodeidx_set(uint8_t nodeidx);
void lap_times_nodeidx_roll(int8_t dir);
uint8_t lap_times_nodeidx_get(void);

void lap_times_reset(void);
void lap_times_handle(esp_now_send_lap_s * lap_info);

uint32_t lapt_time_race_idx_get(void);
void lapt_time_race_idx_set(uint8_t race_id);

uint8_t lapt_time_race_num_laps(void);
lap_time_t lapt_time_laptime_get(uint8_t lap, uint8_t &fastest);

lap_time_t lapt_time_best_consecutives_get(uint8_t const consecutives, uint8_t &first);

#endif /* _LAP_TIMES_H_ */
