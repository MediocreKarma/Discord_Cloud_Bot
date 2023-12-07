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

std::pair<dpp::snowflake, ServerMessage> signIn(const char email[], const char pass[], SQL_DB& loginDB);
std::pair<ServerMessage, std::string> sendSignUpEmail(const char email[], const std::unordered_map<std::string, std::string>& secrets);
ServerMessage verifyEmailAlreadyInUse(SQL_DB& loginDB, const char email[]);
ServerMessage finalizeSignup(
    const char* email, 
    const char* password,
    dpp::cluster& discord, 
    SQL_DB& loginDB, 
    dpp::snowflake clientInfoChannelSnowflake
);

}