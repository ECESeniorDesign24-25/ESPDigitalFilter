#ifndef EMAIL_H
#define EMAIL_H

#include <ESP_Mail_Client.h>
#include "Constants.h"

class EmailClient {
    public:
        EmailClient();
        void sendEmail(const char* recipientName, const char* recipientEmail, const char* subject, const String &timestampFormatted);

    private:
        SMTPSession smtp;
        Session_Config config;
        static void smtpCallback(SMTP_Status status);
};

#endif
