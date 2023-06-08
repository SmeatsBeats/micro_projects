#include <Arduino.h>

// Define Pins
#define BLUE 14
#define GREEN 12
#define RED 13
// add pins for buttons 
#define BUTTONAPIN 27
#define BUTTONBPIN 26

// put function declarations here:
int myFunction(int, int);
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

  //analogWrite(RED, redValue);

  // this is unnecessary as we've either turned on RED in SETUP
  // or in the previous loop ... regardless, this turns RED off
  // analogWrite(RED, 0);
  // delay(1000);

  /*
  for(int i = 0; i < 255; i += 1) // fades out red bring green full when i=255
  {

    if(digitalRead(BUTTONAPIN) == LOW) {
      turnLightOff();
      Serial.println("Turn off.");
    }
    else {
      redValue -= 1;
      greenValue += 1;
      // The following was reversed, counting in the wrong directions
      // analogWrite(RED, 255 - redValue);
      // analogWrite(GREEN, 255 - greenValue);
      analogWrite(RED, redValue);
      analogWrite(GREEN, greenValue);
      delay(delayTime);
    }
  }

  redValue = 0;
  greenValue = 255;
  blueValue = 0;

  for(int i = 0; i < 255; i += 1) // fades out green bring blue full when i=255
  {
  
    if(digitalRead(BUTTONAPIN) == LOW) {
      turnLightOff();
      Serial.println("Turn off.");
    }
    else {
      greenValue -= 1;
      blueValue += 1;
      // The following was reversed, counting in the wrong directions
      // analogWrite(GREEN, 255 - greenValue);
      // analogWrite(BLUE, 255 - blueValue);
      analogWrite(GREEN, greenValue);
      analogWrite(BLUE, blueValue);
      delay(delayTime);
    }
  }

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
      // The following code has been rearranged to match the other two similar sections
      blueValue -= 1;
      redValue += 1;
      // The following was reversed, counting in the wrong directions
      // analogWrite(BLUE, 255 - blueValue);
      // analogWrite(RED, 255 - redValue);
      analogWrite(BLUE, blueValue);
      analogWrite(RED, redValue);
      delay(delayTime);
    }
  }
  */
  /*
  if(digitalRead(BUTTONBPIN) == LOW) {

    digitalWrite(BLUE, LOW);
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);

  }
  */
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