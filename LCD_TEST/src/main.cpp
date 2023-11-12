#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneButton.h>
#include <cmath>
#include <Wire.h>
#include <LiquidCrystal.h>

// be sure not to use an input only pin 35, 34, 39, 36
const int rs = 27, en = 26, d4 = 25, d5 = 33, d6 = 32, d7 = 18;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// put function declarations here:
//int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  //int result = myFunction(2, 3);
    // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // put your main code here, to run repeatedly:
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
  //Serial.println(millis() / 1000);
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }