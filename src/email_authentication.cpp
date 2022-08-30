#include <iostream>
#include <string>
#include <ctime>
#include <algorithm>
#include <random>
#include <curl/curl.h>
#include <security/pam_modules.h>

#include "token_generator.h"
#include "db_connection.cpp"

class Email
{
public:
   Email(const std::string &to,
         const std::string &from,
         const std::string &nameFrom,
         const std::string &subject,
         const std::string &body,
         const std::string &cc);

   CURLcode send(const std::string &url,
                 const std::string &userName,
                 const std::string &password);

private:
   std::string to_;
   std::string from_;
   std::string cc_;
   std::string name_from_;
   std::string subject_;
   std::string body_;

   struct StringData
   {
      std::string msg;
      size_t bytesleft;
      StringData(std::string &&m) : msg{m}, bytesleft{msg.size()} {}
      StringData(std::string &m) = delete;
   };

   static std::string _dateTimeNow();
   std::string _generateMessageId() const;
   static size_t _payloadSource(void *ptr, size_t size, size_t nmemb, void *userp);
   std::string _setPayloadText();
};

Email::Email(const std::string &to,
             const std::string &from,
             const std::string &nameFrom,
             const std::string &subject,
             const std::string &body,
             const std::string &cc = "") : to_{to},
                                           cc_{cc},
                                           from_{from},
                                           name_from_{nameFrom},
                                           subject_{subject},
                                           body_{body}
{
}

CURLcode Email::send(const std::string &url,
                     const std::string &userName,
                     const std::string &password)
{
   CURLcode ret = CURLE_OK;

   struct curl_slist *recipients = NULL;

   CURL *curl = curl_easy_init();

   StringData textData{_setPayloadText()};

   if (curl)
   {
      curl_easy_setopt(curl, CURLOPT_USERNAME, userName.c_str());
      curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

      curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
      // curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");

      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + from_ + ">").c_str());
      recipients = curl_slist_append(recipients, ("<" + to_ + ">").c_str());
      // recipients = curl_slist_append(recipients,   ("<" + cc_   + ">").c_str());

      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, _payloadSource);
      curl_easy_setopt(curl, CURLOPT_READDATA, &textData);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

      ret = curl_easy_perform(curl);

      if (ret != CURLE_OK)
      {
         std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(ret) << "\n";
      }

      curl_slist_free_all(recipients);
      curl_easy_cleanup(curl);
   }

   return ret;
}

std::string Email::_dateTimeNow()
{
   const int RFC5322_TIME_LEN = 32;

   std::string ret;
   ret.resize(RFC5322_TIME_LEN);

   time_t tt;

#ifdef _MSC_VER
   time(&tt);
   tm *t = localtime(&tt);
#else
   tm tv, *t = &tv;
   tt = time(&tt);
   localtime_r(&tt, t);
#endif

   strftime(&ret[0], RFC5322_TIME_LEN, "%a, %d %b %Y %H:%M:%S %z", t);

   return ret;
}

std::string Email::_generateMessageId() const
{
   const size_t MESSAGE_ID_LEN = 37;

   tm t;
   time_t tt;
   time(&tt);

   std::string ret;
   ret.resize(15);

#ifdef _MSC_VER
   gmtime_s(&t, &tt);
#else
   gmtime_r(&tt, &t);
#endif

   ret.resize(MESSAGE_ID_LEN);
   size_t datelen = std::strftime(&ret[0], MESSAGE_ID_LEN, "%Y%m%d%H%M%S", &t);

   static const std::string alpha_num{
       "0123456789"
       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
       "abcdefghijklmnopqrstuvwxyz"};

   std::mt19937 gen;
   std::uniform_int_distribution<> dis(0, alpha_num.length() - 1);
   std::generate_n(ret.begin() + datelen, MESSAGE_ID_LEN - datelen, [&]()
                   { return alpha_num[dis(gen)]; });

   return ret;
}

size_t Email::_payloadSource(void *ptr, size_t size, size_t nmemb, void *userp)
{
   StringData *text = reinterpret_cast<StringData *>(userp);

   if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1) || (text->bytesleft == 0))
   {
      return 0;
   }

   if ((nmemb * size) >= text->msg.size())
   {
      text->bytesleft = 0;
      return text->msg.copy(reinterpret_cast<char *>(ptr), text->msg.size());
   }

   return 0;
}

std::string Email::_setPayloadText()
{
   std::string payloadText = "Date: " + _dateTimeNow() + "\r\n"
                                                         "To: <" +
                             to_ + ">\r\n"
                                   "From: \"" +
                             name_from_ + "\" <" + from_ + ">\r\n";
   if (cc_.length())
   {
      payloadText += "Cc: <" + cc_ + ">\r\n";
   }
   payloadText += "Message-ID: <" + _generateMessageId() + "@" + from_.substr(from_.find('@') + 1) + ">\r\n"
                                                                                                     "Subject: " +
                  subject_ + "\r\n"
                             "\r\n" +
                  body_ + "\r\n"
                          "\r\n";
   return payloadText;
}

bool send_verification_code(User user)
{
   DbConnection db_connection;
   std::string token = generate_random_alphanumeric_string(6);
   Email email(user.get_email(),
               "ecka18_04@yahoo.com",
               "PAM Email Authentication",
               "Authentication Verification",
               "Hello,\nYour authentication token is below:\n" + token);
   
   db_connection.insert_token(user.get_username(), token);
   CURLcode ret_code = email.send("smtp://smtp.mail.yahoo.com:587",
                                  "ecka18_04@yahoo.com",
                                  "xrhkuhudukkqjgwm");
   return ret_code == CURLE_OK;
}

int check_verification_code(User user, std::string code)
{
   DbConnection db_connection;
   std::optional<Token> token_optional = db_connection.retrieve_token_by_username_token_content(user.get_username(), code);
   if (token_optional)
   {
      Token token = token_optional.value();
      std::time_t current_time = std::time(nullptr);
      std::time_t new_time = token.get_created_at() + (60 * EXPIRATION_TIME);
      if (difftime(new_time, current_time) < 0)
      {
         std::cout << "\nVerification code is expired!" << std::endl;
         return PAM_AUTH_ERR;
      }
      db_connection.set_token_as_used(token.get_id());
      return PAM_SUCCESS;
   }
   return PAM_AUTH_ERR;
}
