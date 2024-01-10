#ifndef _REQUEST_HANDLER__
#define _REQUEST_HANDLER__

#include <random>
#include "../Common/commons.hpp"
#include "SQL_DB.hpp"
#include "smtpclientlibrary-src/src/cpp/opportunisticsecuresmtpclient.hpp"
#include "smtpclientlibrary-src/src/cpp/plaintextmessage.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <functional>
#include "utils.hpp"

namespace Request {

struct UserInfo {
    dpp::snowflake managerFile;
    SQL_DB db;
    std::string tree;
};

std::pair<std::unique_ptr<Request::UserInfo>, ServerMessage> signIn(
    const char email[], 
    const char pass[], 
    SQL_DB& loginDB, 
    BotWrapper& discord
);
std::pair<ServerMessage, std::string> sendSignUpEmail(const char email[], const std::unordered_map<std::string, std::string>& secrets);
ServerMessage verifyEmailAlreadyInUse(SQL_DB& loginDB, const char email[]);
ServerMessage finalizeSignup(
    const char* email, 
    const char* password,
    BotWrapper& discord, 
    SQL_DB& loginDB
);

bool sendTreeFile(int client, const std::string& encoding);

void updateDiscord(UserInfo& info, SQL_DB& loginDB, BotWrapper& discord);

void deleteFile(int client, const std::string& id, SQL_DB& loginDB, BotWrapper& discord);

}

#endif