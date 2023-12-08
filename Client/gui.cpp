#include "gui.hpp"

void GUI::displayCurrentDirectory(sf::RenderWindow& window, DirectoryTree& current) {
    sf::RoundedRectangleShape uploadButton({174, 174}, 15, 15);
    uploadButton.setOrigin(uploadButton.getSize().x / 2, uploadButton.getSize().y / 2);
    uploadButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y - 450
    );
    uploadButton.setTexture(&GUI::uploadIcon, true);
    sf::RoundedRectangleShape downloadButton({174, 174}, 15, 15);
    downloadButton.setTexture(&GUI::downloadIcon, true);
    downloadButton.setOrigin(downloadButton.getSize().x / 2, downloadButton.getSize().y / 2);
    downloadButton.setPosition(
        (window.getSize().x / 2 - 400) / 2,
        window.getSize().y - 200
    );
    sf::RoundedRectangleShape deleteButton({174, 174}, 15, 15);
    deleteButton.setTexture(&GUI::deleteIcon, true);
    deleteButton.setOrigin(deleteButton.getSize().x / 2, deleteButton.getSize().y / 2);
    deleteButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y - 200
    );
    sf::RoundedRectangleShape renameButton({174, 174}, 15, 15);
    renameButton.setTexture(&GUI::renameIcon, true);
    renameButton.setOrigin(renameButton.getSize().x / 2, renameButton.getSize().y / 2);
    renameButton.setPosition(
        window.getSize().x / 2 + (window.getSize().x / 2 + 400) / 2,
        window.getSize().y - 450
    );
    const sf::View startingView = window.getView();
    sf::View activeView = startingView;
    std::string path = current.path();
    std::vector<TextBox> fileSquares;
    std::vector<std::reference_wrapper<sf::Drawable>> drawables = {
        std::ref(uploadButton),
        std::ref(downloadButton),
        std::ref(deleteButton),
        std::ref(renameButton)
    };
    for (size_t i = 0; i < current.childrenSize(); ++i) {
        fileSquares.emplace_back(sf::RoundedRectangleShape({800, 45}, 5, 5), GUI::Font, current.child(i).name(), 30, sf::Color::Black, true, 0);
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
                    return;
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
                    for (size_t i = 0; i < fileSquares.size(); ++i) {
                        if (fileSquares[i].hit(mouse)) {
                            selected = i;
                        }
                    }
            }
        }
        window.setView(activeView);
        window.clear(sf::Color::White);
        for (sf::Drawable& drawable : drawables) {
            window.draw(drawable);
        }
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

}
