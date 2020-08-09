#ifndef COMM_ESPNOW_H_
#define COMM_ESPNOW_H_

#include <stdint.h>

void comm_espnow_init(void);
void comm_espnow_deinit(void);
void comm_espnow_send_msp(uint16_t function, uint16_t payloadSize, const uint8_t *payload);

#endif // COMM_ESPNOW_H_
