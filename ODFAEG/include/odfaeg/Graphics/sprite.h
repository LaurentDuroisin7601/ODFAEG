#ifndef ODFAEG_SPRITE_HPP
#define ODFAEG_SPRITE_HPP
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
#include "drawable.h"
#include "transformable.h"
#include "vertexArray.h"
#include "renderTarget.h"
namespace odfaeg {
    namespace graphic {

        ////////////////////////////////////////////////////////////
        /// \brief Drawable representation of a texture, with its
        ///        own transformations, color, etc.
        ///
        ////////////////////////////////////////////////////////////
        class ODFAEG_GRAPHICS_API Sprite : public Drawable, public Transformable {
            public :

                ////////////////////////////////////////////////////////////
                /// \brief Default constructor
                ///
                /// Creates an empty sprite with no source texture.
                ///
                ////////////////////////////////////////////////////////////
                Sprite();

                 ////////////////////////////////////////////////////////////
                /// \brief Construct the sprite from a source texture
                ///
                /// \param texture Source texture
                ///
                /// \param position Sprite position
                ///
                /// \param size Sprite size.
                ///
                /// \see setTexture
                ///
                ////////////////////////////////////////////////////////////
                Sprite (const Texture& tex, math::Vec3f position, math::Vec3f size);

                 ////////////////////////////////////////////////////////////
                /// \brief Construct the sprite from a sub-rectangle of source texture
                ///
                /// \param texture Source texture
                ///
                /// \param position Sprite position
                ///
                /// \param size Sprite size.
                ///
                /// \param rectangle Sub-rectangle of the texture to assign to the sprite
                ///
                /// \see setTexture, setTextureRect
                ///
                ////////////////////////////////////////////////////////////
                Sprite (const Texture& tex, math::Vec3f position, math::Vec3f size, const sf::IntRect& texRect);

                ////////////////////////////////////////////////////////////
                /// \brief Set the global color of the sprite
                ///
                /// This color is modulated (multiplied) with the sprite's
                /// texture. It can be used to colorize the sprite, or change
                /// its global opacity.
                /// By default, the sprite's color is opaque white.
                ///
                /// \param color New color of the sprite
                ///
                /// \see getColor
                ///
                ////////////////////////////////////////////////////////////
                void setColor(const sf::Color& color);

                ////////////////////////////////////////////////////////////
                /// \brief Change the source texture of the sprite
                ///
                /// The \a texture argument refers to a texture that must
                /// exist as long as the sprite uses it. Indeed, the sprite
                /// doesn't store its own copy of the texture, but rather keeps
                /// a pointer to the one that you passed to this function.
                /// If the source texture is destroyed and the sprite tries to
                /// use it, the behaviour is undefined.
                /// If \a resetRect is true, the TextureRect property of
                /// the sprite is automatically adjusted to the size of the new
                /// texture. If it is false, the texture rect is left unchanged.
                ///
                /// \param texture   New texture
                /// \param resetRect Should the texture rect be reset to the size of the new texture?
                ///
                /// \see getTexture, setTextureRect
                ///
                ////////////////////////////////////////////////////////////
                void setTexture(const Texture& texture, bool resetRect = false);

                 ////////////////////////////////////////////////////////////
                /// \brief Set the sub-rectangle of the texture that the sprite will display
                ///
                /// The texture rect is useful when you don't want to display
                /// the whole texture, but rather a part of it.
                /// By default, the texture rect covers the entire texture.
                ///
                /// \param rectangle Rectangle defining the region of the texture to display
                ///
                /// \see getTextureRect, setTexture
                ///
                ////////////////////////////////////////////////////////////
                void setTextureRect(const sf::IntRect& rectangle);

                 ////////////////////////////////////////////////////////////
                /// \brief Get the source texture of the sprite
                ///
                /// If the sprite has no source texture, a NULL pointer is returned.
                /// The returned pointer is const, which means that you can't
                /// modify the texture when you retrieve it with this function.
                ///
                /// \return Pointer to the sprite's texture
                ///
                /// \see setTexture
                ///
                ////////////////////////////////////////////////////////////
                const Texture* getTexture() const;

                ////////////////////////////////////////////////////////////
                /// \brief Get the sub-rectangle of the texture displayed by the sprite
                ///
                /// \return Texture rectangle of the sprite
                ///
                /// \see setTextureRect
                ///
                ////////////////////////////////////////////////////////////
                const sf::IntRect& getTextureRect() const;

                ////////////////////////////////////////////////////////////
                /// \brief Get the global color of the sprite
                ///
                /// \return Global color of the sprite
                ///
                /// \see setColor
                ///
                ////////////////////////////////////////////////////////////
                const sf::Color& getColor() const;
                bool operator== (const Sprite& other) const;
                VertexArray getVertexArray();
            private :

                ////////////////////////////////////////////////////////////
                /// \brief Draw the sprite to a render target
                ///
                /// \param target Render target to draw to
                /// \param states Current render states
                ///
                ////////////////////////////////////////////////////////////
                virtual void draw (RenderTarget& target, RenderStates states);

                ////////////////////////////////////////////////////////////
                /// \brief Update the vertices' texture coordinates
                ///
                ////////////////////////////////////////////////////////////
                void updateTexCoords();
                const Texture* m_texture; ///< Texture of the sprite
                sf::IntRect m_textureRect; ///< Rectangle defining the area of the source texture to display
                VertexArray m_vertices; ///< Vertices defining the sprite's geometry
        };
    }
}
#endif // ODFAEG_CIRCLE_SHAPE_HPP
