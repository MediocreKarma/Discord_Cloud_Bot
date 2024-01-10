#ifndef _BUTTONS__
#define _BUTTONS__

#include <SFML/Graphics.hpp>
#include "RoundedRectangleShape.hpp"
#include "utils.hpp"
#include <string>
#include <array>

class RoundedRectangleTextShape : public sf::Drawable, public sf::Transformable {
public:
    RoundedRectangleTextShape() = default;
    RoundedRectangleTextShape(const sf::RoundedRectangleShape& shape, const sf::Text& text, bool leftAlign = false);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    virtual size_t getPointCount() const;
    virtual sf::Vector2f getPoint(size_t index) const;
    sf::Vector2f getSize() const;

    void replaceShape(const sf::RoundedRectangleShape& shape);

    void setShapeFillColor(sf::Color color);
    void setShapeOutlineColor(sf::Color color);
    void setTextFillColor(sf::Color color);
    void setTextOutlineColor(sf::Color color);

    void setFont(const sf::Font& font);
    void setString(const std::string& text);
    void setStyle(sf::Uint32 underlined);
    std::string getString() const;

    sf::Color getShapeFillColor() const;
    sf::Color getShapeOutlineColor() const;
    sf::Color getTextFillColor() const;
    sf::Color getTextOutlineColor() const;

protected:
    sf::Text text;
    mutable sf::RoundedRectangleShape shape;
    bool leftAlign;
};

class Hittable {
public:
    Hittable() {}
    virtual bool hit(sf::Vector2f click) const = 0;
    virtual bool hit(int x, int y) const {
        return hit(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));
    };
};

class RoundedRectangleButton : public sf::RoundedRectangleShape, public Hittable {
public:
    RoundedRectangleButton() = default;
    RoundedRectangleButton(sf::Vector2f size, float degree = 0, unsigned int points = 1);
    virtual bool hit(sf::Vector2f click) const;
};

class RoundedRectangleTextButton : public RoundedRectangleTextShape, public Hittable {
public:
    RoundedRectangleTextButton() = default;
    RoundedRectangleTextButton(const sf::RoundedRectangleShape& shape, const sf::Text& text, bool leftAlign = false);
    virtual bool hit(sf::Vector2f click) const;
};

class CircularButton : public sf::CircleShape, public Hittable {
public:
    CircularButton(const sf::CircleShape& shape);
    virtual bool hit(sf::Vector2f click) const;
};

#endif



// class RectangleButton : public Hittable, public RectangleLabel {
// public:
//     RectangleButton(sf::Vector2f size, sf::Vector2f offset = {0., 0.});

//     virtual void draw();
//     virtual bool hit() = 0;

// private:
//     std::string text;
// };

// class TextButton : public RectangleButton {
// public:
//     TextButton(sf::Vector2f size, sf::Vector2f offset = {0, 0});

//     virtual void draw();
//     virtual bool hit();

// private:
//     sf::RectangleShape body;
// };

// class RoundedRectangleButton : public Hittable {
// public:
//     RoundedRectangleButton(sf::Vector2f size, float radius, unsigned int cornerPointCount, sf::Vector2f offset = {0, 0});
// };

// class CircularButton : public Hittable {
// public:
//     CircularButton(int r, sf::Vector2f offset = {0, 0});

//     virtual void draw();
//     virtual bool hit();

// private:
//     sf::CircleShape body;
// };

// template<size_t N>
// class DropdownButton : TextButton {
// public:
//     DropdownButton(sf::Vector2f size, sf::Vector2f offset = {0, 0})
//     virtual void draw();
//     virtual bool hit();
    
//     size_t hitButton() const;

// private:
//     std::array<RectangleButton, N> list;
//     size_t lastHit;
// };