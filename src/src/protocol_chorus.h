#ifndef _PROTOCOL_CHORUS_H_
#define _PROTOCOL_CHORUS_H_

#include <stdint.h>

void chorus_race_init(void);

uint8_t chorus_race_is_started(void);
void chorus_race_state_set(uint8_t);

void chorus_race_start(void);
void chorus_race_end(void);
void chorus_race_laps_get(void);

uint8_t chorus_race_lap_time_min(void);
void    chorus_race_lap_time_min_get(void);
void    chorus_race_lap_time_min_change(int val);

int chorus_command_handle(uint8_t const * buff, uint8_t len);

void chorus_nodeidx_set(uint8_t nodeidx);
void chorus_nodeidx_roll(int8_t dir);
uint8_t chorus_nodeidx_get(void);

uint32_t chorus_race_idx_get(void);
void chorus_race_idx_set(uint8_t const race_id);

#endif /* _PROTOCOL_CHORUS_H_ */
