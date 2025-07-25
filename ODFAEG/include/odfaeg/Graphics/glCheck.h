////////////////////////////////////////////////////////////
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

#ifndef ODFAEG_GLCHECK_HPP
#define ODFAEG_GLCHECK_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/config.hpp"
#include <string>
#include "export.hpp"

namespace odfaeg {
    namespace graphic {
        namespace priv
        {
            ////////////////////////////////////////////////////////////
            /// Let's define a macro to quickly check every OpenGL API calls
            ////////////////////////////////////////////////////////////
            #ifdef ODFAEG_DEBUG
                // In debug mode, perform a test on every OpenGL call
                #define glCheck(call) ((call), odfaeg::graphic::priv::glCheckError(__FILE__, __LINE__))
                //#define glCheck(call) (call)
            #else
                // Else, we don't add any overhead
                #define glCheck(call) (call)

            #endif

            ////////////////////////////////////////////////////////////
            /// \brief Check the last OpenGL error
            ///
            /// \param file Source file where the call is located
            /// \param line Line number of the source file where the call is located
            ///
            ////////////////////////////////////////////////////////////
            ODFAEG_GRAPHICS_API void glCheckError(const char* file, unsigned int line);

            ////////////////////////////////////////////////////////////
            /// \brief Make sure that GLEW is initialized
            ///
            ////////////////////////////////////////////////////////////
            ODFAEG_GRAPHICS_API void ensureGlewInit();
        }

    } // namespace priv

} // namespace sf



#endif // ODFAEG_GLCHECK_HPP
