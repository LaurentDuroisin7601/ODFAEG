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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "renderTextureImplDefault.h"
#ifndef VULKAN
#include "glCheck.h"
#include "textureSaver.h"
#include <SFML/System/Err.hpp>

using namespace sf;
namespace odfaeg {
    namespace graphic {
        namespace priv
        {
            ////////////////////////////////////////////////////////////
            RenderTextureImplDefault::RenderTextureImplDefault() :
            m_width  (0),
            m_height (0)
            {

            }


            ////////////////////////////////////////////////////////////
            RenderTextureImplDefault::~RenderTextureImplDefault()
            {

            }


            ////////////////////////////////////////////////////////////
            bool RenderTextureImplDefault::create(unsigned int width, unsigned int height, window::ContextSettings settings, unsigned int)
            {
                // Store the dimensions
                m_width = width;
                m_height = height;
                return true;
            }
            ////////////////////////////////////////////////////////////
            void RenderTextureImplDefault::updateTexture(unsigned int textureId)
            {
                // Make sure that the current texture binding will be preserved
                priv::TextureSaver save;

                // Copy the rendered pixels to the texture
                glCheck(glBindTexture(GL_TEXTURE_2D, textureId));
                glCheck(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_width, m_height));
            }
            unsigned int RenderTextureImplDefault::getFramebufferId() {
                return 0;
            }
            void RenderTextureImplDefault::selectCubemapFace (int face, int textureID) {
            }
            void RenderTextureImplDefault::bind() {
            }
        }
    } // namespace priv

} // namespace sf
#endif
