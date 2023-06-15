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
#include <Adafruit_NeoPixel.h>

// rfid

#define SS_PIN 5
#define RST_PIN 21
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// neopixel 

#define PIN 25
#define NUMPIXELS 10
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// sound sensor

#define ANALOG_PIN 33
#define DIGI_PIN 32
int analogVal = 0; 
int digiVal; 

// declare functions

void colorWipe(uint32_t c, uint8_t wait);

int current_profile;

void setup() 
{
  // LED test
  pixels.begin();
  pixels.setBrightness(50);
  pixels.show(); // Initialize all pixels to 'off'

  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}
void loop() 
{

  // LED test 

  //LED test
  /*
  byte r = random(0,60);
  byte g = random(0,60);
  byte b = random(0,60);
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    pixels.show();   // Send the updated pixel colors to the hardware.
    delay(10); // Pause before next pass through loop
  }
  */


  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    //Serial.println("No new card?");
    //This means no card is presented to the reader - can be same card twice
    // You want the animations to go in here so they don't get interrupted by each loop 

    switch (current_profile) {
    case 1:

      //Serial.println("Loading BlueFob Profile.");

      //colorWipe(pixels.Color(0, 0, 255), 50); //blue
    
        for(int i=0; i<NUMPIXELS; i++) {
          if ( ! mfrc522.PICC_IsNewCardPresent()) { // if a card is presented during the animation - stop it
            pixels.setPixelColor(i, pixels.Color(0, 0, 60));
            pixels.show();   // Send the updated pixel colors to the hardware.
            //delay(10); // Pause before next pass through loop
          }
        }
      break;
    case 2:

      //Serial.println("Loading WhiteCard Profile.");

      //colorWipe(pixels.Color(255, 255, 255), 50); //white

      for(int i=0; i<NUMPIXELS; i++) {
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
          pixels.setPixelColor(i, pixels.Color(60, 60, 60));
          pixels.show();   // Send the updated pixel colors to the hardware.
          delay(10); // Pause before next pass through loop
        }
      }
      break;
    default:
      Serial.println("No profile found.");
      //colorWipe(pixels.Color(255, 0, 0), 50); //red
      byte r = random(0,60);
      byte g = random(0,60);
      byte b = random(0,60);
      for(int i=0; i<NUMPIXELS; i++) {
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
          pixels.setPixelColor(i, pixels.Color(r, g, b));
          pixels.show();   // Send the updated pixel colors to the hardware.
          delay(10); // Pause before next pass through loop
        }
      }
  }

    return;
  }
  else {
    Serial.println("New card detected!");
  }
  // Select one of the cards
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
  String PID = content.substring(1);
  Serial.println("The PID is: " + PID);

  // create map of IDs and Profile names 
  std::map<String, String> profiles{{"94 D5 E9 85", "BlueFob"}, {"9A AF 7D D4", "WhiteCard"}};

  // find function will return iterable containing key value pair
  // if the desired value is not found, it returns an iterable that represents a point beyond the end of the map
  auto iter = profiles.find(PID);

  if(iter != profiles.end()){
    //Serial.println("Load profile: " + iter->second);
    //delay(2000);

    
    if(iter->second == "BlueFob") {

      current_profile = 1;

    }
    else if(iter->second == "WhiteCard") {

      current_profile = 2;

    }
  
  }
  else {
    //Serial.println("No profile found.");
    //delay(2000);
  }
  

  /*
  if (content.substring(1) == "BD 31 15 2B") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(3000);
  }
 
 else   {
    Serial.println(" Access denied");
    delay(3000);
  }
  */

} 

// Fill the dots one after the other with a color

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}
