#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneButton.h>
#include <cmath>
#include <Wire.h>
#include <LiquidCrystal.h>

#define preset_BTN_PIN 19
#define encoderPinA 22
#define encoderPinB 23


// lcd

//be sure not to use an input only pin 35, 34, 39, 36
const int rs = 27, en = 26, d4 = 25, d5 = 33, d6 = 32, d7 = 18;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
boolean resetLCD = false;
String LCDLineOne; 
String LCDLineTwo;

// rotary encoder

volatile unsigned long lastEncRun = 0;
volatile unsigned long lastIntDetect = 0;
int encoderCount = 0;
// could also do this rounding within relevant functions
int roundedEncCount;
int encoderGuess = 0;
int lastRoundedEncCount;
int convertedEncCount;
static boolean rotating=false;
static boolean intDetect=false;
int numGuesses = 0;


// perhaps the remote should be informed of the preset selected at the device 
// if a certain preset has a parameter with a different range of values, the remote could adjust accordingly
// otherwise - could apply a mapping to the remote values on the device end
// what about the number of editable parameters for different presets? This is a real kicker - you will probably want two way coms
// about preset for this reason - can send metadata on classes to remote 
// maybe need a new struct for this

// preset & device selection

int preset_selected = 0;
int numpresets = 5;
int device_selected = 0;
int numDevs = 2;
int numBtnModes = 3; 

// button setup

/*
// Setup a new OneButton on pin A1.  
OneButton button1(DOWN_BTN_PIN, true);
// Setup a new OneButton on pin A2.  
OneButton button2(UP_BTN_PIN, true);

*/

OneButton button1(preset_BTN_PIN, true);

// switch the functionality of the button by adjusting this
int btnMode = 0; 
int btnVal = 0;

// ESP_NOW

// REPLACE WITH YOUR RECEIVER MAC Address
// maybe better to do declare here then define in setup - see random nerd for possible tutorial on doing automatically 

//uint8_t* broadcastAddressSelected; //this is a pointer
uint8_t broadcastAddress1[] = {0x0C, 0xB8, 0x15, 0xC1, 0xBF, 0x9C};
uint8_t broadcastAddress2[] = {0x78, 0xE3, 0x6D, 0x19, 0xFB, 0xEC};

// need to set a default for selected address
uint8_t* broadcastAddressSelected = broadcastAddress1;

//uint8_t broadcastAddress[] = {0C:B8:15:C1:BF:9C};
//78:E3:6D:19:FB:EC

// Variable to add info about peer
esp_now_peer_info_t peerInfo;

typedef struct remoteData {
    char target_dev[18];
    int param_key = 0; 
    int param_val = 0; 
} remoteData;

// feedback from remote
typedef struct remoteResponse {
    // this will store any error 
    // or if param is changed successfully can be string containing the parameter and new value
    char deviceResponse[100];
    int param_key; 
    int param_val; 
} remoteResponse;

remoteResponse deviceFeedback;
remoteData remoteTransfer;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  memcpy(&deviceFeedback, incomingData, sizeof(deviceFeedback));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.println("Device response: ");
  Serial.println(deviceFeedback.deviceResponse);
  Serial.print("Param_Key: ");
  Serial.println(deviceFeedback.param_key);
  Serial.print("Param_Val: ");
  Serial.println(deviceFeedback.param_val);
  Serial.println();

  if (deviceFeedback.param_key == 2) {
    // preset has been updated 
    preset_selected = deviceFeedback.param_val;
    // if you are in welcome screen you need to update line 2 of display
    if (btnMode == 0) {
      lcd.setCursor(0,1);
      lcd.print("     ");
      lcd.setCursor(0,1);
      LCDLineTwo = "Preset: " + String(preset_selected);
      lcd.print(LCDLineTwo);
    }


  }
  else if (deviceFeedback.param_key == 1) {
    // connection established with device 
    lcd.setCursor(0,1);
    lcd.print("     ");
    lcd.setCursor(0,1);
    LCDLineTwo = "Preset: " + String(device_selected);
    lcd.print(LCDLineTwo);
  }

  // print feedback from device to display
  lcd.setCursor(0,1);
  lcd.print("         ");
  lcd.setCursor(0,1);
  lcd.print(deviceFeedback.deviceResponse);

  resetLCD = true;
  // wait

  delay(3000);

  //lcd.clear();
  //lcd.setCursor(0,0);
  //lcd.print(LCDLineOne);
  lcd.setCursor(0,1);
  // get the length of the msg in next version hehe
  lcd.print("                              ");
  lcd.setCursor(0,1);
  lcd.print(LCDLineTwo);

  resetLCD = false;

  // what if there is an input during feedback? 
  // ideally - clear feedback 


  // if feedback is too long, scroll it work on this later :) 
  /*

  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(150);
  }
  */

}

