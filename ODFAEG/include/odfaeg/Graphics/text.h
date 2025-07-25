////////////////////////////////////////////////////////////
//
// /!\ Important : this class is a modification of the circle shape class of the ODFAEG
// that I've adapted for odfaeg with 3D vertices.
// Here is the license and the author of the ODFAEG library.
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

#ifndef ODFAEG_TEXT_HPP
#define ODFAEG_TEXT_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "export.hpp"
#include "drawable.h"
#include "transformable.h"
#include "font.h"
#include "rect.hpp"
#include "vertexArray.h"
#include "../Core/string.hpp"
#include <string>
#include <vector>

namespace odfaeg
{
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Graphical text that can be drawn to a render target
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API Text : public Drawable, public Transformable
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Enumeration of the string drawing styles
            ///
            ////////////////////////////////////////////////////////////
            enum Style
            {
                Regular       = 0,      ///< Regular characters, no style
                Bold          = 1 << 0, ///< Bold characters
                Italic        = 1 << 1, ///< Italic characters
                Underlined    = 1 << 2, ///< Underlined characters
                StrikeThrough = 1 << 3  ///< Strike through characters
            };

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Creates an empty text.
            ///
            ////////////////////////////////////////////////////////////
            Text();

            ////////////////////////////////////////////////////////////
            /// \brief Construct the text from a string, font and size
            ///
            /// Note that if the used font is a bitmap font, it is not
            /// scalable, thus not all requested sizes will be available
            /// to use. This needs to be taken into consideration when
            /// setting the character size. If you need to display text
            /// of a certain size, make sure the corresponding bitmap
            /// font that supports that size is used.
            ///
            /// \param string         Text assigned to the string
            /// \param font           Font used to draw the string
            /// \param characterSize  Base size of characters, in pixels
            ///
            ////////////////////////////////////////////////////////////
            Text(const core::String& string, const Font& font, unsigned int characterSize = 30);

            ////////////////////////////////////////////////////////////
            /// \brief Set the text's string
            ///
            /// The \a string argument is a core::String, which can
            /// automatically be constructed from standard string types.
            /// So, the following calls are all valid:
            /// \code
            /// text.setString("hello");
            /// text.setString(L"hello");
            /// text.setString(std::string("hello"));
            /// text.setString(std::wstring(L"hello"));
            /// \endcode
            /// A text's string is empty by default.
            ///
            /// \param string New string
            ///
            /// \see getString
            ///
            ////////////////////////////////////////////////////////////
            void setString(const core::String& string);

            ////////////////////////////////////////////////////////////
            /// \brief Set the text's font
            ///
            /// The \a font argument refers to a font that must
            /// exist as long as the text uses it. Indeed, the text
            /// doesn't store its own copy of the font, but rather keeps
            /// a pointer to the one that you passed to this function.
            /// If the font is destroyed and the text tries to
            /// use it, the behavior is undefined.
            ///
            /// \param font New font
            ///
            /// \see getFont
            ///
            ////////////////////////////////////////////////////////////
            void setFont(const Font& font);

            ////////////////////////////////////////////////////////////
            /// \brief Set the character size
            ///
            /// The default size is 30.
            ///
            /// Note that if the used font is a bitmap font, it is not
            /// scalable, thus not all requested sizes will be available
            /// to use. This needs to be taken into consideration when
            /// setting the character size. If you need to display text
            /// of a certain size, make sure the corresponding bitmap
            /// font that supports that size is used.
            ///
            /// \param size New character size, in pixels
            ///
            /// \see getCharacterSize
            ///
            ////////////////////////////////////////////////////////////
            void setCharacterSize(unsigned int size);

            ////////////////////////////////////////////////////////////
            /// \brief Set the line spacing factor
            ///
            /// The default spacing between lines is defined by the font.
            /// This method enables you to set a factor for the spacing
            /// between lines. By default the line spacing factor is 1.
            ///
            /// \param spacingFactor New line spacing factor
            ///
            /// \see getLineSpacing
            ///
            ////////////////////////////////////////////////////////////
            void setLineSpacing(float spacingFactor);

            ////////////////////////////////////////////////////////////
            /// \brief Set the letter spacing factor
            ///
            /// The default spacing between letters is defined by the font.
            /// This factor doesn't directly apply to the existing
            /// spacing between each character, it rather adds a fixed
            /// space between them which is calculated from the font
            /// metrics and the character size.
            /// Note that factors below 1 (including negative numbers) bring
            /// characters closer to each other.
            /// By default the letter spacing factor is 1.
            ///
            /// \param spacingFactor New letter spacing factor
            ///
            /// \see getLetterSpacing
            ///
            ////////////////////////////////////////////////////////////
            void setLetterSpacing(float spacingFactor);

