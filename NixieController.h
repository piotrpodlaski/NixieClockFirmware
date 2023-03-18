#ifndef NIXIE_CONTROLLER_H
#define NIXIE_CONTROLLER_H

#include "pinout.h"
#include "mapping.h"
#include "ShiftRegTPIC.h"
#include <sys/time.h>

enum class ENumberOfLamps : uint8_t{
  eFour = 4,
  eSix = 6
};

template<ENumberOfLamps nLampsEnum, uint8_t nDrivers>
class NixieController {
public:
  NixieController() {}
  void setLamp(uint8_t lampId, uint8_t digit){
    if(lampId>=nLamps) return;
    if(digit>9) return;
    if(nxMap.count({lampId,digit})){
        auto globalPin = nxMap.at({lampId,digit});
        reg.setSingle(globalPin,1);
    }
    Serial.printf("lamp %d set to %d\n",lampId,digit);
    
  }
  void displayNumber(int num){
    reg.zeroData();
    for(unsigned int i=nLamps;i>0;i--){
      uint8_t digit = static_cast<int>(num / pow(10,i-1)) % 10;
      uint8_t lamp = nLamps-i;
      setLamp(lamp,digit);
    }
    reg.updateRegisters();
  }

  void displayTime(struct tm &t){
    int number = 0;
    switch(nLampsEnum){
      case ENumberOfLamps::eFour:
        number=100*t.tm_hour+t.tm_min;
        break;
      case ENumberOfLamps::eSix:
        number = 100000*t.tm_hour+100*t.tm_min+t.tm_sec;
    }
    displayNumber(number);
  }
private:
  mapping::nixieMapping_t& nxMap = mapping::nixieDigitMapping;
  mapping::nixieMapping_t& dpMap = mapping::nixieDpMapping;
  mapping::neonMap_t& nTubes = mapping::neonTubes;
  ShiftRegTPIC<nDrivers> reg{TPIC_MOSI,TPIC_CLK,TPIC_LATCH,TPIC_CLR,TPIC_G};
  const uint8_t nLamps = static_cast<uint8_t>(nLampsEnum);
  uint8_t lamps[static_cast<uint8_t>(nLampsEnum)];
  
};

#endif //NIXIE_CONTROLLER_H
