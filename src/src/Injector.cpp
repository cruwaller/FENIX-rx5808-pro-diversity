#include <ESP8266WiFi.h>
#include <espnow.h>

void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len)
{
  for(int i=0; i<data_len; i++)
  {
    Serial.write(data[i]);
  }
}

void setup()
{
  Serial.begin(400000);
  /*
    CURRENTLY NOT REQUIRED! LEAVE SERIAL UNINVERTED WITH DIRECT CONNECTION TO UART2 PINS
    Use inverted for r9m
  */
  // Serial.begin(115200, SERIAL_8N1, SERIAL_FULL, 1, true); // https://github.com/dok-net/arduino-esp8266/blob/master/cores/esp8266/HardwareSerial.h#L92

  /*
      Uncomment below to print mac address.  Add this to VRx setup().
  */
  // Serial.begin(115200);
  // Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);

  if(esp_now_init() != 0)
  {
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
}
