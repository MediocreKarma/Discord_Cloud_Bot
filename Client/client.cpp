#include <iostream>
#include <SFML/Graphics.hpp>
#include "login.hpp"
#include <vector>
#include <fstream>
#include "../Common/commons.hpp"
#include "gui.hpp"
#include <string.h>
#include <poll.h>

int main(int argc, char** argv) {
    if (!GUI::init()) {
        exit(EXIT_FAILURE);
    }
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "My Cloud Drive", sf::Style::Default, settings);
    std::ifstream portFile(Files::PortFile);
    if (!portFile.is_open()) {

        // DISPLAY SERVER IS NOT OPEN

        std::cerr << "Error reading from the port file in the client\n";
        std::cerr << "The server may be unavailable!\n";
        exit(EXIT_FAILURE);
    }
    uint16_t port = 0;
    portFile >> port;
    sockaddr_in server = {};
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0");
    server.sin_port = htons(port);
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("Error using socket in client");
        exit(EXIT_FAILURE);
    }
    if (connect(sd, (sockaddr*) &server, sizeof(sockaddr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }
    std::cout << "Connected to server" << std::endl;
    pollfd poll_sd = {sd, POLLIN, 0};  
    
    while (true) {
        bool login;
        if ((login = LoginScreen::loginProcedure(window, sd)) == false) {
            return 0;
        }
        DirectoryTree root = buildFilesystem(sd);
        DirectoryTree* current = &root;
        std::cout << "Built root filesystem: " + root.encodeTree() << std::endl;
        while (login) {
            GUI::UserRequests ur = GUI::currentDirectoryRequest(window, *current);
            switch (ur.type) {
                case GUI::UserRequests::Upload:
                    FileTransfer::sendFile(sd, std::get<std::string>(ur.data), root, *current);
                    break;
                case GUI::UserRequests::Download:
                    FileTransfer::receiveFile(sd, *std::get<const DirectoryTree*>(ur.data));
                case GUI::UserRequests::ChangeDirectory:
                    // ...
                    break;
                case GUI::UserRequests::Quit:
                    goto clientShutdown;
            }
        }
    }

clientShutdown:
    close(sd);
    return 0;
}