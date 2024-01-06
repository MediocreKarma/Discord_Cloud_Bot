#include "gui.hpp"

std::string userInputString(const std::string& windowTitle, const std::string& startingString, const sf::RenderWindow& mainWindow) {
    const static auto desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow textWindow(sf::VideoMode(900, 400), windowTitle, sf::Style::Titlebar | sf::Style::Close, mainWindow.getSettings());
    textWindow.setPosition({
        static_cast<int>(mainWindow.getPosition().x + mainWindow.getSize().x / 2 - textWindow.getSize().x / 2), 
        static_cast<int>(mainWindow.getPosition().y + mainWindow.getSize().y / 2 - textWindow.getSize().y / 2)
    });
    TextBox inputBox(RoundedRectangleButton({600, 50}, 5, 5), GUI::Font, startingString, 30, Colors::DarkGray);
    inputBox.setOrigin(inputBox.getSize().x / 2, inputBox.getSize().y / 2);
    inputBox.setPosition(textWindow.getSize().x / 2, textWindow.getSize().y / 2);
    while (textWindow.isOpen()) {
        sf::Event event;
        while (textWindow.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    textWindow.close(); 
                    return inputBox.getData();
                case sf::Event::TextEntered:
                    if (event.text.unicode < 128 && event.text.unicode != '\r') {
                        return inputBox.externalType(textWindow, event.text.unicode);
                    }
                    break;
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button != sf::Mouse::Left) {
                        break;
                    }
                    if (inputBox.Hittable::hit(event.mouseButton.x, event.mouseButton.y)) {
                        return inputBox.getDataFromInput(textWindow, 0, 0);
                    }
                    break;
            }
        }
        textWindow.clear(sf::Color::White);
        textWindow.draw(inputBox);
        textWindow.display();
    }
    return inputBox.getData();
}

bool confirmationWindow(const std::string& windowTitle, const std::string& windowOutput, const sf::RenderWindow& mainWindow) {
    const static auto desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(800, 400), windowTitle, sf::Style::Titlebar | sf::Style::Close, mainWindow.getSettings());
    window.setPosition({
        static_cast<int>(mainWindow.getPosition().x + mainWindow.getSize().x / 2 - window.getSize().x / 2), 
        static_cast<int>(mainWindow.getPosition().y + mainWindow.getSize().y / 2 - window.getSize().y / 2)
    });
    sf::Text text(windowOutput, GUI::Font);
    text.setFillColor(sf::Color::Black);
    text.setOrigin(
        text.getGlobalBounds().left + text.getGlobalBounds().width / 2,
        text.getGlobalBounds().top + text.getGlobalBounds().height / 2
    );
    text.setPosition(window.getSize().x / 2, window.getSize().y / 3);
    RoundedRectangleTextButton okButton(sf::RoundedRectangleShape({200, 100}), sf::Text("OK", GUI::Font));
    okButton.setOrigin(okButton.getSize().x / 2, okButton.getSize().y / 2);
    okButton.setPosition(window.getSize().x * 3 / 4, window.getSize().y * 2 / 3);
    okButton.setShapeFillColor(Colors::LightGray);
    RoundedRectangleTextButton cancelButton(sf::RoundedRectangleShape({200, 100}), sf::Text("Cancel", GUI::Font));
    cancelButton.setOrigin(cancelButton.getSize().x / 2, cancelButton.getSize().y / 2);
    cancelButton.setPosition(window.getSize().x * 1 / 4, window.getSize().y * 2 / 3);
    cancelButton.setShapeFillColor(Colors::LightGray);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close(); 
                    return false;
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button != sf::Mouse::Left) {
                        break;
                    }
                    sf::Vector2f mouse = {1.f * event.mouseButton.x, 1.f * event.mouseButton.y};
                    if (okButton.hit(mouse)) {
                        return true;
                    }
                    if (cancelButton.hit(mouse)) {
                        return false;
                    }
                    break;
            }
        }
        window.clear(sf::Color::White);
        window.draw(okButton);
        window.draw(cancelButton);
        window.draw(text);
        window.display();
    }
    return false;
}

