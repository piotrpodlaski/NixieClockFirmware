#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include <SPIFFS.h>
#include "WebHandlers.h"
#include "NixieController.h"


//DNSServer dnsServer;
AsyncWebServer server(80);
NixieController<ENumberOfLamps::eSix,8> nc;

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

  
  nc.displayNumber(2137);

  struct timeval tv;
  tv.tv_sec =   1679177422;  // enter UTC UNIX time (get it from https://www.unixtimestamp.com )
  settimeofday(&tv, NULL);

//  // Set timezone to France (Europe/Paris)
//  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1); // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
//  tzset();
}

void loop() {
  time_t now;
  struct tm timeDetails;

  time(&now);
  localtime_r(&now, &timeDetails);
  nc.displayTime(timeDetails);
  delay(1000);
  Serial.print("\n\n");
}
