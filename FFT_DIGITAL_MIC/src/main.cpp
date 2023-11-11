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
#include <remoteData.h>
#include <paletteData.h>
#include <LightEffects.h>
#include <Rose.h>
#include <Ice.h>
#include <CustomHSV.h>
//#include <LightEffects.h>


// profile selection

int profile_selected = 0;

// ESP_NOW

// for now just use one address to send data back to remote - in future could be cool to communicate with other nearby boards

//uint8_t remoteAddress[] = {0x0C, 0xB8, 0x15, 0xC0, 0xE9, 0x5C};
// remote address
//0C:B8:15:C0:E9:5C

// Variable to add info about peer
esp_now_peer_info_t peerInfo;

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

/*
typedef struct from_remote {
    // what if you send updates individually this avoids resending static data
    // does the remote ever need to send more than one value per message?
    // could even use this structure for profile
    // param_key
    // param_val
    int h;
    int s;
    int v;
    int p;
} from_remote;

typedef struct to_remote {
    int p;
} to_remote;

// Create struct_messages
from_remote remoteData;
to_remote deviceData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&remoteData, incomingData, sizeof(remoteData));
  
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  // Serial.print("Char: ");
  // Serial.println(remoteData.a);
  // Serial.print("Int: ");
  // Serial.println(remoteData.b);
  // Serial.print("Float: ");
  // Serial.println(remoteData.c);
  // Serial.print("Bool: ");
  // Serial.println(remoteData.d);
  // Serial.println();

  
}
*/

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
// would also be cool to set num_readings independently for each one sheesh
// then you could fine tune how different octaves respond to transients etc.

const int num_bins = 16;
const int fft_num_readings = 10;
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

/*
// Palette definitions

DEFINE_GRADIENT_PALETTE (rosebud) {
  0, 131, 58, 180,
  89, 253, 29, 29,
  166, 252, 176, 69, 
  255, 131, 58, 180
};



DEFINE_GRADIENT_PALETTE (ice) {
  0, 63, 58, 156,
  84, 14, 14, 175,
  168, 17, 215, 255, 
  255, 63, 58, 156
};

*/

//CRGBPalette16 roseBud = rosebud;
//CRGBPalette16 iceFade = ice;

//uint8_t paletteIndex = 0;

struct CRGB leds[NUM_LEDS];

//CRGB rgbval(255,255,255);

// multiple tasks without delay

unsigned long prev_millis = 0;
int interval = 300; 

// declare functions

//void palette_move(int palette_interval);
void chase(uint8_t chase_interval);
int chase_i = 1; 
//void breathe(uint8_t breathe_interval, uint8_t breathe_min_bright, uint8_t breathe_max_bright);
uint8_t bright = 0;
//void winter();
void runIce();
void runRose();
//void remote();
void runCustomHSV();
void react();
void auto_light();
void updateProfile(int profile_id);


String UID;


