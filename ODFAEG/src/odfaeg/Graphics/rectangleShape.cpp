////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/Graphics/rectangleShape.h"
#include <cmath>


namespace odfaeg
{
    namespace graphic {

        using namespace sf;
        ////////////////////////////////////////////////////////////
        RectangleShape::RectangleShape(const math::Vec3f& size)
        {
            setSize(size);
            m_size = size;
            update();
        }


        ////////////////////////////////////////////////////////////
        void RectangleShape::onResize(math::Vec3f& scale)
        {
           m_size = getSize();
           getTransform().setScale(math::Vec3f(1, 1, 1));
           update();
        }
        ////////////////////////////////////////////////////////////
        void RectangleShape::onScale(math::Vec3f& scale)
        {
           m_size = getSize();
           getTransform().setScale(math::Vec3f(1, 1, 1));
           update();
        }

        ////////////////////////////////////////////////////////////
        unsigned int RectangleShape::getPointCount() const
        {
            return 4;
        }


        ////////////////////////////////////////////////////////////
        Vector3f RectangleShape::getPoint(unsigned int index) const
        {
            switch (index)
            {
                default:
                case 0: return Vector3f(0, 0, 0);
                case 1: return Vector3f(m_size.x, 0, 0);
                case 2: return Vector3f(m_size.x, m_size.y, m_size.z);
                case 3: return Vector3f(0, m_size.y, m_size.z);
            }
        }
    }
} // namespace sf

