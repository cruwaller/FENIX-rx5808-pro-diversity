#ifndef COMM_ESPNOW_H_
#define COMM_ESPNOW_H_

#include "msp.h"
#include <stdint.h>

typedef enum {
    ESPNOW_ALL = 0,
    ESPNOW_ELRS,
    ESPNOW_CHORUS
} comm_espnow_receiver_e;

void comm_espnow_init(void);
void comm_espnow_deinit(void);
void comm_espnow_send_msp(mspPacket_t * packet, comm_espnow_receiver_e rcvr = ESPNOW_ALL);
void comm_espnow_send(uint8_t * buffer, uint8_t len, comm_espnow_receiver_e rcvr = ESPNOW_ALL);

#endif // COMM_ESPNOW_H_
