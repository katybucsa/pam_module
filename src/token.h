#include <string>

class Token
{
private:
    int id;
    std::string user_id;
    std::string token;
    time_t created_at;
    bool used;

public:
    Token(int id, std::string user_id, std::string token, time_t created_at, bool used) : id{id}, user_id{user_id}, token{token}, created_at{created_at}, used{used} {}

    int get_id()
    {
        return this->id;
    }

    std::string get_user_id()
    {
        return this->user_id;
    }

    std::string get_token()
    {
        return this->token;
    }

    time_t get_created_at()
    {
        return this->created_at;
    }

    bool get_used()
    {
        return this->used;
    }
};
