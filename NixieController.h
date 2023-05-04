#ifndef NIXIE_CONTROLLER_H
#define NIXIE_CONTROLLER_H

#include <sys/time.h>
#include "pinout.h"
#include "mapping.h"
#include "ShiftRegTPIC.h"
#include "CommonStructs.h"


enum class ENumberOfLamps : uint8_t {
  eFour = 4,
  eSix = 6
};

template<ENumberOfLamps nLampsEnum, uint8_t nDrivers>
class NixieController {
  public:
    NixieController() {
      if (nLampsEnum == ENumberOfLamps::eFour) {
        nxMap = &mapping::nixieDigitMappingFourLamp;
        dpMap = &mapping::nixieDpMappingFourLamp;
        nTubes = &mapping::neonTubesFourLamp;
      }
      else if (nLampsEnum == ENumberOfLamps::eSix) {
        nxMap = &mapping::nixieDigitMappingSixLamp;
        dpMap = &mapping::nixieDpMappingSixLamp;
        nTubes = &mapping::neonTubesSixLamp;
      }
      initAdcPwm();
    }
    void setLamp(uint8_t lampId, int8_t digit, bool update = false) {
      if (lampId >= nLamps) return;
      if (digit > 9) return;
      clearLamp(lampId);
      if (nxMap->count({lampId, digit})) {
        auto globalPin = nxMap->at({lampId, digit});
        reg.setSingle(globalPin, 1);
      }
      if (update)
        reg.updateRegisters();
      //Serial.printf("lamp %d set to %d\n", lampId, digit);
    }

    void clearLamp(uint8_t lampId, bool update = false) {
      if (lampId >= nLamps) return;
      for (uint8_t digit = 0; digit < 10; ++digit) {
        if (nxMap->count({lampId, digit})) {
          auto globalPin = nxMap->at({lampId, digit});
          reg.setSingle(globalPin, 0);
        }
      }
      if (update)
        reg.updateRegisters();
    }


    void displayNumber(int num) {
      if (num == lastDisplayedNumber)
        return;

      int digits[nLamps];
      reg.zeroData();
      for (unsigned int i = nLamps; i > 0; i--) {
        uint8_t digit = static_cast<int>(num / pow(10, i - 1)) % 10;
        uint8_t lamp = nLamps - i;
        digits[lamp]=digit;
        setLamp(lamp, digit);
      }
      reg.updateRegisters();
      lastDisplayedNumber = num;
      #ifdef PRINT_LAMPS_TO_SERIAL
        printNiceNumber(digits);
      #endif //PRINT_LAMPS_TO_SERIAL
      
    }

    void displayTime(const tm &t) {
      int number = 0;
      switch (nLampsEnum) {
        case ENumberOfLamps::eFour:
          number = 100 * t.tm_hour + t.tm_min;
          break;
        case ENumberOfLamps::eSix:
          number = 10000 * t.tm_hour + 100 * t.tm_min + t.tm_sec;
      }
      displayNumber(number);
    }

    void setBrightness(BrightnessConfig &br) {
      float bright = 0;
      auto photoRead = analogRead(rPhotoPin);
      br.photoRead = photoRead;
      if (br.isFixed) {
        bright = br.fixedBr;
      }
      else {
        float photoBright = 1.*(br.photoMax - photoRead) / (br.photoMax - br.photoMin);
        bright =  br.minBright;
        bright += (br.maxBright - br.minBright) * photoBright;
      }
      setBrightness(bright);
//      Serial.print(photoRead);
//      Serial.print(" ");
//      Serial.println(bright);
      br.currentBr = bright;
    }

    void setBrightness(float br) {
      br = min(1.f, max(0.f, br));
      uint32_t duty = static_cast<uint32_t>(br * pwmMaxVal);
      ledcWrite(pwmLedcChannwel, duty);
    }

    void setNeonTubes(bool state) {
      if (neonTubeState == state) return;
      for (auto pin : *nTubes) {
        reg.setSingle(pin, state);
      }
      neonTubeState = state;
      reg.updateRegisters();
    }

    void displayIp(IPAddress ip) {
      if (nLampsEnum == ENumberOfLamps::eFour) {
        for (int i = 0; i < 4; i++) {
          Serial.println(ip[i]);
          displayNumber(ip[i]);
          vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
      }
      else if (nLampsEnum == ENumberOfLamps::eSix) {
        for (int i = 0; i < 2; i++) {
          int num = ip[2 * i+1] + 1000 * ip[2 * i ];
          displayNumber(num);
          vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
      }
    }

    void printNiceNumber(int* digits){
      if(nLampsEnum == ENumberOfLamps::eFour){
        Serial.println("\n ___   ___     ___   ___ ");
        Serial.println("/   \\ /   \\   /   \\ /   \\");
        Serial.printf("| %d | | %d |   | %d | | %d |\n",digits[0],digits[1],digits[2],digits[3]);
        Serial.println("\\___/ \\___/   \\___/ \\___/");
      }
      else {
        Serial.println("\n ___   ___     ___   ___     ___   ___ ");
        Serial.println("/   \\ /   \\   /   \\ /   \\   /   \\ /   \\");
        Serial.printf("| %d | | %d |   | %d | | %d |   | %d | | %d |\n",digits[0],digits[1],digits[2],digits[3],digits[4],digits[5]);
        Serial.println("\\___/ \\___/   \\___/ \\___/   \\___/ \\___/");
      }
    }

  private:

    void initAdcPwm() {
      ledcSetup(pwmLedcChannwel, pwmFreq, nBits);
      ledcAttachPin(pwmPin, pwmLedcChannwel);
      analogReadResolution(nBits);
      analogSetAttenuation(ADC_11db);
    }

    mapping::nixieMapping_t* nxMap{nullptr};
    mapping::nixieMapping_t* dpMap{nullptr};
    mapping::neonMap_t* nTubes{nullptr};
    ShiftRegTPIC<nDrivers> reg{TPIC_MOSI, TPIC_CLK, TPIC_LATCH, TPIC_G, TPIC_CLR};
    const uint8_t nLamps = static_cast<uint8_t>(nLampsEnum);
    uint8_t lamps[static_cast<uint8_t>(nLampsEnum)];
    int lastDisplayedNumber{0};
    bool neonTubeState;
    const int rPhotoPin{R_PHOTO};
    const int pwmPin{DIMMING};
    const int nBits{12}; //number of bits for PWM and ADC
    const int pwmLedcChannwel {0};
    const int pwmFreq{100};
    const int pwmMaxVal{(1U << nBits) - 1};
};

#endif //NIXIE_CONTROLLER_H
