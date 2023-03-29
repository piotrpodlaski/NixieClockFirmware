#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "WebHandlers.h"

//uncomment this to print nice-ish digits to serial
#define PRINT_LAMPS_TO_SERIAL

#include "NixieController.h"
#include "TimeManager.h"


AsyncWebServer server(80);
NixieController<ENumberOfLamps::eFour, 8> nc;
TimeManager timeMan("pool.ntp.org");
BrightnessConfig bc;

void everyHourTask(void* param){
  while (true){
    vTaskDelay(3600*1000 / portTICK_PERIOD_MS);
    //sync time every hour if connected:
    if(WiFiManager::GetIp()!=IPAddress(0,0,0,0))
      timeMan.syncTimeNTP();
  }
}

void every1000msTask(void* param) {
  while (true) {
    nc.setBrightness(bc);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void every10msTask(void* param) {
  uint64_t ipDisplayTimeout = 10000000;
  auto startTime = esp_timer_get_time();
  bool timeout=false;

  while(WiFiManager::GetIp()==IPAddress(0,0,0,0)){
    if(esp_timer_get_time() - startTime > ipDisplayTimeout){
      timeout = true;
      break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  //if we got good IP we display it:
  if(!timeout)
    nc.displayIp(WiFiManager::GetIp());
  
  while (true) {

    //nixie drivers will update only when time is changed
    nc.displayTime(timeMan.getTime());

    //toggle neon tube every 500ms:
    nc.setNeonTubes(timeMan.getMs() < 500);


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
  xTaskCreatePinnedToCore(everyHourTask, "every 1h task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(every1000msTask, "every 1000ms task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(every10msTask, "every 10ms task", 8192, NULL, 1, NULL, 0);
}

void loop() {

}
