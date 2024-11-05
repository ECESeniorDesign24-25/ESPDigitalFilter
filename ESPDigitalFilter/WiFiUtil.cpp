#include "WiFiUtil.h"

//==================================================================================================
bool connectToWiFi() {
    Serial.printf("\nAttempting to connect to %s...\n", WIFI_SSID);
    if (strcmp(WIFI_USERNAME, "") != 0) { // enterprise network (eduroam)
        WiFi.mode(WIFI_MODE_STA);
        WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, WIFI_USERNAME, WIFI_USERNAME, WIFI_PASSWORD);
    }
    else { // personal network
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("."));
        if (millis() - startTime > 30000) {
        Serial.printf("\nUnable to connect to %s. Verify your ssid/username/password.", WIFI_SSID);
        return false;
        }
    }
    return true;
}
