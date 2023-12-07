#include "gui.hpp"

void GUI::displayCurrentDirectory(sf::RenderWindow& window, DirectoryTree& current) {
    RoundedRectangleTextShape uploadButton();
    RoundedRectangleTextShape downloadButton();
    RoundedRectangleTextShape deleteButton();
    RoundedRectangleTextShape renameButton();
    const sf::View startingView = window.getView();
    sf::View activeView = startingView;
    std::string path = current.path();
    std::vector<TextBox> fileSquares;
    std::vector<std::reference_wrapper<sf::Drawable>> drawables;
    for (size_t i = 0; i < current.childrenSize(); ++i) {
        fileSquares.emplace_back(sf::RoundedRectangleShape({800, 45}, 5, 5), Text::Font, current.child(i).name(), 30, sf::Color::Black, true, 0);
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
                            std::cout << "Hit the " << i + 1 << " file" << std::endl;
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
