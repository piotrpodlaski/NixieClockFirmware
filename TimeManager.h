#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <sys/time.h>
#include <time.h>
#include <Wire.h>
#include <DS3231.h>

class TimeManager {
  public:
    TimeManager(const String & server)
      : ntpServer{server} {}

    void init() {
      initRTC();
      syncTimeRTC();
    }

    tm getTime() {
      struct tm timeinfo;
      getLocalTime(&timeinfo);
      return timeinfo;
    }

    uint16_t getMs() {
      struct timeval tv_now;
      gettimeofday(&tv_now, NULL);
      return tv_now.tv_usec/1000;
    }

    void syncTimeRTC() {
      //PLACEHOLDER:
      bool century = false;
      bool h12Flag;
      bool pmFlag;
      struct tm tm;
      tm.tm_year = Clock.getYear() +100;
      tm.tm_mon = Clock.getMonth(century) - 1;
      tm.tm_mday = Clock.getDate();
      tm.tm_hour = Clock.getHour(h12Flag, pmFlag);
      tm.tm_min = Clock.getMinute();
      tm.tm_sec = Clock.getSecond();
      time_t tNow = mktime(&tm);
      Serial.printf( "Setting time: %s", asctime(&tm));
      struct timeval now = { .tv_sec = tNow };
      settimeofday(&now, NULL);
      setenv("TZ", timezone.c_str(), 1);
    }

    void syncTimeNTP() {
      struct tm timeinfo;
//      timeinfo.tm_year = 2000-1900;
//      time_t tNow = mktime(&timeinfo);
//      struct timeval now = { .tv_sec = tNow };
//      settimeofday(&now, NULL);   
      Serial.println("Updating time from NTP server...");
      configTime(0, 0, ntpServer.c_str());
      //wait 5 seconds for the sync
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      if(!getLocalTime(&timeinfo)){
        Serial.println("  Failed to obtain time");
        return;
      }
      
      Serial.println("  Got the time from NTP!");
      
      setenv("TZ", timezone.c_str(), 1);
      Serial.println(printLocalTime());
      setTimeRTC(timeinfo); //update RTC each time we get time from the internet
    }

    void setTimeRTC(const tm &tmnow) {
      Serial.println("SettingRTC!");
      Clock.setSecond(tmnow.tm_sec);
      Clock.setMinute(tmnow.tm_min);
      Clock.setHour(tmnow.tm_hour);
      Clock.setDoW(tmnow.tm_wday + 1U);
      Clock.setDate(tmnow.tm_mday);
      Clock.setMonth(tmnow.tm_mon + 1U);
      Clock.setYear(tmnow.tm_year - 100U);
    }

    String printLocalTime() {
      auto timeinfo = getTime();
      //See http://www.cplusplus.com/reference/ctime/strftime/
      char output[80];
      strftime(output, 80, "%d-%b-%y, %H:%M:%S", &timeinfo);
      return String(output);
    }


    void initRTC() {

    }
    const String timezone{"CET-1CEST,M3.5.0,M10.5.0/3"};
    const String ntpServer;
    RTClib myRTC;
    DS3231 Clock;
};

#endif //TIME_MANAGER_H
