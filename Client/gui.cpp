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

GUI::UserRequests GUI::currentDirectoryRequest(sf::RenderWindow& window, DirectoryTree& current) {
    RoundedRectangleButton uploadButton({174, 174}, 30, 15);
    uploadButton.setOrigin(uploadButton.getSize().x / 2, uploadButton.getSize().y / 2);
    uploadButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y - 450
    );
    uploadButton.setTexture(&GUI::uploadIcon, true);
    RoundedRectangleButton downloadButton({174, 174}, 30, 15);
    downloadButton.setTexture(&GUI::downloadIcon, true);
    downloadButton.setOrigin(downloadButton.getSize().x / 2, downloadButton.getSize().y / 2);
    downloadButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y - 200
    );
    RoundedRectangleButton downloadCover(downloadButton);
    downloadCover.setTexture(nullptr);
    downloadCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton deleteButton({174, 174}, 15, 15);
    deleteButton.setTexture(&GUI::deleteIcon, true);
    deleteButton.setOrigin(deleteButton.getSize().x / 2, deleteButton.getSize().y / 2);
    deleteButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y - 200
    );
    RoundedRectangleButton deleteCover(deleteButton);
    deleteCover.setTexture(nullptr);
    deleteCover.setFillColor(sf::Color(200, 200, 200, 192));
    RoundedRectangleButton renameButton({174, 174}, 15, 15);
    renameButton.setTexture(&GUI::renameIcon, true);
    renameButton.setOrigin(renameButton.getSize().x / 2, renameButton.getSize().y / 2);
    renameButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y - 450
    );
    RoundedRectangleButton renameCover(renameButton);
    renameCover.setTexture(nullptr);
    renameCover.setFillColor(sf::Color(200, 200, 200, 192));
    const sf::View startingView = window.getView();
    sf::View activeView = startingView;
    std::string path = current.path();
    std::vector<TextBox> fileSquares;
    std::vector<std::reference_wrapper<sf::Drawable>> drawableCovers = {
        std::ref(downloadCover),
        std::ref(deleteCover),
        std::ref(renameCover)
    };
    std::vector<std::reference_wrapper<sf::Drawable>> drawables = {
        std::ref(uploadButton),
        std::ref(downloadButton),
        std::ref(deleteButton),
        std::ref(renameButton)
    };
    for (size_t i = 0; i < current.childrenSize(); ++i) {
        fileSquares.emplace_back(RoundedRectangleButton({800, 45}, 5, 5), GUI::Font, current.child(i).name(), 30, sf::Color::Black, true, 0);
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
    for (size_t i = 0; i < current.childrenSize(); ++i) {
        drawables.emplace_back(std::ref(fileSquares[i]));
    }
    size_t selected = -1;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    return {UserRequests::Quit, std::monostate()};
                case sf::Event::MouseWheelScrolled: {
                    constexpr float SCROLL_MOVEMENT = 20;
                    if (event.mouseWheelScroll.delta > 0 && activeView.getCenter().y > window.getSize().y / 2) {
                        activeView.move(0, -SCROLL_MOVEMENT);
                    }
                    else if (event.mouseWheelScroll.delta < 0 && activeView.getCenter().y + window.getSize().y / 2 <= fileSquares.back().getPosition().y + fileSquares.back().getSize().y / 2) {
                        activeView.move(0, +SCROLL_MOVEMENT);
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
                    for (size_t i = 0; i < fileSquares.size(); ++i) {
                        if (fileSquares[i].hit(mouse)) {
                            if (selected != static_cast<size_t>(-1)) {
                                fileSquares[selected].setShapeOutlineColor(sf::Color::Black);
                            }
                            selected = i;
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
                    else if (selected) {
                        // implement other commands later
                    }
            }
        }
        window.setView(activeView);
        window.clear(sf::Color::White);
        for (sf::Drawable& drawable : drawables) {
            window.draw(drawable);
        }
        if (selected == static_cast<size_t>(-1)) {
            for (sf::Drawable& drawable : drawableCovers) {
                window.draw(drawable);
            }
        }
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    return {UserRequests::Quit, std::monostate()};
}

