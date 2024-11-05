// A digital frequency selective filter
// A. Kruger, 2019
// revised R. Mudumbai, 2020 & 2024

#include <String.h>
#include <WiFi.h>
#include <time.h>
#include "Constants.h"
#include "Email.h"
#include "TimeUtil.h"
#include "Hysteresis.h"
#include <ESP_Mail_Client.h>


static const char *ssid = WIFI_SSID;
static const char *password = WIFI_PASSWORD;
static const char *username = WIFI_USERNAME;

SMTPSession smtp;
Session_Config config;

int ADC_PIN = 35;    
int LED = 33;     
 
const int ORDER = 8;
int HYSTERESIS_SAMPLES = 10;
float THRESHOLD = 0.25; 

float NUM[] = {0.000001357,         0,   -0.000005430,         0,    0.000008144,        0,   -0.000005430,        0,    0.000001357}
float DEN[] = {1.0000,   -6.4543,  19.4422,  -35.3000,   42.1002,  -33.7258,   17.7467,   -5.6288,    0.8332};
 
 
float x[n],y[n], y_n, s[10];    
 
int Ts = 222; // 4500 Hz sampling rate
bool MESSAGE_SENT = false;
int BAUD_RATE = 115200;

bool online = false;

//==================================================================================================
float scaleADC(int val) {
  return val * (3.3 / 4095.00) - 1.65;
}

//==================================================================================================
void setup()
{
  Serial.begin(BAUD_RATE);  
  initSMTP(smtp, config);
  pinMode(LED,OUTPUT);  
  online = connectToWiFi();

  int i;
  for (i=0; i<ORDER; i++)
    x[i] = y[i] = 0;
 
  for(i = 0; i<HYSTERESIS_SAMPLES; i++)
    s[i] = 0;
  y_n = 0;
}

//==================================================================================================
void loop()
{
   unsigned long t1;
   int i,val;
   int k=0;
   float changet = micros();
 
   int numSent = 0;
   while (1) {
      t1 = micros();

      for(i=ORDER-1; i>0; i--){  
         x[i] = x[i-1];                                            
         y[i] = y[i-1];
      }
     
      for(i=HYSTERESIS_SAMPLES-1; i>0; i--){       
         s[i] = s[i-1];
      }
      val = analogRead(ADC_PIN);  // New input
 
      x[0] = scaleADC(val);  // Scale to match ADC resolution and range
 
      y_n = NUM[0] * x[0];
     
      // Difference equation 
      for(i=1;i<ORDER;i++)         
         y_n = y_n - DEN[i]* y[i] + NUM[i] * x[i];          
    
      y[0] = y_n;             
      s[0] = abs(2*y_n);
      float output = hysteresis(s, m);
     
      // update every second
      if ((micros()-changet) > 1000e3)
      {
        Serial.println(output);
 
        changet = micros();
        if(output < THRESHOLD)
        {
          digitalWrite(LED, HIGH);

          if (numSent == 0) {
            String response;
            bool success = send_message(to_NUMber, from_NUMber, message, response);
              if (success) {
                Serial.println("Sent message successfully!");
              } else {
                Serial.println(response);
              }
              delay(5000);
          }
          numSent++;
        }
        else
        {
          digitalWrite(LED, LOW);
          numSent = 0;
        }
      }

      // check if we missed a sample
      if((micros()-t1) > Ts)
      {
        Serial.println("MISSED A SAMPLE");
      }
      while((micros()-t1) < Ts);  
   }
 
}

