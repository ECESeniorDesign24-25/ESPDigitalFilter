#ifndef TIME_UTIL_H
#define TIME_UTIL_H
#include <String>
#include <time.h>
#include <WiFi.h>

String getFormattedTimestamp();
bool getCurrentTime(struct tm *timeinfo);


#endif