#include <Arduino.h>

// Define Pins
#define BLUE 14
#define GREEN 12
#define RED 13
// add pins for buttons 
#define BUTTONAPIN 27
#define BUTTONBPIN 26

// put function declarations here:
void turnLightOff();
void fadeColors(int fadeInPin, int fadeOutPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(BUTTONAPIN, INPUT_PULLUP);
  pinMode(BUTTONBPIN, INPUT_PULLUP);
  //digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

}

// define variables
int redValue;
int greenValue;
int blueValue;

void loop() {

// put your main code here, to run repeatedly:
  #define delayTime 10 // fading time between colors

  redValue = 255; // choose a value between 1 and 255 to change the color.
  greenValue = 0;
  blueValue = 0;

  fadeColors(GREEN, RED);
  fadeColors(BLUE, GREEN);
  fadeColors(RED, BLUE);


  Serial.println(digitalRead(BUTTONAPIN));

}

// put function definitions here:
void turnLightOff() {
    analogWrite(RED, 0);
    analogWrite(GREEN, 0);
    analogWrite(BLUE, 0);
}

void fadeColors(int fadeInPin, int fadeOutPin) {
  
  redValue = 0;
  greenValue = 0;
  blueValue = 255;

  for(int i = 0; i < 255; i += 1) // fades out blue bring red full when i=255
  {
    if(digitalRead(BUTTONAPIN) == LOW) {
      turnLightOff();
      Serial.println("Turn off.");
    }
    else {
      analogWrite(fadeOutPin, 255 - i);
      analogWrite(fadeInPin, i);
      delay(delayTime);
    }
  }
}