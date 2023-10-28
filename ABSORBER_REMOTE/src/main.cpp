#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneButton.h>

#define POT_LED_PIN 18
#define SLIDE_LED_PIN 19
#define BTN_LED_PIN 21
#define POT_PIN 32
#define SLIDE_PIN 35
#define DOWN_BTN_PIN 26
#define UP_BTN_PIN 27
#define PROFILE_BTN_PIN 14

// profile selection

int new_profile_selected;
int profile_selected = 0;
int device_selected = 0;

// button setup

// Setup a new OneButton on pin A1.  
OneButton button1(DOWN_BTN_PIN, true);
// Setup a new OneButton on pin A2.  
OneButton button2(UP_BTN_PIN, true);

OneButton button3(PROFILE_BTN_PIN, true);

int pot_value;
int slide_value;
int btn_value = 100;

// ESP_NOW

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t* broadcastAddressSelected; //this is a pointer
uint8_t broadcastAddress1[] = {0x0C, 0xB8, 0x15, 0xC1, 0xBF, 0x9C};
uint8_t broadcastAddress2[] = {0x78, 0xE3, 0x6D, 0x19, 0xFB, 0xEC};


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

typedef struct from_remote {
    int h;
    int s;
    int v;
    int p;
} from_remote;

// this comes from all connected devices... 
// so currently if you change the profile on device 1, device 2 will send its unchanged profile to the remote
// which will then reset device 1... 
// you need to separate the data coming in from each device
// you need to create multiple structs and send multiple times to each receiver - or send only once to the appropriate receiver
// maybe I can set the board to send data back to the remote only if it is selected as well
// added int sd to struct - selected device - give each device an id and only if it matches the selected id send data back to remote
// for now I can manually change the id number each time I flash to different device but going forward, best to have just one script
// is it possible to check the sender here and reject one based on mac?


typedef struct to_remote {
    int p;
} to_remote;

// Create a struct_message called remoteData
from_remote remoteData;
to_remote deviceData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

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
    memcpy(&deviceData, incomingData, sizeof(deviceData));
    //Serial.println("Data received from selected mac");
  } else {
    // Received data from an unknown device, reject it or take appropriate action
    //Serial.print("Received data from an unselected device with MAC: ");
    //Serial.println(receivedMacStr);
  }


  /*
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(remoteData.a);
  Serial.print("Int: ");
  Serial.println(remoteData.b);
  Serial.print("Float: ");
  Serial.println(remoteData.c);
  Serial.print("Bool: ");
  Serial.println(remoteData.d);
  Serial.println();

  */
}

// put function declarations here:
//int myFunction(int, int);

// track if button pushed 
int btn3_click = 0;

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

  // Set values to send
  //strcpy(remoteData.a, "THIS IS A CHAR");
  //remoteData.b = random(1,20);

  // get data from device 
  //Serial.println("Profile on device loop: ");
  //Serial.println(deviceData.p);
  profile_selected = deviceData.p;

  // if there is no button click you just want to send the same profile that is currently on the device back to it
  // maybe I should just make a separate struct for the profile data and only send it on the click
  // you already have this struct containing only p... 
  // could be complicated to handle receiving two different structs... 

  // need to set selected profile based on what the device sends back 
  //profile_selected = deviceData.p;

  // if you only send data on click, none of this data gets sent
  // on the other hand, do you really need to send data every loop? 
  // maybe come up with a strategy to only send data when a value changes

  remoteData.h = pot_value;
  remoteData.s = slide_value;
  remoteData.v = btn_value;
  // just send back what you got if there is no click 
  // if there is a click this must be updated to the new selected profile
  //delay(100);
  

  // select correct device
  // is there a possibility here that I will copy the selected profile from the previous device over to this one in transition?


  // this comes from dbl click 3 currently
  switch (device_selected) {
    case 0: 
      broadcastAddressSelected = broadcastAddress1;
      break;
    case 1: 
      broadcastAddressSelected = broadcastAddress2;
      break;
  }
  

  remoteData.p = profile_selected;
 
  if (btn3_click == 0){
    remoteData.p = profile_selected;
  }
  else {
    Serial.println("Button Clicked Use New Value: ");
    Serial.println(new_profile_selected);
    remoteData.p = new_profile_selected;
    
  }

  // Send message via ESP-NOW

  //Serial.println("Sending Profile Selection: ");
  //Serial.println(remoteData.p);

  esp_err_t result = esp_now_send(broadcastAddressSelected, (uint8_t *) &remoteData, sizeof(remoteData));
  
  if (result == ESP_OK) {
    // if you comment this line out the LEDs on the remote go out lol
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
    Serial.println(result);
  }
  

 if (btn3_click) {
  delay(300);
  // this works so I think what is happening is when you send an updated profile, you just need to give it a sec
  // otherwise the other device sends its value before this updates and you get a ping pong feedback effect
 }

 btn3_click = 0;
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
  btn3_click = 1;
  Serial.println("Button 3 click.");
  // if the profile has been updated at the device
  profile_selected = deviceData.p;
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

// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick3() {
  Serial.println("Button 3 doubleclick.");
  if (device_selected == 0) {
    device_selected = 1;
  }
  else {
    device_selected = 0;
  }
} // doubleclick3


// This function will be called once, when the button1 is pressed for a long time.
void longPressStart3() {
  Serial.println("Button 1 longPress start");
} // longPressStart3


// This function will be called often, while the button1 is pressed for a long time.
void longPress3() {
  Serial.println("Button 3 longPress...");
} // longPress3


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop3() {
  Serial.println("Button 3 longPress stop");
} // longPressStop3
