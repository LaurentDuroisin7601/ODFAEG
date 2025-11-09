////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "config.hpp"


////////////////////////////////////////////////////////////
/// This file just includes the OpenGL headers,
/// which have actually different paths on each system
////////////////////////////////////////////////////////////
#if defined(ODFAEG_SYSTEM_WINDOWS)

    // The Visual C++ version of gl.h uses WINGDIAPI and APIENTRY but doesn't define them
    #ifdef _MSC_VER
        #include <windows.h>
    #endif

    #include <GL/gl.h>

#elif defined(ODFAEG_SYSTEM_LINUX) || defined(ODFAEG_SYSTEM_FREEBSD) || defined(ODFAEG_SYSTEM_OPENBSD)

    #if defined(ODFAEG_OPENGL_ES)
        #include <GLES/gl.h>
        #include <GLES/glext.h>
    #else
        #include <GL/gl.h>
    #endif

#elif defined(ODFAEG_SYSTEM_MACOS)

    #include <OpenGL/gl.h>

#elif defined (ODFAEG_SYSTEM_IOS)

    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>

#elif defined (ODFAEG_SYSTEM_ANDROID)

    #include <GLES/gl.h>
    #include <GLES/glext.h>

    // We're not using OpenGL ES 2+ yet, but we can use the sRGB extension
    #include <GLES2/gl2platform.h>
    #include <GLES2/gl2ext.h>

#endif
