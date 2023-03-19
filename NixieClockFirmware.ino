#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include <SPIFFS.h>
#include "WebHandlers.h"
#include "NixieController.h"
#include "TimeManager.h"

//DNSServer dnsServer;
AsyncWebServer server(80);
NixieController<ENumberOfLamps::eFour,8> nc;
TimeManager timeMan("pool.ntp.org");
BrightnessConfig bc;

void setup() {
  SPIFFS.begin();
  Serial.begin(115200);
  //  WiFi.softAP("Nixie Clock");

  timeMan.init();
  WiFiManager::SetTimeManagerPointer(&timeMan);
  WiFiManager::Begin();
  //Web server stuff:
  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("index.html");
  server.addHandler(new ScanRequestHandler("/rest/scan"));
  server.addHandler(new StoreWifiRequestHandler("/rest/wifi_cred"));
  server.addHandler(new WiFiStatusRequestHandler("/rest/wifi_status"));
  server.addHandler(new BrightRequestHandler("/rest/bright",bc));


  //everything else goes to captive portal:
  server.addHandler(new CaptiveRequestHandler("/www/wifiscan.html")).setFilter(ON_AP_FILTER);  //only when requested from AP
  server.begin();

  
  nc.displayNumber(2137);

}

void loop() {


}
