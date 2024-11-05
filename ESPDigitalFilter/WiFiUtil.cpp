#include <Arduino>
#include "WiFiUtil.h"

//==================================================================================================
bool connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  float currTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - currTime > 10000) {
      Serial.println("Failed to connect to WiFi");
      return false;
    }
  }
  Serial.println("Connected to WiFi");
  return true;
}