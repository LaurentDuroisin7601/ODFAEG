////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/Graphics/text.h"
#include "../../../include/odfaeg/Graphics/texture.h"
#include "../../../include/odfaeg/Graphics/renderTarget.h"
#include <cassert>

using namespace std;
namespace
{
    // Add an underline or strikethrough line to the vertex array
    void addLine(odfaeg::graphic::VertexArray& vertices, float lineLength, float lineTop, const sf::Color& color, float offset, float thickness, float outlineThickness = 0)
    {
        float top = std::floor(lineTop + offset - (thickness / 2) + 0.5f);
        float bottom = top + std::floor(thickness + 0.5f);

        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(-outlineThickness,             top    - outlineThickness, 0), color, sf::Vector2f(1, 1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(lineLength + outlineThickness, top    - outlineThickness, 0), color, sf::Vector2f(1, 1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(-outlineThickness,             bottom + outlineThickness, 0), color, sf::Vector2f(1, 1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(-outlineThickness,             bottom + outlineThickness, 0), color, sf::Vector2f(1, 1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(lineLength + outlineThickness, top    - outlineThickness, 0), color, sf::Vector2f(1, 1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(lineLength + outlineThickness, bottom + outlineThickness, 0), color, sf::Vector2f(1, 1)));
    }

    // Add a glyph quad to the vertex array
    void addGlyphQuad(odfaeg::graphic::VertexArray& vertices, sf::Vector2f position, const sf::Color& color, const sf::Glyph& glyph, float italicShear, float outlineThickness = 0)
    {
        float padding = 1.0;

        float left   = glyph.bounds.left - padding;
        float top    = glyph.bounds.top - padding;
        float right  = glyph.bounds.left + glyph.bounds.width + padding;
        float bottom = glyph.bounds.top  + glyph.bounds.height + padding;

        float u1 = static_cast<float>(glyph.textureRect.left) - padding;
        float v1 = static_cast<float>(glyph.textureRect.top) - padding;
        float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + padding;
        float v2 = static_cast<float>(glyph.textureRect.top  + glyph.textureRect.height) + padding;

        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(position.x + left  - italicShear * top    - outlineThickness, position.y + top    - outlineThickness, 0), color, sf::Vector2f(u1, v1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(position.x + right - italicShear * top    - outlineThickness, position.y + top    - outlineThickness, 0), color, sf::Vector2f(u2, v1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(position.x + left  - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness, 0), color, sf::Vector2f(u1, v2)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(position.x + left  - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness, 0), color, sf::Vector2f(u1, v2)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(position.x + right - italicShear * top    - outlineThickness, position.y + top    - outlineThickness, 0), color, sf::Vector2f(u2, v1)));
        vertices.append(odfaeg::graphic::Vertex(sf::Vector3f(position.x + right - italicShear * bottom - outlineThickness, position.y + bottom - outlineThickness, 0), color, sf::Vector2f(u2, v2)));
    }
}
namespace odfaeg
{
    namespace graphic {
        ////////////////////////////////////////////////////////////
        Text::Text() :
        m_string             (),
        m_font               (NULL),
        m_characterSize      (30),
        m_letterSpacingFactor(1.f),
        m_lineSpacingFactor  (1.f),
        m_style              (Regular),
        m_fillColor          (255, 255, 255),
        m_backgroundColor    (255, 255, 255),
        m_outlineColor       (0, 0, 0),
        m_outlineThickness   (0),
        m_vertices           (sf::Triangles),
        m_outlineVertices    (sf::Triangles),
        m_backgroundVertices (sf::Triangles),
        m_bounds             (),
        m_geometryNeedUpdate (false),
        m_fontTextureId      (0),
        indexMin(0),
        indexMax(0)
        {

        }


        ////////////////////////////////////////////////////////////
        Text::Text(const sf::String& string, const Font& font, unsigned int characterSize) :
        m_string             (string),
        m_font               (&font),
        m_characterSize      (characterSize),
        m_letterSpacingFactor(1.f),
        m_lineSpacingFactor  (1.f),
        m_style              (Regular),
        m_fillColor          (255, 255, 255),
        m_backgroundColor    (255, 255, 255),
        m_outlineColor       (0, 0, 0),
        m_outlineThickness   (0),
        m_vertices           (sf::Triangles),
        m_outlineVertices    (sf::Triangles),
        m_backgroundVertices (sf::Triangles),
        m_bounds             (),
        m_geometryNeedUpdate (true),
        m_fontTextureId      (0),
        indexMin(0),
        indexMax(0)
        {

        }


        ////////////////////////////////////////////////////////////
        void Text::setString(const sf::String& string)
        {
            if (m_string != string)
            {
                m_string = string;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setFont(const Font& font)
        {
            if (m_font != &font)
            {
                m_font = &font;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setCharacterSize(unsigned int size)
        {
            if (m_characterSize != size)
            {
                m_characterSize = size;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setLetterSpacing(float spacingFactor)
        {
            if (m_letterSpacingFactor != spacingFactor)
            {
                m_letterSpacingFactor = spacingFactor;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setLineSpacing(float spacingFactor)
        {
            if (m_lineSpacingFactor != spacingFactor)
            {
                m_lineSpacingFactor = spacingFactor;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setStyle(sf::Uint32 style)
        {
            if (m_style != style)
            {
                m_style = style;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setColor(const sf::Color& color)
        {
            setFillColor(color);
        }


        ////////////////////////////////////////////////////////////
        void Text::setFillColor(const sf::Color& color)
        {
            if (color != m_fillColor)
            {
                m_fillColor = color;

                // Change vertex colors directly, no need to update whole geometry
                // (if geometry is updated anyway, we can skip this step)
                if (!m_geometryNeedUpdate)
                {
                    for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i)
                        m_vertices[i].color = m_fillColor;
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Text::setOutlineColor(const sf::Color& color)
        {
            if (color != m_outlineColor)
            {
                m_outlineColor = color;

                // Change vertex colors directly, no need to update whole geometry
                // (if geometry is updated anyway, we can skip this step)
                if (!m_geometryNeedUpdate)
                {
                    for (std::size_t i = 0; i < m_outlineVertices.getVertexCount(); ++i)
                        m_outlineVertices[i].color = m_outlineColor;
                }
            }
        }
        void Text::setBackgroundColor(const sf::Color& color) {
            if (color != m_backgroundColor) {
                m_backgroundColor = color;
                // Change vertex colors directly, no need to update whole geometry
                // (if geometry is updated anyway, we can skip this step)
                if (!m_geometryNeedUpdate)
                {
                    for (std::size_t i = 0; i < m_backgroundVertices.getVertexCount(); ++i)
                        m_backgroundVertices[i].color = m_backgroundColor;
                }
            }
        }

        ////////////////////////////////////////////////////////////
        void Text::setOutlineThickness(float thickness)
        {
            if (thickness != m_outlineThickness)
            {
                m_outlineThickness = thickness;
                m_geometryNeedUpdate = true;
            }
        }


        ////////////////////////////////////////////////////////////
        const sf::String& Text::getString() const
        {
            return m_string;
        }


        ////////////////////////////////////////////////////////////
        const Font* Text::getFont() const
        {
            return m_font;
        }


        ////////////////////////////////////////////////////////////
        unsigned int Text::getCharacterSize() const
        {
            return m_characterSize;
        }


        ////////////////////////////////////////////////////////////
        float Text::getLetterSpacing() const
        {
            return m_letterSpacingFactor;
        }


        ////////////////////////////////////////////////////////////
        float Text::getLineSpacing() const
        {
            return m_lineSpacingFactor;
        }


        ////////////////////////////////////////////////////////////
        sf::Uint32 Text::getStyle() const
        {
            return m_style;
        }


        ////////////////////////////////////////////////////////////
        const sf::Color& Text::getColor() const
        {
            return getFillColor();
        }


        ////////////////////////////////////////////////////////////
        const sf::Color& Text::getFillColor() const
        {
            return m_fillColor;
        }


        ////////////////////////////////////////////////////////////
        const sf::Color& Text::getOutlineColor() const
        {
            return m_outlineColor;
        }


        ////////////////////////////////////////////////////////////
        float Text::getOutlineThickness() const
        {
            return m_outlineThickness;
        }


        ////////////////////////////////////////////////////////////
        sf::Vector2f Text::findCharacterPos(std::size_t index)
        {
            // Make sure that we have a valid font
            if (!m_font)
                return sf::Vector2f();

            // Adjust the index if it's out of range
            if (index > m_string.getSize())
                index = m_string.getSize();

            // Precompute the variables needed by the algorithm
            bool  isBold          = m_style & Bold;
            float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, isBold).advance;
            float letterSpacing   = ( whitespaceWidth / 3.f ) * ( m_letterSpacingFactor - 1.f );
            whitespaceWidth      += letterSpacing;
            float lineSpacing     = m_font->getLineSpacing(m_characterSize) * m_lineSpacingFactor;

            // Compute the position
            sf::Vector2f position;
            sf::Uint32 prevChar = 0;
            for (std::size_t i = 0; i < index; ++i)
            {
                sf::Uint32 curChar = m_string[i];

                // Apply the kerning offset
                position.x += m_font->getKerning(prevChar, curChar, m_characterSize);
                prevChar = curChar;

                // Handle special characters
                switch (curChar)
                {
                    case ' ':  position.x += whitespaceWidth;             continue;
                    case '\t': position.x += whitespaceWidth * 4;         continue;
                    case '\n': position.y += lineSpacing; position.x = 0; continue;
                }

                // For regular characters, add the advance offset of the glyph
                position.x += m_font->getGlyph(curChar, m_characterSize, isBold).advance + letterSpacing;
            }
            math::Vec3f pos (position.x, position.y, 0);
            // Transform the position to global coordinates
            pos = getTransform().transform(pos);
            position = sf::Vector2f(pos.x, pos.y);
            return position;
        }


        ////////////////////////////////////////////////////////////
        physic::BoundingBox Text::getLocalBounds() const
        {
            ensureGeometryUpdate();

            return m_bounds;
        }


        ////////////////////////////////////////////////////////////
        physic::BoundingBox& Text::getGlobalBounds()
        {
            m_globalBounds = getLocalBounds().transform(getTransform());
            return m_globalBounds;
        }


        ////////////////////////////////////////////////////////////
        void Text::draw(RenderTarget& target, RenderStates states)
        {
            if (m_font)
            {
                ensureGeometryUpdate();
                states.transform.combine(getTransform().getMatrix());
                target.draw(m_backgroundVertices, states);
                states.texture = &m_font->getTexture(m_characterSize);
                // Only draw the outline if there is something to draw
                if (m_outlineThickness != 0)
                    target.draw(m_outlineVertices, states);

                target.draw(m_vertices, states);

            }
        }
        void Text::setSelected(int indexMin, int indexMax) {
            this->indexMin = indexMin;
            this->indexMax = indexMax;
            m_geometryNeedUpdate = true;
        }

        ////////////////////////////////////////////////////////////
        void Text::ensureGeometryUpdate() const
        {
            if (!m_font)
                return;

            // Do nothing, if geometry has not changed and the font texture has not changed
            if (!m_geometryNeedUpdate && m_font->getTexture(m_characterSize).m_cacheId == m_fontTextureId)
                return;

            // Save the current fonts texture id
            m_fontTextureId = m_font->getTexture(m_characterSize).m_cacheId;

            // Mark geometry as updated
            m_geometryNeedUpdate = false;

            // Clear the previous geometry
            m_vertices.clear();
            m_outlineVertices.clear();
            m_bounds = physic::BoundingBox();

            // No text: nothing to draw
            if (m_string.isEmpty())
                return;

            // Compute values related to the text style
            bool  isBold             = m_style & Bold;
            bool  isUnderlined       = m_style & Underlined;
            bool  isStrikeThrough    = m_style & StrikeThrough;
            float italicShear        = (m_style & Italic) ? 0.209f : 0.f; // 12 degrees in radians
            float underlineOffset    = m_font->getUnderlinePosition(m_characterSize);
            float underlineThickness = m_font->getUnderlineThickness(m_characterSize);

            // Compute the location of the strike through dynamically
            // We use the center point of the lowercase 'x' glyph as the reference
            // We reuse the underline thickness as the thickness of the strike through as well
            sf::FloatRect xBounds = m_font->getGlyph(L'x', m_characterSize, isBold).bounds;
            float strikeThroughOffset = xBounds.top + xBounds.height / 2.f;

            // Precompute the variables needed by the algorithm
            float whitespaceWidth = m_font->getGlyph(L' ', m_characterSize, isBold).advance;
            float letterSpacing   = ( whitespaceWidth / 3.f ) * ( m_letterSpacingFactor - 1.f );
            whitespaceWidth      += letterSpacing;
            float lineSpacing     = m_font->getLineSpacing(m_characterSize) * m_lineSpacingFactor;
            float x               = 0.f;
            float y               = static_cast<float>(m_characterSize);

            // Create one quad for each character
            float minX = static_cast<float>(m_characterSize);
            float minY = static_cast<float>(m_characterSize);
            float maxX = 0.f;
            float maxY = 0.f;
            sf::Uint32 prevChar = 0;
            for (std::size_t i = 0; i < m_string.getSize(); ++i)
            {
                sf::Uint32 curChar = m_string[i];

                // Skip the \r char to avoid weird graphical issues
                if (curChar == '\r')
                    continue;

                // Apply the kerning offset
                x += m_font->getKerning(prevChar, curChar, m_characterSize);

                // If we're using the underlined style and there's a new line, draw a line
                if (isUnderlined && (curChar == L'\n' && prevChar != L'\n'))
                {
                    addLine(m_vertices, x, y, m_fillColor, underlineOffset, underlineThickness);

                    if (m_outlineThickness != 0)
                        addLine(m_outlineVertices, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
                }

                // If we're using the strike through style and there's a new line, draw a line across all characters
                if (isStrikeThrough && (curChar == L'\n' && prevChar != L'\n'))
                {
                    addLine(m_vertices, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

                    if (m_outlineThickness != 0)
                        addLine(m_outlineVertices, x, y, m_outlineColor, strikeThroughOffset, underlineThickness, m_outlineThickness);
                }

                prevChar = curChar;

                // Handle special characters
                if ((curChar == L' ') || (curChar == L'\n') || (curChar == L'\t'))
                {
                    // Update the current bounds (min coordinates)
                    minX = std::min(minX, x);
                    minY = std::min(minY, y);

                    switch (curChar)
                    {
                        case L' ':  x += whitespaceWidth;     break;
                        case L'\t': x += whitespaceWidth * 4; break;
                        case L'\n': y += lineSpacing; x = 0;  break;
                    }

                    // Update the current bounds (max coordinates)
                    maxX = std::max(maxX, x);
                    maxY = std::max(maxY, y);

                    // Next glyph, no need to create a quad for whitespace
                    continue;
                }

                // Apply the outline
                if (m_outlineThickness != 0)
                {
                    const sf::Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold, m_outlineThickness);

                    float left   = glyph.bounds.left;
                    float top    = glyph.bounds.top;
                    float right  = glyph.bounds.left + glyph.bounds.width;
                    float bottom = glyph.bounds.top  + glyph.bounds.height;

                    // Add the outline glyph to the vertices
                    addGlyphQuad(m_outlineVertices, sf::Vector2f(x, y), m_outlineColor, glyph, italicShear, m_outlineThickness);

                    // Update the current bounds with the outlined glyph bounds
                    minX = std::min(minX, x + left   - italicShear * bottom - m_outlineThickness);
                    maxX = std::max(maxX, x + right  - italicShear * top    - m_outlineThickness);
                    minY = std::min(minY, y + top    - m_outlineThickness);
                    maxY = std::max(maxY, y + bottom - m_outlineThickness);
                }

                // Extract the current glyph's description
                const sf::Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold);
                if (i >= indexMin && i < indexMax) {
                    addGlyphQuad(m_vertices, sf::Vector2f(x, y), sf::Color::White, glyph, italicShear);
                    addGlyphQuad(m_backgroundVertices, sf::Vector2f(x, y), sf::Color::Blue, glyph, italicShear);
                } else {
                    // Add the glyph to the vertices
                    addGlyphQuad(m_vertices, sf::Vector2f(x, y), m_fillColor, glyph, italicShear);
                    addGlyphQuad(m_backgroundVertices, sf::Vector2f(x, y), m_backgroundColor, glyph, italicShear);
                }

                // Update the current bounds with the non outlined glyph bounds
                if (m_outlineThickness == 0)
                {
                    float left   = glyph.bounds.left;
                    float top    = glyph.bounds.top;
                    float right  = glyph.bounds.left + glyph.bounds.width;
                    float bottom = glyph.bounds.top  + glyph.bounds.height;

                    minX = std::min(minX, x + left  - italicShear * bottom);
                    maxX = std::max(maxX, x + right - italicShear * top);
                    minY = std::min(minY, y + top);
                    maxY = std::max(maxY, y + bottom);
                }

                // Advance to the next character
                x += glyph.advance + letterSpacing;
            }

            // If we're using the underlined style, add the last line
            if (isUnderlined && (x > 0))
            {
                addLine(m_vertices, x, y, m_fillColor, underlineOffset, underlineThickness);

                if (m_outlineThickness != 0)
                    addLine(m_outlineVertices, x, y, m_outlineColor, underlineOffset, underlineThickness, m_outlineThickness);
            }

            // If we're using the strike through style, add the last line across all characters
            if (isStrikeThrough && (x > 0))
            {
                addLine(m_vertices, x, y, m_fillColor, strikeThroughOffset, underlineThickness);

                if (m_outlineThickness != 0)
                    addLine(m_outlineVertices, x, y, m_outlineColor, strikeThroughOffset, underlineThickness, m_outlineThickness);
            }

            // Update the bounding rectangle
            m_bounds = physic::BoundingBox(minX, minY, 0, maxX - minX, maxY - minY, 0);
        }
    }

} // namespace sf

