#include "ClientThreads.hpp"

void clientHandler(
    int client, 
    dpp::cluster& discord,
    SQL_DB& loginDB, 
    std::atomic<uint8_t>& flag,
    const std::unordered_map<std::string, dpp::snowflake>& channelSnowflakes,
    const std::unordered_map<std::string, std::string>& secrets
) {
    signal(SIGPIPE, SIG_DFL);
    ClientMessage cmessage;
    ServerMessage smessage;
    bool loggedIn = false;
    dpp::snowflake dbManagerFileSnowflake = 0;
    std::string code;
    std::string username, password;
    pollfd clientPoll = {client, POLLIN | POLLNVAL, 0};
    std::cout << "Looping the client" << std::endl;
    while (true) {
        if (flag.load(std::memory_order_relaxed) & ClientThreads::TERMINATE_FLAG) {
            std::cout << "Received termination flag" << std::endl;
            goto endHandler;
        }
        clientPoll.revents = 0;
        if (poll(&clientPoll, 1, 1 * 1000) == -1) {
            perror("error using poll");
            continue;
        }
        if (clientPoll.revents & POLLNVAL) {
            std::cerr << "Error, invalid client?" << std::endl;
            exit(0);
        }
        if (!(clientPoll.revents & POLLIN)) {
            continue;
        }
        if (Communication::read(client, &cmessage, sizeof(cmessage)) == false) {
            perror("error using read from client handler");
            goto endHandler;
        }
        std::cout << "Read client's message" << std::endl;
        // goto skip read if the current cmessage can still be analysed
        SkipRead:
        switch (cmessage.type) {
            case ClientMessage::Empty:
                break;
            case ClientMessage::SignInRequest:
                // verify Sign In from login database
                std::tie(dbManagerFileSnowflake, smessage) = Request::signIn(cmessage.content.signData.email, cmessage.content.signData.pass, loginDB);
                loggedIn = (smessage.type == ServerMessage::OK);
                break;
            case ClientMessage::SignUpRequest:
                std::cout << "Receiving sign up request" << std::endl;
                // check if email is already in use
                smessage = Request::verifyEmailAlreadyInUse(loginDB, cmessage.content.signData.email);
                if (smessage.type != ServerMessage::OK) {
                    break;
                }
                std::cout << "Sending email" << std::endl;
                std::tie(smessage, code) = Request::sendSignUpEmail(cmessage.content.signData.email, secrets);
                std::cout << "Email sent" << std::endl;
                break;
            case ClientMessage::SignUpCode:
                std::cout << "Receiving client code" << std::endl;
                if (code != cmessage.content.signData.signCode) {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongCode;
                } 
                else {
                    smessage = Request::finalizeSignup(
                        cmessage.content.signData.email, 
                        cmessage.content.signData.pass,
                        discord,
                        loginDB,
                        channelSnowflakes.at(Channels::USER_INFO)
                    );
                    // request main thread to update the login file on discord
                    flag |= ClientThreads::REQUEST_SAVE_FLAG;
                }
                break;
        }
        cmessage.type = ClientMessage::Empty;
        std::cout << "Writing to client: " << smessage.type << std::endl; 
        if (Communication::write(client, &smessage, sizeof(smessage)) == false) {
            perror("error writing to client");
            goto endHandler;
        }
    }

endHandler:
    flag |= ClientThreads::TERMINATE_FLAG;
    std::cout << "Thread done" << std::endl;
    smessage = {ServerMessage::ServerQuit, ServerMessage::NoError};
    if (Communication::write(client, &smessage, sizeof(smessage)) == false) {
        perror("error sending quit to client");
    }
    close(client);
}

void ClientThreads::add(
    int client, 
    dpp::cluster& discord,
    SQL_DB& db,
    const std::unordered_map<std::string, dpp::snowflake>& channelSnowflakes,
    const std::unordered_map<std::string, std::string>& secrets
) {
    flags.emplace_back(0);
    clientHandlers.emplace_back(
        clientHandler, 
        client, 
        std::ref(discord), 
        std::ref(db), 
        std::ref(flags.back()), 
        std::cref(channelSnowflakes), 
        std::cref(secrets)
    );
}

void ClientThreads::terminate() {
    for (std::atomic<uint8_t>& flag : flags) {
        flag |= TERMINATE_FLAG;
        std::cout << (int)flag.load(std::memory_order_relaxed) << ' ';
    }
    std::cout << std::endl;
}

void ClientThreads::trim() {
    auto hndlrIt = clientHandlers.begin();
    auto flagsIt = flags.begin();
    while (hndlrIt != clientHandlers.end()) {
        auto hndlrItCopy = hndlrIt++;
        auto flagsItCopy = flagsIt++;
        if (flagsItCopy->load(std::memory_order_relaxed)) {
            hndlrItCopy->join();
            clientHandlers.erase(hndlrItCopy);
            flags.erase(flagsItCopy);
        }
    }
}

bool ClientThreads::saveRequest() {
    bool requested = false;
    for (std::atomic<uint8_t>& flag : flags) {
        requested |= static_cast<bool>(flag & REQUEST_SAVE_FLAG);
        flag &= (~REQUEST_SAVE_FLAG);
    }
    return requested;
}

bool ClientThreads::empty() const {
    return clientHandlers.empty();
}