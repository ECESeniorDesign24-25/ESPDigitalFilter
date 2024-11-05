#ifndef WIFI_UTIL_H
#define WIFI_UTIL_H
#include <Arduino.h>
#include <WiFi.h>
#include "Constants.h"

//==================================================================================================
bool connectToWiFi() {
  Serial.print("Connecting to WiFi network ;");
  if (strcmp(WIFI_USERNAME, "") != 0) {
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, WPA2_AUTH_PEAP, username, username, password);
  } else {
    WiFi.begin(ssid, password);
  }
  Serial.println("'...");
  Serial.println(WiFi.localIP());

  float currTime = micros();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(500);

    // Timeout after 10 seconds
    if (micros() - currTime > 10e6) {
      Serial.println("Connection timed out. Running offline");
      return false;
    }
  }
  Serial.println("Connected!");
  return True;
}


#endif