bool nameInUse(std::string& name, DirectoryTree& node) {
    for (size_t i = 0; i < node.childrenSize(); ++i) {
        if (node.child(i).name() == name) {
            return true;
        }
    }
    return false;
} 

GUI::UserRequests GUI::currentDirectoryRequest(sf::RenderWindow& window, DirectoryTree& current, ssize_t& selected, bool clipboard) {
    const unsigned int heightOffset = window.getView().getCenter().y - window.getSize().y / 2;
    const int buttonSize = 125;
    RoundedRectangleButton uploadButton({buttonSize, buttonSize}, 30, 15);
    uploadButton.setTexture(&GUI::uploadIcon, true);
    uploadButton.setOrigin(uploadButton.getSize().x / 2, uploadButton.getSize().y / 2);
    uploadButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y * 1 / 6 + heightOffset
    );
    RoundedRectangleButton downloadButton({buttonSize, buttonSize}, 30, 15);
    downloadButton.setTexture(&GUI::downloadIcon, true);
    downloadButton.setOrigin(downloadButton.getSize().x / 2, downloadButton.getSize().y / 2);
    downloadButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y * 2 / 6 + heightOffset
    );
    sf::RoundedRectangleShape downloadCover(downloadButton);
    downloadCover.setTexture(nullptr);
    downloadCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton backButton({buttonSize, buttonSize}, 45, 15);
    backButton.setTexture(&GUI::backIcon, true);
    backButton.setOrigin(backButton.getSize().x / 2, backButton.getSize().y / 2);
    backButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y * 3 / 6 + heightOffset
    );
    sf::RoundedRectangleShape backCover(backButton);
    backCover.setTexture(nullptr);
    backCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton forwardButton({buttonSize, buttonSize}, 45, 15);
    forwardButton.setTexture(&GUI::forwardIcon, true);
    forwardButton.setOrigin(forwardButton.getSize().x / 2, forwardButton.getSize().y / 2);
    forwardButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y * 4 / 6 + heightOffset
    );
    sf::RoundedRectangleShape forwardCover(forwardButton);
    forwardCover.setTexture(nullptr);
    forwardCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton createDirButton({buttonSize, buttonSize}, 30, 15);
    createDirButton.setTexture(&GUI::createDirIcon, true);
    createDirButton.setOrigin(createDirButton.getSize().x / 2, createDirButton.getSize().y / 2);
    createDirButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y * 5 / 6 + heightOffset
    );
    RoundedRectangleButton copyButton({buttonSize, buttonSize}, 30, 15);
    copyButton.setTexture(&GUI::copyIcon, true);
    copyButton.setOrigin(copyButton.getSize().x / 2, copyButton.getSize().y / 2);
    copyButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y * 1 / 6 + heightOffset
    );
    sf::RoundedRectangleShape copyCover(copyButton);
    copyCover.setTexture(nullptr);
    copyCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton cutButton({buttonSize, buttonSize}, 30, 15);
    cutButton.setTexture(&GUI::cutIcon, true);
    cutButton.setOrigin(cutButton.getSize().x / 2, cutButton.getSize().y / 2);
    cutButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y * 2 / 6 + heightOffset
    );
    sf::RoundedRectangleShape cutCover(cutButton);
    cutCover.setTexture(nullptr);
    cutCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton pasteButton({buttonSize, buttonSize}, 30, 15);
    pasteButton.setTexture(&GUI::pasteIcon, true);
    pasteButton.setOrigin(pasteButton.getSize().x / 2, pasteButton.getSize().y / 2);
    pasteButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y * 3 / 6 + heightOffset
    );
    sf::RoundedRectangleShape pasteCover(pasteButton);
    pasteCover.setTexture(nullptr);
    pasteCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton deleteButton({buttonSize, buttonSize}, 15, 15);
    deleteButton.setTexture(&GUI::deleteIcon, true);
    deleteButton.setOrigin(deleteButton.getSize().x / 2, deleteButton.getSize().y / 2);
    deleteButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y * 4 / 6 + heightOffset
    );
    sf::RoundedRectangleShape deleteCover(deleteButton);
    deleteCover.setTexture(nullptr);
    deleteCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton renameButton({buttonSize, buttonSize}, 15, 15);
    renameButton.setTexture(&GUI::renameIcon, true);
    renameButton.setOrigin(renameButton.getSize().x / 2, renameButton.getSize().y / 2);
    renameButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y * 5 / 6 + heightOffset
    );
    sf::RoundedRectangleShape renameCover(renameButton);
    renameCover.setTexture(nullptr);
    renameCover.setFillColor(sf::Color(200, 200, 200, 192));
    const sf::View startingView = window.getView();
    sf::View activeView = startingView;
    std::string path = current.path();
    std::vector<TextBox> fileSquares;
    std::vector<std::reference_wrapper<sf::RoundedRectangleShape>> selectionCovers = {
        std::ref(downloadCover),
        std::ref(deleteCover),
        std::ref(renameCover),
        std::ref(copyCover),
        std::ref(cutCover)
    };
    std::vector<std::reference_wrapper<sf::RoundedRectangleShape>> buttons = {
        std::ref(uploadButton),
        std::ref(downloadButton),
        std::ref(deleteButton),
        std::ref(renameButton),
        std::ref(cutButton),
        std::ref(copyButton),
        std::ref(pasteButton),
        std::ref(createDirButton),
        std::ref(backButton),
        std::ref(forwardButton)
    };
    std::vector<std::reference_wrapper<sf::Transformable>> transformables = {
        std::ref(backCover),
        std::ref(forwardCover),
        std::ref(pasteCover)
    };
    transformables.insert(transformables.end(), selectionCovers.begin(), selectionCovers.end());
    transformables.insert(transformables.end(), buttons.begin(), buttons.end());
    std::vector<std::reference_wrapper<sf::Drawable>> drawAlways(buttons.begin(), buttons.end());
    buttons.insert(buttons.end(), selectionCovers.begin(), selectionCovers.end());
    for (size_t i = 0; i < current.childrenSize(); ++i) {
        fileSquares.emplace_back(RoundedRectangleButton({800, 45}, 5, 5), GUI::Font, current.child(current.childrenSize() - 1 - i).name(), 30, sf::Color::Black, true, 0);
        fileSquares.back().setOrigin(
            fileSquares.back().getSize().x / 2,
            fileSquares.back().getSize().y / 2
        );
        fileSquares.back().setPosition(
            window.getSize().x / 2,
            300 + i * 50
        );
        fileSquares.back().setShapeFillColor(Colors::LightGray);
    }
    ssize_t selectedNode = -1;
    if (selected != -1) {
        fileSquares[selected].setShapeOutlineColor(sf::Color::Blue);
        selectedNode = current.childrenSize() - 1 - selected;
    }
    for (auto& sq : fileSquares) {
        drawAlways.emplace_back(std::ref(sq));
    }
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    return {UserRequests::Quit, std::monostate()};
                case sf::Event::MouseWheelScrolled: {
                    constexpr float SCROLL_MOVEMENT = 20;
                    int mult = 0;
                    if (event.mouseWheelScroll.delta > 0 && activeView.getCenter().y > window.getSize().y / 2) {
                        mult = -1;
                    }
                    else if (event.mouseWheelScroll.delta < 0 && activeView.getCenter().y + window.getSize().y / 2 <= fileSquares.back().getPosition().y + fileSquares.back().getSize().y / 2) {
                        mult = 1;
                    }
                    activeView.move(0, mult * SCROLL_MOVEMENT);
                    for (auto& tr : transformables) {
                        tr.get().move(0, mult * SCROLL_MOVEMENT);
                    }
                    break;
                }
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button != sf::Mouse::Left) {
                        break;
                    }
                    sf::Vector2f mouse = {1.f * event.mouseButton.x, 1.f * event.mouseButton.y};
                    mouse.y += activeView.getCenter().y - window.getSize().y / 2;
                    bool done = false;
                    for (ssize_t i = 0; i < fileSquares.size(); ++i) {
                        if (fileSquares[i].hit(mouse)) {
                            if (selected != -1) {
                                fileSquares[selected].setShapeOutlineColor(sf::Color::Black);
                            }
                            selected = i;
                            selectedNode = static_cast<ssize_t>(current.childrenSize()) - 1 - selected;
                            fileSquares[i].setShapeOutlineColor(sf::Color::Blue);
                            done = true;
                            break;
                        }
                    }
                    if (done) {
                        break;
                    }
                    if (uploadButton.hit(mouse)) {
                        std::string filePath = userInputString("Add file to cloud", "Enter path here", window);
                        std::cout << "Input is: \'" << filePath << '\'' <<  std::endl;
                        if (!std::filesystem::is_regular_file(filePath)) {
                            std::cerr << "Directory instead of file" << std::endl;
                        }
                        return {UserRequests::Upload, filePath};
                    }
                    else if (backButton.hit(mouse) && current.parent() != nullptr) {
                        selected = -1;
                        return {UserRequests::ChangeDirectory, std::monostate()};
                    }
                    else if (forwardButton.hit(mouse) && current.child(selectedNode).isDirectory()) {
                        return {UserRequests::ChangeDirectory, std::monostate()};
                    }
                    else if (selected != -1) {
                        if (downloadButton.hit(mouse)) {
                            return {UserRequests::Download, std::monostate()};
                        }
                        if (deleteButton.hit(mouse)) {
                            if (confirmationWindow("Delete file?", "Are you sure you want to delete this file?", window)) {
                                return {UserRequests::Delete, std::monostate()};
                            }
                        }
                        if (renameButton.hit(mouse)) {
                            std::string rename = userInputString("Rename file", "Enter name here", window);
                            bool trying = true;
                            while (trying && (rename.empty() || nameInUse(rename, current))) {
                                if (rename.empty()) {
                                    if (confirmationWindow("Rename error", "File name cannot be empty", window)) {
                                        rename = userInputString("Rename file", "Enter name here", window);
                                    }
                                    else {
                                        trying = false;
                                    }
                                }
                                else {
                                    if (confirmationWindow("Rename error", "Name is already in use!", window)) {
                                        rename = userInputString("Rename file", "Enter name here", window);
                                    }
                                    else {
                                        trying = false;
                                    }
                                }
                            } 
                            if (trying) {
                                return {UserRequests::Rename, rename};
                            }
                        }
                        // clone child
                        if (copyButton.hit(mouse)) {
                            return {UserRequests::Copy, std::monostate()};
                        }
                        // move child
                        if (cutButton.hit(mouse)) {
                            return {UserRequests::Cut, std::monostate()};
                        }
                        // add child
                        if (pasteButton.hit(mouse) && clipboard) {
                            return {UserRequests::Paste, std::monostate()};
                        }
                        if (createDirButton.hit(mouse)) {
                            std::string name = userInputString("Rename file", "Enter name here", window);
                            bool trying = true;
                            while (trying && (name.empty() || nameInUse(name, current))) {
                                if (name.empty()) {
                                    if (confirmationWindow("Rename error", "File name cannot be empty", window)) {
                                        name = userInputString("Rename file", "Enter name here", window);
                                    }
                                    else {
                                        trying = false;
                                    }
                                }
                                else {
                                    if (confirmationWindow("Rename error", "Name is already in use!", window)) {
                                        name = userInputString("Rename file", "Enter name here", window);
                                    }
                                    else {
                                        trying = false;
                                    }
                                }
                            } 
                            if (trying) {
                                return {UserRequests::Rename, name};
                            }
                        }
                    }
            }
        }
        auto clockStart = std::chrono::steady_clock::now();
        window.setView(activeView);
        window.clear(sf::Color::White);
        for (sf::Drawable& drawable : drawAlways) {
            window.draw(drawable);
        }
        if (selected == -1) {
            for (sf::Drawable& drawable : selectionCovers) {
                window.draw(drawable);
            }
        }
        if (selected == -1 || current.child(selectedNode).isDirectory() == false) {
            window.draw(forwardCover);
        }
        if (current.parent() == nullptr) {
            window.draw(backCover);
        }
        if (clipboard == false) {
            window.draw(pasteCover);
        }
        window.display();
        auto clockEnd = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(clockEnd - clockStart);
        if (dur.count() < 33) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33) - dur);
        }
    }
    return {UserRequests::Quit, std::monostate()};
}

