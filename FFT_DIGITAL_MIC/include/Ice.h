#include <Arduino.h>
#include <FastLED.h>
//#include <paletteData.h>
//#include <LightEffects.h>


class Ice {
    public:
        // attributes 
        // create palette from paletteData.h
        CRGBPalette16 icy = ice;
        Ice() {
            // constructor
        };

        void runPattern(CRGB* leds, int NUM_LEDS, uint8_t max_bright);

    private:
};

void Ice::runPattern(CRGB* leds, int NUM_LEDS, uint8_t max_bright) {

    // call this first to initialize paletteIndex - will this be reset to 0 with each loop?
    LightEffects lightEffects; 

    // what is paletteIndex doing - should this be handled within the class entirely rather than passed in as argument?
    // what about palette_interval?

    fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, icy, 255, LINEARBLEND);
    //breathe(15, 10, max_bright);

    

    lightEffects.paletteMove(1);

    //fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, roseBud, 255, LINEARBLEND);
    
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));
    //breathe(30, 10, max_bright);
    
    lightEffects.breathe(15, 10, max_bright);
    // do I need button tick or FastLED.show?
}