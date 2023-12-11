#include "TextBox.hpp"

#include <iostream>

TextBox::TextBox(
    const sf::RoundedRectangleShape& shape, 
    const sf::Font& font, 
    const std::string& displayString,
    const unsigned int charSize,
    sf::Color displayColor,
    const bool alwaysHasInput,
    const char _cover,
    size_t maxLen
    ) : RoundedRectangleTextButton(shape, sf::Text(sf::String(displayString), font, charSize)),
        buffer(""), noInputText(displayString), noInputColor(displayColor), cursorIndex(0), 
        visibileTextIndex{0, 0}, hasInput(false), alwaysHasInput(false), frame(0), 
        coverChar(_cover), enter(false), mouse({-1, -1}), maxLength(maxLen), alphabet() {
        
    writeLine[0].color = writeLine[1].color = sf::Color::Black;
    text.setFillColor(noInputColor);
    writeLine[0].position.x = writeLine[1].position.x = -10000;
    text.setOrigin(0, text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2);
    text.setPosition(
        25 - text.getLocalBounds().left,
        0
    );
}

void TextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    RoundedRectangleTextButton::draw(target, states);
    if (frame < FLICKER_WRITELINE_PER_FRAMES) {
        states.transform *= getTransform();
        target.draw(writeLine.data(), writeLine.size(), sf::PrimitiveType::Lines, states);
    }
    ++frame;
    if (frame == FLICKER_WRITELINE_PER_FRAMES * 2) {
        frame = 0;
    }
}

bool TextBox::hasUserInput() const {
    return hasInput;
}

void TextBox::setDisplayString(const std::string& str) {
    noInputText = str;
}

void TextBox::setFont(const sf::Font& font) {
    text.setFont(font);
    text.setOrigin(0, text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2);
    text.setPosition(
        25 - text.getLocalBounds().left,
        0
    );
}

std::string TextBox::getDataFromInput(sf::RenderWindow& window, const int x, const int y,  const std::string& initText) {
    writeLine[0].position.y = shape.getPosition().y + getOrigin().y - 20;
    writeLine[1].position.y = shape.getPosition().y + getOrigin().y + 20;
    text.setFillColor(sf::Color::Black);
    if (!hasInput) {
        buffer = initText;
        visibileTextIndex[0] = 0;
        visibileTextIndex[1] = buffer.size();
        text.setString(buffer);
        setWriteLine();
    }
    setIndexFromCursor(x, y);
    frame = 0;
    buffer.reserve(maxLength);
    window.draw(*this);
    window.display();
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        if (Hittable::hit(event.mouseButton.x, event.mouseButton.y)) {
                            setIndexFromCursor(event.mouseButton.x, event.mouseButton.y);
                        }
                        else {
                            mouse = {event.mouseButton.x, event.mouseButton.y};
                            resetBox(window);
                            return buffer;
                        }
                    }
                    break;
                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::Right:
                            modifyIndex(+1);
                            break;
                        case sf::Keyboard::Left:
                            modifyIndex(-1);
                            break;
                        case sf::Keyboard::Enter:
                            enter = true;
                            resetBox(window);
                            return buffer;

                        case sf::Keyboard::BackSpace:
                            if (cursorIndex != 0) {
                                buffer.erase(buffer.begin() + cursorIndex - 1);
                                modifyIndex(-1);
                                slideWindowBackwards();
                                if (buffer.size() == 0) {
                                    hasInput = false;
                                }
                            }
                            break;
                    }
                    break;
                case sf::Event::TextEntered:
                    if (event.text.unicode < 128) {
                        internalType(event.text.unicode);
                    }
                    break;

            }
        }
        window.draw(*this);
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FLICKER_WRITELINE_PER_FRAMES));
    }
    // window was closed, program is dead
    return "";
}

std::string TextBox::externalType(sf::RenderWindow& window, const char ch) {
    internalType(ch);
    return getDataFromInput(window, window.getSize().x, 0);
}