            ////////////////////////////////////////////////////////////
            /// \brief Set the text's style
            ///
            /// You can pass a combination of one or more styles, for
            /// example sf::Text::Bold | sf::Text::Italic.
            /// The default style is sf::Text::Regular.
            ///
            /// \param style New style
            ///
            /// \see getStyle
            ///
            ////////////////////////////////////////////////////////////
            void setStyle(std::uint32_t style);

            ////////////////////////////////////////////////////////////
            /// \brief Set the fill color of the text
            ///
            /// By default, the text's fill color is opaque white.
            /// Setting the fill color to a transparent color with an outline
            /// will cause the outline to be displayed in the fill area of the text.
            ///
            /// \param color New fill color of the text
            ///
            /// \see getFillColor
            ///
            /// \deprecated There is now fill and outline colors instead
            /// of a single global color.
            /// Use setFillColor() or setOutlineColor() instead.
            ///
            ////////////////////////////////////////////////////////////
            void setColor(const Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the fill color of the text
            ///
            /// By default, the text's fill color is opaque white.
            /// Setting the fill color to a transparent color with an outline
            /// will cause the outline to be displayed in the fill area of the text.
            ///
            /// \param color New fill color of the text
            ///
            /// \see getFillColor
            ///
            ////////////////////////////////////////////////////////////
            void setFillColor(const Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the outline color of the text
            ///
            /// By default, the text's outline color is opaque black.
            ///
            /// \param color New outline color of the text
            ///
            /// \see getOutlineColor
            ///
            ////////////////////////////////////////////////////////////
            void setOutlineColor(const Color& color);
            void setBackgroundColor(const Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the thickness of the text's outline
            ///
            /// By default, the outline thickness is 0.
            ///
            /// Be aware that using a negative value for the outline
            /// thickness will cause distorted rendering.
            ///
            /// \param thickness New outline thickness, in pixels
            ///
            /// \see getOutlineThickness
            ///
            ////////////////////////////////////////////////////////////
            void setOutlineThickness(float thickness);

            ////////////////////////////////////////////////////////////
            /// \brief Get the text's string
            ///
            /// The returned string is a core::String, which can automatically
            /// be converted to standard string types. So, the following
            /// lines of code are all valid:
            /// \code
            /// core::String   s1 = text.getString();
            /// std::string  s2 = text.getString();
            /// std::wstring s3 = text.getString();
            /// \endcode
            ///
            /// \return Text's string
            ///
            /// \see setString
            ///
            ////////////////////////////////////////////////////////////
            const core::String& getString() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the text's font
            ///
            /// If the text has no font attached, a NULL pointer is returned.
            /// The returned pointer is const, which means that you
            /// cannot modify the font when you get it from this function.
            ///
            /// \return Pointer to the text's font
            ///
            /// \see setFont
            ///
            ////////////////////////////////////////////////////////////
            const Font* getFont() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the character size
            ///
            /// \return Size of the characters, in pixels
            ///
            /// \see setCharacterSize
            ///
            ////////////////////////////////////////////////////////////
            unsigned int getCharacterSize() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the letter spacing factor
            ///
            /// \return Size of the letter spacing factor
            ///
            /// \see setLetterSpacing
            ///
            ////////////////////////////////////////////////////////////
            float getLetterSpacing() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the line spacing factor
            ///
            /// \return Size of the line spacing factor
            ///
            /// \see setLineSpacing
            ///
            ////////////////////////////////////////////////////////////
            float getLineSpacing() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the text's style
            ///
            /// \return Text's style
            ///
            /// \see setStyle
            ///
            ////////////////////////////////////////////////////////////
            std::uint32_t getStyle() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the fill color of the text
            ///
            /// \return Fill color of the text
            ///
            /// \see setFillColor
            ///
            /// \deprecated There is now fill and outline colors instead
            /// of a single global color.
            /// Use getFillColor() or getOutlineColor() instead.
            ///
            ////////////////////////////////////////////////////////////
            const Color& getColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the fill color of the text
            ///
            /// \return Fill color of the text
            ///
            /// \see setFillColor
            ///
            ////////////////////////////////////////////////////////////
            const Color& getFillColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the outline color of the text
            ///
            /// \return Outline color of the text
            ///
            /// \see setOutlineColor
            ///
            ////////////////////////////////////////////////////////////
            const Color& getOutlineColor() const;
            const Color& getBackgroundColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the outline thickness of the text
            ///
            /// \return Outline thickness of the text, in pixels
            ///
            /// \see setOutlineThickness
            ///
            ////////////////////////////////////////////////////////////
            float getOutlineThickness() const;

            ////////////////////////////////////////////////////////////
            /// \brief Return the position of the \a index-th character
            ///
            /// This function computes the visual position of a character
            /// from its index in the string. The returned position is
            /// in global coordinates (translation, rotation, scale and
            /// origin are applied).
            /// If \a index is out of range, the position of the end of
            /// the string is returned.
            ///
            /// \param index Index of the character
            ///
            /// \return Position of the character
            ///
            ////////////////////////////////////////////////////////////
            math::Vec2f findCharacterPos(std::size_t index);

            ////////////////////////////////////////////////////////////
            /// \brief Get the local bounding rectangle of the entity
            ///
            /// The returned rectangle is in local coordinates, which means
            /// that it ignores the transformations (translation, rotation,
            /// scale, ...) that are applied to the entity.
            /// In other words, this function returns the bounds of the
            /// entity in the entity's coordinate system.
            ///
            /// \return Local bounding rectangle of the entity
            ///
            ////////////////////////////////////////////////////////////
            physic::BoundingBox getLocalBounds() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the global bounding rectangle of the entity
            ///
            /// The returned rectangle is in global coordinates, which means
            /// that it takes into account the transformations (translation,
            /// rotation, scale, ...) that are applied to the entity.
            /// In other words, this function returns the bounds of the
            /// text in the global 2D world's coordinate system.
            ///
            /// \return Global bounding rectangle of the entity
            ///
            ////////////////////////////////////////////////////////////
            physic::BoundingBox& getGlobalBounds();
            void setSelected(int indexMin, int indexMax);

        private:

            ////////////////////////////////////////////////////////////
            /// \brief Draw the text to a render target
            ///
            /// \param target Render target to draw to
            /// \param states Current render states
            ///
            ////////////////////////////////////////////////////////////
            virtual void draw(RenderTarget& target, RenderStates states);

            ////////////////////////////////////////////////////////////
            /// \brief Make sure the text's geometry is updated
            ///
            /// All the attributes related to rendering are cached, such
            /// that the geometry is only updated when necessary.
            ///
            ////////////////////////////////////////////////////////////
            void ensureGeometryUpdate() const;

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            core::String              m_string;              ///< String to display
            const Font*         m_font;                ///< Font used to display the string
            unsigned int        m_characterSize;       ///< Base size of characters, in pixels
            float               m_letterSpacingFactor; ///< Spacing factor between letters
            float               m_lineSpacingFactor;   ///< Spacing factor between lines
            std::uint32_t              m_style;               ///< Text style (see Style enum)
            Color               m_fillColor;           ///< Text fill color
            Color               m_outlineColor;        ///< Text outline color
            Color               m_backgroundColor;
            float               m_outlineThickness;    ///< Thickness of the text's outline
            mutable VertexArray m_vertices;            ///< Vertex array containing the fill geometry
            mutable VertexArray m_outlineVertices;     ///< Vertex array containing the outline geometry
            mutable VertexArray m_backgroundVertices;
            mutable physic::BoundingBox   m_bounds, m_globalBounds;              ///< Bounding rectangle of the text (in local coordinates)
            mutable bool        m_geometryNeedUpdate;  ///< Does the geometry need to be recomputed?
            mutable std::uint64_t      m_fontTextureId;       ///< The font texture id
            int indexMin, indexMax;
        };
    }

} // namespace sf


#endif // ODFAEG_TEXT_HPP


////////////////////////////////////////////////////////////
/// \class sf::Text
/// \ingroup graphics
///
/// sf::Text is a drawable class that allows to easily display
/// some text with custom style and color on a render target.
///
/// It inherits all the functions from sf::Transformable:
/// position, rotation, scale, origin. It also adds text-specific
/// properties such as the font to use, the character size,
/// the font style (bold, italic, underlined), the global color
/// and the text to display of course.
/// It also provides convenience functions to calculate the
/// graphical size of the text, or to get the global position
/// of a given character.
///
/// sf::Text works in combination with the sf::Font class, which
/// loads and provides the glyphs (visual characters) of a given font.
///
/// The separation of sf::Font and sf::Text allows more flexibility
/// and better performances: indeed a sf::Font is a heavy resource,
/// and any operation on it is slow (often too slow for real-time
/// applications). On the other side, a sf::Text is a lightweight
/// object which can combine the glyphs data and metrics of a sf::Font
/// to display any text on a render target.
///
/// It is important to note that the sf::Text instance doesn't
/// copy the font that it uses, it only keeps a reference to it.
/// Thus, a sf::Font must not be destructed while it is
/// used by a sf::Text (i.e. never write a function that
/// uses a local sf::Font instance for creating a text).
///
/// See also the note on coordinates and undistorted rendering in sf::Transformable.
///
/// Usage example:
/// \code
/// // Declare and load a font
/// sf::Font font;
/// font.loadFromFile("arial.ttf");
///
/// // Create a text
/// sf::Text text("hello", font);
/// text.setCharacterSize(30);
/// text.setStyle(sf::Text::Bold);
/// text.setColor(Color::Red);
///
/// // Draw it
/// window.draw(text);
/// \endcode
///
/// \see sf::Font, sf::Transformable
///
////////////////////////////////////////////////////////////

