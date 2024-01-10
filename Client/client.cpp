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
    /*
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
    */
    int sd = 0;
    while (true) {
        // bool login = LoginScreen::loginProcedure(window, sd);
        // if (login == false) {
        //     return 0;
        // }
        //DirectoryTree root = buildFilesystem(sd);
        bool login = true;
        DirectoryTree root = DirectoryTree("00000000", 0, "");
        root.addChild("00000000", 0, "dir");
        for (int i = 0; i < 30; ++i) {
            std::string nr = std::to_string(i + 1);
            root.addChild(std::string(8 - nr.size(), '0') + nr, 1, + "file" + nr + ".txt"); 
        }
        std::unique_ptr<DirectoryTree> clipboard = nullptr;
        ssize_t clipIndex = -1;
        ssize_t selected = -1;
        DirectoryTree* current = &root;
        std::cout << "Built root filesystem" << std::endl;
        while (login) {
            GUI::UserRequests ur = GUI::currentDirectoryRequest(window, *current, selected, clipboard.get());
            ssize_t index;
            if (selected == -1) index = -1 ;
            else index = current->childrenSize() - 1 - selected;
            switch (ur.type) {
                case GUI::UserRequests::Upload:
                    //FileTransfer::sendFile(sd, std::get<std::string>(ur.data), root, *current);
                    break;
                case GUI::UserRequests::Download:
                    //FileTransfer::receiveFile(sd, *std::get<const DirectoryTree*>(ur.data));
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
                    break;
                case GUI::UserRequests::Rename:
                    current->child(index).rename(std::get<std::string>(ur.data));
                    break;
                case GUI::UserRequests::CreateDirectory: 
                    current->addChild("00000000", 0, std::get<std::string>(ur.data));
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
                    }   
                    break;
                }
                case GUI::UserRequests::Quit:
                    goto clientShutdown;
            }
        }
    }

clientShutdown:
    //close(sd);
    return 0;
}