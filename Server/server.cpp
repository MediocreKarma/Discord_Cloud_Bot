#include <fstream>
#include <iostream>
#include <dpp/dpp.h>
#include "SQL_DB.hpp"
#include "utils.hpp"
#include <poll.h>
#include "ClientThreads.hpp"

int main(int argc, char* argv[]) {
    sigset_t mask;
    sigfillset(&mask);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        perror("error using sigprocmask");
        exit(EXIT_FAILURE);
    }
    auto [sd, from] = makeSocket();
    const char * secretsFile = "../Resources/secrets.config";
    if (argc > 1) {
        secretsFile = argv[1];
    }
    std::unordered_map<std::string, std::string> secrets;
    try {
        secrets = readSecretFile(secretsFile);
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    dpp::cluster discord(secrets[Secrets::BOT_TOKEN]);
    discord.on_log(dpp::utility::cout_logger());
    std::unordered_map<std::string, dpp::snowflake> channelSnowflakes;
    dpp::snowflake loginFileMessage = 0;
    try {
        channelSnowflakes = getChannelSnowflakes(discord, std::stoull(secrets[Secrets::GUILD_SNOWFLAKE]));
        loginFileMessage  = generateLoginFile(discord, channelSnowflakes[Channels::LOG_INFO]);
    }
    catch (std::exception& e) {
        std::cerr << e.what();
        exit(EXIT_FAILURE);
    }
    std::cout << "Obtained discord's data" << std::endl;
    ClientThreads clientData;
    SQL_DB loginDB(Files::LOGIN_FILE);
    if (!loginDB.isOpen()) {
        std::cerr << "Bad login db" << std::endl;
        exit(EXIT_FAILURE);
    }
    loginDB.lock();
    if (!loginDB.createStatement(
        "CREATE TABLE IF NOT EXISTS login ("
            "email TEXT PRIMARY KEY,"
            "password TEXT NOT NULL,"
            "salt TEXT NOT NULL,"
            "files_id TEXT NOT NULL"
        ");"
    )) {
        std::cerr << "Statement of table creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // no reason this to fail
    loginDB.nextRow();
    loginDB.unlock();
    while (true) {
        // cleanup clients
        socklen_t length = sizeof(from);
        constexpr size_t SD_INDEX = 0;
        constexpr size_t STDIN_INDEX = 1;
        std::array<pollfd, 2> polledFDs = {
            pollfd{sd, POLLIN, 0}, 
            pollfd{STDIN_FILENO, POLLIN, 0}
        };
        std::cout << "Polling for clients" << std::endl;
        int pollResult = poll(polledFDs.data(), polledFDs.size(), 60 * 1000);
        if (pollResult == -1) {
            perror("Error using poll");
            continue;
        }
        if (pollResult == 0) {
            std::cout << "No poll found" << std::endl;
            continue;
        }
        std::cout << "Poll got: " << pollResult << std::endl;
        if (polledFDs[STDIN_INDEX].revents & POLLIN) {
            std::cout << "Poll found a stdin input: ";
            std::string input;
            std::getline(std::cin, input);
            std::cout << input << std::endl;
            std::transform(input.begin(), input.end(), input.begin(), [](char c) {
                return std::tolower(c);
            });
            // begin ending sequence
            if (input == "quit") {
                break;
            }
            else if (input == "kill") {
                exit(0);
            }
            else {
                std::cerr << "Unrecoginzed command : Only 'quit' is defined" << std::endl;
            }
        }
        if (!(polledFDs[SD_INDEX].revents & POLLIN)) {
            std::cout << "Poll did not find a client connection" << std::endl;
            continue;
        }
        std::cout << "Found a client" << std::endl;
        int client = accept(sd, (sockaddr*) &from, &length);
        std::cout << "Accepted the client" << std::endl;
        if (client == 0) {
            continue;
        }
        if (client < 0) {
            perror("Error accepting client");
            continue;
        }
        std::cout << "Starting thread" << std::endl;
        clientData.add(client, discord, loginDB, channelSnowflakes, secrets);
    }
    clientData.terminate();
    clientData.trim();
    close(sd);
    dpp::message login_file;
    if (loginFileMessage != 0) {
        discord.message_delete(loginFileMessage, channelSnowflakes[Channels::LOG_INFO]);
    }
    login_file.set_channel_id(channelSnowflakes[Channels::LOG_INFO]);
    login_file.add_file(
        Files::LOGIN_FILE.substr(Files::LOGIN_FILE.find_last_of('/')),
        dpp::utility::read_file(Files::LOGIN_FILE)
    );
    discord.message_create_sync(login_file);
}
                  