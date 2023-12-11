#include "../Common/commons.hpp"

namespace FileTransfer {

void receiveFile(int sd, const std::string& filepath);
void    sendFile(int sd, const std::string& filepath);

}
