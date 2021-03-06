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
#include <SPI.h>
#include <SuplaDevice.h>  // SoftVer, "2.3.1" "custom"
#include <io.h>
#include <Adafruit_MCP23017.h>
#include <WiFiManager.h>
#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266TrueRandom.h>
#include <WiFiClientSecure.h>
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("", "");  //------ Do not change----wifimanager takes care------
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

#define BTN_COUNT 16
int wificonfig_pin = 0;   //D3
int led_pin = 16;          //D0
int C_W_state = HIGH; 
int last_C_W_state = HIGH;
unsigned long time_last_C_W_change = 0; 
long C_W_delay = 5000;               // ---------------------- config delay 5 seconds ---------------------------
char Supla_server[81]=("Set server address");
char Email[81]=("set email address");
char Supla_name[51];
char Supla_status[51];
bool shouldSaveConfig = false;
bool initialConfig = false;
bool state10 = true;
bool starting = true;
bool monostable = false;
int s;
char GUID[SUPLA_GUID_SIZE];
char AUTHKEY[SUPLA_AUTHKEY_SIZE];
byte uuidNumber[16]; // UUIDs in binary form are 16 bytes long
static const char logo[] PROGMEM = "<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>";

ESP8266WebServer httpServer(81);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifiManager;
Adafruit_MCP23017 mcp;
Adafruit_MCP23017 mcp2;

typedef struct { 
  int pin;
  char last_val;
  int ms;
  unsigned long last_time;
} _btn_t;

_btn_t btn[BTN_COUNT];

class MyMcp23017 : public Supla::Io {
  public:
    void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {
      if ((pin >= 100)&& (pin <= 115)){
        mcp.digitalWrite(pin - 100, val);
         if (state10 == false){ 
           if( btn[channelNumber].ms < 10 ){ 
             EEPROM.write(pin, val);
             EEPROM.commit();
           }
           if ((monostable == false) && (SuplaDevice.channel_pin[channelNumber].DurationMS != btn[channelNumber].ms)) {
            int durationMS = SuplaDevice.channel_pin[channelNumber].DurationMS;
               Serial.print("button_duration: ");Serial.print(durationMS);              
               Serial.print(" button: ");Serial.println(channelNumber);               
               EEPROM.put((channelNumber * 10) + 400, durationMS);
               EEPROM.commit();
               btn[channelNumber].ms = durationMS;
           }
         }
         monostable = false;
         return;
      }
      if (pin <= 99) {
        return ::digitalWrite(pin,val);   
      }
   }
   
   int customDigitalRead(int channelNumber, uint8_t pin) {
      if ((pin >= 100)&& (pin <= 115)){
        return mcp.digitalRead(pin - 100);     
      }     
      if (pin <= 99){
        return ::digitalRead(pin);  
      }
    }   
}MyMcp23017; 

