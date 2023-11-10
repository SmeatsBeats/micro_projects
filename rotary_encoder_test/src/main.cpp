#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <cmath>

#define POT_LED_PIN 18
#define SLIDE_LED_PIN 19
#define BTN_LED_PIN 21
#define POT_PIN 32
#define SLIDE_PIN 35
#define DOWN_BTN_PIN 26
#define UP_BTN_PIN 27
#define PROFILE_BTN_PIN 14
#define encoderPinA 12
#define encoderPinB 13
// clockwise direction

// counter-clockwise direction 

// #define encoderPinA 33
// #define encoderPinB 25

// rotary encoder


volatile unsigned long lastEncRun = 0;
volatile float encoderCount = 0;
static boolean rotating=false;

void rotEncoder()
{
  rotating=true; // If motion is detected in the rotary encoder,
                 // set the flag to true
}

void setup() {
  // put your setup code here, to run once:
  //int result = myFunction(2, 3);
  Serial.begin(115200);   // Initiate a serial communication
  while(!Serial);

  
  // rotary encoder setup 

  pinMode (encoderPinA,INPUT_PULLUP);
  pinMode (encoderPinB,INPUT_PULLUP);
  //pinMode (encoderPinA,INPUT);
  //pinMode (encoderPinB,INPUT);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), rotEncoder, CHANGE);
   
  // Reads the initial state of the encoderPinA
  //aLastState = digitalRead(encoderPinA);   

}

void loop() {

  while (rotating) {
    //delay(2);  // debounce by waiting 2 milliseconds
               // (Just one line of code for debouncing)

    if(millis() - lastEncRun < 2)
      return;

    if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) {
      //direction = 1;
      //encoderCount--;
      encoderCount = encoderCount - 0.5;
    }
    else {
      //encoderCount++;
      encoderCount = encoderCount + 0.5;
      //direction = 2;
    }                          
    
    lastEncRun = millis();
    rotating=false; // Reset the flag
    
  }


  // convert to int 
  int roundedEncCount = static_cast<int>(std::round(encoderCount));
  Serial.println(roundedEncCount);
  //Serial.println(encoderCount);

}

// put function definitions here:




