////////////////////////////////////////////////////////////
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

#ifndef ODFAEG_RENDER_TEXTURE_IMPL_HPP
#define ODFAEG_RENDER_TEXTURE_IMPL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/config.hpp"
#ifndef VULKAN
#include <SFML/System/NonCopyable.hpp>
#include "../../../include/odfaeg/Window/contextSettings.hpp"
namespace odfaeg
{
    namespace graphic {
        namespace priv
        {
            ////////////////////////////////////////////////////////////
            /// \brief Abstract base class for render-texture implementations
            ///
            ////////////////////////////////////////////////////////////
            class RenderTextureImpl : sf::NonCopyable
            {
            public :

                ////////////////////////////////////////////////////////////
                /// \brief Destructor
                ///
                ////////////////////////////////////////////////////////////
                virtual ~RenderTextureImpl();

                ////////////////////////////////////////////////////////////
                /// \brief Create the render texture implementation
                ///
                /// \param width       Width of the texture to render to
                /// \param height      Height of the texture to render to
                /// \param textureId   OpenGL identifier of the target texture
                /// \param depthBuffer Is a depth buffer requested?
                ///
                /// \return True if creation has been successful
                ///
                ////////////////////////////////////////////////////////////
                virtual bool create(unsigned int width, unsigned int height, window::ContextSettings settings, unsigned int textureId) = 0;
                ////////////////////////////////////////////////////////////
                /// \brief Update the pixels of the target texture
                ///
                /// \param textureId OpenGL identifier of the target texture
                ///
                ////////////////////////////////////////////////////////////
                virtual void updateTexture(unsigned int textureId) = 0;
                virtual unsigned int getFramebufferId() = 0;
                virtual void bind() = 0;
                virtual void selectCubemapFace(int face, int textureID) = 0;
            };
        }

    } // namespace priv

} // namespace sf
#endif


#endif // SFML_RENDERTEXTUREIMPL_HPP
