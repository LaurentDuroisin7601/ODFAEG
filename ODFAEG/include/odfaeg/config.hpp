////////////////////////////////////////////////////////////
//
// ODFAEG - Simple and Fast Game Library
// Copyright (C) 2007-2013 Laurent Duroisin (laurentduroisin@msn.com)
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
#ifndef ODFAEG_CONFIG_HPP
#define ODFAEG_CONFIG_HPP
//#define SFML
//#define VULKAN
////////////////////////////////////////////////////////////
// Define the ODFAEG version
////////////////////////////////////////////////////////////
#define ODFAEG_VERSION_MAJOR 1
#define ODFAEG_VERSION_MINOR 0


////////////////////////////////////////////////////////////
// Identify the operating system
////////////////////////////////////////////////////////////
#if defined(_WIN32) || defined(__WIN32__)

    // Windows
    #define ODFAEG_SYSTEM_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

#elif defined(linux) || defined(__linux)

    // Linux
    #define ODFAEG_SYSTEM_LINUX

#elif defined(__APPLE__) || defined(MACOSX) || defined(macintosh) || defined(Macintosh)

    // MacOS
    #define ODFAEG_SYSTEM_MACOS

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)

    // FreeBSD
    #define ODFAEG_SYSTEM_FREEBSD

#else

    // Unsupported system
    #error This operating system is not supported by ODFAEG library

#endif


////////////////////////////////////////////////////////////
// Define a portable debug macro
////////////////////////////////////////////////////////////
#if !defined(NDEBUG)

    #define ODFAEG_DEBUG

#endif


////////////////////////////////////////////////////////////
// Define helpers to create portable import / export macros for each module
////////////////////////////////////////////////////////////
#if !defined(ODFAEG_STATIC)

    #if defined(ODFAEG_SYSTEM_WINDOWS)

        // Windows compilers need specific (and different) keywords for export and import
        #define ODFAEG_API_EXPORT __declspec(dllexport)
        #define ODFAEG_API_IMPORT __declspec(dllimport)

        // For Visual C++ compilers, we also need to turn off this annoying C4251 warning
        #ifdef _MSC_VER

            #pragma warning(disable : 4251)

        #endif

    #else // Linux, FreeBSD, Mac OS X

        #if __GNUC__ >= 4

            // GCC 4 has special keywords for showing/hidding symbols,
            // the same keyword is used for both importing and exporting
            #define ODFAEG_API_EXPORT __attribute__ ((__visibility__ ("default")))
            #define ODFAEG_API_IMPORT __attribute__ ((__visibility__ ("default")))

        #else

            // GCC < 4 has no mechanism to explicitely hide symbols, everything's exported
            #define ODFAEG_API_EXPORT
            #define ODFAEG_API_IMPORT

        #endif

    #endif

#else

    // Static build doesn't need import/export macros
    #define ODFAEG_API_EXPORT
    #define ODFAEG_API_IMPORT

#endif


#endif // ODFAEG_CONFIG_HPP
