#include "protocol_chorus.h"
#include "comm_espnow.h"


void chorus_race_start(void)
{
    char command[] = "R*R1\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
}

void chorus_race_end(void)
{
    char command[] = "R*R0\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
}

void chorus_race_laps_get(void)
{
    char command[] = "\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
}
