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

#ifndef ODFAEG_SHAPE_HPP
#define ODFAEG_SHAPE_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>
#include "drawable.h"
#include "transformable.h"
#include "vertexArray.h"
#include "../Physics/boundingBox.h"
namespace odfaeg {
    namespace graphic {
        ////////////////////////////////////////////////////////////
        /// \brief Base class for textured shapes with outline
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API Shape : public Drawable, public Transformable, public core::Registered<Shape>
        {
        public :

            ////////////////////////////////////////////////////////////
            /// \brief Virtual destructor
            ///
            ////////////////////////////////////////////////////////////
            virtual ~Shape();

            ////////////////////////////////////////////////////////////
            /// \brief Change the source texture of the shape
            ///
            /// The \a texture argument refers to a texture that must
            /// exist as long as the shape uses it. Indeed, the shape
            /// doesn't store its own copy of the texture, but rather keeps
            /// a pointer to the one that you passed to this function.
            /// If the source texture is destroyed and the shape tries to
            /// use it, the behaviour is undefined.
            /// \a texture can be NULL to disable texturing.
            /// If \a resetRect is true, the TextureRect property of
            /// the shape is automatically adjusted to the size of the new
            /// texture. If it is false, the texture rect is left unchanged.
            ///
            /// \param texture   New texture
            /// \param resetRect Should the texture rect be reset to the size of the new texture?
            ///
            /// \see getTexture, setTextureRect
            ///
            ////////////////////////////////////////////////////////////
            void setTexture(const Texture* texture, bool resetRect = false);

            ////////////////////////////////////////////////////////////
            /// \brief Set the sub-rectangle of the texture that the shape will display
            ///
            /// The texture rect is useful when you don't want to display
            /// the whole texture, but rather a part of it.
            /// By default, the texture rect covers the entire texture.
            ///
            /// \param rect Rectangle defining the region of the texture to display
            ///
            /// \see getTextureRect, setTexture
            ///
            ////////////////////////////////////////////////////////////
            void setTextureRect(const sf::IntRect& rect);

            ////////////////////////////////////////////////////////////
            /// \brief Set the fill color of the shape
            ///
            /// This color is modulated (multiplied) with the shape's
            /// texture if any. It can be used to colorize the shape,
            /// or change its global opacity.
            /// You can use sf::Color::Transparent to make the inside of
            /// the shape transparent, and have the outline alone.
            /// By default, the shape's fill color is opaque white.
            ///
            /// \param color New color of the shape
            ///
            /// \see getFillColor, setOutlineColor
            ///
            ////////////////////////////////////////////////////////////
            void setFillColor(const sf::Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the outline color of the shape
            ///
            /// By default, the shape's outline color is opaque white.
            ///
            /// \param color New outline color of the shape
            ///
            /// \see getOutlineColor, setFillColor
            ///
            ////////////////////////////////////////////////////////////
            void setOutlineColor(const sf::Color& color);

            ////////////////////////////////////////////////////////////
            /// \brief Set the thickness of the shape's outline
            ///
            /// Note that negative values are allowed (so that the outline
            /// expands towards the center of the shape), and using zero
            /// disables the outline.
            /// By default, the outline thickness is 0.
            ///
            /// \param thickness New outline thickness
            ///
            /// \see getOutlineThickness
            ///
            ////////////////////////////////////////////////////////////
            void setOutlineThickness(float thickness);

            ////////////////////////////////////////////////////////////
            /// \brief Get the source texture of the shape
            ///
            /// If the shape has no source texture, a NULL pointer is returned.
            /// The returned pointer is const, which means that you can't
            /// modify the texture when you retrieve it with this function.
            ///
            /// \return Pointer to the shape's texture
            ///
            /// \see setTexture
            ///
            ////////////////////////////////////////////////////////////
            const Texture* getTexture() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the sub-rectangle of the texture displayed by the shape
            ///
            /// \return Texture rectangle of the shape
            ///
            /// \see setTextureRect
            ///
            ////////////////////////////////////////////////////////////
            const sf::IntRect& getTextureRect() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the fill color of the shape
            ///
            /// \return Fill color of the shape
            ///
            /// \see setFillColor
            ///
            ////////////////////////////////////////////////////////////
            const sf::Color& getFillColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the outline color of the shape
            ///
            /// \return Outline color of the shape
            ///
            /// \see setOutlineColor
            ///
            ////////////////////////////////////////////////////////////
            const sf::Color& getOutlineColor() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the outline thickness of the shape
            ///
            /// \return Outline thickness of the shape
            ///
            /// \see setOutlineThickness
            ///
            ////////////////////////////////////////////////////////////
            float getOutlineThickness() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get the total number of points of the shape
            ///
            /// \return Number of points of the shape
            ///
            /// \see getPoint
            ///
            ////////////////////////////////////////////////////////////
            virtual unsigned int getPointCount() const = 0;

            ////////////////////////////////////////////////////////////
            /// \brief Get a point of the shape
            ///
            /// The result is undefined if \a index is out of the valid range.
            ///
            /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
            ///
            /// \return Index-th point of the shape
            ///
            /// \see getPointCount
            ///
            ////////////////////////////////////////////////////////////
            virtual sf::Vector3f getPoint(unsigned int index) const = 0;

            template <typename Archive>
            void vtserialize(Archive& ar) {
                Transformable::serialize(ar);
                ar(m_textureRect.left);
                ar(m_textureRect.top);
                ar(m_textureRect.width);
                ar(m_textureRect.height);
                ar(m_fillColor.r);
                ar(m_fillColor.g);
                ar(m_fillColor.b);
                ar(m_fillColor.a);
                ar(m_outlineColor.r);
                ar(m_outlineColor.g);
                ar(m_outlineColor.b);
                ar(m_outlineColor.a);
                ar(m_outlineThickness);
                ar(m_vertices);
                ar(m_outlineVertices);
                ar(m_insideBounds);
                ar(m_bounds);
            }
            const unsigned int& getId();
            physic::BoundingBox getLocalBounds() const;
            physic::BoundingBox& getGlobalBounds();
        protected :

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            ////////////////////////////////////////////////////////////
            Shape();

            ////////////////////////////////////////////////////////////
            /// \brief Recompute the internal geometry of the shape
            ///
            /// This function must be called by the derived class everytime
            /// the shape's points change (ie. the result of either
            /// getPointCount or getPoint is different).
            ///
            ////////////////////////////////////////////////////////////
            void update();
        private :

            ////////////////////////////////////////////////////////////
            /// \brief Draw the shape to a render target
            ///
            /// \param target Render target to draw to
            /// \param states Current render states
            ///
            ////////////////////////////////////////////////////////////
            virtual void draw(RenderTarget& target, RenderStates states);

            ////////////////////////////////////////////////////////////
            /// \brief Update the fill vertices' color
            ///
            ////////////////////////////////////////////////////////////
            void updateFillColors();

            ////////////////////////////////////////////////////////////
            /// \brief Update the fill vertices' texture coordinates
            ///
            ////////////////////////////////////////////////////////////
            void updateTexCoords();

            ////////////////////////////////////////////////////////////
            /// \brief Update the outline vertices' position
            ///
            ////////////////////////////////////////////////////////////
            void updateOutline();

            ////////////////////////////////////////////////////////////
            /// \brief Update the outline vertices' color
            ///
            ////////////////////////////////////////////////////////////
            void updateOutlineColors();
            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            const Texture* m_texture;          ///< Texture of the shape
            sf::IntRect    m_textureRect;      ///< Rectangle defining the area of the source texture to display
            sf::Color          m_fillColor;        ///< Fill color
            sf::Color          m_outlineColor;     ///< Outline color
            float          m_outlineThickness; ///< Thickness of the shape's outline
            VertexArray    m_vertices;         ///< Vertex array containing the fill geometry
            VertexArray    m_outlineVertices;  ///< Vertex array containing the outline geometry
            physic::BoundingBox           m_insideBounds;     ///< Bounding rectangle of the inside (fill)
            physic::BoundingBox           m_bounds, m_globalBounds;           ///< Bounding rectangle of the whole shape (outline + fill)
            unsigned int id;
            static unsigned int nbShapes;
        };
    }

} // namespace sf
#endif // SFML_SHAPE_HPP


////////////////////////////////////////////////////////////
/// \class sf::Shape
/// \ingroup graphics
///
/// sf::Shape is a drawable class that allows to define and
/// display a custom convex shape on a render target.
/// It's only an abstract base, it needs to be specialized for
/// concrete types of shapes (circle, rectangle, convex polygon,
/// star, ...).
///
/// In addition to the attributes provided by the specialized
/// shape classes, a shape always has the following attributes:
/// \li a texture
/// \li a texture rectangle
/// \li a fill color
/// \li an outline color
/// \li an outline thickness
///
/// Each feature is optional, and can be disabled easily:
/// \li the texture can be null
/// \li the fill/outline colors can be sf::Color::Transparent
/// \li the outline thickness can be zero
///
/// You can write your own derived shape class, there are only
/// two virtual functions to override:
/// \li getPointCount must return the number of points of the shape
/// \li getPoint must return the points of the shape
///
/// \see sf::RectangleShape, sf::CircleShape, sf::ConvexShape, sf::Transformable
///
////////////////////////////////////////////////////////////
