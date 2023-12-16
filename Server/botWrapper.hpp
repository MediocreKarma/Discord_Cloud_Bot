#ifndef _BOT_WRAPPER__
#define _BOT_WRAPPER__

#include <dpp/dpp.h>

class BotWrapper {
public:
    BotWrapper() = delete;
    BotWrapper(const std::string& token, dpp::snowflake guild);

    enum Channel {
        LOG_INFO,
        USER_INFO,
        DATA
    };

    struct File {
        std::string name;
        std::string body;
    };

    dpp::snowflake channel(Channel channel);
    // total size of message must be less than 25 MiB
    dpp::snowflake upload(dpp::snowflake channel, const File& file);
    dpp::snowflake upload(dpp::snowflake channel, const std::vector<File>& files);
    
    std::vector<File> download(dpp::snowflake message, dpp::snowflake channel);
    std::pair<dpp::snowflake, std::vector<File>> latest_download(dpp::snowflake channel);

    bool remove(dpp::snowflake message, dpp::snowflake channel);

private:
    dpp::cluster bot;
    std::mutex uploadLock;
    std::array<dpp::snowflake, 3> snowflakes;
    std::atomic<uint64_t> latestSnowflake;
};

#endif
