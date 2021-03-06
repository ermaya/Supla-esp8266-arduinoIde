/*
Copyright (C) AC SOFTWARE SP. Z O.O.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <FS.h>       // ---- esp board manager 2.5.2 --- iwip Variant V2 Lower Memory
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SuplaDevice.h>
#include <io.h>
#include <WiFiManager.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <supla/network/esp_wifi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
extern "C"
{
#include "user_interface.h"
} 

//#define D0 16  //no internal pullup resistor
//#define D1  5
//#define D2  4
//#define D3  0  //must not be pulled low during power on/reset, toggles value during boot
//#define D4  2  //must not be pulled low during power on/reset, toggles value during boot
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15  //must not be pulled high during power on/reset

Supla::ESPWifi wifi("", "");  //------ Do not change----wifimanager takes care------
int wificonfig_pin = 0; // D3
int led_pin = 2; // D4
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 5000;               // ---------------------- config delay 5 seconds ---------------------------
char Supla_server[81]=("Set server address");
char Email[81]=("set email address");
char Supla_name[51];
char Supla_status[51];
char BotToken[80];                                                   
char Chat_Id[20];                   
char Notifi1[80];
char Notifi2[80];
char Notifi3[80];
char Notifi4[80];
char Notifi5[80];
char Notifi6[80];
char Notifi7[80];
char Notifi8[80];
String notifi = " ";
const char* host = "api.telegram.org";
const int httpsPort = 443;    
bool shouldSaveConfig = false;
bool initialConfig = false;
bool starting = true;
bool state10 = true;
bool send_telegram = false;
String url = "/bot";
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long
ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;

static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";

class MyDigitalWrite : public Supla::Io {
  public:
    void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {
      if ((channelNumber == 0) && (val == 1)) {notifi = Notifi1;send_telegram = true;return;}
      if ((channelNumber == 1) && (val == 1)) {notifi = Notifi2;send_telegram = true;return;}
      if ((channelNumber == 2) && (val == 1)) {notifi = Notifi3;send_telegram = true;return;}
      if ((channelNumber == 3) && (val == 1)) {notifi = Notifi4;send_telegram = true;return;}
      if ((channelNumber == 4) && (val == 1)) {notifi = Notifi5;send_telegram = true;return;}
      if ((channelNumber == 5) && (val == 1)) {notifi = Notifi6;send_telegram = true;return;}
      if ((channelNumber == 6) && (val == 1)) {notifi = Notifi7;send_telegram = true;return;}
      if ((channelNumber == 7) && (val == 1)) {notifi = Notifi8;send_telegram = true;return;}
      else {
        return ::digitalWrite(pin,val);   // ------------------------------ so that the other channels work normally
      }
   }
}MyDigitalWrite;

void saveConfigCallback () {          
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
void ondemandwifiCallback () {
   digitalWrite(led_pin, LOW);
   WiFiManagerParameter custom_Supla_server("server", "supla server", Supla_server, 81,"required");
   WiFiManagerParameter custom_Email("email", "Email", Email, 81,"required");
   WiFiManagerParameter custom_Supla_name("name", "Supla Device Name", Supla_name, 51,"required");
   WiFiManagerParameter custom_Supla_status("status", "Supla Last State", Supla_status, 51,"readonly");
   WiFiManagerParameter custom_BotToken("BotToken", "Bot Token", BotToken, 80);
   WiFiManagerParameter custom_Chat_Id("Chat_Id", "Chat Id", Chat_Id, 20);
   WiFiManagerParameter custom_Notifi1("Notifia", "Notifi 1 Message", Notifi1, 80);
   WiFiManagerParameter custom_Notifi2("Notifib", "Notifi 2 Message", Notifi2, 80);
   WiFiManagerParameter custom_Notifi3("Notific", "Notifi 3 Message", Notifi3, 80);
   WiFiManagerParameter custom_Notifi4("Notifid", "Notifi 4 Message", Notifi4, 80);
   WiFiManagerParameter custom_Notifi5("Notifie", "Notifi 5 Message", Notifi5, 80);
   WiFiManagerParameter custom_Notifi6("Notifif", "Notifi 6 Message", Notifi6, 80);
   WiFiManagerParameter custom_Notifi7("Notifig", "Notifi 7 Message", Notifi7, 80);
   WiFiManagerParameter custom_Notifi8("Notifih", "Notifi 8 Message", Notifi8, 80);
   
   wifiManager.setBreakAfterConfig(true);
   wifiManager.setSaveConfigCallback(saveConfigCallback);
  
   wifiManager.addParameter(&custom_Supla_server);
   wifiManager.addParameter(&custom_Email);
   wifiManager.addParameter(&custom_Supla_name);
   wifiManager.addParameter(&custom_Supla_status);
   wifiManager.addParameter(&custom_BotToken);
   wifiManager.addParameter(&custom_Chat_Id);
   wifiManager.addParameter(&custom_Notifi1);
   wifiManager.addParameter(&custom_Notifi2);
   wifiManager.addParameter(&custom_Notifi3);
   wifiManager.addParameter(&custom_Notifi4);
   wifiManager.addParameter(&custom_Notifi5);
   wifiManager.addParameter(&custom_Notifi6);
   wifiManager.addParameter(&custom_Notifi7);
   wifiManager.addParameter(&custom_Notifi8);

   //wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>");
   wifiManager.setCustomHeadElement(logo);
   wifiManager.setMinimumSignalQuality(8);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
   wifiManager.setConfigPortalTimeout(300);

   if (!wifiManager.startConfigPortal("Supla_Notifi")) { Serial.println("Not connected to WiFi but continuing anyway.");} else { Serial.println("connected...yeey :)");}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    strcpy(BotToken, custom_BotToken.getValue());
    strcpy(Chat_Id, custom_Chat_Id.getValue());
    strcpy(Notifi1, custom_Notifi1.getValue());
    strcpy(Notifi2, custom_Notifi2.getValue());
    strcpy(Notifi3, custom_Notifi3.getValue());
    strcpy(Notifi4, custom_Notifi4.getValue());
    strcpy(Notifi5, custom_Notifi5.getValue());
    strcpy(Notifi6, custom_Notifi6.getValue());
    strcpy(Notifi7, custom_Notifi7.getValue());
    strcpy(Notifi8, custom_Notifi8.getValue());
  
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      Serial.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      ESP.reset(); 
    }  
    WiFi.softAPdisconnect(true);   //  close AP
}
void status_func(int status, const char *msg) {    //    ------------------------ Status --------------------------
  if (s != status){
    s = status; 
      if (s != 10){
        strcpy(Supla_status, msg); 
  }  }            
}
void guid_authkey(void) {
  if (EEPROM.read(300) != 60){
    int eep_gui = 301;

    ESP8266TrueRandom.uuid(uuidNumber);

    String uuidString = "";
    for (int i = 0; i < 16; i++) {
      int topDigit = uuidNumber[i] >> 4;
      int bottomDigit = uuidNumber[i] & 0x0f;
      uuidString += "0123456789abcdef"[topDigit];
      uuidString += "0123456789abcdef"[bottomDigit];
    }
    int length_uuid = uuidString.length();
    for (int i = 0; i < length_uuid; ++i) {
      EEPROM.put(eep_gui + i, uuidString[i]);
    }

    int eep_aut = 341;

    ESP8266TrueRandom.uuid(uuidNumber);

    String uuidString2 = "";
    for (int i = 0; i < 16; i++) {
      int topDigit = uuidNumber[i] >> 4;
      int bottomDigit = uuidNumber[i] & 0x0f;
      uuidString2 += "0123456789abcdef"[topDigit];
      uuidString2 += "0123456789abcdef"[bottomDigit];
    }
    int length_uuid2 = uuidString2.length();
    for (int i = 0; i < length_uuid2; ++i) {
      EEPROM.put(eep_aut + i, uuidString2[i]);
    }
    EEPROM.write(300, 60);
    EEPROM.commit();
  }
  read_guid();
  read_authkey();
  Serial.print("GUID : ");Serial.println(read_guid()); 
  Serial.print("AUTHKEY : ");Serial.println(read_authkey()); 
}

String read_guid(void) {
  String read_eeprom = "";
  int i, ii = 0;
  int eep_star = 301;
  int end_guid = eep_star + SUPLA_GUID_SIZE;
  String temp_read = "0x";
  for (i = eep_star; i < end_guid + 16;  i = i + 1) {
    temp_read += char(EEPROM.read(i));
    read_eeprom += char(EEPROM.read(i));
    if ( (i % 2) == 0) {
      char *_guid = strcpy((char*)malloc(temp_read.length() + 1), temp_read.c_str());
      GUID[ii] = strtoul( _guid, NULL, 16);
      temp_read = "0x";
      ii++;
    }
  }
  return read_eeprom;
}
String read_authkey(void) {
  String read_eeprom = "";
  int i, ii = 0;
  int eep_star = 341;
  int end_authkey = eep_star + SUPLA_AUTHKEY_SIZE;
  String temp_read = "0x";
  for (i = eep_star; i < end_authkey + 16;  i = i + 1) {
    temp_read += char(EEPROM.read(i));
    read_eeprom += char(EEPROM.read(i));
    if ( (i % 2) == 0) {
      char *_authkey = strcpy((char*)malloc(temp_read.length() + 1), temp_read.c_str());
      AUTHKEY[ii] = strtoul( _authkey, NULL, 16);
      temp_read = "0x";
      ii++;
    }
  }
  return read_eeprom;
}

void setup() {
  wifi_set_sleep_type(NONE_SLEEP_T);
  Serial.begin(115200);
  pinMode(wificonfig_pin, INPUT_PULLUP);
  pinMode(led_pin,OUTPUT);   
  digitalWrite(led_pin, HIGH);

  EEPROM.begin(1024);
  if (EEPROM.read(300) != 60){initialConfig = true;} 
  guid_authkey();

  
  if (WiFi.SSID()==""){ initialConfig = true;} 

  if (SPIFFS.begin()) {  // ------------------------- wificonfig read -----------------
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;         
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);   //print config data to serial on startup
        if (json.success()) {Serial.println("\nparsed json");         
          strcpy(Supla_server, json["Supla_server"]);
          strcpy(Email, json["Email"]);
          strcpy(Supla_name, json["Supla_name"]);
          strcpy(BotToken, json["BotToken"]);         
          strcpy(Chat_Id, json["Chat_Id"]);
          strcpy(Notifi1, json["Notifi1"]);
          strcpy(Notifi2, json["Notifi2"]);
          strcpy(Notifi3, json["Notifi3"]);
          strcpy(Notifi4, json["Notifi4"]);
          strcpy(Notifi5, json["Notifi5"]);
          strcpy(Notifi6, json["Notifi6"]);
          strcpy(Notifi7, json["Notifi7"]);
          strcpy(Notifi8, json["Notifi8"]);         
        } else {
          Serial.println("failed to load json config");
          initialConfig = true;
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
  wifi_station_set_hostname(Supla_name);
   
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.

    SuplaDevice.addRelay(100, false);
    SuplaDevice.addRelay(101, false);
    SuplaDevice.addRelay(102, false);
    SuplaDevice.addRelay(103, false);
    SuplaDevice.addRelay(108, false);
    SuplaDevice.addRelay(105, false);
    SuplaDevice.addRelay(106, false);
    SuplaDevice.addRelay(107, false);
     
    SuplaDevice.setName(Supla_name);
    SuplaDevice.setStatusFuncImpl(&status_func);
  
  SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);
                      
}

void loop() {
  if (initialConfig == true){ondemandwifiCallback();}
    if (send_telegram){direct_Link();}
  int C_W_read = digitalRead(wificonfig_pin);{  // ---------------------let's read the status of the Gpio to start wificonfig ---------------------
   if (C_W_read != last_C_W_state) {  time_last_C_W_change = millis();}      
    if ((millis() - time_last_C_W_change) > C_W_delay) {     
     if (C_W_read != C_W_state) {     
       C_W_state = C_W_read;       
       if (C_W_state == LOW) {
        ondemandwifiCallback () ;} } }         
     last_C_W_state = C_W_read;            
   }
      
  if (shouldSaveConfig == true) { // ------------------------ wificonfig save --------------
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Supla_server"] = Supla_server;
    json["Email"] = Email;
    json["Supla_name"] = Supla_name;
    json["BotToken"] = BotToken;
    json["Chat_Id"] = Chat_Id;
    json["Notifi1"] = Notifi1;
    json["Notifi2"] = Notifi2;
    json["Notifi3"] = Notifi3;
    json["Notifi4"] = Notifi4;
    json["Notifi5"] = Notifi5;
    json["Notifi6"] = Notifi6;
    json["Notifi7"] = Notifi7;
    json["Notifi8"] = Notifi8;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {Serial.println("failed to open config file for writing"); }   
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println("config saved");    
    shouldSaveConfig = false;
    initialConfig = false; 
    WiFi.mode(WIFI_STA);   
    delay(5000);
    ESP.restart(); 
  }
  
 SuplaDevice.iterate();
   delay(75);

   if (WiFi.status() == WL_CONNECTED){
    if (starting){
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin(); 
      starting = false;         
     }
    httpServer.handleClient();
   }
   
}
void direct_Link() {
  send_telegram = false;
      url = "/bot";
      url += BotToken;
      url += "/sendMessage?chat_id=";
      url += Chat_Id;
      url += "&text=";
      url += notifi;
  WiFiClientSecure client2; 
  client2.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client2.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  Serial.print("requesting URL: ");
  Serial.println(url);
  client2.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("request sent");
  while (client2.connected()) {
    String line = client2.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client2.readStringUntil('}');
  line = line + "}";
  if (line.indexOf("true") >0) {   
    Serial.println("successfull!");
  } else {
    Serial.println("failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}
