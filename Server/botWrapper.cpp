#include "botWrapper.hpp"

std::array<dpp::snowflake, 3> getChannelSnowflakes(dpp::cluster& bot, const dpp::snowflake guildID) {
    const std::string LOG_INFO = "log-info";
    const std::string USER_INFO = "user-info";
    const std::string DATA = "data";
    std::unordered_map<std::string, BotWrapper::Channel> channelSnowflakes = {
        {LOG_INFO, BotWrapper::LOG_INFO},
        {USER_INFO, BotWrapper::USER_INFO},
        {DATA, BotWrapper::DATA}
    };
    std::array<dpp::snowflake, 3> snowflakes = {0, 0, 0};
    const dpp::channel_map channels = bot.channels_get_sync(guildID);
    for (const auto& [snowflake, channel] : channels) {
        auto it = channelSnowflakes.find(channel.name);
        if (it != channelSnowflakes.end()) {
            snowflakes[static_cast<size_t>(it->second)] = snowflake;
        }
    }
    auto c = dpp::channel();
    c.guild_id = guildID;
    for (auto& [name, value] : channelSnowflakes) {
        if (snowflakes[static_cast<size_t>(value)] == 0) {
            c.name = name;
            snowflakes[static_cast<size_t>(value)] = bot.channel_create_sync(c).id;
        }
    }
    return snowflakes;
}

BotWrapper::BotWrapper(const std::string& token, const dpp::snowflake guild) : 
    bot(token, dpp::i_message_content | dpp::i_default_intents), uploadMutex(), snowflakes() {
    //bot.on_log(dpp::utility::cout_logger());
    bot.on_message_create([&messInfo = this->messageInfo, &uMutex = this->uploadMutex] (const dpp::message_create_t& message) {
        std::lock_guard<std::mutex> lockGuard(uMutex);
        messInfo[message.msg.content] = message.msg.id;
    });
    bot.start();
    snowflakes = getChannelSnowflakes(bot, guild);
}

dpp::snowflake BotWrapper::channel(const Channel chn) const {
    return snowflakes[static_cast<size_t>(chn)];
}

std::string randString() {
    char alphabet[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t maxIndex = sizeof(alphabet) / sizeof(char) - 2;
    static std::random_device rd;
    static std::seed_seq ss = { rd(), rd(), rd(), rd() };
    static std::mt19937 rng(ss);
    static std::mutex randMutex;
    std::lock_guard<std::mutex> lock(randMutex);
    std::uniform_int_distribution<size_t> uid(0, maxIndex);
    std::string result(8, '\0');
    for (char& ch : result) {
        ch = alphabet[uid(rng)];
    }
    return result;
}

dpp::snowflake BotWrapper::upload(const dpp::snowflake channel, const File& file) {
    const std::string code = randString();
    dpp::message message;
    message
        .set_channel_id(channel)
        .set_content(code)
        .add_file(file.name, file.body);
    bot.message_create(message);
    // 60 * 3 second upload time is maximum allowed
    for (int i = 0; i < 3; ++i){
        auto start = std::chrono::steady_clock::now();
        auto stop  = start;
        while (std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() < 60) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            stop = std::chrono::steady_clock::now();
            std::lock_guard<std::mutex> lock(uploadMutex);
            auto it = messageInfo.find(code);
            if (it != messageInfo.end()) {
                dpp::snowflake flake = it->second;
                messageInfo.erase(it);
                return flake;
            }
        }
    }
    return 0;
}

dpp::snowflake BotWrapper::upload(const dpp::snowflake channel, const std::vector<File>& files) {
    const std::string code = randString();
    dpp::message message;
    message
        .set_channel_id(channel)
        .set_content(code);
    for (const auto& [filename, body] : files) {
        message.add_file(filename, body);
    }
    bot.message_create(message);
    auto start = std::chrono::steady_clock::now();
    auto stop  = start;
    // 60 * 2 second upload time is maximum allowed
    for (int i = 0; i < 2; ++i){
        auto start = std::chrono::steady_clock::now();
        auto stop  = start;
        while (std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() < 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            stop = std::chrono::steady_clock::now();
            std::lock_guard<std::mutex> lock(uploadMutex);
            auto it = messageInfo.find(code);
            if (it != messageInfo.end()) {
                dpp::snowflake flake = it->second;
                messageInfo.erase(it);
                return flake;
            }
        }
    }
    return 0;
}

std::vector<BotWrapper::File> BotWrapper::download(const dpp::snowflake message, const dpp::snowflake channel) {
    std::vector<File> files;
    dpp::message msg = bot.message_get_sync(message, channel);
    std::binary_semaphore sem(0);
    for (const dpp::attachment& attach : msg.attachments) {
        std::string body = "";
        bot.request(attach.url, dpp::m_get, [&sem, &body](const dpp::http_request_completion_t& result) {
            if (result.status != 200) {
                std::cerr << result.error << std::endl;
            } 
            else {
                body = result.body;
            }
            sem.release();
        });
        sem.acquire();
        files.emplace_back(attach.filename, std::move(body));
    }
    return files;
}

std::pair<dpp::snowflake, std::vector<BotWrapper::File>> BotWrapper::latest_download(const dpp::snowflake channel) {
    const dpp::message_map mm = bot.messages_get_sync(channel, 0, 0, 0, 1);
    // one or no entries, most likely one
    if (mm.empty()) {
        return {};
    }
    std::binary_semaphore sem(0);
    const auto& [sf, message] = *mm.begin();
    std::vector<File> bodies;
    bodies.reserve(message.attachments.size());
    for (const dpp::attachment& attach : message.attachments) {
        std::string body;
        bot.request(attach.url, dpp::m_get, [&sem, &body] (const dpp::http_request_completion_t& result) {
            if (result.status != 200) {
                std::cerr << result.error << std::endl;
            }
            else {
                body = result.body;
            }
            sem.release();
        });
        sem.acquire();
        bodies.emplace_back(attach.filename, std::move(body));
    }
    return {sf, std::move(bodies)};
}

bool BotWrapper::remove(const dpp::snowflake message, const dpp::snowflake channel) {
    try {
        return bot.message_delete_sync(message, channel).success;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return false;
}