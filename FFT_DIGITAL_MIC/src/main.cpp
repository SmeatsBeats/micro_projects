/*
 * 
 * All the resources for this project: https://randomnerdtutorials.com/
 * Modified by Rui Santos
 * 
 * Created by FILIPEFLOP
 * 
 */

#include <Arduino.h>
#include "audio_reactive.h"
#include <FastLED.h>
#include <SPI.h>
#include <MFRC522.h>
#include <map>
#include <esp_now.h>
#include <WiFi.h>
#include <vector>

// profile selection

int profile_selected = 0;

// ESP_NOW

// Structure example to receive data
// Must match the sender structure
/*
typedef struct struct_message {
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;
*/

typedef struct struct_message {
    int h;
    int s;
    int v;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  /*
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData.a);
  Serial.print("Int: ");
  Serial.println(myData.b);
  Serial.print("Float: ");
  Serial.println(myData.c);
  Serial.print("Bool: ");
  Serial.println(myData.d);
  Serial.println();

  */
}

// pot

#define POT_PIN 34 

// photocell

#define LIGHT_PIN 33 

// smoothing for photocell test 
const int numReadings = 15;

int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int average = 0;            // the average

// you want to average each fft bin of interest

const int num_bins = 16;
const int fft_num_readings = 8;
int fft_readings[num_bins][fft_num_readings];  // the readings from the analog input
int fft_readIndex = 0;          // the index of the current reading
int fft_total[num_bins] = {0};              // the running total
int fft_average[num_bins] = {0};            // the average

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
//#define NUM_LEDS 10          
#define NUM_LEDS 300                                  // Number of LED's.

int thisdelay = 20;
uint8_t max_bright = 85;

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
void winter();
void rose();
void remote();
void react();
void auto_light();


int current_profile;
String UID;

void setup() 
{
  // Serial monitor 
  Serial.begin(115200);   // Initiate a serial communication
  while(!Serial);

  // LED test
  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500); 

  // Setup RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  // digital mic 
  setupAudio();

  // setup photocell smoothing test
    // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  // get MAC address for ESP_NOW
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
  //0C:B8:15:C1:BF:9C

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  
}

