#ifndef  USER_H
#define USER_H
#include <string>

class User
{
private:
    std::string username;
    std::string email;
    std::string hashed_password;

public:
    User(std::string username, std::string email, std::string hashed_password) : username{username}, email{email}, hashed_password{hashed_password} {}

    std::string get_username()
    {
        return this->username;
    }

    std::string get_email()
    {
        return this->email;
    }

    std::string get_password()
    {
        return this->hashed_password;
    }

    void set_username(std::string username)
    {
        this->username = username;
    }

    void set_email(std::string email)
    {
        this->email = email;
    }

    void set_password(std::string password)
    {
        this->hashed_password = password;
    }
};
#endif