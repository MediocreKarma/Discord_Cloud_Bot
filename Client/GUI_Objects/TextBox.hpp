#ifndef _TEXT_BOX__
#define _TEXT_BOX__

#include "Buttons.hpp"
#include <chrono>
#include <thread>
#include <string>
#include <unordered_set>

class TextBox : public RoundedRectangleTextButton {
public:
    TextBox() = default;
    TextBox(
        const sf::RoundedRectangleShape& shape, 
        const sf::Font& font, 
        const std::string& displayString = "",
        unsigned int charSize = 30,
        sf::Color displayColor = Colors::DarkGray,
        bool alwaysHasInput = false,
        char coverChar = '\0',
        size_t maxLength = 256
    );

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    bool hasUserInput() const;
    std::string getDataFromInput(sf::RenderWindow& window, int x, int y, const std::string& initText = "");
    void setDisplayString(const std::string& str);
    bool pressedEnter();
    sf::Vector2i lastMouseInput();
    void setFont(const sf::Font& font);
    std::string externalType(sf::RenderWindow& window, char ch);
    std::string getData() const;
    bool reqRefresh();

    void setAlphabet(const std::unordered_set<char>& alphabet);

private:
    static constexpr size_t FLICKER_WRITELINE_PER_FRAMES = 30;

    std::string buffer;
    std::string noInputText;
    sf::Color noInputColor;
    std::array<sf::Vertex, 2> writeLine;
    size_t cursorIndex;
    std::array<size_t, 2> visibileTextIndex;
    bool hasInput;
    bool alwaysHasInput;
    char coverChar;
    mutable size_t frame;
    bool enter;
    sf::Vector2i mouse;
    size_t maxLength;
    std::unordered_set<char> alphabet;
    bool requestRefresh = false;

    void setWriteLine();
    void modifyIndex(int offset);
    void slideWindowBackwards();
    void slideWindowForwards();
    void setIndexFromCursor(int x, int y);
    void resetBox(sf::RenderWindow& window);
    void setText();
    void internalType(char ch);
};

#endif 