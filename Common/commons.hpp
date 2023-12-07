#ifndef _COMMONS__
#define _COMMONS__

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>
#include <stdexcept>
#include "DirectoryTree/DirectoryTree.hpp"

namespace Files {
    inline const char* PortFile = "/tmp/MyCloudPort.txt";
}


struct ClientMessage {
    enum MessageType {
        Empty,
        SignUpRequest,
        SignInRequest,
        SignUpCode,
        RequestFileTree,
        FileTransfer,
    } type;

    union MessageContent {

        struct SignData { char email[65]; char pass[65]; char signCode[7]; } signData;

        size_t size;

    } content;

};

struct ServerMessage {
    enum MessageType {
        OK,
        RequestCode,
        Error,
        ServerQuit,
        FileTransfer
    } type;

    enum ErrorType {
        NoError,
        EmailAlreadyInUse,
        WrongLogin,
        WrongCode,
        InternalError
    } error;

    static std::string humanReadable(ServerMessage::ErrorType x) {
        switch (x) {
            case NoError:
                return "No error in transmission";
            case EmailAlreadyInUse:
                return "Email is already in use";
            case WrongLogin:
                return "Wrong login information";
            case WrongCode:
                return "Wrong code for signup";
            case InternalError:
                return "Error in a dependency";
        }
        return "Unknown error";
    }
    // when receiving a size, a string of size 'size'
    // will also be supplied following the server message
    union MessageContent {
        size_t size;
    } content;
};

namespace Communication {

    bool write(int sd, void* buffer, size_t len);
    bool read(int sd, void* buffer, size_t len);

}

#endif