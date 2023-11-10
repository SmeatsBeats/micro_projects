#include <Arduino.h>
#include <FastLED.h>
//#include <paletteData.h>
//#include <LightEffects.h>

// could define params here... but they feel better within the class
// however, they may need to be accessed by main so changes can persist


class Rose {
    public:
        // attributes 
        // create palette from paletteData.h
        CRGBPalette16 rosy = rosebud;
        Rose() {
            // constructor
        };

        void runPattern(CRGB* leds, int NUM_LEDS, uint8_t max_bright, uint8_t breathe_interval, uint8_t breathe_min_bright);

    private:
};

void Rose::runPattern(CRGB* leds, int NUM_LEDS, uint8_t max_bright, uint8_t breathe_interval, uint8_t breathe_min_bright) {

    fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, rosy, 255, LINEARBLEND);
    
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));
    //breathe(30, 10, max_bright);

    // the effect is part of the preset so it feels appropriate to call here
    // but then when it comes to manipulating the effect parameters, it makes more sense to call the effect independently

    LightEffects lightEffects; 
    lightEffects.breathe(breathe_interval, breathe_min_bright, max_bright);
    // do I need button tick or FastLED.show?
}