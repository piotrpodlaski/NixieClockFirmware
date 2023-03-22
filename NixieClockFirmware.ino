#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "WebHandlers.h"
#include "NixieController.h"
#include "TimeManager.h"

//working modes of the display
enum class EWorkingMode {
  eNormal = 0, //display time
  eIpDisplay,
  eLampTesting
};

//DNSServer dnsServer;
AsyncWebServer server(80);
NixieController<ENumberOfLamps::eFour, 8> nc;
TimeManager timeMan("pool.ntp.org");
BrightnessConfig bc;
EWorkingMode clkMode{EWorkingMode::eNormal};

void every1000msTask(void* param) {
  while (true) {
    nc.setBrightness(bc);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void every10msTask(void* param) {
  uint64_t ipDisplayTimeout = 10000000;
  auto startTime = esp_timer_get_time();
  while (true) {

    switch (clkMode) {
      case EWorkingMode::eNormal:
        //update time:
        //nixie drivers will update only when time is changed
        nc.displayTime(timeMan.getTime());

        //toggle neon tube every 500ms:
        nc.setNeonTubes(timeMan.getMs() < 500);
        
        break;
      case EWorkingMode::eIpDisplay:
        //WiFi connection timeout:
        if(esp_timer_get_time()-startTime>ipDisplayTimeout)
          clkMode=EWorkingMode::eNormal;

        //display WiFi, NixieController will cycle numbers internally
        if(nc.displayIp(WiFiManager::GetIp()))
          clkMode=EWorkingMode::eNormal; //done, we go to time display
        vTaskDelay(990 / portTICK_PERIOD_MS); // here we go slower, no need to rush
        break;
      case EWorkingMode::eLampTesting:
        clkMode=EWorkingMode::eNormal; //fallback to default
        break;

    }

  vTaskDelay(10 / portTICK_PERIOD_MS);

  }
}





void setup() {
  SPIFFS.begin();
  Serial.begin(115200);

  timeMan.init();
  WiFiManager::SetTimeManagerPointer(&timeMan);
  WiFiManager::Begin();

  //Web server stuff:
  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("index.html");
  server.addHandler(new ScanRequestHandler("/rest/scan"));
  server.addHandler(new StoreWifiRequestHandler("/rest/wifi_cred"));
  server.addHandler(new WiFiStatusRequestHandler("/rest/wifi_status"));
  server.addHandler(new BrightRequestHandler("/rest/bright", bc));


  //everything else goes to captive portal:
  server.addHandler(new CaptiveRequestHandler("/www/index.html")).setFilter(ON_AP_FILTER);  //only when requested from AP
  server.begin();

  //spawn threads:
  xTaskCreatePinnedToCore(every1000msTask, "every 1000ms task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(every10msTask, "every 10ms task", 8192, NULL, 1, NULL, 0);
}

void loop() {
}
