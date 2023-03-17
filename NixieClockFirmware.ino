#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include <SPIFFS.h>
#include "WebHandlers.h"

DNSServer dnsServer;
AsyncWebServer server(80);



void setup() {
  SPIFFS.begin();
  Serial.begin(115200);
  WiFi.softAP("Nixie Clock");
  dnsServer.start(53, "*", WiFi.softAPIP());

  //Web server stuff:
  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("index.html");
  server.addHandler(new ScanRequestHandler("/rest/scan")).setFilter(ON_AP_FILTER);  //only when requested from AP
  server.addHandler(new StoreWifiRequestHandler("/rest/wifi_cred"));

  //everything else goes to captive portal:
  server.addHandler(new CaptiveRequestHandler("/index.html")).setFilter(ON_AP_FILTER);  //only when requested from AP
  server.begin();
  Serial.println(WiFi.softAPIP());
}

void loop() {
  dnsServer.processNextRequest();
}
