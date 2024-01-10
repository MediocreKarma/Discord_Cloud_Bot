#include "../Common/commons.hpp"
#include <filesystem>
#include <string.h>
#include <iostream>

namespace FileTransfer {

void receiveFile(int sd, const DirectoryTree& downloadable);
void    sendFile(int sd, const std::string& filepath, const DirectoryTree& root, DirectoryTree& current);

void  deleteFile(int sd, const DirectoryTree& root, DirectoryTree& parent, size_t index);
}

