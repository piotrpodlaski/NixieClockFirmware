#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "WebHandlers.h"
#include "NixieController.h"
#include "TimeManager.h"

//uncomment this to print nice-ish digits to serial
//#define PRINT_LAMPS_TO_SERIAL


AsyncWebServer server(80);
NixieController<ENumberOfLamps::eFour, 8> nc;
TimeManager timeMan("pool.ntp.org");
BrightnessConfig bc;
int touchThreshold = 15;

void everyHourTask(void* param){
  while (true){
    vTaskDelay(3600*1000 / portTICK_PERIOD_MS);
    WiFiManager::ReconnectIfNecessary();
    //sync time every hour if connected:
    if(WiFiManager::GetIp()!=IPAddress(0,0,0,0))
      timeMan.syncTimeNTP();
  }
}

void every100msTask(void* param) {
  while (true) {
    nc.setBrightness(bc);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
void every10msTask(void* param) {
  uint64_t ipDisplayTimeout = 10000000;
  auto startTime = esp_timer_get_time();
  bool timeout=false;

  nc.displayNumber(2137);//hehe

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

void touchTask(void* param) {
  bool stateH = false;
  bool stateM = false;
  while (true) {
    auto tH = touchRead(TOUCH_HOUR);
    auto tM = touchRead(TOUCH_MINUTE);
    if(stateH){
      if(tH>touchThreshold)
        stateH=false;
    } else if(tH<touchThreshold) {
      stateH=true;
      Serial.println("hours pressed!");
      timeMan.incrementH();
    }
    if(stateM){
      if(tM>touchThreshold)
        stateM=false;
    } else if(tM<touchThreshold) {
      stateM=true;
      Serial.println("hours pressed!");
      timeMan.incrementM();
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  SPIFFS.begin();
  Serial.begin(115200);
  Wire.begin();

  timeMan.init();
  WiFiManager::SetTimeManagerPointer(&timeMan);
  WiFiManager::Begin();

  //Web server stuff:
  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("index.html");
  server.addHandler(new ScanRequestHandler("/rest/scan"));
  server.addHandler(new StoreWifiRequestHandler("/rest/wifi_cred"));
  server.addHandler(new WiFiStatusRequestHandler("/rest/wifi_status"));
  server.addHandler(new BrightRequestHandler("/rest/bright", bc));
  server.addHandler(new TemperatureRequestHandler("/rest/temperature", &timeMan));


  //everything else goes to captive portal:
  server.addHandler(new CaptiveRequestHandler("/www/index.html")).setFilter(ON_AP_FILTER);  //only when requested from AP
  server.begin();

  //spawn threads:
  xTaskCreatePinnedToCore(everyHourTask, "every 1h task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(every100msTask, "every 100ms task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(every10msTask, "every 10ms task", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(touchTask, "touch task", 8192, NULL, 1, NULL, 0);
}


void loop() {

}
