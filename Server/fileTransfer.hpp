#ifndef _FILE_TRANSFER__
#define _FILE_TRANSFER__

#include "requestHandler.hpp"
#include <poll.h>

namespace FileTransfer {

// received alias already
// afterwards send file id
bool receiveFile(
    int client, 
    BotWrapper& discord, 
    Request::UserInfo& info, 
    size_t size, 
    const std::string& alias
);


bool sendFile(int sd, BotWrapper& discord, Request::UserInfo& info, const std::string& fileID);

}

#endif