// put function declarations here:
//int myFunction(int, int);

// rotary encoder 

void rotEncoder();

// track if button pushed 
// int btn3_click = 0;

void confirmConnection(uint8_t* broadcastAddressSelected);
//int readEncoder(int encInc, int limit);
void selectDevice(int device_selected);


// ----- button 1 callback functions

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).

void click1();
void doubleclick1();
void longPressStart1();
void longPress1();
void longPressStop1();


void setup() {
  // put your setup code here, to run once:
  //int result = myFunction(2, 3);
  Serial.begin(115200);   // Initiate a serial communication
  while(!Serial);

  // setup lcd 

  //set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  //Print a message to the LCD.
  lcd.print("Welcome!");


  // get MAC address for ESP_NOW
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
  // 0C:B8:15:C0:E9:5C
  //0C:B8:15:C0:E9:5C

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet

  esp_now_register_send_cb(OnDataSent);

  esp_now_register_recv_cb(OnDataRecv);

  
  //esp_now_peer_info_t peerInfo;


  // Register peer 1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer      
   
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer 1");
    //return;
  }
  

  // Register peer 2
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  //peerInfo.channel = 0;  
  //peerInfo.encrypt = false;
  
  // Add peer 2  
    
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer 2");
    //return;
  }

  // link the button 1 functions.
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);
  
  // rotary encoder setup 

  pinMode (encoderPinA, INPUT_PULLUP);
  pinMode (encoderPinB, INPUT_PULLUP);
  //pinMode (encoderPinA,INPUT);
  //pinMode (encoderPinB,INPUT);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), rotEncoder, CHANGE);

}

