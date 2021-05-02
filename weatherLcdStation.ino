#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>       
#include <Adafruit_ST7735.h> 
#include <SPI.h>
#include <WiFiUdp.h>
#include "weathericons.h"

//-----------------Wifi Settings---------------------------------
const char* ssid = "xxxxxxxxxx"; //SSID 
const char* password = "xxxxxxx";//Password

//------------NTPsettings-------------------------------------------

long utcOffsetInSeconds = 1 * 60 * 60 *2 ; //last *2 is for DST
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//------------Weather settings--------------------------------------
String openWeatherMapApiKey = "xxxxxxxxxxxxxxxxx";//API key OpenWeatherMap

// Replace with your country code and city
String city = "xxxx";//my city name
String countryCode = "xx";//country code
String lang="xx";//language code
String unit="metric"; //imperial or metric

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = -300000;
// Timer set to 10 minutes (600000)
unsigned long timerDelay = 300000;
// Set timer to 10 seconds (10000)
//unsigned long timerDelay = 10000;

String jsonBuffer;

// -----------  Display pins  --------------------------------------
#define TFT_CS         D8      
#define TFT_RST        D3
#define TFT_DC         D4
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); //Those things are for the display
  
void setup() {
  Serial.begin(115200);
  tft.initR(INITR_GREENTAB); 
  tft.setRotation(3);
    WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  timeClient.begin();
  
  
 
 
 
 // --------  TEST TEXT  -------------------------------------------
   tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.drawFastHLine(0,30,144,ST77XX_GREEN);
} 

void drawIcon(String in, int x, int y){
  if(in == "Thunderstorm"){
    tft.drawBitmap(x,y,storm,32,32,ST7735_WHITE);
  }else if(in == "Drizzle"){
    tft.drawBitmap(x,y,rain,32,32,ST7735_WHITE);
  }else if(in == "Rain"){
    tft.drawBitmap(x,y,light_rain,32,32,ST7735_WHITE);
  }else if(in == "Snow"){
    tft.drawBitmap(x,y,snow,32,32,ST7735_WHITE);
  }else if(in == "Fog"){
    tft.drawBitmap(x,y,fog,32,32,ST7735_WHITE);
  }else if(in == "Clear"){
    tft.drawBitmap(x,y,sun,32,32,ST7735_WHITE);
  }else if(in == "Clouds"){
    tft.drawBitmap(x,y,cloud,32,32,ST7735_WHITE);
  }
}
void loop() {
  // Send an HTTP GET request
  
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      timeStr();
      if ((millis() - lastTime) > timerDelay) {
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&units=metric&lang=it&APPID=" + openWeatherMapApiKey;
      
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      //Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      tft.fillRect(0,31,63,45,ST77XX_BLACK);
      tft.fillRect(65,83,63,45,ST77XX_BLACK);
      tft.fillRect(65,31,63,45,ST77XX_BLACK);
      tft.fillRect(0,81,63,45,ST77XX_BLACK);
      const char* condition=myObject["weather"][0]["main"];
      const char* desc = myObject["weather"][0]["description"];
      String descEdit = (String)desc;
      String desc2="";
      if(descEdit.length()>7){
        desc2=descEdit.substring(8);
        descEdit=descEdit.substring(0,7);
        tft.setCursor(70,110);
        tft.setTextSize(0.8);
        tft.print(descEdit);
        tft.setCursor(70,120);
        tft.setTextSize(0.8);
        tft.print(desc2);
      }else{
        tft.setCursor(70,115);
        tft.setTextSize(1);
        tft.print(myObject["weather"][0]["description"]); 
      }
      drawIcon(String(condition),80,82);
      tft.setTextSize(0.7);
      tft.setCursor(0, 40);
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Temperatura");
      tft.setTextSize(1.5);
      tft.setCursor(12, 55);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(myObject["main"]["temp"]);
      tft.setCursor(47, 55);
      tft.setTextColor(ST77XX_WHITE);
      tft.drawCircle(45,54,1,ST77XX_WHITE);
      tft.print("C");
      tft.setTextSize(1.5);
      tft.setCursor(75, 40);
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Umidita");
      tft.setTextSize(1.5);
      tft.setCursor(80, 55);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(myObject["main"]["humidity"]);
      tft.setCursor(100, 55);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("%");
      tft.setTextSize(1.5);
      tft.setCursor(5, 90);
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Pressione");
      tft.setTextSize(1.5);
      tft.setCursor(10, 105);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(myObject["main"]["pressure"]);
      tft.setCursor(30, 105);
      tft.print(" hPa");
      lastTime = millis();
    }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    
  }


String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
void timeStr(){
  tft.setTextWrap(false);
  tft.fillRect(0,0,144,30,ST77XX_BLACK);
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();    
  int currentSecond = timeClient.getSeconds();
  String fixHou=""; 
  String fixMin=""; 
  String fixSec=""; 
  if(currentHour <= 9){
    fixHou="0"+(String)(currentHour);
  }else{
    fixHou=(String)(currentHour);
  }
  if(currentMinute <= 9){
    fixMin="0"+(String)(currentMinute);
  }else{
    fixMin=(String)(currentMinute);
  }
  if(currentSecond <= 9){
    fixSec="0"+(String)(currentSecond);
  }else{
    fixSec=(String)(currentSecond);
  }
  tft.setTextSize(2);
  tft.setCursor(20, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(fixHou+":"+fixMin+":"+fixSec);

  delay(1000);
}
