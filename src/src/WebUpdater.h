#include "settings.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <comm_espnow.h>
#include <esp_wifi.h>

const char *host = "webupdate";
const char *ssid = STASSID;

WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";


void handleMacAddress()
{
  uint8_t primaryChan;
  wifi_second_chan_t secondChan;
  esp_wifi_get_channel(&primaryChan, &secondChan);
  (void)secondChan;

  String message = "WiFi STA MAC: ";
  message += WiFi.macAddress();
  message += "\n  - channel in use: ";
  message += primaryChan;
  message += "\n  - mode: ";
  message += (uint8_t)WiFi.getMode();
  message += "\n\nWiFi SoftAP MAC: ";
  message += WiFi.softAPmacAddress();
  message += "\n  - IP: ";
  message += WiFi.softAPIP().toString();
  message += "\n";
  server.send(200, "text/plain", message);
}


void BeginWebUpdate(void)
{
#if !OTA_UPDATE_STORE
  comm_espnow_deinit();
#endif

#if DEBUG_ENABLED
//  Serial.println("Begin Webupdater");
#endif
  WiFi.mode(WIFI_AP);
  WiFi.softAP(STASSID);

  MDNS.begin(host);

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
#if DEBUG_ENABLED
      Serial.setDebugOutput(true);
      Serial.printf("Update: %s\n", upload.filename.c_str());
#endif
      if (!Update.begin()) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
#if DEBUG_ENABLED
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
#endif
      } else {
        Update.printError(Serial);
      }
#if DEBUG_ENABLED
      Serial.setDebugOutput(false);
#endif
    } else {
#if DEBUG_ENABLED
      Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
#endif
    }
  });
  server.on("/mac", handleMacAddress);

  server.begin();
  MDNS.addService("http", "tcp", 80);

#if DEBUG_ENABLED
//  Serial.printf("Ready! Open http://%s.local in your browser\n", host);
#endif
}

void HandleWebUpdate(void)
{
  server.handleClient();
  yield();
}
