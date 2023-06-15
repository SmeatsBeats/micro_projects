#include <Arduino.h>
#include "FastLED.h"

#define LED_DT 25
#define NUM_LEDS 10
#define LED_TYPE WS2812

uint8_t max_bright = 128;

struct CRGB leds[NUM_LEDS];

CRGB rgbval(255,255,255);


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  delay(1000);
  LEDS.addLeds<LED_TYPE, LED_DT>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
}

void loop() {
  // put your main code here, to run repeatedly:
  fill_solid(leds, NUM_LEDS, rgbval);

  FastLED.show();
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}