void TextBox::internalType(const char ch) {
    if (buffer.size() < maxLength && isprint(ch) && (alphabet.empty() || alphabet.contains(ch))) {
        buffer.insert(buffer.begin() + cursorIndex, static_cast<char>(ch));
        modifyIndex(+1);
        slideWindowForwards();
        hasInput = true;
    }
}

std::string TextBox::getData() const {
    return buffer;
}

void TextBox::setAlphabet(const std::unordered_set<char>& alph) {
    alphabet = alph;
}

void TextBox::setWriteLine() {
    float x = text.findCharacterPos(cursorIndex - visibileTextIndex[0]).x;
    writeLine[0].position.x = writeLine[1].position.x = x;
}

// call with -1 or 1 only
void TextBox::modifyIndex(int offset) {
    frame = 0;
    if (cursorIndex + static_cast<size_t>(offset) > buffer.size()) {
        return;
    }
    if (offset < 0 && cursorIndex - visibileTextIndex[0] == 1 && cursorIndex != 1) {
        slideWindowBackwards();
    }
    if (offset > 0 && cursorIndex == visibileTextIndex[1]) {
        slideWindowForwards();
    }
    cursorIndex += offset;
    setWriteLine();
}

void TextBox::slideWindowBackwards() {
    setText();
    if (visibileTextIndex[0] != 0) {
        visibileTextIndex[0] -= 1;
        visibileTextIndex[1] = buffer.size();
        setText();
        while (text.getGlobalBounds().width > shape.getSize().x - 50) {
            visibileTextIndex[1] -= 1;
            setText();
        }
    }
    else {
        visibileTextIndex[1] = buffer.size();
        setText();
        while (text.getGlobalBounds().width > shape.getSize().x - 50) {
            visibileTextIndex[1] -= 1;
            setText();
        }
    }
    setWriteLine();
}

void TextBox::slideWindowForwards() {
    setText();
    if (visibileTextIndex[1] == buffer.size()) {
        return;
    }
    if (visibileTextIndex[1] == cursorIndex) {
        visibileTextIndex[1]++;
        setText();
        while (text.getGlobalBounds().width > shape.getSize().x - 50) {
            visibileTextIndex[0] += 1;
            setText();
        }
    }
    else {
        visibileTextIndex[1]++;
        setText();
        while (text.getGlobalBounds().width > shape.getSize().x - 50) {
            visibileTextIndex[1] -= 1;
            setText();
        }
    }
    setWriteLine();
}

void TextBox::setIndexFromCursor(int x, int y) {
    if (text.getString().getSize() == 0) {
        cursorIndex = 0;
        setWriteLine();
        return;
    }
    // should check if y is close enough too
    frame = 0;
    for (size_t i = 0; i <= text.getString().getSize(); ++i) {
        float charWidth = text.getFont()->getGlyph(text.getString()[i], text.getCharacterSize(), false).bounds.width;
        float iPosX = text.findCharacterPos(i + 1).x + getPosition().x - getOrigin().x - charWidth / 2;
        if (iPosX > x) {
            cursorIndex = visibileTextIndex[0] + i;
            break;
        }
    }
    setWriteLine();
}

void TextBox::setText() {
    if (!isprint(coverChar)) {
        text.setString(buffer.substr(visibileTextIndex[0], visibileTextIndex[1]  - visibileTextIndex[0]));
    }
    else {
        text.setString(std::string(visibileTextIndex[1] - visibileTextIndex[0], coverChar));
    }
}

void TextBox::resetBox(sf::RenderWindow& window) {
    writeLine[0].position.x = writeLine[1].position.x = -10000;
    if (buffer.size() == 0) {
        text.setString(noInputText);
        text.setFillColor(noInputColor);
    }
    window.draw(*this);
}

bool TextBox::pressedEnter() {
    if (enter) {
        enter = false;
        return true;
    }
    return false;
}

sf::Vector2i TextBox::lastMouseInput() {
    if (mouse.x != -1) {
        int x = mouse.x, y = mouse.y;
        mouse = {-1, -1};
        return {x, y};
    }
    return {-1, -1};
}