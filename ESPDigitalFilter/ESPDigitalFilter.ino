#include "hysteresis.h"

// A digital frequency selective filter
// A. Kruger, 2019
// revised R. Mudumbai, 2020 & 2024
#include <String.h>
#include <WiFi.h>
#include <time.h>
#include "Constants.h"
#include <ESP_Mail_Client.h>


//////////////////////TWILIO SETUP/////////////////////
static const char *ssid = WIFI_SSID;
static const char *password = WIFI_PASSWORD;
static const char *username = WIFI_USERNAME;
SMTPSession smtp;

void smtpCallback(SMTP_Status status);


// Function to send email with a formatted message
void sendEmail(const char* recipientName, const char* recipientEmail, const char* subject, const String &timestampFormatted) {
  // Configure email session
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = -6; // Central Time
  config.time.day_light_offset = 1; // 1-hour DST offset

  // Create email message
  SMTP_Message message;
  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = subject;
  message.addRecipient(recipientName, recipientEmail);

  // Format the message content
  String formattedMessage = "Critical Safety Event at ";
  formattedMessage += timestampFormatted;
  message.text.content = formattedMessage.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // Connect and send email
  if (!smtp.connect(&config)) {
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }
}

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

//////////////////////////////////////////////////////// 

int analogPin = 35;     // Specify analog input pin. Make sure to keep between 0 and 5V.
int LED = 33;           // Specify output analog pin with indicator LED
 
const int n = 4;   // number of past input and output samples to buffer; change this to match order of your filter
int m = 10; // number of past outputs to average for hysteresis
 
float num[] = {1,
-3.287,
4.687,
-3.2627,
0.984}; //Denominator Coefficients
 
float den[] = {0.0000475, 0, -0.000095, 0, 0.0000475}; //Numerator Coefficients
 
 
float x[n],y[n], y_n, s[10];     // Space to hold previous samples and outputs; n'th order filter will require upto n samples buffered
 
float threshold_val = 0.25; // Threshold value. Anything higher than the threshold will turn the LED off, anything lower will turn the LED on
 
// time between samples Ts = 1/Fs. If Fs = 3000 Hz, Ts=333 us
int Ts = 222;

bool message_sent = false;
 
void setup()
{
  Serial.begin(115200);
  int i;

  MailClient.networkReconnect(true);

  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  configTime(-6 * 3600, 3600, "pool.ntp.org", "time.nist.gov"); // UTC-6 for Standard, 1-hour DST adjustment
  delay(10000); // Wait a moment for time to sync

  pinMode(LED,OUTPUT);   // Makes the LED pin an output
 
  for (i=0; i<n; i++)
    x[i] = y[i] = 0;
 
  for(i = 0; i<m; i++)
    s[i] = 0;
  y_n = 0;

  // Serial.print("Connecting to WiFi network ;");
  // if (strcmp(WIFI_USERNAME, "") != 0) {
  //   WiFi.mode(WIFI_MODE_STA);
  //   WiFi.begin(ssid, WPA2_AUTH_PEAP, username, username, password);
  // } else {
  //   WiFi.begin(ssid, password);
  // }
  // Serial.println("'...");
  // Serial.println(WiFi.localIP());

  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("Connecting...");
  //   delay(500);
  // }
  // Serial.println("Connected!");

  // // Example usage of sendEmail function
  // String formattedTime = getFormattedTimestamp();
  // sendEmail("Recipient Name", RECIPIENT_EMAIL, "Critical Safety Alert", formattedTime);
}
 
 
 
void loop()
{
   unsigned long t1;
   int i,count,val;
   int k=0;
   float changet = micros();
 
   count = 0;
   while (1) {
      t1 = micros();
 
      // Calculate the next value of the difference equation.
 
      for(i=n-1; i>0; i--){             // Shift samples
         x[i] = x[i-1];                                            
         y[i] = y[i-1];
      }
     
      for(i=m-1; i>0; i--){             // Shift absoulute output
         s[i] = s[i-1];
      }
      val = analogRead(analogPin);  // New input
 
      x[0] = val*(3.3/4095.00)-1.65;  // Scale to match ADC resolution and range
 
      y_n = num[0] * x[0];
     
      for(i=1;i<n;i++)             // Incorporate previous outputs (y[n])
         y_n = y_n - den[i]* y[i] + num[i] * x[i];          
         
 
       y[0] = y_n;                  // New output
 
      //  The variable yn is the output of the filter at this time step.
      //  Now we can use it for its intended purpose:
      //       - Apply theshold
      //       - Apply hysteresis
      //       - What to do when the beam is interrupted, turn on a buzzer, send SMS alert.
      //       - etc.
 
      s[0] = abs(2*y_n);  // Absolute value of the filter output.
 
      float output = hysteresis(s, m);
     
      // Check the output value against the threshold value every 10^6 microseconds or 1 second
      if ((micros()-changet) > 1000e3)
      {
        Serial.println(output);
 
        changet = micros();
        if(output < threshold_val)
        {
          digitalWrite(LED, HIGH);
          // String response;
          // if (!message_sent) {
          //   bool success = send_message(to_number, from_number, message, response);
          //   if (success) {
          //     Serial.println("Sent message successfully!");
          //   } else {
          //     Serial.println(response);
          //   }
          //   delay(5000);
          //   message_sent = true;
          // }
        }
        else
        {
          digitalWrite(LED, LOW);
        }
      }
      // The filter was designed for a 3000 Hz sampling rate. This corresponds
      // to a sample every 333 us. The code above must execute in less time
      // (if it doesn't, it is not possible to do this filtering on this processor).
      // Below we tread some water until it is time to process the next sample
 
     
      if((micros()-t1) > Ts)
      {
        // if this happens, you must reduce Fs, and/or simplify your filter to run faster        
        Serial.println("MISSED A SAMPLE");
      }
      while((micros()-t1) < Ts);  
     
   }
 
}


/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  if (status.success()) {
    Serial.println("Email sent successfully.");
  } else {
    Serial.println("Email sending failed.");
  }

  /* Print the sending result */
  if (status.success()){
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}

