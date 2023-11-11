#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneButton.h>
#include <cmath>
#include <Wire.h>
#include <LiquidCrystal.h>

#define PROFILE_BTN_PIN 19
#define encoderPinA 22
#define encoderPinB 23


// lcd

//be sure not to use an input only pin 35, 34, 39, 36
const int rs = 27, en = 26, d4 = 25, d5 = 33, d6 = 32, d7 = 18;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// rotary encoder

volatile unsigned long lastEncRun = 0;
volatile float encoderCount = 0;
// could also do this rounding within relevant functions
int roundedEncCount;
int lastRoundedEncCount;
static boolean rotating=false;


// perhaps the remote should be informed of the profile selected at the device 
// if a certain profile has a parameter with a different range of values, the remote could adjust accordingly
// otherwise - could apply a mapping to the remote values on the device end
// what about the number of editable parameters for different presets? This is a real kicker - you will probably want two way coms
// about profile for this reason - can send metadata on classes to remote 
// maybe need a new struct for this

// profile & device selection

int new_profile_selected;
int profile_selected = 0;
int numProfiles = 5;
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

OneButton button1(PROFILE_BTN_PIN, true);

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

  if(remoteTransfer.param_key == 0) {

    //update profile if remote has sent value for it

    profile_selected = remoteTransfer.param_val;

    //Serial.println("Profile from remote: ");
    //Serial.println(profile_selected);

  }

  

  // if there is no button click you just want to send the same profile that is currently on the device back to it
  // maybe I should just make a separate struct for the profile data and only send it on the click
  // you already have this struct containing only p... 
  // could be complicated to handle receiving two different structs... 

  // need to set selected profile based on what the device sends back 
  //profile_selected = deviceData.p;

  // if you only send data on click, none of this data gets sent
  // on the other hand, do you really need to send data every loop? 
  // maybe come up with a strategy to only send data when a value changes
  

  // behavior of rotary encoder depends which mode it is in 
  // increment size for encoder 
  // will you ever really use more than 1?
  // default
  float encInc = 0.5; 

  // encLimit when to loop back to start of possible selections
  
  int encLimit = 100; 
  // what if there is a hard ceiling? as in you don't want to loop back but stop at max value
  // set this to 1 
  int hasCeiling = 0;

  // in some instances you will want the device to update as you scroll e.g. adjust hue 
  // in others, nothing should happen until a button click confirms e.g. update profile 


  switch(btnMode) {
    case 0: 
      // nothing
   
      break;
    case 1:
      // device mode
      encInc = 0.5; 
      // set limit to equal number of devices/profiles etc 
      // not index of highest one 
      encLimit = 2;
      break;
    case 2: 
      // profile mode
      encInc = 0.5;
      encLimit = 5;
      break;
    case 3: 
      // parameter mode
      break; 
  }

  while (rotating) {
    //delay(2);  // debounce by waiting 2 milliseconds
               // (Just one line of code for debouncing)

    if(millis() - lastEncRun < 2)
      return;

    if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) {
      //direction = 1;
      //encoderCount--;
      encoderCount = encoderCount - encInc;
    }
    else {
      //encoderCount++;
      encoderCount = encoderCount + encInc;
      //direction = 2;
    }                          
    
    lastEncRun = millis();
    rotating=false; // Reset the flag
    
  }

  // convert to int 
  // what does static mean here?
  roundedEncCount = static_cast<int>(std::round(encoderCount));

  // do I need to now set encoderCount to this? 
  // wow this made sense in my head but did not work
  // maybe best to do this conversion later, before actually using the value as index
  //encoderCount = static_cast<float>(roundedEncCount);

  //Serial.println(roundedEncCount);

  // apply limits if applicable

  roundedEncCount = max(0, ((roundedEncCount + 1) % (encLimit)));

  if (roundedEncCount != lastRoundedEncCount) {
    Serial.println("Current Selection: ");
    Serial.println(roundedEncCount);
 
  }

  // put your main code here, to run repeatedly:
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.clear();
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  lcd.print(roundedEncCount);

  // when you go to double digits and back to single, the 0 will stay

  lastRoundedEncCount = roundedEncCount;

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
      selectDevice(roundedEncCount);
      break;
    case 2: 
      // select profile with chosen ID
      // profile is tracked at the device
      remoteTransfer.param_key = 2; 
      // reserve this to mean increment pattern by 1
      remoteTransfer.param_val = roundedEncCount; 
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
  // e.g. skip to next profile in profile mode 
  // skip to next device in device mode

  switch(btnMode) {
    case 0: 
      // default mode -- do nothing
      break; 
    case 1:
      // it is safe to handle device selection here, because the device cannot be selected from the absorber
      // what if it is selected from webapp?
      // the remote or app are still the final element that need to know which device is selected
      // two devices but index starts at 0 so 
      device_selected = (device_selected + 1) % (numDevs);
      
      selectDevice(device_selected);
      // after selection exit to default mode
      //btnMode = 0; 
      break;
    case 2: 
      // increment profile
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

  btnMode = (btnMode + 1) % (numBtnModes); 

  Serial.println("Set button to mode: ");
  Serial.println(btnMode);

  // may need to request info on current value of relevant parameter from device now 
  // or can I continue to handle this entirely on device side?

  // update the screen when mode changed
  switch(btnMode) {
    case 0: 
      // default mode 
      // do nothing or just display some info / graphic
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Welcome!");
      break;
    case 1: 
      // select device with chosen ID
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Select Device:");

      break;
    case 2: 
      // select profile with chosen ID
      // profile is tracked at the device
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Select Profile:");
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
};




