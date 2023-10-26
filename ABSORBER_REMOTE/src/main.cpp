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

// button setup

// Setup a new OneButton on pin A1.  
OneButton button1(DOWN_BTN_PIN, true);
// Setup a new OneButton on pin A2.  
OneButton button2(UP_BTN_PIN, true);

int pot_value;
int slide_value;
int btn_value = 0;

// ESP_NOW

// REPLACE WITH YOUR RECEIVER MAC Address
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

typedef struct struct_message {
    int h;
    int s;
    int v;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// put function declarations here:
//int myFunction(int, int);

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

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Register peer 1
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  //peerInfo.channel = 0;  
  //peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register peer 2
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  //peerInfo.channel = 0;  
  //peerInfo.encrypt = false;
  
  // Add peer 2        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
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


}

void loop() {
  // put your main code here, to run repeatedly:

  // keep watching the push buttons:
  button1.tick();
  button2.tick();

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
  //strcpy(myData.a, "THIS IS A CHAR");
  //myData.b = random(1,20);
  myData.h = pot_value;
  myData.s = slide_value;
  myData.v = btn_value;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    //Serial.println("Sent with success");
  }
  else {
    //Serial.println("Error sending the data");
  }
 delay(10);

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