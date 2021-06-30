////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the SFML
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the SFML library.
//
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2013 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef ODFAEG_RECTANGLE_SHAPE_2D_HPP
#define ODFAEG_RECTANGLE_SHAPE_2D_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "export.hpp"
#include "shape.h"


namespace odfaeg
{
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Specialized shape representing a rectangle
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API RectangleShape : public Shape
        {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// \param size Size of the rectangle
            ///
            ////////////////////////////////////////////////////////////
            explicit RectangleShape(const math::Vec3f& size = math::Vec3f(0, 0, 0));
            //void onScale(Vec3f& s);
            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the rectangle
            ///
            /// \return Size of the rectangle
            ///
            /// \see setSize
            ///
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            /// \brief Get the number of points defining the shape
            ///
            /// \return Number of points of the shape
            ///
            ////////////////////////////////////////////////////////////
            virtual unsigned int getPointCount() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get a point of the shape
            ///
            /// The result is undefined if \a index is out of the valid range.
            ///
            /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
            ///
            /// \return Index-th point of the shape
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector3f getPoint(unsigned int index) const;
            template <typename Archive>
            void vtserialize(Archive& ar) {
                Shape::vtserialize(ar);
                ar(m_size);
            }
            void onResize(math::Vec3f& scale);
            void onScale(math::Vec3f& scale);
        private :

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            math::Vec3f m_size; ///< Size of the rectangle
        };
    }

} // namespace sf


#endif // SFML_RECTANGLESHAPE_HPP


////////////////////////////////////////////////////////////
/// \class sf::RectangleShape
/// \ingroup graphics
///
/// This class inherits all the functions of sf::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of sf::Shape (outline, color, texture, ...).
///
/// Usage example:
/// \code
/// sf::RectangleShape rectangle;
/// rectangle.setSize(sf::Vector2f(100, 50));
/// rectangle.setOutlineColor(sf::Color::Red);
/// rectangle.setOutlineThickness(5);
/// rectangle.setPosition(10, 20);
/// ...
/// window.draw(rectangle);
/// \endcode
///
/// \see sf::Shape, sf::CircleShape, sf::ConvexShape
///
////////////////////////////////////////////////////////////

