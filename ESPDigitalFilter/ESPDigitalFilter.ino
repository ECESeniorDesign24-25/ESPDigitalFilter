// A digital frequency selective filter
// A. Kruger, 2019
// revised R. Mudumbai, 2020 & 2024

#include <String.h>
#include <WiFi.h>
#include <time.h>
#include "Constants.h"
#include "Email.h"
#include "TimeUtil.h"
#include "hysteresis.h"
#include <ESP_Mail_Client.h>
// #include "WiFiUtil.h"

EmailClient emailClient;

const char* ssid = WIFI_SSID;
const char* username = WIFI_USERNAME;
const char* password = WIFI_PASSWORD;

int ADC_PIN = 35;    
int LED = 33;     
 
const int N_COEFFS = 5;
const int SAMPLES = 20;
float THRESHOLD = 0.15; 
float UPDATE_TIME = 1000e3;

float NUM[] = {0.0103  , -0.0241  ,  0.0332   ,-0.0241   , 0.0103}; 
float DEN[] = {1.0000 ,  -2.4477 ,   3.4382 ,  -2.3761  ,  0.9423}; 
 
float x[N_COEFFS],y[N_COEFFS], y_n, s[SAMPLES];    
 
int Ts = 333; // 4500 Hz sampling rate
bool MESSAGE_SENT = false;
int BAUD_RATE = 115200;

bool ONLINE = false;

//==================================================================================================
float scaleADC(int val) {
  // return val * (3.3 / 4095.00) - 1.65;
  return val*(5.0/1023.0)-2.5;
}

//==================================================================================================
void setup()
{
  Serial.begin(BAUD_RATE);  
  Serial.println("Initializing ESP...");
  // Serial.begin(BAUD_RATE);  
  pinMode(LED,OUTPUT);  

  Serial.println("Attempting WiFi Connection...");
  // ONLINE = connectToWiFi();

  Serial.printf("\nAttempting to connect to %s...\n", ssid);
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
      Serial.printf("\nUnable to connect to %s. Verify your ssid/username/password.", ssid);
      }
  }

  ONLINE = true;


  configTime(-6 * 3600, 3600, "pool.ntp.org", "time.nist.gov"); // UTC-6 for Standard, 1-hour DST adjustment
  delay(2000); 


  // int i;
  // for (i=0; i<N_COEFFS; i++)
  //   x[i] = y[i] = 0;
 
  // for(i = 0; i<SAMPLES; i++)
  // for(i = 0; i<SAMPLES; i++)
  //   s[i] = 0;
  // y_n = 0;
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
     
      // for(i=SAMPLES-1; i>0; i--)    
      for(i=SAMPLES-1; i>0; i--){       
         s[i] = s[i-1];
      }


      val = analogRead(ADC_PIN);  // New input
      x[0] = scaleADC(val);  // Scale to match ADC resolution and range
      // Serial.printf("x %f\n", x[0]);
      // Serial.printf("val %f\n", val);
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
            // String response;
            Serial.println("Sending message");
            emailClient.sendEmail("Cavan Riley", "rileycavan93@gmail.com", "Alert", getFormattedTimestamp());
            Serial.println("Message sent");
            delay(2000);
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

