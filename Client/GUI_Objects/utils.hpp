#pragma once

#include <SFML/Graphics.hpp>
#include "Buttons.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include "../../Common/commons.hpp"
#include <dpp/utility.h>

namespace GUI {
    inline sf::Font Font;
    inline sf::Texture renameIcon;
    inline sf::Texture downloadIcon;
    inline sf::Texture uploadIcon;
    inline sf::Texture deleteIcon;
    inline sf::Texture copyIcon;
    inline sf::Texture cutIcon;
    inline sf::Texture pasteIcon;
    inline sf::Texture backIcon;
    inline sf::Texture forwardIcon;
    inline sf::Texture createDirIcon;
    bool init();

    sf::Text& centerOrigin(sf::Text&);
    sf::RoundedRectangleShape& centerOrigin(sf::RoundedRectangleShape&);
}

namespace Colors {
    const sf::Color LightGray = sf::Color(233, 233, 233); 
    const sf::Color Gray = sf::Color(218, 223, 225);
    const sf::Color DarkGray = sf::Color(169, 169, 169);
}

void loadingMessage(sf::RenderWindow& window);

DirectoryTree buildFilesystem(int sd);