////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the SFML
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the SFML library.
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

#ifndef ODFAEG_CIRCLE_SHAPE_2D_HPP
#define ODFAEG_CIRCLE_SHAPE_2D_HPP

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
        class ODFAEG_GRAPHICS_API CircleShape : public Shape
        {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// \param size Size of the rectangle
            ///
            ////////////////////////////////////////////////////////////
            explicit CircleShape(float radius = 0, unsigned int pointCount = 30);
            ////////////////////////////////////////////////////////////
            /// \brief Set the radius of the circle
            ///
            /// \param radius New radius of the circle
            ///
            /// \see getRadius
            ///
            ////////////////////////////////////////////////////////////
            void setRadius(float radius);

            ////////////////////////////////////////////////////////////
            /// \brief Get the radius of the circle
            ///
            /// \return Radius of the circle
            ///
            /// \see setRadius
            ///
            ////////////////////////////////////////////////////////////
            float getRadius() const;

            ////////////////////////////////////////////////////////////
            /// \brief Set the number of points of the circle
            ///
            /// \param count New number of points of the circle
            ///
            /// \see getPointCount
            ///
            ////////////////////////////////////////////////////////////
            void setPointCount(unsigned int count);

            ////////////////////////////////////////////////////////////
            /// \brief Get the number of points of the circle
            ///
            /// \return Number of points of the circle
            ///
            /// \see setPointCount
            ///
            ////////////////////////////////////////////////////////////
            virtual unsigned int getPointCount() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get a point of the circle
            ///
            /// The returned point is in local coordinates, that is,
            /// the shape's transforms (position, rotation, scale) are
            /// not taken into account.
            /// The result is undefined if \a index is out of the valid range.
            ///
            /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
            ///
            /// \return index-th point of the shape
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector3f getPoint(unsigned int index) const;

        private :

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            float        m_radius;     ///< Radius of the circle
            unsigned int m_pointCount; ///< Number of points composing the circle
        };
    }

}


#endif // SFML_CIRCLSHAPE_HPP

//////////////////////////////////////////////////////

