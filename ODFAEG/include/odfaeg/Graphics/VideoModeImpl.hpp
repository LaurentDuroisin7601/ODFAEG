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

#ifndef ODFAEG_VIDEOMODEIMPL_HPP
#define ODFAEG_VIDEOMODEIMPL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <ODFAEG/Window/VideoMode.hpp>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief OS-specific implementation of video modes functions
///
////////////////////////////////////////////////////////////
class ODFAEG_GRAPHICS_API VideoModeImpl
{
public :

    ////////////////////////////////////////////////////////////
    /// \brief Get the list of all the supported fullscreen video modes
    ///
    /// \return Array filled with the fullscreen video modes
    ///
    ////////////////////////////////////////////////////////////
    static std::vector<VideoMode> getFullscreenModes();

    ////////////////////////////////////////////////////////////
    /// \brief Get the current desktop video mode
    ///
    /// \return Current desktop video mode
    ///
    ////////////////////////////////////////////////////////////
    static VideoMode getDesktopMode();
};

} // namespace priv

} // namespace sf


#endif // ODFAEG_VIDEOMODEIMPL_HPP
