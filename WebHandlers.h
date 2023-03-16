#include <WiFi.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class ScanRequestHandler : public AsyncWebHandler {
private:
  const String _uri;
public:
  ScanRequestHandler(const String& uri) :_uri{uri} {}
  ScanRequestHandler() = delete;
  virtual ~ScanRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    if(request->url()!=_uri && !request->url().startsWith(_uri+"/"))
      return false;
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(1000);
    auto arr = doc.to<JsonArray>();
  auto n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (int i = 0; i < n; ++i){
      auto o = arr.createNestedObject();
      o["ssid"]=WiFi.SSID(i);
      o["rssi"]=WiFi.RSSI(i);
      o["open"]= (WiFi.encryptionType(i)==WIFI_AUTH_OPEN);
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  auto response = request->beginResponseStream("application/json");
  serializeJson(arr,*response);
  request->send(response);
  }
};

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler(const String& uri) : captiveUri{uri} {}
  virtual ~CaptiveRequestHandler() {}

  //handle everything
  bool canHandle(AsyncWebServerRequest *request){
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, captiveUri);
  }
private:
  String captiveUri;
};

class StoreWifiRequestHandler : public AsyncCallbackJsonWebHandler {
public:
  StoreWifiRequestHandler(const String& uri) :AsyncCallbackJsonWebHandler(uri,{}) {
    onRequest([](AsyncWebServerRequest *request, JsonVariant &json) {
      serializeJson(json,Serial);
      Serial.println();
      request->send(200,"application/json","{\"ok\":true}");
    });
    }
};
