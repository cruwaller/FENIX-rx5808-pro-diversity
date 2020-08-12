#ifndef COMM_ESPNOW_H_
#define COMM_ESPNOW_H_

#include "msp.h"
#include <stdint.h>

void comm_espnow_init(void);
void comm_espnow_deinit(void);
void comm_espnow_send_msp(mspPacket_t * packet);

#endif // COMM_ESPNOW_H_
