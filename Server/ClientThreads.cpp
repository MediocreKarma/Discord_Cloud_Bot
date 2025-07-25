#include "ClientThreads.hpp"

void clientHandler(
    int client, 
    BotWrapper& discord,
    SQL_DB& loginDB, 
    std::atomic<uint8_t>& flag,
    const std::unordered_map<std::string, std::string>& secrets
) {
    signal(SIGPIPE, SIG_DFL);
    ClientMessage cmessage;
    ServerMessage smessage;
    bool loggedIn = false;
    bool updated = false;
    std::unique_ptr<Request::UserInfo> userManagerFiles = nullptr;
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
                std::tie(userManagerFiles, smessage) = Request::signIn(
                    cmessage.content.signData.email, 
                    cmessage.content.signData.pass, 
                    loginDB, 
                    discord
                );
                loggedIn = (smessage.type == ServerMessage::OK);
                std::cout << "Log in request finished" << std::endl;
                break;
            case ClientMessage::SignUpRequest:
                // check if email is already in use
                smessage = Request::verifyEmailAlreadyInUse(loginDB, cmessage.content.signData.email);
                if (smessage.type != ServerMessage::OK) {
                    break;
                }
                std::cout << "Sending email" << std::endl;
                std::tie(smessage, code) = Request::sendSignUpEmail(cmessage.content.signData.email, secrets);
                std::cout << "Email sent (if possible)" << std::endl;
                break;
            case ClientMessage::SignUpCode:
                if (code != cmessage.content.signData.signCode) {
                    std::cout << "Invalid code" << std::endl;
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongCode;
                } 
                else {
                    std::cout << "Good code, finalizing signup" << std::endl;
                    smessage = Request::finalizeSignup(
                        cmessage.content.signData.email, 
                        cmessage.content.signData.pass,
                        discord,
                        loginDB
                    );
                    // request main thread to update the login file on discord
                    flag |= ClientThreads::REQUEST_SAVE_FLAG;
                }
                break;
            case ClientMessage::RequestFileTree:
                if (!loggedIn) {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongLogin;
                    goto endHandler;
                }
                if (Request::sendTreeFile(client, userManagerFiles->tree)) {
                    std::cout << "File Tree Sent" << std::endl;
                }
                else {
                    std::cout << "Failed to send file tree" << std::endl;
                }
                break;
            case ClientMessage::FileUpload: {
                if (!loggedIn) {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongLogin;
                    goto endHandler;
                }
                std::cout << "File upload requested" << std::endl;
                bool newUpdate = FileTransfer::receiveFile(
                    client,
                    discord,
                    *userManagerFiles,
                    cmessage.content.file.size,
                    std::string(cmessage.content.file.alias)
                );
                if (newUpdate) {
                    updated = true;
                    smessage.type = ServerMessage::OK;
                }
                else {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::InternalError;
                }
                break;
            }
            case ClientMessage::FileDownload:
                if (!loggedIn) {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongLogin;
                    goto endHandler;
                }
                std::cout << "Download requested" << std::endl;
                if (FileTransfer::sendFile(
                    client,
                    discord,
                    *userManagerFiles,
                    cmessage.content.file.id
                )) {
                    smessage.type = ServerMessage::OK;
                }
                else {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::InternalError;
                }
                break;
            case ClientMessage::UpdateFileTree:
                if (!loggedIn) {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongLogin;
                    goto endHandler;
                }
                std::cout << "Tree file update requested" << std::endl;
                if (FileTransfer::updateFileTree(client, *userManagerFiles, cmessage.content.file.size)) {
                    smessage.type = ServerMessage::OK;
                    updated = true;
                } 
                else {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::InternalError;
                }
                break;
            case ClientMessage::FileDelete:
                if (!loggedIn) {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::WrongLogin;
                    goto endHandler;
                }
                std::cout << "Delete requested" << std::endl;
                if (Request::deleteFile(client, cmessage.content.file.id, userManagerFiles->db, discord)) {
                    smessage.type = ServerMessage::OK;
                    updated = true;
                }
                else {
                    smessage.type = ServerMessage::Error;
                    smessage.error = ServerMessage::InternalError;
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
    if (loggedIn) {
        Request::loggedMutex.lock();
        Request::loggedEmails.erase(userManagerFiles->email);
        Request::loggedMutex.unlock();
    }
    flag |= ClientThreads::TERMINATE_FLAG;
    if (updated) {
        Request::updateDiscord(*userManagerFiles, loginDB, discord);
        flag |= ClientThreads::REQUEST_SAVE_FLAG;
    }
    std::cout << "Thread done" << std::endl;
    smessage = {ServerMessage::ServerQuit, ServerMessage::NoError};
    if (Communication::write(client, &smessage, sizeof(smessage)) == false) {
        perror("error sending quit to client");
    }
    close(client);
}

void ClientThreads::add(
    int client, 
    BotWrapper& discord,
    SQL_DB& db,
    const std::unordered_map<std::string, std::string>& secrets
) {
    flags.emplace_back(0);
    clientHandlers.emplace_back(
        clientHandler, 
        client, 
        std::ref(discord), 
        std::ref(db), 
        std::ref(flags.back()), 
        std::cref(secrets)
    );
}

void ClientThreads::terminate() {
    for (std::atomic<uint8_t>& flag : flags) {
        flag |= TERMINATE_FLAG;
    }
    std::cout << std::endl;
}

void ClientThreads::trim() {
    auto hndlrIt = clientHandlers.begin();
    auto flagsIt = flags.begin();
    while (hndlrIt != clientHandlers.end()) {
        auto hndlrItCopy = hndlrIt++;
        auto flagsItCopy = flagsIt++;
        if (flagsItCopy->load(std::memory_order_relaxed) & TERMINATE_FLAG) {
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