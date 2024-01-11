#pragma once

#include <atomic>
#include <list>
#include <thread>
#include "requestHandler.hpp"
#include <dpp/dpp.h>
#include "SQL_DB.hpp"
#include "../Common/commons.hpp"
#include <poll.h>
#include "fileTransfer.hpp"

class ClientThreads {
public:

    static constexpr uint8_t TERMINATE_FLAG = 0b00000001;
    static constexpr uint8_t REQUEST_SAVE_FLAG = 0b00000010;

    ClientThreads() = default;
    ClientThreads(const ClientThreads&) = delete;
    void add(
        int client, 
        BotWrapper& discord,
        SQL_DB& loginDB,
        const std::unordered_map<std::string, std::string>& secrets
    );
    void terminate();
    void trim();
    // get save requests, and clear save request flags
    bool saveRequest();
    bool empty() const;

private:
    std::list<std::atomic<uint8_t>> flags;
    std::list<std::thread> clientHandlers;
};
