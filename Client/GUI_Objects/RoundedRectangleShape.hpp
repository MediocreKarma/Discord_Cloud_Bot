////////////////////////////////////////////////////////////
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
// you must not claim that you wrote the original software.
// If you use this software in a product, an acknowledgment
// in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
// and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef ROUNDEDRECTANGLESHAPE_HPP
#define ROUNDEDRECTANGLESHAPE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Shape.hpp>

namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Specialized shape representing a rectangle
/// with rounded corners
////////////////////////////////////////////////////////////
class RoundedRectangleShape : public sf::Shape
{
    public:
        ////////////////////////////////////////////////////////////
        /// \brief Default constructor
        ///
        /// \param size Size of the rectangle
        /// \param radius Radius for each rounded corner
        /// \param cornerPointCount Number of points of each corner
        ///
        ////////////////////////////////////////////////////////////
        /// The following constructor had a default parameter 
        /// modified by Chirvasa Matei
        ////////////////////////////////////////////////////////////
        explicit RoundedRectangleShape(const Vector2f& size = Vector2f(0, 0), float radius = 0, unsigned int cornerPointCount = 1);

        ////////////////////////////////////////////////////////////
        /// \brief Set the size of the rounded rectangle
        ///
        /// \param size New size of the rounded rectangle
        ///
        /// \see getSize
        ///
        ////////////////////////////////////////////////////////////
        void setSize(const Vector2f& size);

        ////////////////////////////////////////////////////////////
        /// \brief Get the size of the rounded rectangle
        ///
        /// \return Size of the rounded rectangle
        ///
        /// \see setSize
        ///
        ////////////////////////////////////////////////////////////
        const Vector2f& getSize() const;

        ////////////////////////////////////////////////////////////
        /// \brief Set the radius of the rounded corners
        ///
        /// \param radius Radius of the rounded corners
        ///
        /// \see getCornersRadius
        ///
        ////////////////////////////////////////////////////////////
        void setCornersRadius(float radius);

        ////////////////////////////////////////////////////////////
        /// \brief Get the radius of the rounded corners
        ///
        /// \return Radius of the rounded corners
        ///
        /// \see setCornersRadius
        ///
        ////////////////////////////////////////////////////////////
        float getCornersRadius() const;

        ////////////////////////////////////////////////////////////
        /// \brief Set the number of points of each corner
        ///
        /// \param count New number of points of the rounded rectangle
        ///
        /// \see getPointCount
        ///
        ////////////////////////////////////////////////////////////
        void setCornerPointCount(unsigned int count);

        ////////////////////////////////////////////////////////////
        /// \brief Get the number of points defining the rounded rectangle
        ///
        /// \return Number of points of the rounded rectangle
        ///
        ////////////////////////////////////////////////////////////
        virtual std::size_t getPointCount() const;

        ////////////////////////////////////////////////////////////
        /// \brief Get a point of the rounded rectangle
        ///
        /// The result is undefined if \a index is out of the valid range.
        ///
        /// \param index Index of the point to get, in range [0 .. GetPointCount() - 1]
        ///
        /// \return Index-th point of the shape
        ///
        ////////////////////////////////////////////////////////////
        virtual sf::Vector2f getPoint(std::size_t index) const;

    private:
         ////////////////////////////////////////////////////////////
        // Member data
        ////////////////////////////////////////////////////////////
        Vector2f mySize;
        float myRadius;
        unsigned int myCornerPointCount;
};
}
#endif // ROUNDEDRECTANGLESHAPE_HPP

////////////////////////////////////////////////////////////
/// \class sf::RoundedRectangleShape
/// \ingroup graphics
///
/// This class inherits all the functions of sf::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of sf::Shape (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::RoundedRectangleShape roundedRectangle;
/// rectangle.setSize(sf::Vector2f(100, 50));
/// rectangle.setCornersRadius(5);
/// rectangle.setOutlineThickness(5);
/// rectangle.setPosition(10, 20);
/// ...
/// window.draw(rectangle);
/// \endcode
///
/// \see sf::Shape, sf::CircleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////