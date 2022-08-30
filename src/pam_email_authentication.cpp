#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"
#include "db_authentication.cpp"
#include "email_authentication.cpp"

// sudo g++ -std=gnu++17 -I/usr/include pam_email_authentication.cpp -shared -o /lib64/security/pam_email_authentication.so -fPIC -lpqxx -lcurl

static int converse(pam_handle_t *pamh, int nargs,
                    const struct pam_message **message,
                    struct pam_response **response)
{
  struct pam_conv *conv;
  int retval = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
  if (retval != PAM_SUCCESS)
  {
    return retval;
  }
  return conv->conv(nargs, message, response, conv->appdata_ptr);
}

static char *request_code(pam_handle_t *pamh, int echocode,
                          const char *prompt)
{
  // Query user for verification code
  const struct pam_message msg = {.msg_style = echocode,
                                  .msg = prompt};
  const struct pam_message *msgs = &msg;
  struct pam_response *resp = NULL;
  int retval = converse(pamh, 1, &msgs, &resp);
  char *ret = NULL;
  if (retval != PAM_SUCCESS || resp == NULL || resp->resp == NULL || *resp->resp == '\000')
  {
    // log_message(LOG_ERR, pamh, "Did not receive verification code from user");
    if (retval == PAM_SUCCESS && resp && resp->resp)
    {
      ret = resp->resp;
    }
  }
  else
  {
    ret = resp->resp;
  }

  // Deallocate temporary storage
  if (resp)
  {
    if (!ret)
    {
      free(resp->resp);
    }
    free(resp);
  }

  return ret;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *handle, int flags, int argc, const char **argv)
{

  const char *username, *password;
  int pam_returned_status;

  // openlog("my_pam_authentication", LOG_CONS, LOG_AUTHPRIV);
  // syslog(1, "\n\nHello from my pam authentication!\n\n");
  // closelog();
  std::cout << "\n\nHello from PAM Email Authentication!\n\n";
  pam_returned_status = pam_get_user(handle, &username, "\nUsername: ");
  if (pam_returned_status != PAM_SUCCESS)
  {
    std::cout << "Cannot get username!" << std::endl;
    return pam_returned_status;
  }

  pam_returned_status = pam_get_authtok(handle, PAM_AUTHTOK, &password, "\nPassword: ");
  if (pam_returned_status != PAM_SUCCESS)
  {
    std::cout << "Cannot get password!" << std::endl;
    return pam_returned_status;
  }

  DbConnection db_connection;
  std::optional<User> user_optional = db_connection.retrieve_user_by_username(std::string(username));
  if (!user_optional)
  {
    std::cout << "Invalid user!" << std::endl;
    return PAM_PERM_DENIED;
  }

  User user = user_optional.value();
  pam_returned_status = db_authenticate(user, std::string(password));
  if (pam_returned_status != PAM_SUCCESS)
  {
    std::cout << "Cannot authenticate user!" << std::endl;
    return pam_returned_status;
  }

  bool email_is_sent = send_verification_code(user);
  if (!email_is_sent)
  {
    std::cout << "Could not send verification code to: " << user.get_email() << std::endl;
    return PAM_AUTH_ERR;
  }

  const char *prompt = "\nEnter verification code: ";
  char *entered_c = request_code(handle, PAM_PROMPT_ECHO_ON, prompt);
  std::string entered_code = std::string(entered_c);
  std::cout << "\nVerification code entered: " << entered_code << std::endl;

  pam_returned_status = check_verification_code(user, entered_code);
  if (pam_returned_status != PAM_SUCCESS)
  {
    std::cout << "Cannot authenticate user!" << std::endl;
    return pam_returned_status;
  }

  std::cout << "\nUser " << user.get_username() << " successfully authenticated!" << std::endl;
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc,
                                const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc,
                              const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc,
                                   const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc,
                                    const char **argv)
{
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc,
                                const char **argv)
{
  return PAM_SUCCESS;
}