// include patterns after global variables defined
// going forward probably better to create a seaprate header with shared variables?
//#include <Rose.h>


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

  // Convert the MAC address to a string
  //char receivedMacStr[18];
  //snprintf(receivedMacStr, sizeof(receivedMacStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  //0C:B8:15:C1:BF:9C

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_register_send_cb(OnDataSent);

   // Register peer 1
  memcpy(peerInfo.peer_addr, remoteAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer      
   
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer 1");
    //return;
  }
  
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
  // this probably belongs in a separate file 
  // also, the FFT is only necessary for reactive patterns - running it for 
  // non-reactive patterns is inefficient 

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


  
  // if you do this here, UID never matters. What criteria can be used to decide when one should be used?
  // if new button value is different from previous, then set based on the button value, otherwise use UID
  // what if you go off of same UID?

  // if the button is different from the UID value... hopefully 
  //if (remoteData.p != profile_selected) {
    //profile_selected = remoteData.p;
  //}

  //////////////// PROCESS REMOTE DATA //////////////////

  // first thing to do is confirm this is the intended device for the communication 
  // compare MAC of this device to selected device
  // maybe this really belongs in the callback onDataRecv

  if(correctDevice == 1) {
    // correct device has been reached
    // feedback in this case will be the parameter that is updated and value
    // as a mock test: 

    //deviceFeedback.deviceResponse = "Correct device successfully reached!";

    Serial.println("proceed to process data");

    if(remoteTransfer.param_key == 2) {

      //update profile if remote has sent value for it

      Serial.println("Parameter value receieved: ");
      Serial.println(remoteTransfer.param_val);

      if (remoteTransfer.param_val == 111) {
        // reserved value signal to increment current id by one
        // start with 0 so use one less than actual number 
        int numProfiles = 5; 
        profile_selected = (profile_selected + 1) % numProfiles;

      }
      else {
        profile_selected = remoteTransfer.param_val;
      }

      Serial.println("Loading profile with id: ");
      Serial.println(profile_selected);

      //updateProfile(profile_selected);

    }

    // only run this when data is received from the remote
    correctDevice = 0;

  }
  else {
    //deviceFeedback.deviceResponse = "Incorrect device contacted.";
  }


  updateProfile(profile_selected);

  // just set profile in this main file
  // then map params accordingly in separate files





  // this will override the profile selected based on UID not if remoteData is sending that back

  // profile_selected = remoteData.p;

  // Serial.println("Profile from remote: ");
  // Serial.println(profile_selected);


  // you will need to communicate this selection to the remote whether or not RFID scanned 

  // feedback to remoteData -- you need to actually send this back or the other device will keep sending over what it thinks p is


  /*
  // run profile based on selection - this needs to happen whether or not RFID scanned 
  switch(profile_selected) {
    case 0: 
      runRose();
      break;
    case 1:
      //winter();
      runIce();
      break;
    case 2:
      react();
      break;
    case 3: 
      remote();
      break;
    case 4:
      auto_light();
      break;
  }
  
  FastLED.show();

  */

  // Reset loop when idle
  if ( ! mfrc522.PICC_IsNewCardPresent()) {

    // no card scanned so just sending back the same profile
    // is it really necessary to send data if the profile has not changed?
    
    //remoteTransfer.param_key = 0;
    //remoteTransfer.param_val = profile_selected;
    /*

    esp_err_t result1 = esp_now_send(remoteAddress, (uint8_t *) &remoteTransfer, sizeof(remoteTransfer));
    
    if (result1 == ESP_OK) {
      //Serial.println("Sent the idle data with success");
    }
    else {
      Serial.println("Error sending the same data");
    }
    */

    return;
  }

  // if a new card is present, the loop proceeds 

    
 
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

  // select profile via RFID tag
  // very annoyed by lack of switch for string
  
  if (UID == "6B 49 B8 B6") {
    // Blue
    // winter
    profile_selected = 1;
  }
  else if (UID == "2B 72 BE B6") {
    // yellow
    // react
    profile_selected = 2;
  }
  else if (UID == "0B 83 C7 B6") {
    // White 
    //remote
    profile_selected = 3;
  }
  else if (UID == "CB 9A BF B6") {
    // green
    // auto_light
    profile_selected = 4;
  }
  else {
    //pink - unknown
    // not recognized
    // rose
    profile_selected = 0;
  }

  /*
  // feedback to remoteData -- you need to actually send this back or the other device will keep sending over what it thinks p is
  deviceData.p = profile_selected;

  esp_err_t result = esp_now_send(remoteAddress, (uint8_t *) &deviceData, sizeof(deviceData));
   
  if (result == ESP_OK) {
    Serial.println("Sent updated profile with success");
  }
  else {
    Serial.println("Error sending the updated data");
  }
  //delay(800);
  */
  // feedback to remoteData -- you need to actually send this back or the other device will keep sending over what it thinks p is
  //remoteTransfer.param_key = 0;
  //remoteTransfer.param_val = profile_selected;

  /*

  esp_err_t result = esp_now_send(remoteAddress, (uint8_t *) &remoteTransfer, sizeof(remoteTransfer));
   
  if (result == ESP_OK) {
    Serial.println("Sent updated profile with success");
  }
  else {
    Serial.println("Error sending the updated data");
  }
  */

} 
/*
void breathe(uint8_t breathe_interval, uint8_t breathe_min_bright, uint8_t breathe_max_bright) {

  uint8_t sinBright = beatsin8(breathe_interval, breathe_min_bright, breathe_max_bright, 0, 0);
  FastLED.setBrightness(sinBright);

} // breathe
*/

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

/*
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

*/

void runIce() {
  Ice ice;
  ice.runPattern(leds, NUM_LEDS, max_bright);
}

void runRose() {
  Rose rose;
  // use default params if not provided 
  // otherwise use provided params
  rose.runPattern(leds, NUM_LEDS, max_bright, 30, 10);
}

void runCustomHSV() {
  CustomHSV customHSV;
  customHSV.runPattern(leds, NUM_LEDS, remoteTransfer);
}



/*
void remote() {
    fill_solid(leds, NUM_LEDS, CRGB::White);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(remoteData.h, remoteData.s, remoteData.v);
    }
}
*/


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

void updateProfile(int profile_id) {
  // run profile based on selection - this needs to happen whether or not RFID scanned 
  switch(profile_id) {
    case 0: 
      runRose();
      break;
    case 1:
      //winter();
      runIce();
      break;
    case 2:
      react();
      break;
    case 3: 
      runCustomHSV();
      break;
    case 4:
      auto_light();
      break;
  }
  FastLED.show();
}