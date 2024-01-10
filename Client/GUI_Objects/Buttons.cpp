#include "Buttons.hpp"

//////////////////////////////////////////////////
///
/// Implementation for RoundedRectangleTextShape
///
//////////////////////////////////////////////////

RoundedRectangleTextShape::RoundedRectangleTextShape(const sf::RoundedRectangleShape& _shape, const sf::Text& _text, bool _leftAlign) :
    text(), shape(_shape), leftAlign(_leftAlign) {
    text.setFont(*_text.getFont());
    setString(_text.getString());
    shape.setFillColor(Colors::Gray);
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color::Black);
    text.setFillColor(sf::Color::Black);
    text.setOutlineColor(sf::Color::Black);
}

void RoundedRectangleTextShape::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(shape, states);
    target.draw(text, states);
}

size_t RoundedRectangleTextShape::getPointCount() const {
    return shape.getPointCount();
}

sf::Vector2f RoundedRectangleTextShape::getPoint(const size_t index) const {
    return shape.getPoint(index);
}

void RoundedRectangleTextShape::setShapeFillColor(sf::Color color) {
    shape.setFillColor(color);
}

void RoundedRectangleTextShape::setShapeOutlineColor(sf::Color color) {
    shape.setOutlineColor(color);
}

void RoundedRectangleTextShape::setTextFillColor(sf::Color color) {
    text.setFillColor(color);
}

void RoundedRectangleTextShape::setTextOutlineColor(sf::Color color) {
    text.setFillColor(color);
}

void RoundedRectangleTextShape::setFont(const sf::Font& font) {
    text.setFont(font);
    text.setOrigin(
        text.getGlobalBounds().width / 2 + text.getLocalBounds().left - shape.getSize().x / 2,
        text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2
    );
}

void RoundedRectangleTextShape::setString(const std::string& _text) {
    const float DEFAULT_OFFSET = 50;
    text.setString(_text);
    sf::FloatRect frect = text.getGlobalBounds();
    if (_text.size() > 0 && frect.width > shape.getSize().x - DEFAULT_OFFSET) {
        size_t i = 1;
        while (text.getString().getSize() > 3 && frect.width > shape.getSize().x - DEFAULT_OFFSET) {
            text.setString(_text.substr(0, _text.size() - i) + "...");
            i++;
            frect = text.getGlobalBounds();
        }
    }
    if (leftAlign) {
        text.setOrigin(0, text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2);
        text.setPosition(
            DEFAULT_OFFSET / 2 - text.getLocalBounds().left,
            0
        );
    }
    else {
        text.setOrigin(
            text.getGlobalBounds().width / 2 + text.getLocalBounds().left - shape.getSize().x / 2,
            text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2
        );
    }
}

void RoundedRectangleTextShape::setStyle(const sf::Uint32 style) {
    text.setStyle(style);
}

std::string RoundedRectangleTextShape::getString() const {
    return text.getString();
}

sf::Color RoundedRectangleTextShape::getShapeFillColor() const {
    return shape.getFillColor();
}

sf::Color RoundedRectangleTextShape::getShapeOutlineColor() const {
    return shape.getOutlineColor();
}

sf::Color RoundedRectangleTextShape::getTextFillColor() const {
    return text.getFillColor();
}

sf::Color RoundedRectangleTextShape::getTextOutlineColor() const {
    return text.getOutlineColor();
}

sf::Vector2f RoundedRectangleTextShape::getSize() const {
    return shape.getSize();
}

void RoundedRectangleTextShape::replaceShape(const sf::RoundedRectangleShape& _shape) {
    sf::Color cols[2] = {
        shape.getFillColor(),
        shape.getOutlineColor()
    };
    float thickness = shape.getOutlineThickness();
    shape = _shape;
    shape.setFillColor(cols[0]);
    shape.setOutlineColor(cols[1]);
    shape.setOutlineThickness(thickness);
    // reinitialise basically
    setString(text.getString().toAnsiString());
}

//////////////////////////////////////////////////
///
/// Implementation for RoundedRectangleButton
///
//////////////////////////////////////////////////

RoundedRectangleButton::RoundedRectangleButton(sf::Vector2f size, float degree, unsigned int points) :
    sf::RoundedRectangleShape(size, degree, points) {}

bool RoundedRectangleButton::hit(const sf::Vector2f click) const {
    sf::FloatRect fr = getGlobalBounds();
    return fr.contains(click);
}

//////////////////////////////////////////////////
///
/// Implementation for RoundedRectangleTextButton
///
//////////////////////////////////////////////////

RoundedRectangleTextButton::RoundedRectangleTextButton(const sf::RoundedRectangleShape& shape, const sf::Text& text, bool leftAlign) :
    RoundedRectangleTextShape(shape, text, leftAlign) {}

bool RoundedRectangleTextButton::hit(const sf::Vector2f click) const {
    sf::FloatRect fr = shape.getGlobalBounds();
    fr.width *= getScale().x;
    fr.height *= getScale().y;
    return fr.contains(click - getPosition() + getOrigin());
}


//////////////////////////////////////////////////
///
/// Implementation for CircularButton
///
//////////////////////////////////////////////////

CircularButton::CircularButton(const sf::CircleShape& base) : CircleShape(base) {}

bool CircularButton::hit(const sf::Vector2f click) const {
    sf::FloatRect fr = getGlobalBounds();
    return fr.contains(click);
}
