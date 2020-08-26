#include <ESP8266WiFi.h>

const char *ssid = "SSID";
const char *password = "PASS";

IPAddress staticIP(192, 168, 1, 114);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void wifiConnect()
{
    WiFi.config(staticIP, gateway, subnet);
    WiFi.begin(ssid, password);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());    
}
