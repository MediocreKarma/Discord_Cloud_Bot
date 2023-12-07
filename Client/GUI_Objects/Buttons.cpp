#include "Buttons.hpp"

//////////////////////////////////////////////////
///
/// Implementation for RoundedRectangleTextShape
///
//////////////////////////////////////////////////

RoundedRectangleTextShape::RoundedRectangleTextShape(const sf::RoundedRectangleShape& _shape, const sf::Text& _text) :
    text(_text), shape(_shape) {
    const float DEFAULT_OFFSET = 50;
    text.setOrigin(
        text.getGlobalBounds().width / 2 + text.getLocalBounds().left - shape.getSize().x / 2,
        text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2
    );
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
    text.setString(_text);
    text.setOrigin(
        text.getGlobalBounds().width / 2 + text.getLocalBounds().left - shape.getSize().x / 2,
        text.getGlobalBounds().height / 2 + text.getLocalBounds().top - shape.getSize().y / 2
    );
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

//////////////////////////////////////////////////
///
/// Implementation for RoundedRectangleButton
///
//////////////////////////////////////////////////

RoundedRectangleButton::RoundedRectangleButton(const sf::RoundedRectangleShape& shape, const sf::Text& text) :
    RoundedRectangleTextShape(shape, text) {}

bool RoundedRectangleButton::hit(sf::Vector2f click) const {
    sf::FloatRect fr = shape.getGlobalBounds();
    fr.width *= getScale().x;
    fr.height *= getScale().y;
    return fr.contains(click - getPosition() + getOrigin());
}

bool RoundedRectangleButton::hit(int x, int y) const {
    return hit(sf::Vector2f({1.f * x, 1.f * y}));
}

CircularButton::CircularButton(const sf::CircleShape& base) : CircleShape(base) {}

bool CircularButton::hit(sf::Vector2f click) const {
    sf::FloatRect fr = getGlobalBounds();
    fr.width *= getScale().x;
    fr.height *= getScale().y;
    return fr.contains(click - getPosition() + getOrigin());
}

bool CircularButton::hit(int x, int y) const {
    return hit(sf::Vector2f({1.f * x, 1.f * y}));
}