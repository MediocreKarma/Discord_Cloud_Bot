#include "../Common/commons.hpp"
#include <filesystem>
#include <string.h>
#include <iostream>

namespace FileTransfer {

void receiveFile(int sd, const std::string& filepath);
void    sendFile(int sd, const std::string& filepath, const DirectoryTree& root, DirectoryTree& current);

}
