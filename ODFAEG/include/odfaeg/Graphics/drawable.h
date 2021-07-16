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
#ifndef ODFAEG_DRAWABLE_2D_HPP
#define ODFAEG_DRAWABLE_2D_HPP
#include "renderStates.h"
namespace odfaeg {
    namespace graphic {

        class RenderTarget;

        class ODFAEG_GRAPHICS_API Drawable
        {
        public :

            virtual ~Drawable() {}
        protected :

            friend class RenderTarget;

            ////////////////////////////////////////////////////////////
            /// \brief Draw the object to a render target
            ///
            /// This is a pure virtual function that has to be implemented
            /// by the derived class to define how the drawable should be
            /// drawn.
            ///
            /// \param target Render target to draw to
            /// \param states Current render states
            ///
            ////////////////////////////////////////////////////////////
            virtual void draw(RenderTarget& target, RenderStates states) = 0;
        };
    }
}
#endif // DRAWABLE_3D
