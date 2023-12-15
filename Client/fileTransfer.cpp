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
    std::string chunk(MAX_FILE_TRANSFER, '\0');
    while (current < total) {
        size_t readLength = std::min(MAX_FILE_TRANSFER, total - current);
        if (Communication::read(sd, chunk.data(), readLength) == false) {
            perror("error writing to server");
            throw std::ios::failure("Write failure to socket");
        }
        if (!file.write(chunk.c_str(), readLength)) {
            std::remove(filepath.c_str());
            throw std::ios::failure("Writing to file failed");
        }
        current += readLength;
        if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false) {
            perror("error reading response from server");
            throw std::ios::failure("Read from socket failed");
        }
    }
    
}

void FileTransfer::sendFile(int sd, const std::string& filepath, const DirectoryTree& root, DirectoryTree& current) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::invalid_argument("Bad filepath!");
    }
    std::size_t fileSize = std::filesystem::file_size(filepath);
    ClientMessage cmsg;
    cmsg.type = ClientMessage::FileUpload;
    cmsg.content.file.size = fileSize;
    std::string alias = filepath.substr(filepath.find_last_of('/') + 1);
    if (alias.empty()) {
        throw std::ios::failure("Incorrect file");
    }
    std::cout << "\'" + alias + "\' " << fileSize << std::endl;
    strncpy(cmsg.content.file.alias, alias.c_str(), 255);
    cmsg.content.file.alias[255] = '\0';
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
    std::string chunk(MAX_FILE_TRANSFER, '\0');
    bool reading = true;
    while (reading) {
        reading = static_cast<bool>(file.read(chunk.data(), MAX_FILE_TRANSFER));
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
    std::cout << "Finished upload" << std::endl;
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        perror("error reading request");
        throw std::ios::failure("Read from socket failed");
    }
    if (smsg.type != ServerMessage::SendID) {
        std::cerr << "Invalid message type: " << (int)smsg.type << std::endl;
        throw std::ios::failure("Server went haywire");
    }
    std::string id = smsg.content.file_id;
    current.addChild(id, alias);
    chunk = root.encodeTree();
    cmsg.type = ClientMessage::UpdateFileTree;
    cmsg.content.file.size = chunk.size();
    if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false ||
        Communication::write(sd, chunk.c_str(), chunk.size()) == false) {
        perror("error writing to server");
        throw std::ios::failure("Write failure to socket");
    }
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        perror("error reading ok from server");
        throw std::ios::failure("Failed right at the end of the upload");
    }
    if (smsg.type != ServerMessage::OK) {
        // what the hell happened here???
        std::cerr << "Server refused to finish operation" << std::endl;
        throw std::ios::failure("Server refused upload");
    }
    // YAY!!!!
}