void loop() {
  
  button1.tick();

  // is this doing anything?

  // if(remoteTransfer.param_key == 0) {

  //   //update preset if remote has sent value for it

  //   preset_selected = remoteTransfer.param_val;

  //   //Serial.println("preset from remote: ");
  //   //Serial.println(preset_selected);

  // }

  

  // if there is no button click you just want to send the same preset that is currently on the device back to it
  // maybe I should just make a separate struct for the preset data and only send it on the click
  // you already have this struct containing only p... 
  // could be complicated to handle receiving two different structs... 

  // need to set selected preset based on what the device sends back 
  //preset_selected = deviceData.p;

  // if you only send data on click, none of this data gets sent
  // on the other hand, do you really need to send data every loop? 
  // maybe come up with a strategy to only send data when a value changes
  
  //encoderCount = lastRoundedEncCount;

  // behavior of rotary encoder depends which mode it is in 
  // increment size for encoder 
  // will you ever really use more than 1?
  // default

  // need to move this functionality into the part that guesses the increment

  float encInc = 0.5; 

  // encLimit when to loop back to start of possible selections
  
  int encLimit = 60; 
  // what if there is a hard ceiling? as in you don't want to loop back but stop at max value
  // set this to 1 
  int encLoop = 0;

  // in some instances you will want the device to update as you scroll e.g. adjust hue 
  // in others, nothing should happen until a button click confirms e.g. update preset 


  switch(btnMode) {
    case 0: 
      // nothing
   
      break;
    case 1:
      // device mode
      encInc = 0.5; 
      // set limit to equal number of devices/presets etc 
      // not index of highest one 
      encLimit = 2;
      encLoop = 1;
      break;
    case 2: 
      // preset mode
      encInc = 0.5;
      encLimit = 5;
      encLoop = 1;
      break;
    case 3: 
      // parameter mode
      break; 
  }

  while (rotating) {
    //delay(2);  // debounce by waiting 2 milliseconds
               // (Just one line of code for debouncing)

    // what if I fully commit to this rounding idea and instead of debouncing with timer 
    // just assume that the majority of registered pulses will be correct
    // bad assumption

    // ideally - isolate group between 5 and 50 

    int timeDiff = millis() - lastEncRun;

    if(timeDiff < 5)
      return;
 
    if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) {
      //direction = 1;
      //encoderCount--;
      // this is not ruling out all occurrences more than 10 ms after beginning of detent 
      // this rules out occurrences that do not occur with in 10 ms of prev
      //encoderCount = encoderCount - encInc;
      encoderGuess--;
      //Serial.println("decrement");
      
    }
    else {
      //encoderCount++;
      //encoderCount = encoderCount + encInc;
      //Serial.println("Increment");
      encoderGuess++;
      //direction = 2;
    }    

    //Serial.println(millis() - lastEncRun);              
    numGuesses++;
    lastEncRun = millis();
    rotating=false; // Reset the flag
    
  }

  // if you base this timer on last encoder run, and someone spams the encoder, will it update during rotation?
  // maybe just call periodically after each detected rotation 
  // you want to use a variable you can reset to 0 here rather than in the if statement for debouncing 
  // but you want to base it on the time that a detent takes on average
  // this is so sketchy surely there is a better encoder 

  // don't want to run instantly on first detent
  // if you make this too low, you are back to square 1 - multiple guess per detent
  // the whole idea is to wait a few detents 
  // how do I handle fast spinning? 
  // buffer? 
  // if you get a huge positive of negative, beef up the increment?
  // will this be millis from first or last guess generated?
  if ((intDetect == true && ((millis() - lastIntDetect) > 100))) {

    Serial.println("Num Guesses: ");
    Serial.println(numGuesses);
    //boolean fastSpin;
    int spinInc = 1;
    // try to detect fast spin 
    // maybe go off sheer number of guesses 
    if (abs(numGuesses) > 10) {
      Serial.println("fast spin");
      //fastSpin = true;
      spinInc = 5;
    }


    //make assessment based on madness of each detent
    if (encoderGuess >= 0) {
      encoderCount = encoderCount + spinInc;
    }
    else {
      encoderCount = encoderCount - spinInc;
    }

    Serial.println("Guess: ");
    Serial.println(encoderGuess);
    Serial.println("Current: ");
    Serial.println(encoderCount);
    // wait until two occurrences of relevant increment have occurred? Sometimes there is only one... 
    // can do two, then wait for a specified interval and handle if only one 
    // convert to int 
    // what does static mean here?
    //roundedEncCount = static_cast<int>(std::round(encoderCount));
    // just round here
    // Serial.println("Unrounded: ");
    // Serial.println(encoderCount);

    //roundedEncCount = std::round(encoderCount);
    //roundedEncCount = std::floor(encoderCount);

  

    // do I need to now set encoderCount to this? 
    // wow this made sense in my head but did not work
    // maybe best to do this conversion later, before actually using the value as index
    //encoderCount = static_cast<float>(roundedEncCount);

    //Serial.println(roundedEncCount);

    // apply limits if applicable
    //encoderCount = std::round(encoderCount);

    //encoderCount = static_cast<int>(encoderCount);

    if (encLoop) {
      if (encoderCount < 0) {
        encoderCount = encLimit - 1;
      }
      encoderCount = max(0, ((encoderCount) % (encLimit)));
    }
    else {
      encoderCount = max(0, min(encoderCount, encLimit - 1));
    }

   

    // Serial.println("Rounded: ");
    // Serial.println(roundedEncCount);

    // Serial.println("Capped: ");
    // Serial.println(roundedEncCount);

    // make sure the two don't deviate
    //encoderCount = roundedEncCount;

    // Serial.println("Last rounded: ");
    // Serial.println(lastRoundedEncCount);




    // and now convert 
    //convertedEncCount = static_cast<int>(roundedEncCount);

    // if (roundedEncCount != lastRoundedEncCount) {
    //   Serial.println("Current Selection: ");
    //   Serial.println(roundedEncCount);
    //   Serial.println("Unrounded: ");
    //   Serial.println(encoderCount);
  
    // }

    // put your main code here, to run repeatedly:
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    //lcd.clear();
    // what if this happens while feedback displayed? 
    if (resetLCD) {
      lcd.setCursor(0,1);
      // get the length of the msg in next version hehe
      lcd.print("                              ");
    }

    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    //lcd.print(millis() / 1000);
    LCDLineTwo = String(encoderCount);
    lcd.print(encoderCount);

    // when you go to double digits and back to single, the 0 will stay

    // reset encoder guess 
    encoderGuess = 0;
    //lastRoundedEncCount = roundedEncCount;

    //encoderCount = roundedEncCount;
    intDetect = false; 
    numGuesses = 0;
  }
 

}