void loop()
{
  //print selected bands to plotter

  /*

  int startBand = 0;
  int endBand = 7;

  for(int i = startBand; i <= endBand; i++) {
    Serial.print("Band " + String(i) +  ":");
    //Serial.print(fftResult[i]); 
    if (i >= endBand) {
      Serial.println(fftResult[i]);
    }
    else {
      Serial.print(fftResult[i]); 
      Serial.print(",");
    }
   
  }
  
  */

 // test pot 

 int potVal = analogRead(POT_PIN);
 //Serial.println("POT VALUE: ");
 //Serial.println(potVal);

  // test photocell 

  int ambLight = analogRead(LIGHT_PIN);
  //Serial.println(ambLight);

  // smoothing avg for photocell 
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = ambLight;
    // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:

  
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  

  // calculate the average:
  average = total / numReadings;


  // use same averaging approach for fft bins 

  for (int bin = 0; bin < num_bins; bin++) {
    // Subtract the oldest value from the running sum
    fft_total[bin] = fft_total[bin] - fft_readings[bin][fft_readIndex];

    fft_readings[bin][fft_readIndex] = fftResult[bin]; 
    
    // Add the new FFT data value to the running sum
    fft_total[bin] = fft_total[bin] + fft_readings[bin][fft_readIndex]; // Assuming fftResult contains FFT data for the bin
    
    // Update the running average for this bin
   // fft_average[bin] = fft_total[bin] / fft_num_readings; // You can change numReadings if needed
  }

// Update the index for the next FFT data point
fft_readIndex = fft_readIndex + 1;

// Wrap around if necessary
if (fft_readIndex >= fft_num_readings) {
  fft_readIndex = 0;
}

  for (int bin = 0; bin < num_bins; bin++) {
 
    // Update the running average for this bin
    fft_average[bin] = fft_total[bin] / fft_num_readings; // You can change numReadings if needed
  }



  //Serial.println(255 - (average/20));


  //if (UID == "94 D5 E9 85") {
  // switch to new adhesive disks
  // Blue
  if (UID == "6B 49 B8 B6") {
    profile_selected = 1;

    //Serial.println("Blue Fob");
    //fill_solid(leds, NUM_LEDS, CRGB::Blue);

    // 10/26
    /*
    fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, iceFade, 255, LINEARBLEND);
    breathe(15, 10, max_bright);
    palette_move(1);
    */
  }
  // White 
    else if (UID == "0B 83 C7 B6") {
    
    //remote
    profile_selected = 4;
    
    //Serial.println("White Fob");
    //chase(60);

    //Serial.println("Band 1: ");
    //Serial.println(fftResult[1]);

    // 10/26
    /*

    fill_solid(leds, NUM_LEDS, CRGB::White);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(myData.h, myData.s, myData.v);
    }
    */
    
  }
  //yellow
  else if (UID == "2B 72 BE B6") {
    // react

    profile_selected = 2;

    //Serial.println("White Fob");
    //chase(60);
    //Serial.println("Major Peak: ");
    //Serial.println(FFT_MajorPeak);

    //Serial.println("Band 1: ");
    //Serial.println(fftResult[1]);

    
    //fill_solid(leds, NUM_LEDS, CHSV( FFT_MajorPeak/5, 0, 55 ));


    // 10/26
    /*
    

    int groups = 10;
    int lights = NUM_LEDS / groups;

    //kick flash 

    if (fftResult[1] > 700) {

      fill_solid(leds, NUM_LEDS, CRGB::White);
    
    }

    for (int r = 1; r <= groups; r++) {
      for (int i = r * lights - lights; i < r * lights; i++) {
        //leds[i] = CHSV(fftResult[r+3], 150, fftResult[r+3]);
        leds[i] = CHSV(fft_average[r+3], 150, fft_average[r+3]);
        //leds[i] = CHSV(90, 150, fftResult[i+3]);
      } // each LED
    }

  */

  /*
    EVERY_N_MILLIS(10) { // maybe this will smooth it out a little? - see rough avg approach using prevfft
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(fftResult[i+4], 150, fftResult[i+4]);
        //leds[i] = CHSV(90, 150, fftResult[i+3]);
      } // each LED
    } // every N millis
    */

  }
  
  // green
  else if (UID == "CB 9A BF B6") {

    profile_selected = 5;

    // 10/26

    /*

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(0, 0, 0);
    }

    // try using photocell to set brightness according to ambient light 
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CHSV(0, 0, max(0, (255 - average/17)));
      }
    
    //fill_solid(leds, NUM_LEDS, CRGB::Green);

    */

  }
  //pink - unknown
  else {

    profile_selected = 0;

    //Serial.println("Unrecognized.");
    //Serial.println("UID: " + UID);
    //fill_solid(leds, NUM_LEDS, CRGB::Red);

    // 10/26
    fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, roseBud, 255, LINEARBLEND);
    
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));

    // 10/26
    //breathe(30, 10, max_bright);

    /*
    if (fftResult[1] > 1100) {

      FastLED.setBrightness(0);
    
    }

    if (fftResult[7] > 3100) {

      FastLED.setBrightness(max_bright);
    
    }

    */

    //Serial.println("Band 3: ");
    //Serial.println(fftResult[7]);


  }

  // run profile based on selection
  switch(profile_selected) {
    case 0: 
      rose();
      break;
    case 1:
      winter();
      break;
    case 3:
      react();
      break;
    case 4: 
      remote();
      break;
    case 5:
      auto_light();
      break;
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
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
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

void winter() {
    fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, iceFade, 255, LINEARBLEND);
    breathe(15, 10, max_bright);
    palette_move(1);
}

void rose() {
    fill_palette(leds, NUM_LEDS, 0, 255 / NUM_LEDS, roseBud, 255, LINEARBLEND);
    
    //FastLED.setBrightness(map(analogVal, 3000, 3275, 0, max_bright));
    breathe(30, 10, max_bright);
}

void remote() {
    fill_solid(leds, NUM_LEDS, CRGB::White);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(myData.h, myData.s, myData.v);
    }
}

void react() {
      int groups = 10;
    int lights = NUM_LEDS / groups;

    //kick flash 

    if (fftResult[1] > 700) {

      fill_solid(leds, NUM_LEDS, CRGB::White);
    
    }

    for (int r = 1; r <= groups; r++) {
      for (int i = r * lights - lights; i < r * lights; i++) {
        //leds[i] = CHSV(fftResult[r+3], 150, fftResult[r+3]);
        leds[i] = CHSV(fft_average[r+3], 150, fft_average[r+3]);
        //leds[i] = CHSV(90, 150, fftResult[i+3]);
      } // each LED
    }
}

void auto_light() {
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(0, 0, 0);
    }

    // try using photocell to set brightness according to ambient light 
      for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CHSV(0, 0, max(0, (255 - average/17)));
      }
}