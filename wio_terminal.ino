#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "lcd_backlight.hpp"

#define WIFI_SSID "LoRaAp"
#define WIFI_PASS "estonotieneinternet"
#define DATA_URL "http://192.168.1.69/api/data"

TFT_eSPI display = TFT_eSPI();
static LCDBackLight backlight;
int sleepTime = 20000;
unsigned long prevMillis;
const long refreshInterval = 500;

String gpsLat;
String gpsLon;
String alt;
String temp;
String press;
String rads;

HTTPClient client;

void setup() {

  client.setTimeout(100);
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  Serial.begin(115200);
  display.init();

  display.begin();
  backlight.initialize();
  display.setRotation(3);
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(3);
  display.drawString("Inicializando", 40, 100);
/*
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
*/
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  display.fillScreen(TFT_BLACK);
  display.drawString("Conectando WiFi", 20, 100);

  display.setCursor(20, 120);
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    display.print(".");
  }

  display.fillScreen(TFT_BLACK);
  Serial.println("Connected to wifi");
  Serial.println(WiFi.localIP());

  display.setTextSize(3);
  display.drawString("Datos cansat", 10, 10);
  display.setTextWrap(true, true);

}

void loop() {

  handleTimeoutBtn();
  
  unsigned long currentMillis = millis();
  if(currentMillis - prevMillis >= refreshInterval){
    prevMillis = currentMillis;
    if(sleepTime > 0){
      sleepTime -= 500;
    }

    if(sleepTime <= 5000 && sleepTime > 0){
      if(backlight.getBrightness() != sleepTime / 50){
        backlight.setBrightness(sleepTime / 50);
      }
    }
    if(sleepTime == 0){
      if(backlight.getBrightness() != 0){
        backlight.setBrightness(0);
      }
    }

    if(sleepTime != 0){
      while (WiFi.status() != WL_CONNECTED){
        display.fillRect(0, 50, 320, 240, TFT_RED);
        display.setTextSize(3);
        display.setCursor(10, 125);
      }
      if(sleepTime > 5000){
        if(backlight.getBrightness() != backlight.getMaxBrightness()){
        backlight.setBrightness(backlight.getMaxBrightness());
      }
    }
      queryHTTPData();
      drawScreen();
    }
  }
}


void handleTimeoutBtn(){
  if(digitalRead(WIO_KEY_A) == LOW){
    sleepTime = 20000;
  }
  if(digitalRead(WIO_KEY_B) == LOW){
    sleepTime = 20000;
  }
  if(digitalRead(WIO_KEY_C) == LOW){
    sleepTime = 20000;
  }
}

void queryHTTPData(){
  client.begin(DATA_URL);
  client.setTimeout(100);
   if(client.GET() > 0){
    StaticJsonDocument<50> doc;
    String payload = client.getString();
    DeserializationError error = deserializeJson(doc,payload);

    gpsLat = doc["gps"]["lat"].as<String>();
    gpsLon = doc["gps"]["lon"].as<String>();
    alt = doc["alt"].as<String>();
    temp = doc["temp"].as<String>();
    press = doc["press"].as<String>();
    rads = doc["rads"].as<String>();
  }else {
    display.fillRect(0, 50, 320, 240, TFT_RED);
    display.setTextSize(3);
    display.setCursor(10, 125);
    display.println("Error al conectar con la API");
    return;
  }
  client.end();
}
void drawScreen(){
  gpsLat.replace("Lat: " ,"");
  gpsLon.replace("Lon: ", "");
  display.setTextSize(2);
  display.fillRect(0, 50, 320, 240, TFT_PURPLE);
  display.drawString("GPS: Latitud: " + gpsLat, 5, 70);
  display.drawString("Longitud: " + gpsLon, 65, 94);
  display.drawString("Altitud: " + alt, 5, 118);
  display.drawString("Temperatura: " + temp, 5, 142);
  display.drawString("Presion: " + press, 5, 164);
  display.drawString("Radiacion: " + rads, 5, 188);
}
