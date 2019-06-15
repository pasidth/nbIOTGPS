#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <TimerOne.h>
#include <Arduino.h>
#include <avr/wdt.h>
#include "Magellan.h"

#include "SoftwareSerial.h"

Magellan magel;

//Token Key you can get from magellan platform
char auth[]="6a92db70-8d20-11e9-96dd-9fb5d8a71344";     
String payload;
String dataStr[100];

static const int RXPin = 10, TXPin = 11;//D1 -> Tx, D2 -> RX
SoftwareSerial GPSModule(RXPin, TXPin); // TX, RX
int updates;
int failedUpdates;
int pos;
int stringplace = 0;
String timeUp;
String nmea[15];
String labels[12] {"Time: ", "Status: ", "Latitude: ", "Hemisphere: ", "Longitude: ", "Hemisphere: ", "Speed: ", "Track Angle: ", "Date: "};

String fileName = "nbIOTMacellanGPS.ino";

String serverIP = "34.87.44.102"; // Your Server IP
String serverPort = "2002"; // Your Server Port
String jsonData ="";
float t,h;
float tsht,hsht;

LiquidCrystal_I2C lcd(0x27,16,2); 
const long interval = 20000;  //millisecond
unsigned long previousMillis = 0;
#include "DHT.h"
#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float tDHT;
float hDHT;
int dispCounter =0;

bool refreshFlag =0;
String errTxt = "";
String refreshTxt = "Refreshing";
String readString;
String data;
String payloadStr = "";
String output = "";
int cnt =0;

void setup()
{ 
  Serial.begin(9600);Serial.println(fileName); 
  Serial.println("Starting up (2s delay)");
  // initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("NB IOT Project  ");
  lcd.setCursor(0,1);
  lcd.print("+ macellan  ...");
  lcdBlink(3,500);
  //init Magellan LIB
  magel.begin(auth);  
  delay(1000);
  lcd.setCursor(0,2);
  lcd.print("Connected");

  GPSModule.begin(9600);
  previousMillis = millis();          
//  wdt_enable(WDTO_8S);//watch dog timer
  wdt_reset();
}
void lcdBlink(int a,int b){
    for (int f = 0; f <a;f++){
    lcd.noBacklight();
    delay(b);
    lcd.backlight();
    delay(b);
  }
}
void sleepDisp(){
      lcd.setCursor(15,1);
      lcd.print(" ");
      lcd.noBacklight();
}
void readDHT11(){
      // get data from DHT22 sensor
      float htempo = dht.readHumidity();
      float ttempo = dht.readTemperature();
      delay(50);
        if (isnan(htempo)){
          Serial.println("h = NAN");
        }
        else {
          hDHT = htempo;
          Serial.print(" Humi= ");Serial.println(hDHT);
        }
        if (isnan(ttempo)){
          Serial.println("t = NAN");
        }
        else {
          tDHT = ttempo;
          Serial.print("Temp= ");Serial.println(tDHT);
        }
}
void gpsRead(){
    wdt_reset();
    GPSModule.flush();
    while (GPSModule.available() > 0)
    {
      GPSModule.read();
    }
    if (GPSModule.find("$GPRMC,")) {
        String tempMsg = GPSModule.readStringUntil('\n');
        for (int i = 0; i < tempMsg.length(); i++) {
            if (tempMsg.substring(i, i + 1) == ",") {
              nmea[pos] = tempMsg.substring(stringplace, i);
              stringplace = i + 1;
              pos++;
            }
            if (i == tempMsg.length() - 1) {
              nmea[pos] = tempMsg.substring(stringplace, i);
            }
        }
        updates++;
        nmea[2] = ConvertLat();
        nmea[4] = ConvertLng();
        
        for (int i = 0; i < 9; i++) {
            Serial.print(labels[i]);
            Serial.print(nmea[i]);
            Serial.println("");
        }
        
    }
    else {
    failedUpdates++;
    }
    stringplace = 0;
    pos = 0;
}
//*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*
void loop()
{ 
  wdt_reset();
  readDHT11(); 
  //set time to send data
  unsigned long currentMillis = millis();
  unsigned int timeleft = (interval - (currentMillis - previousMillis))/1000;
  Serial.print("Time left= ");
  Serial.println(timeleft);//display time to send
  if (currentMillis - previousMillis >= interval)
    {    
      // Send data in String  to udp format
      wdt_reset();
      cnt++;
      previousMillis = currentMillis;
      //lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Payload --> Sent");
      magel.post(jsonData);
      lcd.backlight();
      lcd.setCursor(15,1);
      lcd.print("*");
      lcd.backlight();
      Serial.println("Data Sent Out");
    }
    if (currentMillis - previousMillis >= interval/4)
    {    
      //lcd.setCursor(15,1);
      //lcd.print("-");
      //Serial.println("Lcd Dim");
      lcd.noBacklight();
    }
  gpsRead();
  jsonFormat();//combine jason format, manual
  jsonString();//sterelize jason
  lcdDisplay();
  delay(500);
}
void jsonFormat(){
  jsonData = '{';

  jsonData += '"';
  jsonData += "ddate";
  jsonData += '"';
  jsonData += ':';
  jsonData += '"';
  jsonData += String(nmea[8]);
  jsonData += '"';
  jsonData += ',';

  jsonData += '"';
  jsonData += "speed";
  jsonData += '"';
  jsonData += ':';
  jsonData += '"';
  jsonData += String(nmea[6]);
  jsonData += '"';
  jsonData += ',';

//nested array

  jsonData += '"';
  jsonData += "position";
  jsonData += '"';
  jsonData += ":[";
  jsonData += '"';
  jsonData += String(nmea[2]);
  jsonData += '"';
  jsonData += ',';
  jsonData += '"';
  jsonData += String(nmea[4]);
  jsonData += '"';
  jsonData += "],";
 
//  jsonData += ',';
  jsonData += '"';
  jsonData += "temp";
  jsonData += '"';
  jsonData += ':';
  jsonData += '"';
  jsonData += String(tDHT);
  jsonData += '"';
  jsonData += ',';
/*  
  jsonData += '"';
  jsonData += "humi";
  jsonData += '"';  
  jsonData += ':';
  jsonData += '"';  
  jsonData += String(h);
  jsonData += '"';
  jsonData += ',';
*/  
  jsonData += '"';
  jsonData += "cnt";
  jsonData += '"';  
  jsonData += ':';
  jsonData += '"';  
  jsonData += String(cnt);
  jsonData += '"';
    
  jsonData += "}";
  
  Serial.print("jsonData= ");
  Serial.println(jsonData);
  Serial.print(".length= ");
  Serial.println(jsonData.length());

  if (cnt>=100) {
    cnt =0;
  }
}
void jsonString(){
    // Add values in the document
  char charBuf[8];
  output = "";
  dtostrf(tDHT,6,2,charBuf);
  String tDHTStr = "";
  for (int i =0;i<sizeof(charBuf);i++){
    tDHTStr += charBuf[i];
  }
    dtostrf(hDHT,6,2,charBuf);
  String hDHTStr = "";
  for (int i =0;i<sizeof(charBuf);i++){
    hDHTStr += charBuf[i];
  }
  StaticJsonDocument<500> doc;
  doc["lat"] = nmea[2];//value to sterilize
  doc["long"] = nmea[4];
  doc["date"] = nmea[8];
  doc["speed"] = nmea[6];
  doc["ambT"] = tDHTStr;
  doc["ambH"] = hDHTStr;
  JsonArray data = doc.createNestedArray("data");
      data.add(nmea[2]);
      data.add(nmea[4]);
  serializeJson(doc, output);
  Serial.print("output= ");
  Serial.println(output);
  //serializeJsonPretty(doc,Serial);
  //Serial.println();
  //Serial.print("jsonMeasure= ");
  //Serial.println(measureJsonPretty(doc));
}

