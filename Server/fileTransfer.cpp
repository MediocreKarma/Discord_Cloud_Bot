#include "fileTransfer.hpp"

std::string generateStringNotInTree(const DirectoryTree& t) {
    static std::random_device rd;
    static std::seed_seq ss = { rd(), rd(), rd(), rd() };
    static std::mt19937 rng(ss);
    static const char alphabet[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<size_t> uid(0, sizeof(alphabet) / sizeof(char) - 2);
    std::string code(DirectoryTree::ID_LEN, '\0');
    for (char& c : code) {
        c = alphabet[uid(rng)];
    }
    // stop when ID is not taken
    while (t.findID(code)) {
        for (char& c : code) {
            c = alphabet[uid(rng)];
        }
    }
    return code;
}

bool FileTransfer::receiveFile(
    const int client, 
    BotWrapper& discord, 
    Request::UserInfo& info, 
    const size_t size, 
    const std::string& alias
) {
    std::cout << '\'' << alias << "\' " << size << std::endl;
    pollfd sdPoll = {client, POLLIN, 0};
    ServerMessage smsg = {ServerMessage::OK, ServerMessage::NoError};
    // prepare id for file
    const DirectoryTree t = DirectoryTree::buildTree(info.tree);
    const std::string id = generateStringNotInTree(t);
    size_t index = 0;
    // confirm client may continue upload
    if (Communication::write(client, &smsg, sizeof(smsg)) == false) {
        perror("Could not write to client");
        throw std::ios::failure("Write failure to socket");
    }
    std::string fileparts = "";
    std::string chunk(MAX_FILE_TRANSFER, '\0');
    size_t readSize = 0;
    while (readSize < size) {
        size_t readLength = std::min(MAX_FILE_TRANSFER, size - readSize);
        if (
            poll(&sdPoll, 1, 60 * 1000) == -1 || 
            (sdPoll.revents & POLLIN) == 0 ||
            (sdPoll.revents & POLLNVAL) == 1
        ) {
            perror("error using poll");
            goto deleteFiles;
        }
        sdPoll.revents = 0;
        if (Communication::read(client, chunk.data(), readLength) == false) {
            perror("error reading chunk from client");
            goto deleteFiles;
        }
        if (readLength < MAX_FILE_TRANSFER) {
            chunk.resize(readLength);
        }
        readSize += readLength;
        std::string indexStr = std::to_string(index++);
        indexStr = std::string(5 - indexStr.size(), '0') + indexStr;
        const std::string part_filename = id + "_" + indexStr;
        dpp::snowflake val = discord.upload(discord.channel(BotWrapper::DATA), {part_filename, chunk});
        if (val == 0) {
            goto deleteFiles;
        }
        else {
            fileparts += val.str() + " ";
        }
        if (Communication::write(client, &smsg, sizeof(smsg)) == false) {
            perror("Could not write to client");
            goto deleteFiles;
        }
    }
    fileparts.pop_back();
    std::cout << "Finished upload" << std::endl;
    // created all parts, now add to db
    info.db.lock();
    if (info.db.createStatement(
        std::string("INSERT INTO info (file, data) ") +
        "VALUES(\'" + id + "\', \'" + fileparts + "\');"
    ) == false) {
        std::cerr << "Statement to insert failed" << std::endl;
    }
    if (info.db.nextRow() == false) {
        std::cerr << "Failure to insert" << std::endl;
    }
    info.db.unlock();

    // send file id
    smsg.type = ServerMessage::SendID;
    strncpy(smsg.content.file_id, id.c_str(), 9);
    if (Communication::write(client, &smsg, sizeof(smsg)) == false) {
        perror("Could not write to client SendID");
        goto deleteFiles;
    }
    // read file tree string
    ClientMessage cmsg;
    if (
        poll(&sdPoll, 1, 60 * 1000) == -1 || 
        (sdPoll.revents & POLLIN) == 0 ||
        (sdPoll.revents & POLLNVAL) == 1
    ) {
        perror("error using poll");
        goto deleteFiles;
    }
    sdPoll.revents = 0;
    if (Communication::read(client, &cmsg, sizeof(cmsg)) == false) {
        perror("Could not write to client");
        goto deleteFiles;
    }
    if (cmsg.type != ClientMessage::UpdateFileTree) {
        std::cerr << "Invalid response" << std::endl;
        goto deleteFiles;
    }
    {
        size_t updateSizeFile = cmsg.content.file.size;
        chunk.resize(updateSizeFile);
        if (Communication::read(client, chunk.data(), updateSizeFile) == false) {
            perror("error reading updated file tree");
            goto deleteFiles;
        }
    }
    std::cout << "Succesful file upload!" << std::endl;
    info.tree = std::move(chunk);
    return true;
deleteFiles:
    std::cerr << "Error encountered, deleting files" << std::endl;
    std::stringstream ss(fileparts);
    for (size_t i = 0; i * 8 < fileparts.size(); ++i) {
        uint64_t id = 0;
        ss >> id;
        discord.remove(id, discord.channel(BotWrapper::DATA));
    }
    info.db.lock();
    // failure is irrelevant
    info.db.createStatement(
        std::string("DELETE FROM info WHERE ") + "file = \'" + id + "\';"
    );
    info.db.nextRow();
    info.db.unlock();
    smsg.type = ServerMessage::Error;
    // any potential error is socket IO or discord related
    smsg.error = ServerMessage::InternalError;
    return false;
}

bool FileTransfer::sendFile(int sd, BotWrapper& discord, Request::UserInfo& info, const std::string& fileID) {
    // request received / user already knows size
    ServerMessage smsg = {ServerMessage::OK, ServerMessage::NoError};
    if (Communication::write(sd, &smsg, sizeof(smsg)) == false) {
        perror("error using write");
        return false;
    }
    ClientMessage cmsg;
    info.db.lock();
    info.db.createStatement(
        "SELECT data FROM info WHERE file = \'" + fileID + "\';"
    );
    info.db.nextRow();
    std::stringstream ss(info.db.extract<std::string>(0));
    info.db.unlock();
    size_t i = 0;
    while (ss.good()) {
        uint64_t id = 0;
        ss >> id;
        std::cout << id << std::endl;
        dpp::snowflake snowflake = id;
        std::vector<BotWrapper::File> files = discord.download(snowflake, discord.channel(BotWrapper::DATA));
        if (files.size() != 1) {
            // error, wrong amount of files?
        }
        std::string body = std::move(files[0].body);
        if (Communication::write(sd, body.c_str(), body.size()) == false) {
            perror("error writing file to client");
            return false;
        }
        if (Communication::read(sd, &cmsg, sizeof(cmsg)) == false) {
            perror("Error reading client response");
            return false;
        }
        if (cmsg.type != ClientMessage::OK) {
            std::cerr << "Client canceled transfer operation" << std::endl;
            return false;
        }
    }
    return true;
}

bool FileTransfer::updateFileTree(int sd, Request::UserInfo& info, const size_t size) {
    std::string treeChunk(size, '\0');
    if (Communication::read(sd, treeChunk.data(), size) == false) {
        perror("error reading updated file tree");
        return false;
    }
    info.tree = std::move(treeChunk);
    return true;
}
