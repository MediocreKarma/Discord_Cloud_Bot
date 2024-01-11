#pragma once

#include <iostream>
#include <unistd.h>
#include <SFML/Graphics.hpp>
#include "GUI_Objects/RoundedRectangleShape.hpp"
#include "GUI_Objects/Buttons.hpp"
#include "GUI_Objects/TextBox.hpp"
#include <vector>
#include <string.h>

namespace LoginScreen {

    struct UserInfoData {
        std::string email;
        std::string password;
        bool signup;
    };

    std::string getEmailConfirmationCode(sf::RenderWindow& window, bool ceva);

    std::string loginProcedure(sf::RenderWindow& window, int sd);

};
