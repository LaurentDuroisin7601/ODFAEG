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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/Graphics/glExtensions.hpp"
#include "../../../include/odfaeg/Window/context.hpp"
#include <iostream>
#if !defined(GL_MAJOR_VERSION)
    #define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
    #define GL_MINOR_VERSION 0x821C
#endif
namespace odfaeg
{
namespace graphic {
namespace priv
{
////////////////////////////////////////////////////////////
void ensureExtensionsInit()
{
#if !defined(ODFAEG_OPENGL_ES)
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;

        sfogl_LoadFunctions();

        // Retrieve the context version number
        int majorVersion = 0;
        int minorVersion = 0;

        // Try the new way first
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

        if (glGetError() == GL_INVALID_ENUM)
        {
            // Try the old way
            const GLubyte* version = glGetString(GL_VERSION);
            if (version)
            {
                // The beginning of the returned string is "major.minor" (this is standard)
                majorVersion = version[0] - '0';
                minorVersion = version[2] - '0';
            }
            else
            {
                // Can't get the version number, assume 1.1
                majorVersion = 1;
                minorVersion = 1;
            }
        }

        if ((majorVersion < 1) || ((majorVersion == 1) && (minorVersion < 1)))
        {
            std::cerr<< "sfml-graphics requires support for OpenGL 1.1 or greater" << std::endl;
            std::cerr << "Ensure that hardware acceleration is enabled if available" << std::endl;
        }
    }
#endif
}

} // namespace priv
}
} // namespace sf
