#ifndef _UTILS__
#define _UTILS__

#include <SFML/Graphics.hpp>
#include "Buttons.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include "../../Common/commons.hpp"

namespace Text {
    inline sf::Font Font;
    bool init();
}

namespace Colors {
    const sf::Color LightGray = sf::Color(218, 223, 225); 
    const sf::Color Gray = sf::Color(211, 211, 211);
    const sf::Color DarkGray = sf::Color(169, 169, 169);
}

void loadingMessage(sf::RenderWindow& window);

DirectoryTree buildFilesystem(int sd);

#endif 