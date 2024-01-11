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
        std::string email = LoginScreen::loginProcedure(window, sd);
        if (email.empty()) {
            return 0;
        }
        DirectoryTree root = buildFilesystem(sd);
        std::unique_ptr<DirectoryTree> clipboard = nullptr;
        ssize_t clipIndex = -1;
        ssize_t selected = -1;
        DirectoryTree* current = &root;
        std::cout << "Built root filesystem" << std::endl;
        while (true) {
            GUI::UserRequests ur = GUI::currentDirectoryRequest(window, *current, selected, clipboard.get());
            ssize_t index;
            if (selected == -1) index = -1 ;
            else index = current->childrenSize() - 1 - selected;
            switch (ur.type) {
                case GUI::UserRequests::Upload:
                    FileTransfer::sendFile(sd, std::get<std::string>(ur.data), root, *current, email);
                    break;
                case GUI::UserRequests::Download:
                    FileTransfer::receiveFile(sd, current->child(index), email);
                    break;
                case GUI::UserRequests::ChangeDirectory: {
                    selected = -1;
                    sf::View view = window.getView();
                    view.setCenter(window.getSize().x / 2, window.getSize().y / 2);
                    window.setView(view);
                    if (index != -1) {
                        current = &(current->child(index));
                    }
                    else {
                        current = current->parent();
                    }
                    break;
                }
                case GUI::UserRequests::Copy:
                    clipboard = std::make_unique<DirectoryTree>(current->child(index).clone());
                    clipIndex = -1;
                    break;
                case GUI::UserRequests::Cut:
                    clipboard = std::make_unique<DirectoryTree>(current->child(index).clone());
                    clipIndex = index;
                    break;  
                case GUI::UserRequests::Paste:
                    current->addChild(clipboard->clone());
                    if (clipIndex != -1) {
                        clipboard->parent()->erase(clipIndex);
                        clipIndex = -1;
                    }
                    FileTransfer::updateFileTree(sd, root);
                    break;
                case GUI::UserRequests::Rename:
                    current->child(index).rename(std::get<std::string>(ur.data));
                    FileTransfer::updateFileTree(sd, root);
                    break;
                case GUI::UserRequests::CreateDirectory: 
                    current->addChild("00000000", 0, std::get<std::string>(ur.data));
                    FileTransfer::updateFileTree(sd, root);
                    break;
                case GUI::UserRequests::Delete: {
                    DirectoryTree& deleteTarget = current->child(index);
                    if (deleteTarget.isDirectory()) {
                        if (deleteTarget.name() == "") {
                            std::cerr << "Cannot delete root" << std::endl;
                        }
                        else {
                            current->erase(index);
                        }
                    }
                    else {
                        FileTransfer::deleteFile(sd, root, *current, index);
                        FileTransfer::updateFileTree(sd, root);
                    }
                    selected = -1;
                    break;
                }
                case GUI::UserRequests::Quit:
                    goto clientShutdown;
            }
        }
    }

clientShutdown:
    close(sd);
    return 0;
}