// put function definitions here:
/*
int myFunction(int x, int y) {
  return x + y;
}
*/

void click1() {

  // base behavior on btnMode 
  // need a button mode that does nothing! or just reports something 

  switch(btnMode) {
    case 0: 
      // default mode 
      // do nothing or just display some info / graphic

      break;
    case 1: 
      // select device with chosen ID
      remoteTransfer.param_key = 1;
      // should param val be used to store the device id in future?
      remoteTransfer.param_val = 1; 
      device_selected = encoderCount;
      selectDevice(device_selected);
      break;
    case 2: 
      // select preset with chosen ID
      // preset is tracked at the device
      remoteTransfer.param_key = 2; 
      // reserve this to mean increment pattern by 1
      remoteTransfer.param_val = encoderCount; 
      Serial.println("Select preset with ID: "); 
      Serial.println(remoteTransfer.param_val);

      // confirm the connection and pass the data
      confirmConnection(broadcastAddressSelected);
      break;
  }
 
} // click3

// This function will be called when the button3 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("Button 3 doubleclick.");

  // use double click for mode specific increment function 
  // e.g. skip to next preset in preset mode 
  // skip to next device in device mode

  switch(btnMode) {
    case 0: 
      // default mode -- do nothing
      break; 
    case 1:

      // this also seems to copy the preset from the previous connected device
      // and causes lcd issue like this: 
      // Select Device: 
      // Preset: 3

      // it is safe to handle device selection here, because the device cannot be selected from the absorber
      // what if it is selected from webapp?
      // the remote or app are still the final element that need to know which device is selected
      // two devices but index starts at 0 so 
      device_selected = (device_selected + 1) % (numDevs);
      
      selectDevice(device_selected);
      remoteTransfer.param_key = 1;
      remoteTransfer.param_val = 1; 
      // after selection exit to default mode
      //btnMode = 0; 
      break;
    case 2: 
      // increment preset
      // handle the incrementing on the device side 
      // just send code here that tells it to increment by one 
      remoteTransfer.param_key = 2; 
      // reserve this to mean increment pattern by 1
      remoteTransfer.param_val = 111; 
      // confirm the connection and pass the data
      confirmConnection(broadcastAddressSelected);
      break;
  }


} // doubleclick3

