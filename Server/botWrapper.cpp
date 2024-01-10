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
    bot.on_log(dpp::utility::cout_logger());
    bot.on_message_create([&messInfo = this->messageInfo, &uMutex = this->uploadMutex] (const dpp::message_create_t& message) {
        std::lock_guard<std::mutex> lockGuard(uMutex);
        messInfo[message.msg.attachments[0].filename] = message.msg.id;
    });
    bot.start();
    snowflakes = getChannelSnowflakes(bot, guild);
}

dpp::snowflake BotWrapper::channel(const Channel chn) const {
    return snowflakes[static_cast<size_t>(chn)];
}

dpp::snowflake BotWrapper::upload(const dpp::snowflake channel, const File& file) {
    dpp::message message;
    message
        .set_channel_id(channel)
        .add_file(file.name, file.body);
    bot.message_create(message);
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::lock_guard<std::mutex> lock(uploadMutex);
        if (messageInfo.contains(file.name)) {
            dpp::snowflake flake = messageInfo.at(file.name);
            messageInfo.erase(file.name);
            return flake;
        }
    }
}

dpp::snowflake BotWrapper::upload(const dpp::snowflake channel, const std::vector<File>& files) {
    dpp::message message;
    message.set_channel_id(channel);
    for (const auto& [filename, body] : files) {
        message.add_file(filename, body);
    }
    bot.message_create(message);
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::lock_guard<std::mutex> lock(uploadMutex);
        if (messageInfo.contains(files[0].name)) {
            dpp::snowflake flake = messageInfo.at(files[0].name);
            messageInfo.erase(files[0].name);
            return flake;
        }
    }
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