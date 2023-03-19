#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <sys/time.h>
#include "time.h"

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

    void syncTimeRTC() {
      //PLACEHOLDER:
      struct tm tm;
      tm.tm_year = 2023 - 1900;
      tm.tm_mon = 3 - 1;
      tm.tm_mday = 19;
      tm.tm_hour = 18;
      tm.tm_min = 0;
      tm.tm_sec = 0;
      time_t t = mktime(&tm);
      Serial.printf( "Setting time: %s", asctime(&tm));
      struct timeval now = { .tv_sec = t };
      settimeofday(&now, NULL);
    }

    void syncTimeNTP() {
      configTime(0, 0, ntpServer.c_str());
      setenv("TZ", timezone.c_str(), 1);
    }

    String printLocalTime() {
      auto timeinfo = getTime();
      //See http://www.cplusplus.com/reference/ctime/strftime/
      char output[80];
      strftime(output, 80, "%d-%b-%y, %H:%M:%S", &timeinfo);
      return String(output);
    }


    void initRTC() {}
    const String timezone{"CET-1CEST,M3.5.0,M10.5.0/3"};
    const String ntpServer;
};

#endif //TIME_MANAGER_H
