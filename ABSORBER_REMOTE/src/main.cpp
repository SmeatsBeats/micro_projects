#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneButton.h>
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
#define DIRECTION_CW 0  
// counter-clockwise direction 
#define DIRECTION_CCW 1  
// #define encoderPinA 33
// #define encoderPinB 25

// rotary encoder

volatile unsigned long lastEncRun = 0;
volatile float encoderCount = 0;
// could also do this rounding within relevant functions
int roundedEncCount;
int lastRoundedEncCount;
static boolean rotating=false;

/*
volatile int direction = DIRECTION_CW;
volatile unsigned long lastEncRun = 0;

volatile int currValA;
volatile int currValB;

volatile int encoderCount = 0;
int lastEncoderCount = 0;

int counter = 0; 
int aState;
int aLastState;  

int runningInt = 0;

*/

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
int numBtnModes = 5; 

// button setup

// Setup a new OneButton on pin A1.  
OneButton button1(DOWN_BTN_PIN, true);
// Setup a new OneButton on pin A2.  
OneButton button2(UP_BTN_PIN, true);

OneButton button3(PROFILE_BTN_PIN, true);

int pot_value;
int slide_value;
int btn_value = 100;
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

// Structure example to send data
// Must match the receiver structure
/*
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;
*/

// typedef struct from_remote {
//     int h;
//     int s;
//     int v;
//     int p;
// } from_remote;

// this comes from all connected devices... 
// so currently if you change the profile on device 1, device 2 will send its unchanged profile to the remote
// which will then reset device 1... 
// you need to separate the data coming in from each device
// you need to create multiple structs and send multiple times to each receiver - or send only once to the appropriate receiver
// maybe I can set the board to send data back to the remote only if it is selected as well
// added int sd to struct - selected device - give each device an id and only if it matches the selected id send data back to remote
// for now I can manually change the id number each time I flash to different device but going forward, best to have just one script
// is it possible to check the sender here and reject one based on mac?

/*
typedef struct to_remote {
    int p;
} to_remote;
*/

// my new struct approach: 



typedef struct remoteData {
    // what if you send updates individually this avoids resending static data
    // does the remote ever need to send more than one value per message?
    // could even use this structure for profile
    // may also need mac address of target device - does this need to be sent back to remote?
    // instead of blocking out all irrelevant incoming data, only send data if you are the selected device
    // better yet, only send data if there is actually a change to send! 
    // param_key
    // param_val
    //int h;
    //int s;
    //int v;
    //int p;
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

// Create a struct_message called remoteData
//from_remote remoteData;
//to_remote deviceData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  /*

  // do this in if statement below so only data from selected device is allowed
  //memcpy(&deviceData, incomingData, sizeof(deviceData));

  // Convert the MAC address to a string
  char receivedMacStr[18];
  snprintf(receivedMacStr, sizeof(receivedMacStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  // Print the MAC address
  //Serial.print("Received data from MAC: ");
  //Serial.println(receivedMacStr);

  // Now you can use receivedMacStr to identify the sender if needed.
    // Convert your known MAC address to a string
  char selectedMacStr[18];
  snprintf(selectedMacStr, sizeof(selectedMacStr), "%02X:%02X:%02X:%02X:%02X:%02X", broadcastAddressSelected[0], broadcastAddressSelected[1], broadcastAddressSelected[2], broadcastAddressSelected[3], broadcastAddressSelected[4], broadcastAddressSelected[5]);

  // Compare the received MAC address with the selected MAC address
  if (strcmp(receivedMacStr, selectedMacStr) == 0) {
    // Received data from the known device, so process the data
    // You can put your data processing logic here
    // ...
    // copy the data to memory
    memcpy(&remoteTransfer, incomingData, sizeof(remoteTransfer));
    //Serial.println("Data received from selected mac");
  } else {
    // Received data from an unknown device, reject it or take appropriate action
    //Serial.print("Received data from an unselected device with MAC: ");
    //Serial.println(receivedMacStr);
  }

  */

  // for (int i = 0; i < len; i++) {
  //   Serial.print(String(incomingData[i], HEX) + " ");
  // }
  //Serial.println();

  memcpy(&deviceFeedback, incomingData, sizeof(deviceFeedback));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.println("Device response: ");
  Serial.println(deviceFeedback.deviceResponse);

}

// put function declarations here:
//int myFunction(int, int);

// rotary encoder 

void rotEncoder()
{
  rotating=true; // If motion is detected in the rotary encoder,
                 // set the flag to true
}

// track if button pushed 
// int btn3_click = 0;

void confirmConnection(uint8_t* broadcastAddressSelected);
int readEncoder(int encInc, int limit);
void selectDevice(int device_selected);
void handleEncoder();
void IRAM_ATTR ISR_encoder();

// ----- button 1 callback functions

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1();
void doubleclick1();
void longPressStart1();
void longPress1();
void longPressStop1();

// ... and the same for button 2:

void click2();
void doubleclick2();
void longPressStart2();
void longPress2();
void longPressStop2();

// ... and the same for button 3:

void click3();
void doubleclick3();
void longPressStart3();
void longPress3();
void longPressStop3();


void setup() {
  // put your setup code here, to run once:
  //int result = myFunction(2, 3);
  Serial.begin(115200);   // Initiate a serial communication
  while(!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POT_LED_PIN, OUTPUT);
  pinMode(SLIDE_LED_PIN, OUTPUT);
  pinMode(BTN_LED_PIN, OUTPUT);
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

  // Setup Buttons

  // link the button 1 functions.
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);

  // link the button 2 functions.
  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);


  // link the button 3 functions.
  button3.attachClick(click3);
  button3.attachDoubleClick(doubleclick3);
  button3.attachLongPressStart(longPressStart3);
  button3.attachLongPressStop(longPressStop3);
  button3.attachDuringLongPress(longPress3);

  
  // rotary encoder setup 

  pinMode (encoderPinA,INPUT_PULLUP);
  pinMode (encoderPinB,INPUT_PULLUP);
  //pinMode (encoderPinA,INPUT);
  //pinMode (encoderPinB,INPUT);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), rotEncoder, CHANGE);

}

