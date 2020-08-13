#include "protocol_chorus.h"
#include "comm_espnow.h"

static uint8_t started;

uint8_t chorus_race_is_start(void)
{
    return started;
}

void chorus_race_state_set(uint8_t state)
{
    started = state;
}

void chorus_race_start(void)
{
    char command[] = "R*R1\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
    started = 1;
}

void chorus_race_end(void)
{
    char command[] = "R*R0\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
    started = 0;
}

void chorus_race_laps_get(void)
{
    char command[] = "\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
}
