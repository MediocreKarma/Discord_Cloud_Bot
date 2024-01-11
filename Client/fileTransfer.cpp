#include "fileTransfer.hpp"

// need ref, don't copy strings of 25MB
std::string& encrypt(std::string& data, const std::string& encryptionKey) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= encryptionKey[i % encryptionKey.size()];
    }
    return data;
}

std::string& decrypt(std::string& data, const std::string& encryptionKey) {
    return encrypt(data, encryptionKey);
}

void FileTransfer::receiveFile(int sd, const DirectoryTree& downloadable, const std::string& encKey) {
    const std::string downloadPath = getenv("HOME") + std::string("/Downloads/");
    std::filesystem::create_directories(downloadPath);
    const std::string targetName = downloadable.name();
    const std::string beforeExtension = targetName.substr(0, targetName.find_last_of('.'));
    const std::string extension = targetName.substr(targetName.find_last_of('.'));
    std::string filepath = downloadPath + targetName;
    size_t iteration = 0;
    while (std::filesystem::exists(filepath)) {
        iteration++;
        filepath = downloadPath + beforeExtension + " (" + std::to_string(iteration) + ")" + extension;
    }
    std::cout << filepath << " " << downloadable.size() << std::endl;
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::invalid_argument("Bad filepath!");
    }
    ClientMessage cmsg = {ClientMessage::FileDownload};
    strncpy(cmsg.content.file.id, downloadable.id().c_str(), 8);
    cmsg.content.file.id[8] = '\0';
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
    size_t total = downloadable.size();
    size_t current = 0;
    std::string chunk(MAX_FILE_TRANSFER, '\0');
    while (current < total) {
        size_t readLength = std::min(MAX_FILE_TRANSFER, total - current);
        if (Communication::read(sd, chunk.data(), readLength) == false) {
            perror("error reading from server");
            throw std::ios::failure("Read failure to socket");
        }
        if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false) {
            perror("error writing response from server");
            throw std::ios::failure("Writing from socket failed");
        }
        decrypt(chunk, encKey);
        if (!file.write(chunk.c_str(), readLength)) {
            std::remove(filepath.c_str());
            throw std::ios::failure("Writing to file failed");
        }
        current += readLength;
    }
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        perror("error reading final message");
        throw std::ios::failure("Weird error");
    }
    std::cout << "Download finalized" << std::endl;
}

void FileTransfer::sendFile(int sd, const std::string& filepath, const DirectoryTree& root, DirectoryTree& current, const std::string& encKey) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::invalid_argument("Bad filepath!");
    }
    size_t fileSize = std::filesystem::file_size(filepath);
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
        encrypt(chunk, encKey);
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
    current.addChild(id, fileSize, alias);
    chunk = root.encodeTree();
    updateFileTree(sd, root);
    // YAY!!!!
}

void FileTransfer::deleteFile(int sd, const DirectoryTree& root, DirectoryTree& parent, size_t index) {
    // verify if last link to file
    if (root.countLinks(parent.child(index).id()) > 1) {
        parent.erase(index);
        std::cout << "Soft link deletion" << std::endl;
        return;
    }
    ClientMessage cmessage = {ClientMessage::FileDelete};
    ServerMessage smessage;
    strncpy(cmessage.content.file.id, parent.child(index).id().c_str(), 9);
    cmessage.content.file.id[8] = '\0';
    if (Communication::write(sd, &cmessage, sizeof(cmessage)) == false) {
        perror("Error using write");
        throw std::ios::failure("Failure to write to socket");
    }
    if (Communication::read(sd, &smessage, sizeof(smessage)) == false) {
        perror("Error using read");
        throw std::ios::failure("Failure to read from socket");
    }
    if (smessage.type != ServerMessage::OK) {
        std::cerr << "Error deleting file: " << ServerMessage::humanReadable(smessage.error) << std::endl;
        return;
    }
    parent.erase(index);
    std::cout << "Full file deletion" << std::endl;
}

void FileTransfer::updateFileTree(int sd, const DirectoryTree& root) {
    ClientMessage cmsg;
    ServerMessage smsg;
    cmsg.type = ClientMessage::UpdateFileTree;
    std::string treeChunk = root.encodeTree();
    cmsg.content.file.size = treeChunk.size();
    if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false ||
        Communication::write(sd, treeChunk.c_str(), treeChunk.size()) == false) {
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
}