#ifndef TIME_UTIL_H
#define TIME_UTIL_H
#include <String>
#include <time.h>
#include <WiFi.h>


//==================================================================================================
String getFormattedTimestamp() {
  struct tm timeinfo;
  if (!getCurrentTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "00:00 AM on 01/01/2024"; // Default value
  }

  int hour = timeinfo.tm_hour % 12;
  if (hour == 0) hour = 12;  // handle midnight and noon cases
  int minute = timeinfo.tm_min;
  bool isPM = timeinfo.tm_hour >= 12;

  char buffer[30];
  snprintf(buffer, sizeof(buffer), "%02d:%02d %s on %02d/%02d/%04d",
           hour, minute, isPM ? "PM" : "AM",
           timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_year + 1900);

  return String(buffer);
}

//==================================================================================================
bool getCurrentTime(struct tm *timeinfo) {
  const int maxRetries = 10;
  int retryCount = 0;

  while (!getLocalTime(timeinfo) && retryCount < maxRetries) {
    Serial.println("Waiting for NTP time sync...");
    delay(1000);  // Wait 1 second before retrying
    retryCount++;
  }

  return retryCount < maxRetries; // Returns true if successful, false if it timed out
}


#endif