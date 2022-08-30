#include <string>
#include <regex>

std::string DB_HOST = "127.0.0.1";
std::string DB_PORT = "5432";
std::string DB_NAME = "auth_db";
std::string DB_USERNAME = "postgres";
std::string DB_PASSWORD = "postgres";
std::string KEYSTROKES_LISTENER = "/lib64/security/keystrokes_models/keystrokes_listener.py ";
std::string KEYSTROKES_AUTH = "/lib64/security/keystrokes_models/keystrokes_authentication.py ";
std::string PRED_PATH = "/lib64/security/keystrokes_models/{username}/pred_val.txt";
int EXPIRATION_TIME = 10;



std::string keystrokes_listen(std::string username){

    return "python3 " + KEYSTROKES_LISTENER + username;
}

std::string keystrokes_auth(std::string username){

    return "python3 " + KEYSTROKES_AUTH + username + " 2> /dev/null";
}

std::string pred_path(std::string username){

    return std::regex_replace(PRED_PATH, std::regex("\\{username\\}"), username);
}

// std::string DB_CONFIG_FILE = "db_config.txt";

// std::map<std::string, std::string> read_db_properties(){

//     // std::ifstream is RAII, i.e. no need to call close
//     std::ifstream cFile ("../config/" + DB_CONFIG_FILE);
//     std::map<std::string, std::string> props;
//     if (cFile.is_open())
//     {
//         std::string line;
//         while(std::getline(cFile, line)){
//             line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
//             if(line[0] == '#' || line.empty())
//                 continue;
//             auto delimiterPos = line.find("=");
//             auto name = line.substr(3, delimiterPos - 3);
//             auto value = line.substr(delimiterPos + 1);
//             props[name] =  value;
//             // std::cout << name << " " << props[name] << '\n';
//         }

//     }
//     return props;
// }