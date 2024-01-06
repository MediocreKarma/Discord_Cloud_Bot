#include "utils.hpp"

bool GUI::init() {
    bool succesful = true;
    const std::string base = "../Resources/";
    succesful &= Font.loadFromFile(base + "Roboto-Regular.ttf");
    succesful &= downloadIcon.loadFromFile(base + "download.png");
    succesful &= renameIcon.loadFromFile(base + "rename.png");
    succesful &= uploadIcon.loadFromFile(base + "upload.png");
    succesful &= deleteIcon.loadFromFile(base + "trash.png");
    succesful &= cutIcon.loadFromFile(base + "cut.png");
    succesful &= copyIcon.loadFromFile(base + "copy.png");
    succesful &= pasteIcon.loadFromFile(base + "paste.png");
    succesful &= changeDirIcon.loadFromFile(base + "changeDir.png");
    succesful &= backIcon.loadFromFile(base + "back.png");
    return succesful;
}

void loadingMessage(sf::RenderWindow& window) {
    RoundedRectangleTextShape loadingText(sf::RoundedRectangleShape({300, 100}, 0, 0), sf::Text("Loading", GUI::Font));
    loadingText.setTextFillColor(sf::Color::Black);
    loadingText.setShapeFillColor(sf::Color::Transparent);
    loadingText.setShapeOutlineColor(sf::Color::Black);
    loadingText.setOrigin(
        loadingText.getSize().x / 2,
        loadingText.getSize().y / 2
    );
    loadingText.setPosition(
        window.getSize().x / 2,
        window.getSize().y - 100
    );
    window.draw(loadingText);
    window.display();
}   

DirectoryTree buildFilesystem(const int sd) {
    ClientMessage cmsg = {ClientMessage::RequestFileTree};
    ServerMessage smsg;
    if (Communication::write(sd, &cmsg, sizeof(cmsg)) == false) {
        throw std::ios::failure("File tree could not be requested");
    }
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        throw std::ios::failure("File tree message could not be read");
    }
    std::cout << "Read server message" << std::endl;
    size_t size = smsg.content.size;
    std::string treeString(size, '\0');
    if (Communication::read(sd, treeString.data(), size) == false) {
        throw std::ios::failure("File tree could not be read");
    }
    if (Communication::read(sd, &smsg, sizeof(smsg)) == false) {
        perror("did not read OK");
        throw std::ios::failure("OK not received");
    }
    return DirectoryTree::buildTree(treeString);
}

// need ref, don't copy strings of 25MB
std::string& encrypt(std::string& data, const std::string& encryptionKey) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= encryptionKey[i % encryptionKey.size()];
    }
    return data;
}

std::string& decrypt(std::string& data, const std::string& encryptionKey) {
    return encrypt(data, encryptionKey);
}