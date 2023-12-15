#include "utils.hpp"
#include <iostream>


std::string passwordHash(const std::string& saltedPassword) {
    // verify if this number is reasonable or not?
    constexpr size_t ITERATIONS = 1'000'000;
    SHA256 sha256;
    std::string result = saltedPassword;
    for (size_t i = 0; i < ITERATIONS; ++i) {
        result = sha256(saltedPassword);
    }
    return result;
}

std::string generateSalt() {
    static constexpr size_t SALT_SIZE = 32;
    static std::random_device rd;
	static std::seed_seq sq = { rd(), rd(), rd(), rd() };
	static std::mt19937 rng(sq); 
    static const char alphabet[] = 
        "0123456789"
        "!@#$%^&*"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"; 
    std::uniform_int_distribution<size_t> uid(0, sizeof(alphabet) / sizeof(char) - 2);
    std::string salt(SALT_SIZE, '\0');
    for (char& c : salt) {
        c = alphabet[uid(rng)];
    }
    return salt;
}

std::unordered_map<std::string, std::string> readSecretFile(const char* filename) {
    const std::unordered_set<std::string> configs = {
        Secrets::EMAIL, Secrets::EMAIL_PASSWORD, 
        Secrets::BOT_TOKEN, Secrets::GUILD_SNOWFLAKE
    };
    std::unordered_map<std::string, std::string> secrets;
    secrets.reserve(4);
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        throw std::invalid_argument("Could not open secret file");
    }
    std::string line;
    while (std::getline(fin, line)) {
        size_t equalOperatorIndex = line.find_first_of('=');
        if (equalOperatorIndex == std::string::npos) {
            break;
        }
        std::string key, value;
        size_t i = 0, j = equalOperatorIndex - 1;
        while (isspace(line[i])) {
            ++i;
        }
        if (i == equalOperatorIndex) {
            break;
        }
        while (j != -1 && isspace(line[j])) {
            --j;
        }
        if (j == -1) {
            break;
        }
        key = line.substr(i, j - i + 1);
        if (!configs.contains(key)) {
            break;
        }
        i = equalOperatorIndex + 1, j = line.size() - 1;
        while (i != line.size() && isspace(line[i])) {
            ++i;
        }
        if (i == line.size()) {
            break;
        }
        while (isspace(line[j])) {
            j--;
        }
        if (j < i) {
            break;
        }
        value = line.substr(i, j - i + 1);
        secrets.emplace(key, value);
    }
    if (secrets.size() != configs.size()) {
        throw std::invalid_argument("Invalid secrets file!");
    }
    return secrets;
}

std::unordered_map<std::string, dpp::snowflake> getChannelSnowflakes(dpp::cluster& discord, const dpp::snowflake guildID) {
    std::unordered_map<std::string, dpp::snowflake> channelSnowflakes = {
        {Channels::LOG_INFO, 0}, 
        {Channels::USER_INFO, 0}, 
        {Channels::DATA, 0}
    };
    const dpp::channel_map channels = discord.channels_get_sync(guildID);
    for (const auto& [snowflake, channel] : channels) {
        auto it = channelSnowflakes.find(channel.name);
        if (it != channelSnowflakes.end()) {
            it->second = snowflake;
        }
    }
    auto c = dpp::channel();
    c.guild_id = guildID;
    for (auto& [name, snowflake] : channelSnowflakes) {
        if (snowflake == 0) {
            c.name = name;
            snowflake = discord.channel_create_sync(c).id;
        }
    }
    return channelSnowflakes;
}

dpp::snowflake generateLoginFile(dpp::cluster& discord, dpp::snowflake loginChannel) {
    const dpp::message_map mm = discord.messages_get_sync(loginChannel, 0, 0, 0, 1);
    std::string dbFile;
    // one or no entries here, probably one
    for (const auto& [snowflake, message] : mm) {
        for (const auto& attach : message.attachments) {
            std::atomic<bool> flag = false;
            discord.request(attach.url, dpp::m_get, [&dbFile, &flag] (const dpp::http_request_completion_t& result) {
                if (result.status != 200) {
                    std::cerr << "Login file could not be read: " << result.error << std::endl;
                }
                else {
                    dbFile = result.body;
                }
                flag = true;
            });
            while (!flag) std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    std::error_code ec;
    std::filesystem::remove_all(Files::PATH);
    if (std::filesystem::create_directories(Files::PATH, ec) == false && ec) {
        std::cerr << ec.message() << std::endl;
    }
    std::ofstream sqldbFileStream(Files::LOGIN_FILE, std::ios::trunc | std::ios::binary);
    if (sqldbFileStream.is_open() == false) {
        throw std::ios_base::failure("Could not prepare login file");
    } 
    sqldbFileStream.write(dbFile.c_str(), dbFile.size());
    if (mm.empty()) {
        return 0;
    }
    return mm.begin()->first;
}

std::pair<int, sockaddr_in> makeSocket() {
    uint16_t port = 8237;
    sockaddr_in server = {};
    sockaddr_in from = {};
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("Error initializing server socket");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    // try port until we find one
    while (bind(sd, (sockaddr*) &server, sizeof(sockaddr)) == -1) {
        if (errno != EADDRINUSE) {
            perror("Error using bind from the server");
            exit(1);
        }
        port++;
        server.sin_port = htons(port);
    }
    std::ofstream portFile(Files::PortFile);
    if (!portFile.is_open()) {
        std::cerr << "Bad port file call" << std::endl;
        exit(1);
    }
    portFile << port;
    portFile.close();
    if (listen(sd, 50) == -1) {
        perror("error using listen");
        exit(1);
    }
    return {sd, from};
}

std::string generateFilename(const std::string& APPEND) { 
    static std::random_device rd;
	static std::mt19937 rng(rd()); 
    static const char alphabet[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"; 
    std::uniform_int_distribution<size_t> uid(0, sizeof(alphabet) / sizeof(char) - 2);
    std::string str(16, '\0');
    for (char& c : str) {
        c = alphabet[uid(rng)];
    }
    const std::string PREFIX = Files::PATH + APPEND;
    while (std::filesystem::exists(PREFIX + str)) {
        // reroll filename
        for (char& c : str) {
            c = alphabet[uid(rng)];
        }
    }
    return PREFIX + str;
}