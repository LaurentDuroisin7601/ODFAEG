////////////////////////////////////////////////////////////
//
// ODFAEG - Simple and Fast Multimedia Library
// Copyright (C) 2007-2014 Laurent Gomila (laurent.gom@gmail.com)
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

#ifndef ODFAEG_GLYPH_HPP
#define ODFAEG_GLYPH_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "export.hpp"
#include "rect.hpp"


namespace odfaeg
{
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Structure describing a glyph
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API Glyph
        {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            ////////////////////////////////////////////////////////////
            Glyph() : advance(0) {}

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            int     advance;     ///< Offset to move horizontically to the next character
            FloatRect bounds;      ///< Bounding rectangle of the glyph, in coordinates relative to the baseline
            IntRect textureRect; ///< Texture coordinates of the glyph inside the font's texture
        };
    }

} // namespace sf


#endif // ODFAEG_GLYPH_HPP


////////////////////////////////////////////////////////////
/// \class sf::Glyph
/// \ingroup graphics
///
/// A glyph is the visual representation of a character.
///
/// The sf::Glyph structure provides the information needed
/// to handle the glyph:
/// \li its coordinates in the font's texture
/// \li its bounding rectangle
/// \li the offset to apply to get the starting position of the next glyph
///
/// \see sf::Font
///
////////////////////////////////////////////////////////////