void supla_btn_init() {
  mcp2.begin(1); 
   for(int a=0;a<BTN_COUNT;a++)
    if (btn[a].pin > 0) {
        mcp2.pinMode(btn[a].pin-1, INPUT); 
        mcp2.pullUp(btn[a].pin-1, HIGH);  // turn on a 100K pullup internally
        btn[a].last_val = mcp2.digitalRead(btn[a].pin-1);
        btn[a].last_time = millis();
    }
}
void mcp_btn() {
  char v;
  unsigned long now = millis();
  {
  for(int a=0;a<BTN_COUNT;a++)
       if (btn[a].pin > 0) {
           v = mcp2.digitalRead(btn[a].pin-1);
        if (v != btn[a].last_val && now - btn[a].last_time ) {
           btn[a].last_val = v;
           btn[a].last_time = now;
           if (v==0)
             {
              delay(75);
               if (mcp2.digitalRead(btn[a].pin-1)==0) {
                if ( btn[a].ms > 0 ) {
                     monostable = true;
                     SuplaDevice.relayOn(a, btn[a].ms);
                     Serial.println(" monostable");
                 } else {
                 if ( mcp.digitalRead(a) == 1 ) { 
                  SuplaDevice.relayOff(a);
                  Serial.print("BTN Switsh off relay ");
                  Serial.println(a + 1);
                 } else {
                  SuplaDevice.relayOn(a, 0);
                  Serial.print("BTN Switsh on relay ");
                  Serial.println(a + 1);
                 }        
               }
             }
          }
       } 
    }  
  }
}

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
   
   wifiManager.setBreakAfterConfig(true);
   wifiManager.setSaveConfigCallback(saveConfigCallback);
  
   wifiManager.addParameter(&custom_Supla_server);
   wifiManager.addParameter(&custom_Email);
   wifiManager.addParameter(&custom_Supla_name);
   wifiManager.addParameter(&custom_Supla_status);

   wifiManager.setCustomHeadElement(logo);
   wifiManager.setMinimumSignalQuality(8);
   //wifiManager.setShowStaticFields(true); // force show static ip fields
   //wifiManager.setShowDnsFields(true);    // force show dns field always
   wifiManager.setConfigPortalTimeout(300);

   if (!wifiManager.startConfigPortal("Supla_16X16")) { Serial.println("Not connected to WiFi but continuing anyway.");} else { Serial.println("connected...yeey :)");}                
    strcpy(Supla_server, custom_Supla_server.getValue());
    strcpy(Email, custom_Email.getValue());
    strcpy(Supla_name, custom_Supla_name.getValue());
    if(strcmp(Supla_server, "get_new_guid_and_authkey") == 0){
      Serial.println("new guid & authkey.");
      EEPROM.write(300, 0);
      EEPROM.commit();
      delay(100);
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
    int eep_aut = 321;
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
    delay(0);
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
  int eep_star = 321;
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
  delay(10);
  Serial.println(" ");
  Serial.println(" ");
  pinMode(wificonfig_pin, INPUT_PULLUP);
  pinMode(led_pin,OUTPUT);   
  digitalWrite(led_pin, HIGH);
  EEPROM.begin(1024);
  if (EEPROM.read(300) != 60){initialConfig = true;} 
  guid_authkey();
  
  if (WiFi.SSID()==""){ initialConfig = true;} 

  mcp.begin(); 
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  mcp.pinMode(8, OUTPUT);
  mcp.pinMode(9, OUTPUT);
  mcp.pinMode(10, OUTPUT);
  mcp.pinMode(11, OUTPUT);
  mcp.pinMode(12, OUTPUT);
  mcp.pinMode(13, OUTPUT);
  mcp.pinMode(14, OUTPUT);
  mcp.pinMode(15, OUTPUT); 

  memset(btn, 0, sizeof(btn));
  btn[0].pin =1; EEPROM.get(400,btn[0].ms);Serial.print("initial_button_duration 0: ");Serial.println(btn[0].ms);          
  btn[1].pin =2; EEPROM.get(410,btn[1].ms);Serial.print("initial_button_duration 1: ");Serial.println(btn[1].ms);    
  btn[2].pin =3; EEPROM.get(420,btn[2].ms);Serial.print("initial_button_duration 2: ");Serial.println(btn[2].ms);    
  btn[3].pin =4; EEPROM.get(430,btn[3].ms);Serial.print("initial_button_duration 3: ");Serial.println(btn[3].ms);    
  btn[4].pin =5; EEPROM.get(440,btn[4].ms);Serial.print("initial_button_duration 4: ");Serial.println(btn[4].ms);    
  btn[5].pin =6; EEPROM.get(450,btn[5].ms);Serial.print("initial_button_duration 5: ");Serial.println(btn[5].ms);    
  btn[6].pin =7; EEPROM.get(460,btn[6].ms);Serial.print("initial_button_duration 6: ");Serial.println(btn[6].ms);    
  btn[7].pin =8; EEPROM.get(470,btn[7].ms);Serial.print("initial_button_duration 7: ");Serial.println(btn[7].ms);    
  btn[8].pin =9; EEPROM.get(480,btn[8].ms);Serial.print("initial_button_duration 8: ");Serial.println(btn[8].ms);  
  btn[9].pin =10; EEPROM.get(490,btn[9].ms);Serial.print("initial_button_duration 9: ");Serial.println(btn[9].ms);  
  btn[10].pin =11; EEPROM.get(500,btn[10].ms);Serial.print("initial_button_duration 10: ");Serial.println(btn[10].ms);  
  btn[11].pin =12; EEPROM.get(510,btn[11].ms);Serial.print("initial_button_duration 11: ");Serial.println(btn[11].ms);  
  btn[12].pin =13; EEPROM.get(520,btn[12].ms);Serial.print("initial_button_duration 12: ");Serial.println(btn[12].ms);  
  btn[13].pin =14; EEPROM.get(530,btn[13].ms);Serial.print("initial_button_duration 13: ");Serial.println(btn[13].ms);  
  btn[14].pin =15; EEPROM.get(540,btn[14].ms);Serial.print("initial_button_duration 14: ");Serial.println(btn[14].ms);  
  btn[15].pin =16; EEPROM.get(550,btn[15].ms);Serial.print("initial_button_duration 15: ");Serial.println(btn[15].ms);  
    supla_btn_init();

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
   WiFi.mode(WIFI_STA); 

      SuplaDevice.addRelay(100, false);
      SuplaDevice.addRelay(101, false); 
      SuplaDevice.addRelay(102, false); 
      SuplaDevice.addRelay(103, false); 
      SuplaDevice.addRelay(104, false); 
      SuplaDevice.addRelay(105, false); 
      SuplaDevice.addRelay(106, false);
      SuplaDevice.addRelay(107, false);
      SuplaDevice.addRelay(108, false);
      SuplaDevice.addRelay(109, false); 
      SuplaDevice.addRelay(110, false); 
      SuplaDevice.addRelay(111, false); 
      SuplaDevice.addRelay(112, false); 
      SuplaDevice.addRelay(113, false); 
      SuplaDevice.addRelay(114, false);
      SuplaDevice.addRelay(115, false); 

    SuplaDevice.setName(Supla_name);
    SuplaDevice.setStatusFuncImpl(&status_func);

    SuplaDevice.begin(GUID,Supla_server,Email,AUTHKEY);

    for (int i = 0; i < 16; i++) {if ((EEPROM.read(i +100) == 1) && (btn[i].ms<10)){mcp.digitalWrite(i, HIGH);} } 
          
}

void loop() {
  if (initialConfig == true){ondemandwifiCallback();}

  int C_W_read = digitalRead(wificonfig_pin);{  
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
   delay(25);

     if (WiFi.status() == WL_CONNECTED){
    if (starting){
      httpUpdater.setup(&httpServer, "/update", "admin", "pass");
      httpServer.begin(); 
      starting = false;         
     }
     httpServer.handleClient();
   }

   switch (s) { 
    case 17:      // -----     STATUS_REGISTERED_AND_READY
     if (state10 == true){ 
      for (int i = 0; i < 16; i++) {
        if (btn[i].ms<10){
         if (EEPROM.read(i +100) == 1){
          SuplaDevice.relayOn(i, 0);
         }
       }
      }            
      state10 = false; 
     }   
     break;     
    case 10:      // --------------------- REGISTER_IN_PROGRESS  ----------------------
     state10 = true;
     break;  
  }
  mcp_btn();
}
