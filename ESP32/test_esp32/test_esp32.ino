#include <HardwareSerial.h>
#include "WiFi.h"
#include <ArduinoWebsockets.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>



const char* ssid = "Student5";
const char* password = "***REMOVED***";

int btnGPIO = 0;
int btnState = false;

WiFiClient client;

HardwareSerial SerialPort(2);
int number = 0;
void setup() {

  SerialPort.begin(38400,SERIAL_8N1, 16);
  Serial.begin(115200);

  pinMode(btnGPIO, INPUT);
  Serial.println();
  Serial.print("[WiFi-test] Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int tryDelay = 500;
  int numberOfTries = 20;
  bool is_not_connected = true;
  while (is_not_connected) {
        switch(WiFi.status()) {
          case WL_NO_SSID_AVAIL:
            Serial.println("[WiFi] SSID not found");
            break;
          case WL_CONNECT_FAILED:
            Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
            return;
            break;
          case WL_CONNECTION_LOST:
            Serial.println("[WiFi] Connection was lost");
            break;
          case WL_SCAN_COMPLETED:
            Serial.println("[WiFi] Scan is completed");
            break;
          case WL_DISCONNECTED:
            Serial.println("[WiFi] WiFi is disconnected");
            break;
          case WL_CONNECTED:
            Serial.println("[WiFi] WiFi is connected!");
            Serial.print("[WiFi] IP address: ");
            Serial.println(WiFi.localIP());
            is_not_connected = false;
            break;
          default:
            Serial.print("[WiFi] WiFi Status: ");
            Serial.println(WiFi.status());
            break;
        }
        delay(tryDelay);
        
        if(numberOfTries <= 0){
          Serial.print("[WiFi] Failed to connect to WiFi!");
          // Use disconnect function to force stop trying to connect
          WiFi.disconnect();
          return;
        } else {
          numberOfTries--;
        }
  }
  delay(1000); 
}






void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, " %%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}

bool active = true;
int buffer_place = 0;
byte buf[4];
int my_place = 0;
int value[300];
int l_ones_score = 0;
int l_tens_score = 0;
int left_score = 0;
int r_ones_score = 0;
int r_tens_score = 0;
int right_score = 0;
int l_yellow = 0;
int l_red = 0;
int r_yellow = 0;
int time_min = 0;
int time_tens = 0;
int time_ones = 0;
int my_time = 0;
int left_light = 0;
int right_light = 0;
int left_white_light = 0;
int right_white_light = 0;
int holder =0;

// Default Header 0x01 0x13 0x44
// End Header? 0x30 0x77 0x30 0x04
// Timer Tick Header 0x01 0x13 0x52
// Timer Update Header 0x01 0x13 0x42
// Timer Reset Header 0x01 0x13 0x4E
// Lights Header 0x01 0x14 0x52
// Data Indicator 0x30 
// Start Flag 0x01
// Stop Flag 0x04

void loop() {

  StaticJsonDocument<200> doc;
  doc["time"] = my_time;
  doc["left_score"] = left_score;
  doc["right_score"] = right_score;
  doc["left_touch"] = left_light;
  doc["right_touch"] = right_light;
  doc["right_offtarget"] = right_white_light;
  doc["left_offtarget"] = left_white_light;
  
  String mydata;
  serializeJson(doc, mydata);  

  HTTPClient http;

  http.begin("http://10.4.145.139:5000/api/score");               //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  
  int httpResponseCode = http.POST(mydata);  

  if ( holder == 0){
    Serial.println("WiFi Connected in loop!");
    Serial.println(WiFi.localIP());
    holder =1;
  }


  while(SerialPort.available()){
    char serial_data_char = SerialPort.read();
    value[my_place] = serial_data_char;
    buf[buffer_place] = serial_data_char;
    my_place++;
    buffer_place = (buffer_place +1)%4;
    active = true;
  }

  int i = buffer_place;
  if ( active && ( (buf[i]==0x30) && (buf[(i+1)%4]==0x77) && (buf[(i+2)%4]==0x30)&& (buf[(i+3)%4]==0x4) ) ){
    for(int a=0; a<my_place; a++){
      printHex(value[a],2);
      value[a];
    }
      
    if( (value[0]==0x01) && (value[1]==0x13) && (value[2]==0x44)){
      // right/left score, left yellow/red card, & right yellow card

      r_ones_score = value[5] - 0x30;
      r_tens_score = value[4] - 0x30;
      right_score = 10*r_tens_score + r_ones_score;

      
      l_ones_score = value[8] - 0x30;
      l_tens_score = value[7] - 0x30;
      left_score = 10*l_tens_score + l_ones_score;

      l_yellow = value[17] - 0x30;
      l_red = value[19] - 0x30;

      r_yellow = value[11] - 0x30;
      int httpResponseCode = http.POST(mydata); 
    } else if( (value[0]==0x01) && (value[1]==0x13) && (value[2]==0x52)) {
      // time countdown
      time_min = value[5] - 0x30; 
      time_tens = value[7] - 0x30; 
      time_ones = value[8] - 0x30;
      my_time = 60*time_min + (10*time_tens + time_ones);

      r_ones_score = value[18] - 0x30;
      r_tens_score = value[17] - 0x30;
      right_score = 10*r_tens_score + r_ones_score;

      l_ones_score = value[21] - 0x30;
      l_tens_score = value[20] - 0x30;
      left_score = 10*l_tens_score + l_ones_score;

      l_yellow = value[30] - 0x30;
      l_red = value[32] - 0x30;


      r_yellow = value[24] - 0x30;
      int httpResponseCode = http.POST(mydata); 
    } else if( (value[0]==0x01) && (value[1]==0x13) && (value[2]==0x42)) {
      // time is paused & increment/decrement time
      r_ones_score = value[18] - 0x30;
      r_tens_score = value[17] - 0x30;
      right_score = 10*r_tens_score + r_ones_score;

      
      l_ones_score = value[21] - 0x30;
      l_tens_score = value[20] - 0x30;
      left_score = 10*l_tens_score + l_ones_score;
      
      time_min = value[5] - 0x30; 
      time_tens = value[7] - 0x30; 
      time_ones = value[8] - 0x30;
      my_time = 60*time_min + (10*time_tens + time_ones);

      int httpResponseCode = http.POST(mydata); 
    } else if( (value[0]==0x01) && (value[1]==0x13) && (value[2]==0x4E)) {
      // time reset indicator
      time_min = value[5] - 0x30; 
      time_tens = value[7] - 0x30; 
      time_ones = value[8] - 0x30;
      my_time = 60*time_min + (10*time_tens + time_ones);

      int httpResponseCode = http.POST(mydata); 
    } else if( (value[0]==0x01) && (value[1]==0x14) && (value[2]==0x52)) {
      // lights
      left_light = value[3] - 0x30;
      right_light = value[5] - 0x30;

      left_white_light = value[9] - 0x30;
      right_white_light = value[7] - 0x30;

      int httpResponseCode = http.POST(mydata);       
    }
    
    my_place=0;
    active = false;  
  }
}
