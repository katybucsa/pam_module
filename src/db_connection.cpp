#include <string>
#include <pqxx/pqxx>
#include <optional>
#include <functional>
#include <cstdlib>
#include <iostream>

#include "token.h"
#include "user.h"

class DbConnection
{
private:
    std::string s = "dbname = " + DB_NAME + " user = " + DB_USERNAME + " password = " + DB_PASSWORD + " hostaddr = " + DB_HOST + " port = " + DB_PORT;
public:
    DbConnection(){
    }

    std::optional<User> retrieve_user_by_username(std::string username)
    {
        pqxx::connection connection(s);
        pqxx::work w(connection);
        pqxx::result r{w.exec("SELECT * from users where username ='" + username + "'")};
        if (r.size())
        {
            User u{r[0]["username"].c_str(), r[0]["email"].c_str(), r[0]["password"].c_str()};
            connection.close();
            return u;
        }
        connection.close();
        return std::nullopt;
    }

    void insert_token(std::string user_id, std::string token)
    {
        pqxx::connection connection(s);
        pqxx::work w(connection);

        w.exec("INSERT INTO tokens (user_id, token_s, created_at) VALUES ('" + user_id + "', '" + token + "', '" + std::to_string(std::time(nullptr)) + "')");
        w.commit();

        connection.close();
    }

    std::optional<Token> retrieve_token_by_username_token_content(std::string username, std::string token_str)
    {
        pqxx::connection connection(s);
        pqxx::work w(connection);
        pqxx::result r{w.exec("SELECT * from tokens where token_s ='" + token_str + "' and user_id ='" + username + "' and used ='false'")};
        if (r.size())
        {
            char *end;
            int id = atoi(r[0]["id"].c_str());
            time_t created_at = strtoll(r[0]["created_at"].c_str(), &end, 10);
            Token t{id, r[0]["user_id"].c_str(), r[0]["token_s"].c_str(), created_at, r[0]["used"].c_str() == "true" ? true : false};
            connection.close();
            return t;
        }
        connection.close();
        return std::nullopt;
    }

    void set_token_as_used(int id)
    {
        pqxx::connection connection(s);
        pqxx::work w(connection);
        w.exec("UPDATE tokens SET used ='true' where id ='" + std::to_string(id) + "'");
        w.commit();
        connection.close();
    }
};