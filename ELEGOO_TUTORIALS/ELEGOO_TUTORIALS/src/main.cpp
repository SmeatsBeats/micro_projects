/*
 * 
 * All the resources for this project: https://randomnerdtutorials.com/
 * Modified by Rui Santos
 * 
 * Created by FILIPEFLOP
 * 
 */
 
#include <SPI.h>
#include <MFRC522.h>
#include <map>
#include "FastLED.h"
#include "arduinoFFT.h"

// fft 

#define NUM_BANDS 10
#define NOISE 1000
const uint16_t samples = 128; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 9000; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long newTime;

int bandValues[NUM_BANDS] = {0}; 

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */


// rfid

#define SS_PIN 5
#define RST_PIN 21
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// sound sensor

#define ANALOG_PIN 33
#define DIGI_PIN 32
int analogVal = 0; 
int digiVal; 

// neopixel 

#define LED_DT 25                                             // Data pin to connect to the strip.
//#define LED_CK 11                                             // Clock pin for WS2801 or APA102.
#define COLOR_ORDER GRB                                       // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812                                       // Using APA102, WS2812, WS2801. Don't forget to modify LEDS.addLeds to suit.
#define NUM_LEDS 10                                           // Number of LED's.

int thisdelay = 20;
uint8_t max_bright = 50;

// Palette definitions

DEFINE_GRADIENT_PALETTE (rosebud) {
  0, 131, 58, 180,
  89, 253, 29, 29,
  166, 252, 176, 69, 
  255, 131, 58, 180
};

CRGBPalette16 roseBud = rosebud;

DEFINE_GRADIENT_PALETTE (ice) {
  0, 63, 58, 156,
  84, 14, 14, 175,
  168, 17, 215, 255, 
  255, 63, 58, 156
};

CRGBPalette16 iceFade = ice;
uint8_t paletteIndex = 0;

struct CRGB leds[NUM_LEDS];

//CRGB rgbval(255,255,255);

// multiple tasks without delay

unsigned long prev_millis = 0;
int interval = 300; 

// declare functions

void palette_move(int palette_interval);
void chase(uint8_t chase_interval);
int chase_i = 1; 
void breathe(uint8_t breathe_interval, uint8_t breathe_min_bright, uint8_t breathe_max_bright);
uint8_t bright = 0;
bool inhale = true;


int current_profile;
String UID;

void setup() 
{
  // Serial monitor 
  Serial.begin(9600);   // Initiate a serial communication
  while(!Serial);
  Serial.println("Ready");

  // fft sampling rate 
  sampling_period_us = round(1000000*(1.0/samplingFrequency));

  // LED test
  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500); 

  // Setup RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  // Setup audio
  pinMode(DIGI_PIN, INPUT);
}

void loop()
{

  // test sampling rate 
  // last result 11,000 Hz with analog read

  // Sample the audio pin
  for (int i = 0; i < samples; i++) {
    newTime = micros();
    vReal[i] = analogRead(ANALOG_PIN); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }

  // remove DC offset?
  FFT.DCRemoval();

  // perform fft 
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */

  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

  // store results in frequency bands

    for(int i = 2; i < (samples/2); i++) {
    // noise filter is really too crude! need something frequency dependent 
    // could maybe write function to calibrate when music off
    //if (vReal[i] > NOISE) {

      // 128 samples - this is way quicker and won't miss kicks, but I can't figure out wtf is going on with the other bands 

      //10 bands, 4.5kHz top band
      if (i <= 1 && vReal[i] > 1000) bandValues[0] += (int)vReal[i];
      if (i > 1 && i <= 2) bandValues[1] += (int)vReal[i];
      if (i > 2 && i <= 4) bandValues[2] += (int)vReal[i];
      if (i > 4 && i <= 6) bandValues[3] += (int)vReal[i];
      if (i > 6 && i <= 9) bandValues[4] += (int)vReal[i];
      if (i > 9 && i <= 14) bandValues[5] += (int)vReal[i];
      if (i > 14 && i <= 21) bandValues[6] += (int)vReal[i];
      if (i > 21 && i <= 34 && vReal[i] > 3000) bandValues[7] += (int)vReal[i];
      if (i > 34 && i <= 52 && vReal[i] > 3000) bandValues[8] += (int)vReal[i];
      if (i > 52) bandValues[9]  += (int)vReal[i];



    //} // NOISE

  } // bandVals


  if (UID == "94 D5 E9 85") {
    //Serial.println("Blue Fob");
    //fill_solid(leds, NUM_LEDS, CRGB::Blue);
    fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, iceFade, 255, LINEARBLEND);
    breathe(15, 10, max_bright);
    palette_move(1);

  // try ease
  /*
    EVERY_N_MILLISECONDS(thisdelay) {                           // FastLED based non-blocking delay to update/display the sequence.
      ease();
    }
  */
  }
  else if (UID == "9A AF 7D D4") {
    //Serial.println("White Fob");
    chase(60);

    //Serial.println("Band 1: ");
    //Serial.println(bandValues[1]);


    if (bandValues[1] > 1100) {

      fill_solid(leds, NUM_LEDS, CRGB::White);
    
    }


    /*
    EVERY_N_MILLIS(10) { // maybe this will smooth it out a little?
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(map(bandValues[i], 0, 1500, 0, 255), 150, map(bandValues[i], 0, 1500, 0, 255));
        //leds[i] = CHSV(90, 150, bandValues[i]);
      } // each LED
    } // every N millis
    */

  }
  else {
    //Serial.println("Unrecognized.");
    //Serial.println("UID: " + UID);
    //fill_solid(leds, NUM_LEDS, CRGB::Red);
    fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, roseBud, 255, LINEARBLEND);
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));
    breathe(30, 10, max_bright);
    if (bandValues[1] > 1100) {

      FastLED.setBrightness(0);
    
    }

    if (bandValues[7] > 3100) {

      FastLED.setBrightness(max_bright);
    
    }

    //Serial.println("Band 3: ");
    Serial.println(bandValues[7]);

  }
  
  FastLED.show();

  // Reset loop when idle
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
    return;
    
 
  // Verify that the data has been read - if not, reset
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  //Show UID on serial monitor
  //Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     //Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  UID = content.substring(1);

} 

void breathe(uint8_t breathe_interval, uint8_t breathe_min_bright, uint8_t breathe_max_bright) {

  uint8_t sinBright = beatsin8(breathe_interval, breathe_min_bright, breathe_max_bright, 0, 0);
  FastLED.setBrightness(sinBright);

} // breathe


void chase(uint8_t chase_interval) {

  // if I do NUM_LEDS - 1 (which actually gives values in the range of possible indexes) the final LED only seems to light up every other 4th time
  uint8_t pos = map(beat8(chase_interval, 0), 0, 255, 0, NUM_LEDS);

  // introduce second wave at 1/2 wavelength 
  uint8_t pos2 = map(beat8(chase_interval, (((60 / chase_interval) * 1000) / 2)), 0, 255, 0, NUM_LEDS);
  leds[pos] = CHSV(0, 0, 255);

  // second wave
  //leds[pos2] = CHSV(0, 0, 255);

  fadeToBlackBy(leds, NUM_LEDS, 70);

} // chase

void palette_move(int palette_interval) {

  EVERY_N_MILLIS(palette_interval) {
    paletteIndex++;
  }

}