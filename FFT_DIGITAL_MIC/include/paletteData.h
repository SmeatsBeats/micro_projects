#include <Arduino.h>
#include <FastLED.h>

// not sure I'm thrilled with this living here 
// do I need a different index for each pattern? Only one can run at a time so perhaps not.
uint8_t paletteIndex = 0;

DEFINE_GRADIENT_PALETTE (ice) {
  0, 63, 58, 156,
  84, 14, 14, 175,
  168, 17, 215, 255, 
  255, 63, 58, 156
};



DEFINE_GRADIENT_PALETTE (rosebud) {
  0, 131, 58, 180,
  89, 253, 29, 29,
  166, 252, 176, 69, 
  255, 131, 58, 180
};
//CRGBPalette16 roseBud = rosebud;
