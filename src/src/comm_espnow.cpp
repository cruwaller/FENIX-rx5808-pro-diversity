#include "comm_espnow.h"
#include "lap_times.h"
#include "settings.h"
#include <esp_now.h>
#include <WiFi.h>


uint8_t broadcastAddress[][ESP_NOW_ETH_ALEN] = {
    {0x50, 0x02, 0x91, 0xDA, 0x56, 0xCA},   // esp32 tx 50:02:91:DA:56:CA
    {0x50, 0x02, 0x91, 0xDA, 0x37, 0x84},   // r9 tx    50:02:91:DA:37:84
};


static void esp_now_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
#if DEBUG_ENABLED
    Serial.printf("ESP NOW message received!\n");
#endif
    if (data_len >= sizeof(esp_now_send_lap_s)) {
        esp_now_send_lap_s * lap_info = (esp_now_send_lap_s*)data;
        lap_times_handle(lap_info);
    }
    char hello[] = "CHORUS_CB\n";
    esp_now_send(mac_addr, (uint8_t*)hello, strlen(hello)); // send to all registered peers
}


void comm_espnow_init(void)
{
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
#if DEBUG_ENABLED
        Serial.println("Error initializing ESP-NOW");
#endif
        return;
    }
    esp_now_register_recv_cb(esp_now_recv_cb);

    // Adds broadcastAddress
    esp_now_peer_info_t injectorInfo;
    injectorInfo.channel = 0;
    injectorInfo.encrypt = false;
    for (int i = 0; i < sizeof(broadcastAddress) / ESP_NOW_ETH_ALEN; i++) {
        memcpy(injectorInfo.peer_addr, broadcastAddress[i], ESP_NOW_ETH_ALEN);
        if (esp_now_add_peer(&injectorInfo) != ESP_OK) {
#if DEBUG_ENABLED
            Serial.printf("Failed to add peer[%u]\n", i);
#endif
            //return;
        }
    }
}


uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}


void comm_espnow_send_msp(uint16_t function, uint16_t payloadSize, const uint8_t *payload)
{
    uint8_t nowDataOutput[9 + payloadSize] = {0};
    uint8_t crc = 0, iter, data;

    // MSP header
    nowDataOutput[0] = '$';
    nowDataOutput[1] = 'X';
    nowDataOutput[2] = '<';
    nowDataOutput[3] = '0';
    nowDataOutput[4] = function & 0xff;
    nowDataOutput[5] = (function >> 8) & 0xff;
    nowDataOutput[6] = payloadSize & 0xff;
    nowDataOutput[7] = (payloadSize >> 8) & 0xff;

    // Calc CRC, starts from [3]
    for(iter = 3; iter < 8; iter++)
        crc = crc8_dvb_s2(crc, nowDataOutput[iter]);

    // Add payload and calc CRC on the fly
    while (payloadSize--) {
        data = *payload++;
        nowDataOutput[iter++] = data;
        crc = crc8_dvb_s2(crc, data);
    }

    // Add CRC
    nowDataOutput[iter++] = crc;

#if 0
    for (int i = 0; i < sizeof(broadcastAddress) / 6; i++)
    {
        uint8_t tempBroadcastAddress[6];
        memcpy(tempBroadcastAddress, broadcastAddress + (6 * i), 6);
        esp_now_send(tempBroadcastAddress, (uint8_t *) &nowDataOutput, sizeof(nowDataOutput));
    }
#else
    // Send to all listed peers
    esp_now_send(NULL, (uint8_t *) &nowDataOutput, iter);
#endif
}
