#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include <SPIFFS.h>
#include "WebHandlers.h"
#include "pinout.h"
#include "ShiftRegTPIC.h"


//DNSServer dnsServer;
AsyncWebServer server(80);

ShiftRegTPIC<8> reg(TPIC_MOSI,TPIC_CLK,TPIC_LATCH,TPIC_CLR,TPIC_G);

void setup() {
  SPIFFS.begin();
  Serial.begin(115200);
  //  WiFi.softAP("Nixie Clock");


  WiFiManager::Begin();
  //Web server stuff:
  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("index.html");
  server.addHandler(new ScanRequestHandler("/rest/scan"));
  server.addHandler(new StoreWifiRequestHandler("/rest/wifi_cred"));
  server.addHandler(new WiFiStatusRequestHandler("/rest/wifi_status"));


  //everything else goes to captive portal:
  server.addHandler(new CaptiveRequestHandler("/www/wifiscan.html")).setFilter(ON_AP_FILTER);  //only when requested from AP
  server.begin();
}

void loop() {
}
