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
    // std::ifstream portFile(Files::PortFile);
    // if (!portFile.is_open()) {

    //     // DISPLAY SERVER IS NOT OPEN

    //     std::cerr << "Error reading from the port file in the client\n";
    //     std::cerr << "The server may be unavailable!\n";
    //     exit(EXIT_FAILURE);
    // }
    // uint16_t port = 0;
    // portFile >> port;
    // sockaddr_in server = {};
    // server.sin_family = AF_INET;
    // server.sin_addr.s_addr = inet_addr("0");
    // server.sin_port = htons(port);
    // int sd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sd == -1) {
    //     perror("Error using socket in client");
    //     exit(EXIT_FAILURE);
    // }
    // if (connect(sd, (sockaddr*) &server, sizeof(sockaddr)) == -1) {
    //     perror("Error connecting to server");
    //     exit(EXIT_FAILURE);
    // }
    // std::cout << "Connected to server" << std::endl;
    // pollfd poll_sd = {sd, POLLIN, 0};  
    
    while (true) {
        // bool login;
        // if ((login = LoginScreen::loginProcedure(window, sd)) == false) {
        //     return 0;
        // }
        // DirectoryTree root = buildFilesystem();
        
        DirectoryTree root("");
        for (int i = 0; i < 25; ++i) {
            root.addChild("file.txt");
        }
        std::cout << root.encodeTree() << std::endl;
        GUI::displayCurrentDirectory(window, root);
        return 0;
        // while (login) {

            // display current directory

        // }
    }


    // close(sd);
    return 0;
}