#ifndef _GUI__
#define _GUI__

#include <iostream>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <SFML/Graphics.hpp>
#include "GUI_Objects/RoundedRectangleShape.hpp"
#include "GUI_Objects/Buttons.hpp"
#include "GUI_Objects/TextBox.hpp"
#include "fileTransfer.hpp"
#include <vector>
#include <string.h>
#include <variant>

namespace GUI {

    // return next directory -> 0 = current, -1 = previous, > 0 = child of current
    // boolean will be 

    struct UserRequests {
        enum Type {
            ChangeDirectory,
            Upload,
            Download,
            Rename,
            Cut,
            Copy,
            Paste,
            Delete,
            SignOut,
            Quit
        } type;

        std::variant<std::monostate, std::string, ssize_t, const DirectoryTree*> data;
    };

    UserRequests currentDirectoryRequest(sf::RenderWindow& window, DirectoryTree& current);

}

#endif