#include <esp_now.h>
#include <WiFi.h> 

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  for(int i=0; i<data_len; i++)
  {
    Serial.print(data[i]);
  }
} 

void setup()
{
  Serial.begin(115200); 
//   Serial.begin(115200, true); // Use inverted for r9m

    /*
        Uncomment below to print mac address.  Add this to VRx setup().
    */
//   WiFi.mode(WIFI_MODE_STA);
//   Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);

  if(esp_now_init() != ESP_OK)
  {
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv); 
} 

void loop()
{
}