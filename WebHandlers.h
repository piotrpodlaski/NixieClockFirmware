#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <WiFi.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"

class ScanRequestHandler : public AsyncWebHandler {
  private:
    const String _uri;
  public:
    ScanRequestHandler(const String &uri)
      : _uri{ uri } {}
    ScanRequestHandler() = delete;
    virtual ~ScanRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      if (request->url() != _uri && !request->url().startsWith(_uri + "/"))
        return false;
      if (request->method() != HTTP_GET)
        return false;
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      DynamicJsonDocument doc(1000);
      auto arr = doc.to<JsonArray>();
      auto n = WiFi.scanComplete();
      if (n == -2) {
        WiFi.scanNetworks(true);
      } else if (n) {
        for (int i = 0; i < n; ++i) {
          auto o = arr.createNestedObject();
          o["ssid"] = WiFi.SSID(i);
          o["rssi"] = WiFi.RSSI(i);
          o["open"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        }
        WiFi.scanDelete();
        if (WiFi.scanComplete() == -2) {
          WiFi.scanNetworks(true);
        }
      }
      auto response = request->beginResponseStream("application/json");
      serializeJson(arr, *response);
      request->send(response);
    }
};

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler(const String &uri)
      : captiveUri{ uri } {}
    virtual ~CaptiveRequestHandler() {}

    //handle everything
    bool canHandle(AsyncWebServerRequest *request) {
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
    StoreWifiRequestHandler(const String &uri)
      : AsyncCallbackJsonWebHandler(uri, {}) {
      onRequest([](AsyncWebServerRequest * request, JsonVariant & json) {
        String msg = "Missing:";
        auto status = WiFiManager::ValidateJson(json, &msg);
        auto response = new AsyncJsonResponse();
        auto root = response->getRoot();
        if (status) {
          root["ok"] = true;
          root["message"] = "Connecting to network...";
          response->setCode(200);
        } else {
          root["ok"] = false;
          root["message"] = msg;
          response->setCode(400);
        }
        response->setLength();
        request->send(response);
        if (status) {
          WiFiManager::Connect(json);
        }
      });
    }
};


class WiFiStatusRequestHandler : public AsyncWebHandler {
  private:
    const String _uri;
  public:
    WiFiStatusRequestHandler(const String &uri)
      : _uri{ uri } {}
    WiFiStatusRequestHandler() = delete;
    virtual ~WiFiStatusRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      if (request->url() != _uri && !request->url().startsWith(_uri + "/"))
        return false;
      if (request->method() != HTTP_GET)
        return false;
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      DynamicJsonDocument doc(1000);
      auto root = doc.to<JsonObject>();

      auto cl = root.createNestedObject("client");
      auto status = WiFi.status();
      cl["status"] = status;
      cl["ssid"] = status == WL_CONNECTED ? WiFi.SSID() : "";
      cl["rssi"] = WiFi.RSSI();
      cl["ip"] = WiFi.localIP().toString();

      auto ap = root.createNestedObject("ap");

      bool apStat = WiFiManager::IsApActive();
      ap["status"] = apStat ? 1 : 0;
      ap["ssid"] = apStat ? WiFi.softAPSSID() : "";
      ap["clients"] = WiFi.softAPgetStationNum();
      ap["lifetime"] = WiFiManager::GetApLifetime();


      auto response = request->beginResponseStream("application/json");
      serializeJson(root, *response);
      request->send(response);
    }
};

#endif //WEB_HANDLERS_H
