#include <Arduino.h>
// don't want to double call paletteData 
//#include <paletteData.h>
#include <FastLED.h>

class LightEffects {
    public:
        // attributes 
        //uint8_t paletteIndex = 0;

        LightEffects() {
            // default constructor with no arguments
        };

        void breathe(uint8_t breathe_interval, uint8_t breathe_min_bright, uint8_t breathe_max_bright);

        void paletteMove(int palette_interval);

    private:
};


void LightEffects::breathe(uint8_t breathe_interval, uint8_t breathe_min_bright, uint8_t breathe_max_bright) {

    uint8_t sinBright = beatsin8(breathe_interval, breathe_min_bright, breathe_max_bright, 0, 0);
    FastLED.setBrightness(sinBright);

} // breathe

void LightEffects::paletteMove(int palette_interval) {
    EVERY_N_MILLIS(palette_interval) {
    // is this defined here? does it need to be passed in as argument?
    paletteIndex++;
  }
}
