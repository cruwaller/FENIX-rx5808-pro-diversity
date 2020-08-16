#ifndef _PROTOCOL_CHORUS_H_
#define _PROTOCOL_CHORUS_H_

#include <stdint.h>

uint8_t chorus_race_is_start(void);
void chorus_race_state_set(uint8_t);

void chorus_race_start(void);
void chorus_race_end(void);
void chorus_race_laps_get(void);

#endif /* _PROTOCOL_CHORUS_H_ */
