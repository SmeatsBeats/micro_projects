#include <Arduino.h>
// is this being double included here and in main?
#include <esp_now.h>

#ifndef MY_HEADER_H
#define MY_HEADER_H

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
    int param_key; 
    int param_val; 
} remoteData;

typedef struct remoteResponse {
    // this will store any error 
    // or if param is changed successfully can be string containing the parameter and new value
    char deviceResponse[100];
} remoteResponse;

// this is the address to send profile data back to remote
uint8_t remoteAddress[] = {0x0C, 0xB8, 0x15, 0xC0, 0xE9, 0x5C};
char thisDevMac[18] = "0C:B8:15:C1:BF:9C";

// does this need to be created yet?
// yes because communication from remote could come at any time
remoteData remoteTransfer;

remoteResponse deviceFeedback;

// tell program if the correct device has been targeted - if not, don't proceed 
int correctDevice = 0;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  
    //copy the data to remoteTransfer 
    memcpy(&remoteTransfer, incomingData, sizeof(remoteTransfer));

    
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("TargetDevMac: ");
    Serial.println(remoteTransfer.target_dev);
    Serial.print("Param_Key: ");
    Serial.println(remoteTransfer.param_key);
    Serial.print("Param_Val: ");
    Serial.println(remoteTransfer.param_val);
    Serial.println();
    

    // before taking action validate that this was the intended device 

    //Serial.println("Data received");
    Serial.println("This device mac address:");
    Serial.println(thisDevMac);
    Serial.println("Target mac:");
    Serial.println(remoteTransfer.target_dev);

    char deviceSelectResponse[18];
    char fdbk[100];

    if (strcmp(thisDevMac, remoteTransfer.target_dev) == 0) {
        //correct device
        Serial.println("Correct device.");
        correctDevice = 1;
        // send feedback - how to store it? ideally in the same target_dev var but this may overcomplicate things 
        // maybe better to send a separate struct with one bit of feedback 
        //fdbk = "Correct device successfully reached!";
        strcpy(fdbk, "Correct device successfully reached!");
        //deviceFeedback.deviceResponse = strdup("Correct device successfully reached!");
        //deviceFeedback.deviceResponse = "Correct device successfully reached!";

        // proceed to process data and then fdbk should be the parameter and value set 
        // if you process data in the loop it will keep running
        // but it feels messy to do it all in here... 
        // maybe use correct_device flag to only execute when new data received


    }
    else {
        Serial.println("Not the droid you're looking for...");
        // handle error 
        correctDevice = 0;
        //deviceFeedback.deviceResponse = "Incorrect device contacted.";
        strcpy(fdbk, "Incorrect device contacted.");
        //deviceFeedback.deviceResponse = strdup("Incorrect device contacted.");
    }

    // send feedback regardless of outcome but only when this device has actually been contacted aka only on data received hmm 

    //strncpy(deviceFeedback.deviceResponse, fdbk, sizeof(deviceFeedback.deviceResponse));
    //deviceFeedback.deviceResponse[strlen(deviceFeedback.deviceResponse)] = '\0';

    // need to ensure the length of the response matches the memory allocated

    strcpy(deviceFeedback.deviceResponse, fdbk);

    Serial.println("Size of response: ");
    Serial.println(sizeof(deviceFeedback.deviceResponse));
    // you may want to track the actual length and store in the struct as well

    Serial.println("Sending response: ");
    Serial.println(deviceFeedback.deviceResponse);

    esp_err_t result = esp_now_send(remoteAddress, (uint8_t *) &deviceFeedback, sizeof(deviceFeedback));
   
    if (result == ESP_OK) {
        Serial.println("Sent device feedback with success");
    }
    else {
        Serial.println("Error sending the device feedback");
    }

    //free(deviceFeedback.deviceResponse);
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

#endif