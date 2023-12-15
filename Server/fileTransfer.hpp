#ifndef _FILE_TRANSFER__
#define _FILE_TRANSFER__

#include "requestHandler.hpp"
#include <poll.h>

namespace FileTransfer {

// received alias already
// afterwards send file id
bool receiveFile(
    int client, 
    dpp::cluster& discord, 
    const dpp::snowflake filesSnowflake, 
    Request::UserInfo& info, 
    size_t size, 
    const std::string& alias
);


//void    sendFile(int sd, dpp::cluster& discord, SQL_DB& db, const std::string& filename);

}

#endif