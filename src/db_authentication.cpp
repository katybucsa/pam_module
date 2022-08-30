#include <iostream>
#include <algorithm>
#include <string_view>
#include <ctime>
#include <security/pam_modules.h>

#include "sha256.cpp"
#include "user.h"

int db_authenticate(User user, std::string password)
{

    SHA256 sha256 = SHA256();
    sha256.add(&password[0], password.size());
    std::string hashed_password = sha256.getHash();

    if (password.empty() || user.get_password() != hashed_password)
        return PAM_AUTH_ERR;
    return PAM_SUCCESS;
}