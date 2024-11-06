#include "Email.h"

//==================================================================================================
EmailClient::EmailClient() {
    MailClient.networkReconnect(true);
    smtp.debug(1);
    smtp.callback(smtpCallback);

    // Setting up SMTP server 
    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = "";

    // configTime(-6 * 3600, 3600, "pool.ntp.org", "time.nist.gov"); // UTC-6 for Standard, 1-hour DST adjustment
    // delay(2000); 
}

//==================================================================================================
void EmailClient::sendEmail(const char* recipientName, const char* recipientEmail, const char* subject, const String &timestampFormatted) {
    SMTP_Message message;

    // Setting up the email content
    message.sender.name = F("ESP");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = subject;
    message.addRecipient(recipientName, recipientEmail);

    String formattedMessage = "Critical Safety Event at ";
    formattedMessage += timestampFormatted;
    message.text.content = formattedMessage.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    if (!smtp.connect(&config)) {
        ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return;
    }

    if (!MailClient.sendMail(&smtp, &message)) {
        ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    }
}

//==================================================================================================
void EmailClient::smtpCallback(SMTP_Status status) {
    Serial.println(status.info());

    if (status.success()) {
        Serial.println("Email sent successfully.");
    } else {
        Serial.println("Email sending failed.");
    }
}
