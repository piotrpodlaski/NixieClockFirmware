#ifndef BRIGHTNESS_CONFIG_H
#define BRIGHTNESS_CONFIG_H

struct BrightnessConfig {
  float minBright = 0.5;
  float maxBright = 1.;
  uint32_t photoMin = 0;
  uint32_t photoMax = 4095;
  bool isFixed = true; //when false, variable brightness is used
  float fixedBr = 1;
  uint32_t photoRead=0;
  float currentBr = 1;
};

#endif
