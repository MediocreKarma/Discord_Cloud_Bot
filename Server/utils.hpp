#ifndef _UTILS__
#define _UTILS__

#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <dpp/dpp.h>
#include "sha256.h"
#include <random>
#include "../Common/commons.hpp"

std::string passwordHash(const std::string& saltedPassword);

std::string generateSalt();

std::unordered_map<std::string, std::string> readSecretFile(const char* filename);

std::unordered_map<std::string, dpp::snowflake> getChannelSnowflakes(dpp::cluster& discord, dpp::snowflake guildID);

dpp::snowflake generateLoginFile(dpp::cluster& discord, dpp::snowflake loginChannel);

std::pair<int, sockaddr_in> makeSocket();

namespace Secrets {

    static inline const std::string 
        BOT_TOKEN = "bot_token",
        EMAIL = "email",
        EMAIL_PASSWORD = "password",
        GUILD_SNOWFLAKE = "guild_snowflake";

}

namespace Channels {

    static inline const std::string
        LOG_INFO = "log-info",
        USER_INFO = "user-info",
        DATA = "data";

}

namespace Files {
    static inline const std::string LOGIN_FILE = "/tmp/login.sqlite3";
}

#endif