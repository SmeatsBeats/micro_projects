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

// sample rate test
int analogTest;
unsigned long newTime;

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
  unsigned long current_millis = millis();

  // test sampling rate 
  /*
  newTime = micros();

  for(int i = 0; i < 1000000; i++) {
    analogTest = analogRead(ANALOG_PIN);
  }

  float conversionTime = (micros() - newTime) / 1000000.0;

  Serial.print("Conversion time: ");
  Serial.print(conversionTime);
  Serial.println(" uS");

  Serial.print("Max sampling frequency: ");
  Serial.print((1.0 / conversionTime) * 1000000);
  Serial.println(" Hz"); 
  */

 // last result 11,000 Hz with analog read


  // read sound sensor digital and analog inputs
  //digiVal = digitalRead(DIGI_PIN);
  analogVal = analogRead(ANALOG_PIN);

  // send data to serial monitor
  // delay to not overwhelm - remove this when not testing
  //if (current_millis - prev_millis >= interval) {
  //  prev_millis = current_millis;
    Serial.println(analogVal);
  //}


  if (UID == "94 D5 E9 85") {
    Serial.println("Blue Fob");
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
    Serial.println("White Fob");
    //fill_solid(leds, NUM_LEDS, CRGB::White);

    chase(60);
    //breathe(30, 50);

    //sawtooth();
  }
  else {
    //Serial.println("Unrecognized.");
    //Serial.println("UID: " + UID);
    //fill_solid(leds, NUM_LEDS, CRGB::Red);
    fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, roseBud, 255, LINEARBLEND);
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));
    //breathe(30, 10, max_bright);
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
  Serial.print("UID tag :");
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

  /*

  FastLED.setBrightness(bright);
  EVERY_N_MILLIS(breathe_interval) {
    if (inhale){
      bright++;
    }
    else{
      bright--;
    }
  }
  // when breathe in is complete, breathe out
  // also need to stop breathing in
  EVERY_N_MILLIS(breathe_interval * (breathe_max_bright + 1)) {
    if (inhale == true) {
      inhale = false;
    }
    else {
      inhale = true; 
    }
  }
  */
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

/*
// when full cycle completes set first one to white and rest to black
//int chase_interval = 30;
EVERY_N_MILLIS(chase_interval * (NUM_LEDS + 1)) {
  chase_i = 0;
  //fill_solid(leds, NUM_LEDS, CHSV(0, 0, 0));
  leds[0].setHSV(0, 0, 255);
}
EVERY_N_MILLIS(chase_interval) {
  leds[chase_i].setHSV(0, 0, 255);
  if(chase_i > 0) {
    leds[chase_i - 1].setHSV(0, 0, 0);
  }
  else {
    leds[NUM_LEDS - 1].setHSV(0, 0, 0);
  }
    
  EVERY_N_MILLIS(chase_interval) {
    chase_i++;
  }
}
  */
} // chase

void palette_move(int palette_interval) {
  EVERY_N_MILLIS(palette_interval) {
    paletteIndex++;
  }
}