String ConvertLat() {
    String posneg = "";
    if (nmea[3] == "S") {
        posneg = "-";
    }
    String latfirst;
    float latsecond;
    for (int i = 0; i < nmea[2].length(); i++) {
        if (nmea[2].substring(i, i + 1) == ".") {
          latfirst = nmea[2].substring(0, i - 2);
          latsecond = nmea[2].substring(i - 2).toFloat();
        }
    }
    latsecond = latsecond / 60;
    String CalcLat = "";
    
    char charVal[9];
    dtostrf(latsecond, 4, 6, charVal);
    for (int i = 0; i < sizeof(charVal); i++)
    {
      CalcLat += charVal[i];
    }
    latfirst += CalcLat.substring(1);
    latfirst = posneg += latfirst;
    return latfirst;
}

String ConvertLng() {
    String posneg = "";
    if (nmea[5] == "W") {
        posneg = "-";
    }
    String lngfirst;
    float lngsecond;
    for (int i = 0; i < nmea[4].length(); i++) {
        if (nmea[4].substring(i, i + 1) == ".") {
            lngfirst = nmea[4].substring(0, i - 2);
            //Serial.println(lngfirst);
            lngsecond = nmea[4].substring(i - 2).toFloat();
            //Serial.println(lngsecond);
        }
    }
    lngsecond = lngsecond / 60;
    String CalcLng = "";
    char charVal[9];
    dtostrf(lngsecond, 4, 6, charVal);
    for (int i = 0; i < sizeof(charVal); i++)
    {
        CalcLng += charVal[i];
    }
    lngfirst += CalcLng.substring(1);
    lngfirst = posneg += lngfirst;
    return lngfirst;
}

void hardReset(){
  wdt_disable();
  wdt_enable(WDTO_15MS);
}

void lcdDisplay(){
  wdt_reset();
  switch (dispCounter){
    case 0:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Date:");
        lcd.print(nmea[8]);
        lcd.setCursor(0,1);
        lcd.print("Speed:");
        lcd.print(nmea[6]);
        dispCounter ++;
        break;  
    case 1:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Lat:");
        lcd.print(nmea[2]);
        lcd.setCursor(0,1);
        lcd.print("Lon:");
        lcd.print(nmea[4]);
        dispCounter ++;
        break;   
    case 2:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("ambT= ");
          lcd.setCursor(6,0);
          lcd.print(tDHT);
          lcd.setCursor(0,1);
          lcd.print("ambH= ");
          lcd.setCursor(6,1);
          lcd.print(hDHT);
        dispCounter =0;
        break;
      default:
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Error !!!");
          dispCounter =0;
          break;
  }
}
