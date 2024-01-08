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

    struct UserRequests {
        enum Type {
            ChangeDirectory,
            CreateDirectory,
            Upload,
            Download,
            Rename,
            Cut,
            Copy,
            Paste,
            Delete,
            ChangeTree,
            SignOut,
            Quit,
            Reload
        } type;

        std::variant<std::monostate, std::string> data;
    };
    
    UserRequests currentDirectoryRequest(sf::RenderWindow& window, DirectoryTree& current, ssize_t& selected, DirectoryTree* clipboard);

}

#endif