void loop() {
  // put your main code here, to run repeatedly:
  // keep watching the push buttons:
  button1.tick();
  button2.tick();
  button3.tick();

  // if the profile has been changed at the device: 
  //Serial.println("Profile from device: ");
  //Serial.println(deviceData.p);
  //profile_selected = deviceData.p;

  // currently you are reading a load of analog vals which will fluctuate without some involved averaging 
  // goal is to switch to rotary encoders and only send relevant data i.e. changed

  /*
  pot_value = analogRead(POT_PIN);
 
  //Serial.println("POT VAL: ");
  //Serial.println(pot_value);
  pot_value = map(pot_value, 0, 4095, 0, 255); //Map value 0-1023 to 0-255 (PWM)
  analogWrite(POT_LED_PIN, pot_value);
    
  slide_value = analogRead(SLIDE_PIN);
  //Serial.println("SLIDE VAL: ");
  //Serial.println(slide_value);
  slide_value = map(slide_value, 0, 4095, 0, 255); //Map value 0-1023 to 0-255 (PWM)
  analogWrite(SLIDE_LED_PIN, slide_value);

  //Serial.println("SLIDE VAL: ");
  //Serial.println(slide_value);

  btn_value = max(0, min(btn_value, 250));
  analogWrite(BTN_LED_PIN, btn_value);

  */

  // Set values to send
  //strcpy(remoteData.a, "THIS IS A CHAR");
  //remoteData.b = random(1,20);

  // get data from device 
  //Serial.println("Profile on device loop: ");
  //Serial.println(deviceData.p);
  //profile_selected = deviceData.p;

  /*

  if(remoteTransfer.param_key == 0) {

    //update profile if remote has sent value for it

    profile_selected = remoteTransfer.param_val;

    //Serial.println("Profile from remote: ");
    //Serial.println(profile_selected);

  }

  */

  // if there is no button click you just want to send the same profile that is currently on the device back to it
  // maybe I should just make a separate struct for the profile data and only send it on the click
  // you already have this struct containing only p... 
  // could be complicated to handle receiving two different structs... 

  // need to set selected profile based on what the device sends back 
  //profile_selected = deviceData.p;

  // if you only send data on click, none of this data gets sent
  // on the other hand, do you really need to send data every loop? 
  // maybe come up with a strategy to only send data when a value changes

  //remoteData.h = pot_value;
  //remoteData.s = slide_value;
  //remoteData.v = btn_value;
  // just send back what you got if there is no click 
  // if there is a click this must be updated to the new selected profile
  //delay(100);
  

  // select correct device
  // is there a possibility here that I will copy the selected profile from the previous device over to this one in transition?

  /*
  // this comes from dbl click 3 currently
  switch (device_selected) {
    case 0: 
      broadcastAddressSelected = broadcastAddress1;
      break;
    case 1: 
      broadcastAddressSelected = broadcastAddress2;
      break;
  }

  // set target device to this address converted to char 
  char targetDevStr[18];
  snprintf(targetDevStr, sizeof(targetDevStr), "%02X:%02X:%02X:%02X:%02X:%02X", broadcastAddressSelected[0], broadcastAddressSelected[1], broadcastAddressSelected[2], broadcastAddressSelected[3], broadcastAddressSelected[4], broadcastAddressSelected[5]);
  remoteTransfer.target_dev = targetDevStr;
  //strcpy(remoteTransfer.target_dev, targetDevStr);
  */

  //remoteData.p = profile_selected;
  // remoteTransfer.param_key = 0; 
  // remoteTransfer.param_val = profile_selected;
 
  /*

  if (btn3_click == 0){
    // do I really need to repeat this?
    //remoteData.p = profile_selected;
  }
  else {
    Serial.println("Button Clicked Use New Value: ");
    Serial.println(new_profile_selected);
    //remoteData.p = new_profile_selected;
    //remoteTransfer.param_key = 0; 
    //remoteTransfer.param_val = new_profile_selected;
    
  }
  */

  // Send message via ESP-NOW

  //Serial.println("Sending Profile Selection: ");
  //Serial.println(remoteData.p);

  /*

  esp_err_t result = esp_now_send(broadcastAddressSelected, (uint8_t *) &remoteTransfer, sizeof(remoteTransfer));
  
  if (result == ESP_OK) {
    // if you comment this line out the LEDs on the remote go out lol
    //Serial.println("Sent with success");
  }
  else {
    //Serial.println("Error sending the data");
    //Serial.println(result);
  }
  */

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
      // in mode 0 use it to scroll through devices

   
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
  roundedEncCount = static_cast<int>(std::round(encoderCount));
  //Serial.println(roundedEncCount);

  // apply limits if applicable

  roundedEncCount = max(0, ((roundedEncCount + 1) % (encLimit)));

  

  if (roundedEncCount != lastRoundedEncCount) {
    Serial.println("Current Selection: ");
    Serial.println(roundedEncCount);
  }

  lastRoundedEncCount = roundedEncCount;
  

  //Serial.println("EncoderCounter: ");
  //Serial.println(encoderCount);
  
  
  // rotary encoder test 
  // Reads the "current" state of the encoderPinA
  
  
  //aState = digitalRead(encoderPinA); 
  //Serial.println(aState);
  /*
   // If the previous and the current state of the encoderPinA are different, that means a Pulse has occured
   // in one tick, it goes 101 so I've tried to modify this so it only runs on completion of the tick
   if (aState != aLastState && aState == 1){     
     // If the encoderPinB state is different to the encoderPinA state, that means the encoder is rotating clockwise
    if (digitalRead(encoderPinB) != aState) { 
      counter ++;
      if (hasCeiling != 0) {
        btnVal = min(btnVal + encInc, hasCeiling);
      }
      else {
        btnVal = (btnVal + encInc) % encLimit;
      }
       

    } 
    else {
      counter --;
      if (hasCeiling != 0) {
        btnVal = max(btnVal - encInc, 0);
      }
      else {
        // shouldn't ever go negative 
        // this will set floor at 0
        //btnVal = max(((btnVal - encInc) % encLimit), 0);
        // this will take abs 
        //btnVal = abs((btnVal - encInc) % encLimit);
        btnVal = abs((btnVal - encInc)) % encLimit;
      }

      //btn_value = btn_value - 10;
    }
    Serial.print("btnVal: ");
    Serial.println(btnVal);
    Serial.print("counter: ");
    Serial.println(counter);
   } 
   // Updates the previous state of the encoderPinA with the current state
   aLastState = aState; 
  */
  



  /*

  if (btn3_click) {
    delay(300);
    // this works so I think what is happening is when you send an updated profile, you just need to give it a sec
    // otherwise the other device sends its value before this updates and you get a ping pong feedback effect
  }
  */

 //runningInt = 0;

 //btn3_click = 0;
 // this delay is necessary for button functionality but is a shame because it makes the lights less responsive to controls 
 // either find a way to capture the button click without delay - or use different input source 
 // or or separate the profile data from the HSV data 
 // probably makes the most sense to separate the data since I relay only need to send profile data when there is an action
 // what if I put the delay only when the button is clicked?
 //delay(100);

}

