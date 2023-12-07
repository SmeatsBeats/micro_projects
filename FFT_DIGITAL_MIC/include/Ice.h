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

        void runPattern(CRGB* leds, int NUM_LEDS, uint8_t pal_interval, uint8_t breathe_rate, uint8_t max_bright, uint8_t min_bright);

    private:
};

// parameter names are all over the map here - try to be more consistent across presets

void Ice::runPattern(CRGB* leds, int NUM_LEDS, uint8_t pal_interval, uint8_t breathe_rate, uint8_t max_bright, uint8_t min_bright) {

    // call this first to initialize paletteIndex - will this be reset to 0 with each loop?
    LightEffects lightEffects; 

    // what is paletteIndex doing - should this be handled within the class entirely rather than passed in as argument?
    // what about palette_interval?

    // where is paletteIndex now? should it be specific to this class?

    fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, icy, 255, LINEARBLEND);
    //breathe(15, 10, max_bright);

    

    lightEffects.paletteMove(pal_interval);

    //fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, roseBud, 255, LINEARBLEND);
    
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));
    //breathe(30, 10, max_bright);
    
    lightEffects.breathe(breathe_rate, min_bright, max_bright);
    // do I need button tick or FastLED.show?
}