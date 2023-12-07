#ifndef _GUI__
#define _GUI__

#include <iostream>
#include <unistd.h>
#include <SFML/Graphics.hpp>
#include "GUI_Objects/RoundedRectangleShape.hpp"
#include "GUI_Objects/Buttons.hpp"
#include "GUI_Objects/TextBox.hpp"
#include <vector>
#include <string.h>

namespace GUI {

    void displayCurrentDirectory(sf::RenderWindow& window, DirectoryTree& current);

}

#endif