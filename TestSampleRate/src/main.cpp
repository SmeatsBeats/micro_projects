#include <Arduino.h>

// sample rate test
#define ANALOG_PIN 35

int analogTest;
unsigned long newTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
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
}