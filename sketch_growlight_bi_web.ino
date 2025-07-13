/*
  Basic Growbox controller

  There are 3 functions:
   * LED lighting contol with imitation of sun rise and fall and time sync by WiFi 
     and NTP.
     By light's constants you can set sunrise and sunset time.
     Red LED time is main light and blue is secondary. It's nature thing, don't ask.
     
     By RedBlueDiffHours const you can set the difference between red and light LED.
     if you set it to 2 it means that red time duration will be 2 hours longer than 
     blue time duration. 2 devided by half. 1 half of red at the beginning of the 
     day and one at ending.
     If you set it to 0 red and blue LED dirations will be same.
     
     My recomendation for chillies: full light day should be 14h long.
     Try to set RedBlueDiffHours to 2. It will be 1h of only red at morning then 12h 
     of red and blue then 1 hour of only red at the evening. You cant olso try to set
     it to 4.
     
     
   * Water pump control. 
     You can set irrigation interval and duration. I use hybrid of 
     DWC and Kratky hydroponics method. Then i use irrigation every hour for 1-5 mins.


   * Temperature indication via JSON.
     Indicates current temperature from connected DS18b20 sensor (If you connect it).


  You can get current device status (like leds, pump and temp via get request in 
  json format. You can add authentification and change GET requests to POST 
  when it changes states of pins for adherence to standards. 
  
  The built-in LED will blinks.
*/


#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <FS.h>                                                         // Lib for file system
#include <ESP8266FtpServer.h>                                           // Lib for work with SPIFFS via FTP
#include <Ticker.h>

#include <Time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "WifiCreds.h"

#define ONE_WIRE_PIN D4
#define PUMP_PIN 12
#define RED_LED_PIN  16
#define BLUE_LED_PIN 14
 
OneWire oneWire(ONE_WIRE_PIN);                                          //Temperature sensor pin via one wire
DallasTemperature sensors(&oneWire);

const unsigned int offset = 28800; //timezone offset. My is +8h
WiFiUDP ntpUDP;
const unsigned int IRRIGATION_DURATION = 60; //60 1 min
const unsigned int  IRRIGATION_INTERVAL = 3600; //3600 1 hour

Ticker irrigationTimer;
Ticker irrigationStopTimer;

enum state {AUTO, MANUAL};
state curMode = AUTO;

byte lightRiseHour = 6;
byte lightRiseMinute = 0;
byte lightSunsetHour = 20;
byte lightSunsetMinute = 0;
byte RedBlueDiffHours = 2;
Ticker lightTimer;


// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", offset, 600000);
MDNSResponder mdns;
//web server
ESP8266WebServer server(80);
FtpServer ftpSrv;
String webPage = "";

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  
  irrigationTimer.attach(IRRIGATION_INTERVAL, feed); //start irrigation timer
  lightTimer.attach(1, processLighting); //process lighting observation every 1 sec

  Serial.print("IP address: ");  //  "IP-адрес: "
  Serial.println(WiFi.localIP());

  if (mdns.begin("growcon", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  // Add service to MDNS-SD
  mdns.addService("http", "tcp", 80);
  
//  server.on("/", [](){
//    server.send(200, "text/html", getMainPage());
//  });

  server.on("/mode/AUTO", [](){
    curMode = AUTO;
    String msg = "{\"Mode\":\"";msg+=getStateName(curMode);msg+="\"}";
    server.send(200, "application/json", msg);
  });

  server.on("/mode/MANUAL", [](){
    curMode = MANUAL;
    String msg = "{\"Mode\":\"";msg+=getStateName(curMode);msg+="\"}";
    server.send(200, "application/json", msg);
  });

  server.on("/status", [](){
    server.send(200, "application/json", getStatus());
  });

  server.on("/waterOn", [](){
    digitalWrite(PUMP_PIN, HIGH); 
    server.send(200, "application/json", Jsonify("Pump",String(digitalRead(PUMP_PIN))));
  });
  
  server.on("/waterOff", [](){
    digitalWrite(PUMP_PIN, LOW);
    server.send(200, "application/json", Jsonify("Pump",String(digitalRead(PUMP_PIN))));
  });

  server.on("/redOn", [](){
    relayOn(RED_LED_PIN);
    server.send(200, "application/json", Jsonify("Red",String(digitalRead(RED_LED_PIN))));
  });

  server.on("/redOff", [](){
    relayOff(RED_LED_PIN);
    server.send(200, "application/json", Jsonify("Red",String(digitalRead(RED_LED_PIN))));
  });

  server.on("/blueOn", [](){
    relayOn(BLUE_LED_PIN);
    server.send(200, "application/json", Jsonify("Blue",String(digitalRead(BLUE_LED_PIN))));
  });

  server.on("/blueOff", [](){
    relayOff(BLUE_LED_PIN);
    server.send(200, "application/json", Jsonify("Blue",String(digitalRead(BLUE_LED_PIN))));
  });

  server.onNotFound([](){
  if(!handleFileRead(server.uri())) 
      server.send(404, "text/plain", "Not Found");
  });

  SPIFFS.begin();
  server.begin();
  Serial.println("HTTP server started");
  ftpSrv.begin("grow","grow"); 
  Serial.println("FTP server started");
  curMode = AUTO;
}

const String getStateName(enum state st) 
{
   switch (st) 
   {
      case AUTO: return "AUTO";
      case MANUAL: return "MANUAL";
      default: return "AUTO";
   }
}

String Jsonify(String param, String value) {
  String message = "{\n";
  message += "\"" + param + "\" : "; 
  message += value + "\n}";
  return message;
}

String getStatus() {                                                 // Returns current device status in json 
  String message = "";
  message += "{\n";
  message += "\"Mode\" : \""; message+=String(getStateName(curMode)); message+="\",\n";
  message += "\"Red\" : " + String(digitalRead(RED_LED_PIN));message+=",\n";
  message += "\"Blue\" : " + String(digitalRead(BLUE_LED_PIN));message+=",\n";
  message += "\"Pump\" : " + String(digitalRead(PUMP_PIN));message+=",\n";
  message += "\"Temp\" : " + getTemp() + ",\n";
  message += "\"TempMode\" : \"°C\"\n";
  message += "}";
  return message;
}

String getTemp() {
  sensors.requestTemperatures();
  return String(sensors.getTempCByIndex(0));  
}

int relayOn(int pin) {
  digitalWrite(pin, LOW);
  return LOW;
}

int relayOff(int pin) {
  digitalWrite(pin, HIGH);
  return HIGH;
}

void feed() {
  digitalWrite(PUMP_PIN, HIGH);  
  irrigationStopTimer.attach(IRRIGATION_DURATION, stopFeed);
}

void stopFeed () {
  irrigationStopTimer.detach();
  digitalWrite(PUMP_PIN, LOW); 
}

void processLighting() {
  byte currRedPin = digitalRead(RED_LED_PIN);
  byte currBluePin = digitalRead(BLUE_LED_PIN);
  byte currPumpState  = digitalRead(PUMP_PIN); 

  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  
  if (curMode == MANUAL) return;
  
  // LOW = open relay; HIGH = close relay;
  if (shouldBeRedOn()) {
    //digitalWrite(RED_LED_PIN, LOW);
    relayOn(RED_LED_PIN);
  } else {
    //digitalWrite(RED_LED_PIN, HIGH);
    relayOff(RED_LED_PIN);
  }
  if (shouldBeBlueOn()) {
    //digitalWrite(BLUE_LED_PIN, LOW);
    relayOn(BLUE_LED_PIN);
  } else {
    //digitalWrite(BLUE_LED_PIN, HIGH);
    relayOff(BLUE_LED_PIN);
  }
}

bool shouldBeRedOn() {
  return shouldBeOn(lightRiseHour, lightRiseMinute, lightSunsetHour, lightSunsetMinute);
}

bool shouldBeBlueOn() { 
  return shouldBeOn(lightRiseHour+(RedBlueDiffHours/2), lightRiseMinute, lightSunsetHour-(RedBlueDiffHours/2), lightSunsetMinute);
}

bool shouldBeOn(const byte riseH, const byte riseM, const byte sunsetH, const byte sunsetM) { 
  const byte currH = timeClient.getHours();
  const byte currM = timeClient.getMinutes();
  
  if (currH > riseH) {
    if (currH < sunsetH) {
      return true;
    }
    else if (currH == sunsetH) {
      if (currM < sunsetM) {
        return true;
      } else {
        return false;
      }
    }
    else {
      return false;
    }   
  }
  else if (currH == riseH) {
      if (currM >= riseM) {
        return true;
      } else {
        return false;
      }    
  } else {
    return false;
  }
}

void loop() {
  mdns.update();
  server.handleClient();
  ftpSrv.handleFTP();
}

String getContentType(String filename){                                 // Returns mime tipe depends of a file extensions
  if (filename.endsWith(".html")) return "text/html";                   
  else if (filename.endsWith(".css")) return "text/css";                
  else if (filename.endsWith(".js")) return "application/javascript";   
  else if (filename.endsWith(".png")) return "image/png";               
  else if (filename.endsWith(".jpg")) return "image/jpeg";              
  else if (filename.endsWith(".gif")) return "image/gif";               
  else if (filename.endsWith(".ico")) return "image/x-icon";            
  return "text/plain";                                                  
}

bool handleFileRead(String path){                                       // Works wif file system and sends to user our index.html if user  requests  root "/" 
  if(path.endsWith("/")) path += "index.html";                          // if a user requests not "/" then sends to the user a file with a valid mime type
  String contentType = getContentType(path);                            
  if(SPIFFS.exists(path)){                                              
    File file = SPIFFS.open(path, "r");                                 
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
