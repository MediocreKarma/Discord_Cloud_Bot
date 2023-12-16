#ifndef _UTILS__
#define _UTILS__

#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <dpp/dpp.h>
#include "sha256.h"
#include <random>
#include "../Common/commons.hpp"
#include "botWrapper.hpp"

std::string passwordHash(const std::string& saltedPassword);

std::string generateSalt();

std::unordered_map<std::string, std::string> readSecretFile(const char* filename);

dpp::snowflake generateLoginFile(BotWrapper& discord);

std::pair<int, sockaddr_in> makeSocket();

std::string generateFilename(const std::string& APPEND = "");

namespace Secrets {

    static inline const std::string 
        BOT_TOKEN = "bot_token",
        EMAIL = "email",
        EMAIL_PASSWORD = "password",
        GUILD_SNOWFLAKE = "guild_snowflake";

}

namespace Files {
    static inline const std::string PATH       = "/tmp/Discord_Cloud/";
    static inline const std::string LOGIN_FILE = PATH + "login.sqlite3";
    static inline const std::string SQL_APPEND = "USER_SQL_";
}

#endif