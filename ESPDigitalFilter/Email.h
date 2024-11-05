#ifndef EMAIL_H
#define EMAIL_H
#include <ESP_Mail_Client.h>
#include "Constants.h"

//==================================================================================================
void initSMTP(SMTPSession &smtp, Session_Config &config);
void sendEmail(const char* recipientName, const char* recipientEmail, const char* subject, const String &timestampFormatted);
void smtpCallback(SMTP_Status status);

#endif