#ifndef EMAIL_H
#define EMAIL_H
#include <Arduino.h>
#include <ESP_Mail_Client.h>
#include "Constants.h"

//==================================================================================================
void initSMTP(SMTPSession &smtp, Session_Config &config) {
  MailClient.networkReconnect(true);
  smtp.debug(1);
  smtp.callback(smtpCallback);

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  configTime(-6 * 3600, 3600, "pool.ntp.org", "time.nist.gov"); // UTC-6 for Standard, 1-hour DST adjustment
  delay(10000); // Wait a moment for time to sync

}

//==================================================================================================
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

//==================================================================================================
void smtpCallback(SMTP_Status status){
  Serial.println(status.info());

  if (status.success()) {
    Serial.println("Email sent successfully.");
  } else {
    Serial.println("Email sending failed.");
  }

  if (status.success()){

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(i);
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}



#endif