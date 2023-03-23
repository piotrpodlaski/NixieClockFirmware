#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include "TimeManager.h"

class WiFiManager {
  public:
    struct WiFiCred {
      String ssid;
      String pass;
    };
    static void Begin() {
      WiFi.mode(WIFI_STA);
      //start the scan right away:
      WiFi.scanNetworks(true);
      xTaskCreatePinnedToCore(WiFiManager::WiFiTask, "WiFiManagerTask", 10 * 8192, NULL, 1, NULL,0);
      bool validConfig = ReadCredFromFile();
      if (!validConfig) {
        Serial.println(F("No valid WiFi configuration! Starting softAP!"));
        StartAP();
        return;
      }
      //request connection to WiFi from manager thread:
      startStaReq = true;
    }

    static void End() {
      WiFi.disconnect(true);
      WiFi.softAPdisconnect(true);
      dns.stop();
    }
    static void StoreWiFiCred(const JsonVariant& json) {
      auto f = SPIFFS.open(configFile, FILE_WRITE);
      if (!f) Serial.println("Error opening " + configFile + " for writing!");
      serializeJson(json, f);
      f.close();
    }
    static bool ValidateJson(const JsonVariant& json, String* msg = nullptr) {
      auto hasSsid = json.containsKey("ssid");
      auto hasPass = json.containsKey("password");
      if (msg) {
        if (!hasSsid) *msg += " ssid";
        if (!hasPass) *msg += " password";
      }
      return hasSsid && hasPass;
    }

    static bool ValidateJson(const JsonVariant& json, std::vector<String> keys, String* msg = nullptr) {

      bool isGood = true;
      for(const auto & key : keys){
        if(!json.containsKey(key)){
          isGood=false;
          if(msg)
            *msg+=" "+key;
        }
      }
      return isGood;
    }

    static bool ReadCredFromFile() {
      auto f = SPIFFS.open(configFile, FILE_READ);
      if (!f) Serial.println(F("Failed to open WiFi config file!"));
      DynamicJsonDocument doc(1000);
      auto err = deserializeJson(doc, f);
      if (err) {
        Serial.println(F("Failed to read WiFi credentials from file!"));
        return false;
      }
      if (!ValidateJson(doc)) {
        Serial.println(F("Content of the WiFi config file is invalid!"));
        return false;
      }
      cred.ssid = doc["ssid"].as<String>();
      cred.pass = doc["password"].as<String>();
      return true;
    }
    static void StartAP() {
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("Nixie Clock");
      dns.start(53, "*", WiFi.softAPIP());
      Serial.print("NixieClock is available at ");
      Serial.println(WiFi.softAPIP());
      sApActive = true;
    }

    static void StopAP() {
      Serial.println("Switching softAP OFF...");
      WiFi.softAPdisconnect(true);
      sApActive = false;
      WiFi.mode(WIFI_STA);
    }

    bool startSta() {

    }
    static void Connect(const JsonVariant& json) {
      StoreWiFiCred(json);
      WiFi.disconnect();
      ReadCredFromFile();
      startStaReq = true;
    }

    static void WiFiTask(void* params) {
      while (true) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (sApActive)
          dns.processNextRequest();
        if (startStaReq) {

          Serial.println("Connecting to \"" + cred.ssid + "\" network...");
          WiFi.begin(cred.ssid.c_str(), cred.pass.c_str());

          int cnt = 0;
          while (WiFi.status() != WL_CONNECTED) {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            Serial.print(".");
            if (++cnt == connTimeout) {
              Serial.println("\nConnection timed out!");
              if (!sApActive) {
                WiFi.disconnect();
                Serial.println("Failed to connect, starting AP!");
                StartAP();
              }
              break;
            }
          }
          if (cnt < connTimeout) {
            Serial.print("\nSuccessfully connected! IP address: ");
            Serial.println(WiFi.localIP());
            if (sApActive) {
              Serial.printf("AP will stop in %.1f seconds\n", aApLifetme_us / (1e6));
              aApCloseRequestTime_us = esp_timer_get_time();
            }
            //we have access to WiFi! sync ntp!
            if(timeMan)
              timeMan->syncTimeNTP();
          }

          startStaReq = false;
        }

        if (aApCloseRequestTime_us) {
          auto now = esp_timer_get_time();
          if (now - aApCloseRequestTime_us > aApLifetme_us) {
            StopAP();
            aApCloseRequestTime_us = 0;
          }
        }
      }
    }

    static bool IsApActive() {
      return sApActive;
    }

    static int32_t GetApLifetime() {
      if (aApCloseRequestTime_us == 0) return -1;
      int64_t lt = aApLifetme_us - (esp_timer_get_time() - aApCloseRequestTime_us);
      return std::max(lt / 1000, 0LL);
    }

    static void SetTimeManagerPointer(TimeManager* man){
      timeMan=man;
    }

    static IPAddress GetIp(){return WiFi.localIP();}

    static DNSServer dns;
    static const String apSsid;
    
  private:
    static const String configFile;
    static const unsigned int connTimeout;
    static bool startStaReq;
    static WiFiCred cred;
    static bool sApActive;
    static const uint64_t aApLifetme_us;
    static uint64_t aApCloseRequestTime_us;
    static TimeManager* timeMan;

};

const String WiFiManager::configFile{"/wifi.json"};
const String WiFiManager::apSsid{"Nixie Clock"};
const unsigned int WiFiManager::connTimeout{20};
DNSServer WiFiManager::dns;
bool WiFiManager::startStaReq{false};
WiFiManager::WiFiCred WiFiManager::cred{};
bool WiFiManager::sApActive{false};
const uint64_t WiFiManager::aApLifetme_us{60000000};
uint64_t WiFiManager::aApCloseRequestTime_us{0};
TimeManager* WiFiManager::timeMan{nullptr};

#endif //WIFI_MANAGER_H
