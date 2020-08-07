#ifndef _LAP_TIMES_H_
#define _LAP_TIMES_H_

#include <stdint.h>

enum {
    ESPNOW_TYPE_INVALID,
    ESPNOW_TYPE_RECE_START,
    ESPNOW_TYPE_LAP_TIME,
};

typedef struct {
  uint32_t lap_time;
  uint8_t lap;
  uint8_t node;
  uint8_t race_id;
  uint8_t type;
} esp_now_send_lap_s;


void lap_times_reset(void);
void lap_times_handle(esp_now_send_lap_s * lap_info);


#endif /* _LAP_TIMES_H_ */
