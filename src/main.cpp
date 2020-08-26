#include <Arduino.h>
#include "hw_timer.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <wifiConnect.h>

const byte zcPin = 12;
const byte pwmPin = 13;

byte fade = 1;
byte state = 1;
byte tarBrightness = 255;
byte curBrightness = 0;
byte zcState = 0; // 0 = ready; 1 = processing;

AsyncWebServer server(80);

ICACHE_RAM_ATTR void dimTimerISR()
{
  if (fade == 1)
  {
    if (curBrightness > tarBrightness || (state == 0 && curBrightness > 0))
    {
      --curBrightness;
    }
    else if (curBrightness < tarBrightness && state == 1 && curBrightness < 255)
    {
      ++curBrightness;
    }
  }
  else
  {
    if (state == 1)
    {
      curBrightness = tarBrightness;
    }
    else
    {
      curBrightness = 0;
    }
  }

  if (curBrightness == 0)
  {
    state = 0;
    digitalWrite(pwmPin, 0);
  }
  else if (curBrightness == 255)
  {
    state = 1;
    digitalWrite(pwmPin, 1);
  }
  else
  {
    digitalWrite(pwmPin, 1);
  }

  zcState = 0;
}

ICACHE_RAM_ATTR void zcDetectISR()
{
  if (zcState == 0)
  {
    zcState = 1;

    if (curBrightness < 255 && curBrightness > 0)
    {
      digitalWrite(pwmPin, 0);

      // edo ginete olo to skato???
      int dimDelay = 30 * (255 - curBrightness) + 400; //400
      hw_timer_arm(dimDelay);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(zcPin, INPUT_PULLUP);
  pinMode(pwmPin, OUTPUT);
  attachInterrupt(zcPin, zcDetectISR, RISING); // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  hw_timer_init(NMI_SOURCE, 0);
  hw_timer_set_func(dimTimerISR);

  SPIFFS.begin();

  delay(1000);
  File levelFile = SPIFFS.open("/level.txt", "r");
  if (!levelFile){
    Serial.println("Couldn't open the level.txt file");
  }else{
    int val = levelFile.parseInt();
    levelFile.close();
    if(val > 0){
      tarBrightness = val;
    }
  }

  wifiConnect();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/level", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(tarBrightness));
  });
  server.on("/level", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value", true))
    {
      int val = request->getParam("value", true)->value().toInt();
      if (val >= 0)
      {
        tarBrightness = val;
      }
      if (val > 0 && state == 0)
      {
        state = 1;
      }
      File levelFile = SPIFFS.open("/level.txt", "w");
      if (!levelFile)
      {
        request->send(200, "text/plain", "Got it but couldn't write it in the file");
      }
      else
      {
        levelFile.print(tarBrightness);
        levelFile.close();
        request->send(200, "text/plain", "Got it and wrote it in the file");
      }
    }
    else
    {
      request->send(200, "text/plain", "Did not get it");
    }
  });

  server.begin();
}

bool inc = true;
void loop()
{
}
