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

#ifndef ODFAEG_RENDER_TEXTURE_IMPL_FBO_HPP
#define ODFAEG_RENDER_TEXTURE_IMPL_FBO_HPP
#include "renderTextureImpl.h"
#ifndef VULKAN
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

namespace odfaeg {
    namespace graphic {
        namespace priv
        {
            ////////////////////////////////////////////////////////////
            /// \brief Specialization of RenderTextureImpl using the
            ///        FrameBuffer Object OpenGL extension
            ///
            ////////////////////////////////////////////////////////////
            class RenderTextureImplFBO : public RenderTextureImpl
            {
            public :

                ////////////////////////////////////////////////////////////
                /// \brief Default constructor
                ///
                ////////////////////////////////////////////////////////////
                RenderTextureImplFBO();

                ////////////////////////////////////////////////////////////
                /// \brief Destructor
                ///
                ////////////////////////////////////////////////////////////
                ~RenderTextureImplFBO();

                ////////////////////////////////////////////////////////////
                /// \brief Check whether the system supports FBOs or not
                ///
                /// \return True if FBO render textures are supported
                ///
                ////////////////////////////////////////////////////////////
                static bool isAvailable();

            private :
                virtual void bind();
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
                virtual bool create(unsigned int width, unsigned int height, window::ContextSettings settings, unsigned int textureId);
                ////////////////////////////////////////////////////////////
                /// \brief Update the pixels of the target texture
                ///
                /// \param textureId OpenGL identifier of the target texture
                ///
                ////////////////////////////////////////////////////////////
                virtual void updateTexture(unsigned textureId);
                virtual unsigned int getFramebufferId();
                void selectCubemapFace(int face, int textureID);
                ////////////////////////////////////////////////////////////
                // Member data
                ////////////////////////////////////////////////////////////
                unsigned int m_frameBuffer; ///< OpenGL frame buffer object
                unsigned int m_depthBuffer; ///< Optional depth buffer attached to the frame buffer
                unsigned int m_versionMajor, m_versionMinor;
            };
        }

    } // namespace priv

} // namespace sf

#endif

#endif // SFML_RENDERTEXTUREIMPLFBO_HPP
