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
uint8_t customH = 100; 
uint8_t customS = 100; 
uint8_t customV = 100;


class CustomHSV {
    public:
        // attributes 
        // set defaults but only if not defined
        // otherwise each time this is called, you will reset these
        // uint8_t customH = 100; 
        // uint8_t customS = 100; 
        // uint8_t customV = 100;
      
        CustomHSV() {
            // constructor
        };

        void runPattern(CRGB* leds, int NUM_LEDS, remoteData remoteTransfer);

    private:
};

void CustomHSV::runPattern(CRGB* leds, int NUM_LEDS, remoteData remoteTransfer) {

    // set blank slate for default
    fill_solid(leds, NUM_LEDS, CRGB::White);

    // now process data from remoteTransfer 

    switch(remoteTransfer.param_key) {

        case 1:
        //update hue
        customH = remoteTransfer.param_val;
        break;
        case 2:
        //update saturation
        customS = remoteTransfer.param_val;
        break;
        case 3: 
        //update value
        customV = remoteTransfer.param_val;
        break;
  }



    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(customH, customS, customV);
    }

    // do I need button tick or FastLED.show?
}