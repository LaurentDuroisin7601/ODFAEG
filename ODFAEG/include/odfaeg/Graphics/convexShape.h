////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the ODFAEG
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the ODFAEG library.
//
// ODFAEG - Simple and Fast Multimedia Library
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

#ifndef ODFAEG_CONVEX_SHAPE_HPP
#define ODFAEG_CONVEX_SHAPE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "shape.h"
#include <vector>

namespace odfaeg
{
    namespace graphic {

        ////////////////////////////////////////////////////////////
        /// \brief Specialized shape representing a convex polygon
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API ConvexShape : public Shape
        {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// \param pointCount Number of points of the polygon
            ///
            ////////////////////////////////////////////////////////////
            explicit ConvexShape(unsigned int pointCount = 0);
            void onScale(math::Vec3f& s);
            ////////////////////////////////////////////////////////////
            /// \brief Set the number of points of the polygon
            ///
            /// \a count must be greater than 2 to define a valid shape.
            ///
            /// \param count New number of points of the polygon
            ///
            /// \see getPointCount
            ///
            ////////////////////////////////////////////////////////////
            void setPointCount(unsigned int count);

            ////////////////////////////////////////////////////////////
            /// \brief Get the number of points of the polygon
            ///
            /// \return Number of points of the polygon
            ///
            /// \see setPointCount
            ///
            ////////////////////////////////////////////////////////////
            virtual unsigned int getPointCount() const;

            ////////////////////////////////////////////////////////////
            /// \brief Set the position of a point
            ///
            /// Don't forget that the polygon must remain convex, and
            /// the points need to stay ordered!
            /// setPointCount must be called first in order to set the total
            /// number of points. The result is undefined if \a index is out
            /// of the valid range.
            ///
            /// \param index Index of the point to change, in range [0 .. getPointCount() - 1]
            /// \param point New position of the point
            ///
            /// \see getPoint
            ///
            ////////////////////////////////////////////////////////////
            void setPoint(unsigned int index, const math::Vec3f& point);

            ////////////////////////////////////////////////////////////
            /// \brief Get the position of a point
            ///
            /// The result is undefined if \a index is out of the valid range.
            ///
            /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
            ///
            /// \return Position of the index-th point of the polygon
            ///
            /// \see setPoint
            ///
            ////////////////////////////////////////////////////////////
            virtual math::Vec3f getPoint(unsigned int index) const;

        private :

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::vector<math::Vec3f> m_points; ///< Points composing the convex polygon
        };
    }

} // namespace sf

#endif // ODFAEG_CONVEXSHAPE_HPP


////////////////////////////////////////////////////////////
/// \class sf::ConvexShape
/// \ingroup graphics
///
/// This class inherits all the functions of sf::Transformable
/// (position, rotation, scale, bounds, ...) as well as the
/// functions of sf::Shape (outline, color, texture, ...).
///
/// It is important to keep in mind that a convex shape must
/// always be... convex, otherwise it may not be drawn correctly.
/// Moreover, the points must be defined in order; using a random
/// order would result in an incorrect shape.
///
/// Usage example:
/// \code
/// sf::ConvexShape polygon;
/// polygon.setPointCount(3);
/// polygon.setPoint(0, math::Vec2f(0, 0));
/// polygon.setPoint(1, math::Vec2f(0, 10));
/// polygon.setPoint(2, math::Vec2f(25, 5));
/// polygon.setOutlineColor(Color::Red);
/// polygon.setOutlineThickness(5);
/// polygon.setPosition(10, 20);
/// ...
/// window.draw(polygon);
/// \endcode
///
/// \see sf::Shape, sf::RectangleShape, sf::CircleShape
///
////////////////////////////////////////////////////////////
