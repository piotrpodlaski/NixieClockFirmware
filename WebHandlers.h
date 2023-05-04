#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <WiFi.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "WiFiManager.h"
#include "CommonStructs.h"
#include "TimeManager.h"

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
    CaptiveRequestHandler(const String &f)
      : captiveFile{ f } {}
    virtual ~CaptiveRequestHandler() {}

    //handle everything
    bool canHandle(AsyncWebServerRequest *request) {
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      request->send(SPIFFS, captiveFile);
    }
  private:
    String captiveFile;
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

class BrightRequestHandler : public AsyncWebHandler {
  public:
    BrightRequestHandler(const String &uri, BrightnessConfig& bCfg)
      : _uri{ uri }, bc{bCfg} {
      restoreBright();
    }
    virtual ~BrightRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      if (request->url() != _uri && !request->url().startsWith(_uri + "/"))
        return false;
      if (!(request->method() & (HTTP_GET | HTTP_POST)) )
        return false;
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      auto method = request->method();
      auto response = new AsyncJsonResponse();
      auto root = response->getRoot(  );
      if (method & HTTP_POST) {
        if (request->_tempObject != NULL) {
          DynamicJsonDocument jsonBuffer(1024);
          auto error = deserializeJson(jsonBuffer, (uint8_t*)(request->_tempObject));
          if (!error) {
            auto json = jsonBuffer.as<JsonVariant>();
            String msg{"Missing:"};
            if (!WiFiManager::ValidateJson(json, neededKeys, &msg)) {
              response->setCode(400);
              root["message"] = msg;
            }
            else {
              bc.minBright = json["minBright"].as<float>();
              bc.maxBright = json["maxBright"].as<float>();
              bc.photoMin = json["photoMin"].as<uint32_t>();
              bc.photoMax = json["photoMax"].as<uint32_t>();
              bc.isFixed = json["isFixed"].as<bool>();
              bc.fixedBr = json["fixedBr"].as<float>();
              storeBright(json);
            }
          }
          else {
            response->setCode(400);
            root["message"] = "Error while deserializing JSON message!";
          }
        }
      }
      root["minBright"] = bc.minBright;
      root["maxBright"] = bc.maxBright;
      root["photoMin"]  = bc.photoMin;
      root["photoMax"]  = bc.photoMax;
      root["isFixed"]   = bc.isFixed;
      root["fixedBr"]   = bc.fixedBr;
      root["photoRead"] = bc.photoRead;
      root["currentBr"] = bc.currentBr;

      response->setLength();
      request->send(response);
    }
    void storeBright(const JsonVariant& json) {
      auto f = SPIFFS.open(configFile, FILE_WRITE);
      if (!f) Serial.println("Error opening " + configFile + " for writing!");
      DynamicJsonDocument doc(1000);

      serializeJson(json, f);
      f.close();
    }

    void restoreBright() {
      auto f = SPIFFS.open(configFile, FILE_READ);
      if (!f) Serial.println(F("Failed to open WiFi config file!"));
      DynamicJsonDocument doc(1000);
      auto err = deserializeJson(doc, f);
      if (err) {
        Serial.println(F("Failed to read brightness config from file!"));
        return;
      }
      if (!WiFiManager::ValidateJson(doc, neededKeys)) {
        Serial.println(F("Content of the brightness config file is invalid!"));
        return;
      }
      bc.minBright = doc["minBright"].as<float>();
      bc.maxBright = doc["maxBright"].as<float>();
      bc.photoMin = doc["photoMin"].as<uint32_t>();
      bc.photoMax = doc["photoMax"].as<uint32_t>();
      bc.isFixed = doc["isFixed"].as<bool>();
      bc.fixedBr = doc["fixedBr"].as<float>();
    }

    virtual void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override final {
      if (total > 0 && request->_tempObject == NULL && total < 16384) {
        request->_tempObject = malloc(total);
      }
      if (request->_tempObject != NULL) {
        memcpy((uint8_t*)(request->_tempObject) + index, data, len);
      }

    }

  private:
    String _uri;
    BrightnessConfig& bc;
    const std::vector<String> neededKeys = {"minBright", "maxBright", "photoMin", "photoMax", "isFixed", "fixedBr"};
    String configFile{"/brightness.json"};
};

class TemperatureRequestHandler : public AsyncWebHandler {
  private:
    const String _uri;
    TimeManager *timeMan{nullptr};
  public:
    TemperatureRequestHandler(const String &uri, TimeManager* man)
      : _uri{ uri }, timeMan{man} {}
    TemperatureRequestHandler() = delete;
    virtual ~TemperatureRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      if (request->url() != _uri && !request->url().startsWith(_uri + "/"))
        return false;
      if (request->method() != HTTP_GET)
        return false;
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      DynamicJsonDocument doc(1000);
      auto arr = doc.to<JsonVariant>();
      arr["temperature"] = timeMan->getTempRTC();
      auto response = request->beginResponseStream("application/json");
      serializeJson(arr, *response);
      request->send(response);
    }
};

#endif //WEB_HANDLERS_H
