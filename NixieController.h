#ifndef NIXIE_CONTROLLER_H
#define NIXIE_CONTROLLER_H

#include "pinout.h"
#include "mapping.h"
#include "ShiftRegTPIC.h"
#include "BrightnessConfig.h"
#include <sys/time.h>

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
    }
    void setLamp(uint8_t lampId, uint8_t digit) {
      if (lampId >= nLamps) return;
      if (digit > 9) return;
      if (nxMap->count({lampId, digit})) {
        auto globalPin = nxMap->at({lampId, digit});
        reg.setSingle(globalPin, 1);
      }
      Serial.printf("lamp %d set to %d\n", lampId, digit);

    }
    void displayNumber(int num) {
      if (num == lastDisplayedNumber)
        return;
      reg.zeroData();
      for (unsigned int i = nLamps; i > 0; i--) {
        uint8_t digit = static_cast<int>(num / pow(10, i - 1)) % 10;
        uint8_t lamp = nLamps - i;
        setLamp(lamp, digit);
      }
      reg.updateRegisters();
      lastDisplayedNumber = num;
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
      float bright=0;
      auto photoRead = analogRead(rPhotoPin);
      br.photoRead=photoRead;
      if (br.isFixed) {
        bright=br.fixedBr;
      }
      else {
        float photoBright = 1.*(photoRead-br.photoMin)/(br.photoMax-br.photoMin);
        bright =  br.minBright;
        bright+= (br.maxBright-br.minBright)*photoBright;
      }
      setBrightness(bright);
      br.currentBr=bright;
    }

    void setBrightness(float br) {
      br = min(1.f, max(0.f, br));
      uint32_t duty = static_cast<uint32_t>(br * pwmMaxVal);
      ledcWrite(pwmLedcChannwel, duty);
    }

    void initAdcPwm() {
      ledcSetup(pwmLedcChannwel, pwmFreq, nBits);
      ledcAttachPin(pwmPin, pwmLedcChannwel);
      analogReadResolution(nBits);
      analogSetAttenuation(ADC_6db);
    }
  private:
    mapping::nixieMapping_t* nxMap{nullptr};
    mapping::nixieMapping_t* dpMap{nullptr};
    mapping::neonMap_t* nTubes{nullptr};
    ShiftRegTPIC<nDrivers> reg{TPIC_MOSI, TPIC_CLK, TPIC_LATCH, TPIC_CLR, TPIC_G};
    const uint8_t nLamps = static_cast<uint8_t>(nLampsEnum);
    uint8_t lamps[static_cast<uint8_t>(nLampsEnum)];
    int lastDisplayedNumber{0};
    const int rPhotoPin{R_PHOTO};
    const int pwmPin{DIMMING};
    const int nBits{12}; //number of bits for PWM and ADC
    const int pwmLedcChannwel{0};
    const int pwmFreq{100};
    const int pwmMaxVal{(1U << nBits) - 1};
};

#endif //NIXIE_CONTROLLER_H
