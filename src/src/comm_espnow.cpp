#include "comm_espnow.h"
#include "lap_times.h"
#include "settings.h"
#include <esp_now.h>
#include <WiFi.h>


#define WIFI_CHANNEL 1

uint8_t broadcastAddress[][ESP_NOW_ETH_ALEN] = {
    {0x5C, 0xCF, 0x7F, 0xAC, 0xD9, 0x0F},   // R9M LOGGER STA: 5C:CF:7F:AC:D9:0F (ESP8266)
    {0x5E, 0xCF, 0x7F, 0xAC, 0xD9, 0x0F},   // R9M LOGGER  AP: 5E:CF:7F:AC:D9:0F (ESP8266)
    //{0xF0, 0x08, 0xD1, 0xD4, 0xED, 0x7C},   // Chorus32 STA: F0:08:D1:D4:ED:7C (ESP32)
    {0xF0, 0x08, 0xD1, 0xD4, 0xED, 0x7D},   // Chorus32  AP: F0:08:D1:D4:ED:7D (ESP32)
};


static void esp_now_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
#if DEBUG_ENABLED
    Serial.printf("ESP NOW CB!\n");
#endif

    // Check if message is MSP

    if (data_len >= sizeof(esp_now_send_lap_s)) {
        esp_now_send_lap_s * lap_info = (esp_now_send_lap_s*)data;
        lap_times_handle(lap_info);
    }
    //char hello[] = "FENIX_CB\n";
    //esp_now_send(mac_addr, (uint8_t*)hello, strlen(hello)); // send to all registered peers
}


void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
#if DEBUG_ENABLED && 1
  Serial.print("ESPNOW Sent:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
#endif
}

void comm_espnow_init(void)
{
    WiFi.mode(WIFI_MODE_APSTA);
#if DEBUG_ENABLED
    // STA MAC address: D8:A0:1D:4C:72:18

    //Serial.print("My MAC address: ");
    //Serial.println(WiFi.macAddress());
    Serial.printf("STA MAC Address: %s\n", WiFi.macAddress().c_str());
    Serial.printf("AP MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
#endif
    WiFi.disconnect();

    Serial.println("ESPNOW initialize...");
    if (esp_now_init() == ESP_OK) {
        esp_now_register_send_cb(esp_now_send_cb);
        esp_now_register_recv_cb(esp_now_recv_cb);

        esp_now_peer_info_t peer_info = {
            .peer_addr = {0},
            .lmk = {0},
            .channel = WIFI_CHANNEL,
            .ifidx = ESP_IF_WIFI_STA,
            .encrypt = 0,
            .priv = NULL
        };

        uint8_t num_peers = sizeof(broadcastAddress) / ESP_NOW_ETH_ALEN;
        for (uint8_t iter = 0; iter < num_peers; iter++) {
            memcpy(peer_info.peer_addr, broadcastAddress[iter], ESP_NOW_ETH_ALEN);
            esp_err_t err = esp_now_add_peer(&peer_info);
            if (ESP_OK != err) {
#if DEBUG_ENABLED
                Serial.printf("Failed to add peer[%u], error: %d\n", iter, (int)err);
#endif
            }
        }

#if 1
    char hello[] = "FENIX_HELLO\n";
    esp_now_send(NULL, (uint8_t*)hello, strlen(hello)); // send to all registered peers
#endif
#if DEBUG_ENABLED
        Serial.println(" ... init DONE");
    } else {
        Serial.println("ESPNOW init failed!");
#endif
    }
}

void comm_espnow_deinit(void)
{
    if (ESP_OK != esp_now_deinit()) {
#if DEBUG_ENABLED
        Serial.printf("ESPNOW deinit failed\n");
#endif
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