// put function definitions here:
/*
int myFunction(int x, int y) {
  return x + y;
}
*/

// ----- button 1 callback functions

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1() {
  // this does not seem to be working or only after long press?
  Serial.println("Button 1 click.");
  btn_value = btn_value - 5;
} // click1


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("Button 1 doubleclick.");
  if (btn_value > 0) {
    btn_value = 0;
  }
  else {
    btn_value = 100;
  }
} // doubleclick1


// This function will be called once, when the button1 is pressed for a long time.
void longPressStart1() {
  Serial.println("Button 1 longPress start");
} // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
  Serial.println("Button 1 longPress...");
  btn_value = btn_value - 1;
} // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  Serial.println("Button 1 longPress stop");
} // longPressStop1


// ... and the same for button 2:

void click2() {
  Serial.println("Button 2 click.");
  btn_value = btn_value + 5;
} // click2


void doubleclick2() {
  Serial.println("Button 2 doubleclick.");
  if (btn_value > 0) {
    btn_value = 0;
  }
  else {
    btn_value = 100;
  }
} // doubleclick2


void longPressStart2() {
  Serial.println("Button 2 longPress start");
} // longPressStart2


void longPress2() {
  Serial.println("Button 2 longPress...");
  btn_value = btn_value + 1;
} // longPress2

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
} // longPressStop2


