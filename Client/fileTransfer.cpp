#include "fileTransfer.hpp"

void FileTransfer::receiveFile(int sd, const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::invalid_argument("Bad filepath!");
    }
    ClientMessage cmsg = {ClientMessage::FileDownload};
    ServerMessage smsg;
    if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false) {
        perror("error writing to server");
        throw std::ios::failure("Write failure to socket");
    }
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        perror("error reading response from server");
        throw std::ios::failure("Read from socket failed");
    }
    if (smsg.type != ServerMessage::OK) {
        throw std::ios::failure("Server refused file transfer");
    }
    cmsg.type = ClientMessage::OK;
    size_t total = smsg.content.size;
    size_t current = 0;
    std::string chunk(MAX_FILE_TRANSFER);
    while (current < total) {
        size_t readLength = std::min(MAX_FILE_TRANSFER, total - current);
        if (Communication::read(sd, chunk.data(), readLength) == false) {
            perror("error writing to server");
            throw std::ios::failure("Write failure to socket");
        }
        if (!file.write(chunk.c_str(), readLength)) {
            std::remove(file);
            throw std::ios::failure("Writing to file failed");
        }
        current += readLength;
        if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false) {
            perror("error reading response from server");
            throw std::ios::failure("Read from socket failed");
        }
    }
    
}

void FileTransfer::sendFile(int sd, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::invalid_argument("Bad filepath!");
    }
    std::size_t fileSize = std::filesystem::file_size(filepath);
    ClientMessage cmsg = {ClientMessage::FileUpload, fileSize};
    ServerMessage smsg;
    if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false) {
        perror("error writing to server");
        throw std::ios::failure("Write failure to socket");
    }
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        perror("error reading response from server");
        throw std::ios::failure("Read from socket failed");
    }
    if (smsg.type != ServerMessage::OK) {
        throw std::ios::failure("Server refused file transfer");
    }
    std::string chunk(MAX_FILE_TRANSFER);
    while (file.read(chunk.data(), MAX_FILE_TRANSFER)) {
        if (Communication::write(sd, chunk.c_str(), file.gcount()) == false) {
            perror("error writing to server");
            throw std::ios::failure("Write failure to socket");
        }
        if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
            perror("error reading response from server");
            throw std::ios::failure("Read from socket failed");
        }
        // server refuses write/read (possibly closing)
        if (smsg.type != ServerMessage::OK) {
            throw std::ios::failure("Server canceled file request");
        }
    }
}

