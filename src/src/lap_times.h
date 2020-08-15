#ifndef _LAP_TIMES_H_
#define _LAP_TIMES_H_

#include <stdint.h>

#define MAX_LAP_TIMES   32
#define MAX_NODES       6

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
uint8_t lap_times_nodeidx_get(void);

void lap_times_reset(void);
void lap_times_handle(esp_now_send_lap_s * lap_info);

uint8_t lapt_time_race_num_laps(void);

uint32_t lapt_time_race_idx_get(void);
lap_time_t lapt_time_laptime_get(uint8_t lap, uint8_t &fastest);

#endif /* _LAP_TIMES_H_ */
