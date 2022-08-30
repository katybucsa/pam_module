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
#include <thread>
#include <exception>

#include "db_authentication.cpp"
#include "keystrokes_authentication.cpp"
#include "db_connection.cpp"

//sudo g++ -std=gnu++17 -I/usr/include pam_keystrokes_authentication.cpp -shared -o /lib64/security/pam_keystrokes_authentication.so -fPIC -lpqxx -lcurl
//sudo g++ -std=gnu++17 -I/usr/include -I/usr/include/python3.10 -lpython3.10 pam_keystrokes_authentication.cpp -shared -o /lib64/security/pam_keystrokes_authentication.so -fPIC -lpqxx -lcurl

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

static char *pam_get_authtok_internal(pam_handle_t *pamh, int echocode,
                          const char *prompt)
{
  const struct pam_message msg = {.msg_style = echocode,
                                  .msg = prompt};
  const struct pam_message *msgs = &msg;
  struct pam_response *resp = NULL;
  int retval = converse(pamh, 1, &msgs, &resp);
  char *ret = NULL;
  if (retval != PAM_SUCCESS || resp == NULL || resp->resp == NULL || *resp->resp == '\000')
  {
    if (retval == PAM_SUCCESS && resp && resp->resp)
    {
      ret = resp->resp;
    }
  }
  else
  {
    ret = resp->resp;
  }
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

  std::cout << "\n\nHello from pam keystrokes authentication!\n\n";
  pam_returned_status = pam_get_user(handle, &username, "Username: ");
  std::string u = std::string(username);
  if (pam_returned_status != PAM_SUCCESS || u.empty())
  {
    // std::cout << "\nCannot get username!" << std::endl;
    return PAM_AUTH_ERR;
  }
  else{
    std::string cmd = keystrokes_listen(username);
    try{
      std::thread t(system, cmd.c_str());

      password = pam_get_authtok_internal(handle, PAM_PROMPT_ECHO_OFF,"\nPassword: ");
      if (pam_returned_status != PAM_SUCCESS)
      {
        std::cout << "\nCannot get password!" << std::endl;
        return pam_returned_status;
      }
      t.join();
    }catch(std::exception& e){
      return PAM_AUTH_ERR;
    }

    DbConnection db_connection;
    std::optional<User> user_optional = db_connection.retrieve_user_by_username(u);
    if (!user_optional)
    {
      std::cout << "\nInvalid user!" << std::endl;
      return PAM_PERM_DENIED;
    }

    User user = user_optional.value();
    pam_returned_status = db_authenticate(user, std::string(password));
    if (pam_returned_status != PAM_SUCCESS)
    {
      std::cout << "\nCannot authenticate user!\n" << std::endl;
      return pam_returned_status;
    }

    cmd = keystrokes_auth(u);
    pam_returned_status = system(cmd.c_str());
    pam_returned_status = authenticate_keystrokes(std::string(username));
    if (pam_returned_status != PAM_SUCCESS)
    {
      std::cout << "\nCannot authenticate user!  Invalid keystrokes!\n" << std::endl;
      return pam_returned_status;
    }

    std::cout << "\nUser " << u << " successfully authenticated!" << std::endl;
    return PAM_SUCCESS;
  }
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