void click3() {

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





  /*
  btn3_click = 1;
  Serial.println("Button 3 click.");
  // if the profile has been updated at the device
  //profile_selected = deviceData.p;
  Serial.println("Profile on device: ");
  Serial.println(profile_selected);
  //btn_value = btn_value + 5;
  if (profile_selected == 4) {
    new_profile_selected = 0;
  }
  else {
     new_profile_selected = profile_selected + 1;
     Serial.println("Adding to Profile Selected...");
  }
  Serial.println("Profile Selected: ");
  Serial.println(profile_selected);
  */
  /*
  // Send message via ESP-NOW

  Serial.println("Click: Sending Profile Selection: ");
  Serial.println(profile_selected);

  esp_err_t result = esp_now_send(broadcastAddressSelected, (uint8_t *) &remoteData, sizeof(remoteData));
  
  if (result == ESP_OK) {
    //Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  */
 
} // click3

// This function will be called when the button3 was pressed 2 times in a short timeframe.
void doubleclick3() {
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
void longPressStart3() {


  // use this to increment through the button modes 

  
  btnMode = (btnMode + 1) % (numBtnModes); 

  Serial.println("Set button to mode: ");
  Serial.println(btnMode);

  // may need to request info on current value of relevant parameter from device now 
  // or can I continue to handle this entirely on device side?

  /*
  Serial.println("Button 1 longPress start");
  
  // switch selected device

  if (device_selected == 0) {
    Serial.println("Connecting to device2.");
    device_selected = 1;
    broadcastAddressSelected = broadcastAddress2;
  }
  else {
    Serial.println("Connecting to device1.");
    device_selected = 0;
    broadcastAddressSelected = broadcastAddress1;
  }

  confirmConnection(broadcastAddressSelected);

  */

} // longPressStart3


// This function will be called often, while the button1 is pressed for a long time.
void longPress3() {
  //Serial.println("Button 3 longPress...");
} // longPress3


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop3() {
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




