#include <Arduino.h>
#include <FastLED.h>
// I don't understand when to include headers 
#include <remoteData.h>
//#include <paletteData.h>
//#include <LightEffects.h>

// Should remote really be a class? 
// Would be better to be able to tweak parameters of any profile with the remote
// in addition to creating a custom profile from scratch
// perhaps each class could include sensible default knob mappings... 

// can I do this... ?


class CustomHSV {
    public:
        // attributes 
        // set defaults but only if not defined
      
        CustomHSV() {
            // constructor
        };

        void runPattern(CRGB* leds, int NUM_LEDS, uint8_t hue, uint8_t sat, uint8_t val);

    private:
};

void CustomHSV::runPattern(CRGB* leds, int NUM_LEDS, uint8_t hue, uint8_t sat, uint8_t val) {

    // set blank slate for default
    //fill_solid(leds, NUM_LEDS, CRGB::White);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, sat, val);
    }

    // do I need button tick or FastLED.show?
}