// This function will be called once, when the button1 is pressed for a long time.
void longPressStart1() {

  // use this to increment through the button modes 
  // welcome mode probably shouldn't be part of this loop 
  // just a mode to go to lacking another selection 

  btnMode = (btnMode + 1) % (numBtnModes); 

  Serial.println("Set button to mode: ");
  Serial.println(btnMode);

  // reset encoder count 
  encoderCount = 0;
  encoderGuess = 0;

  // may need to request info on current value of relevant parameter from device now 
  // or can I continue to handle this entirely on device side?

  // update the screen when mode changed
  switch(btnMode) {
    case 0: 
      // default mode 
      // do nothing or just display some info / graphic
      lcd.clear();
      lcd.setCursor(0,0);
      // show current device
      LCDLineOne = "Device: " + String(device_selected);
      lcd.print(LCDLineOne);
      lcd.setCursor(0,1);
      LCDLineTwo = "Preset: " + String(preset_selected);
      lcd.print(LCDLineTwo);
      break;
    case 1: 
      // select device with chosen ID
      lcd.clear();
      lcd.setCursor(0,0);
      LCDLineOne = "Select Device:";
      lcd.print(LCDLineOne);
      lcd.setCursor(0,1);
      LCDLineTwo = String(encoderCount);
      lcd.print(LCDLineTwo);

      break;
    case 2: 
      // select preset with chosen ID
      // preset is tracked at the device
      lcd.clear();
      lcd.setCursor(0,0);
      LCDLineOne = "Select Preset:";
      lcd.print(LCDLineOne);
      lcd.setCursor(0,1);
      LCDLineTwo = String(encoderCount);
      lcd.print(LCDLineTwo);
      break;
  }



} // longPressStart3


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
  //Serial.println("Button 3 longPress...");
} // longPress3


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  //Serial.println("Button 3 longPress stop");
} // longPressStop3

void confirmConnection(uint8_t* broadcastAddressSelected) {

  // set target device to this address converted to char 
  char targetDevStr[18];
  snprintf(targetDevStr, sizeof(targetDevStr), "%02X:%02X:%02X:%02X:%02X:%02X", broadcastAddressSelected[0], broadcastAddressSelected[1], broadcastAddressSelected[2], broadcastAddressSelected[3], broadcastAddressSelected[4], broadcastAddressSelected[5]);

  strncpy(remoteTransfer.target_dev, targetDevStr, sizeof(remoteTransfer.target_dev));
  remoteTransfer.target_dev[sizeof(remoteTransfer.target_dev) - 1] = '\0'; // Ensure null-terminated

  Serial.println("Identify yoself! Device confirmation requested for:");
  Serial.println(remoteTransfer.target_dev);

  // request confirmation from device

  // this will send over whatever param_key and val are set to currently
  // be sure to define them appropriately before sending

  esp_err_t result = esp_now_send(broadcastAddressSelected, (uint8_t *) &remoteTransfer, sizeof(remoteTransfer));
  
  if (result == ESP_OK) {
    // if you comment this line out the LEDs on the remote go out lol
    Serial.println("Device confirmation request sent with success for:");
    Serial.println(remoteTransfer.target_dev);
  }
  else {
    Serial.println("Error sending the device confirmation request");
    Serial.println(result);
  }

};

void selectDevice(int device_selected) {
  switch(device_selected) {
    case 0: 
      Serial.println("Connecting to device1.");
      device_selected = 0;
      broadcastAddressSelected = broadcastAddress1;
      break;
    case 1:
      Serial.println("Connecting to device2.");
      device_selected = 1;
      broadcastAddressSelected = broadcastAddress2;
      break; 
    case 2: 
      Serial.println("You don't have this many devices.");
  }
  confirmConnection(broadcastAddressSelected);
};

void rotEncoder()
{
  rotating=true; // If motion is detected in the rotary encoder,
                 // set the flag to true
  intDetect = true;
  lastIntDetect = millis();
};




