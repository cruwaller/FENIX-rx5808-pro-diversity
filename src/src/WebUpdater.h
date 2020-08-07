#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#define STASSID "FENIX"

const char *host = "webupdate";
const char *ssid = STASSID;

WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void BeginWebUpdate(void) {

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
  server.begin();
  MDNS.addService("http", "tcp", 80);

#if DEBUG_ENABLED
//  Serial.printf("Ready! Open http://%s.local in your browser\n", host);
#endif
}

void HandleWebUpdate(void) {
  server.handleClient();
  yield();
}
