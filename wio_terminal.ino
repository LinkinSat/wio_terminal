#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "lcd_backlight.hpp"

#define WIFI_SSID "LoRaAp"
#define WIFI_PASS "estonotieneinternet"
#define DATA_URL "http://192.168.69.69/api/data"

TFT_eSPI display = TFT_eSPI();
static LCDBackLight backlight;
int sleepTime = 20000;
unsigned long prevMillis;
const long refreshInterval = 500;

String alt;
String temp;
String press;

HTTPClient client;

void setup() {

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
  display.drawString("Datos LiNkInSaT", 10, 10);
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
      Serial.println("Debug2");
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
  client.setTimeout(50);
  client.setConnectTimeout(50);
  Serial.println("Debug3");
  int httpcode = client.GET();
  Serial.println(httpcode);
   if(httpcode > 0){
    String payload = client.getString();
    client.end();
    DynamicJsonDocument doc(1024);
    Serial.println("Debug4");
    Serial.println("Debug5");
    DeserializationError error = deserializeJson(doc,payload);
    Serial.println("Debug6");
    Serial.println(error.c_str());

    alt = doc["alt"].as<String>();
    temp = doc["temp"].as<String>();
    press = doc["press"].as<String>();
  }else {
    client.end();
    display.fillRect(0, 50, 320, 240, TFT_RED);
    display.setTextSize(3);
    display.setCursor(10, 125);
    display.println("Error al conectar con la API");
    return;
  }
}
void drawScreen(){
  display.setTextSize(2);
  display.fillRect(0, 50, 320, 240, TFT_PURPLE);
  display.drawString("Altitud: " + alt, 5, 80);
  display.drawString("Temperatura: " + temp, 5, 120);
  display.drawString("Presion: " + press, 5, 160);
}
