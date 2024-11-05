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
#include "WiFiUtil.h"

// EmailClient emailClient;

int ADC_PIN = 35;    
int LED = 33;     
 
const int N_COEFFS = 7;
const int SAMPLES = 10;
float THRESHOLD = 0.5; 
float UPDATE_TIME = 1000e3;

float NUM[] = {0.0035,   -0.0087,    0.0087,         0,   -0.0087,    0.0087,   -0.0035}; 
float DEN[] = {1.0000,   -3.7114,    7.4848,   -9.0596,    7.2336,   -3.4664,    0.9026}; 
 
float x[N_COEFFS],y[N_COEFFS], y_n, s[SAMPLES];    
 
int Ts = 333; // 4500 Hz sampling rate
bool MESSAGE_SENT = false;
int BAUD_RATE = 115200;

bool ONLINE = false;

//==================================================================================================
float scaleADC(int val) {
  return val * (3.3 / 4095.00) - 1.65;
}

//==================================================================================================
void setup()
{
  Serial.println("Initializing ESP...");
  Serial.begin(BAUD_RATE);  
  pinMode(LED,OUTPUT);  

  Serial.println("Attempting WiFi Connection...");
  // ONLINE = connectToWiFi();

  int i;
  for (i=0; i<N_COEFFS; i++)
    x[i] = y[i] = 0;
 
  for(i = 0; i<SAMPLES; i++)
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

      for(i=N_COEFFS-1; i>0; i--){  
         x[i] = x[i-1];                                            
         y[i] = y[i-1];
      }
     
      for(i=SAMPLES-1; i>0; i--){       
         s[i] = s[i-1];
      }

      val = analogRead(ADC_PIN);  // New input
      x[0] = scaleADC(val);  // Scale to match ADC resolution and range
      y_n = NUM[0] * x[0];
     
      // Difference equation 
      for(i=1;i<N_COEFFS;i++)       
         y_n = y_n - DEN[i]* y[i] + NUM[i] * x[i];          

      
      y[0] = y_n;             
      s[0] = abs(2*y_n);

      float output = hysteresis_avg(s, SAMPLES);
     
      // update every second
      if ((micros()-changet) > UPDATE_TIME)
      {
        Serial.println(output);
        changet = micros();
        if(output < THRESHOLD)
        {
          digitalWrite(LED, HIGH);

          if (numSent == 0 && ONLINE) {
            String response;
            Serial.println("Sending message");
            // emailClient.sendEmail("Joseph Krueger", "josephkrueger242@gmail.com", "Test", getFormattedTimestamp());
            Serial.println("Message sent");
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

