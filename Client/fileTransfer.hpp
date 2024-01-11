#include "../Common/commons.hpp"
#include <filesystem>
#include <string.h>
#include <iostream>

namespace FileTransfer {

void receiveFile(int sd, const DirectoryTree& downloadable, const std::string& key);
void    sendFile(int sd, const std::string& filepath, const DirectoryTree& root, DirectoryTree& current, const std::string& key);

void  deleteFile(int sd, const DirectoryTree& root, DirectoryTree& parent, size_t index);
void  updateFileTree(int sd, const DirectoryTree& root);

}

