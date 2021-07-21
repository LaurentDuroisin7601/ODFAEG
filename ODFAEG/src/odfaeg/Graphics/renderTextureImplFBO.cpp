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
#include "renderTextureImplFBO.h"
#ifndef VULKAN
#include "../../../include/odfaeg/Graphics/renderTarget.h"
#include "../../../include/odfaeg/Graphics/texture.h"
#include "glCheck.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/System/Err.hpp>
#include <iostream>
using namespace sf;
namespace odfaeg {
    namespace graphic {
        namespace priv
        {
            ////////////////////////////////////////////////////////////
            RenderTextureImplFBO::RenderTextureImplFBO() :
            m_frameBuffer(0),
            m_depthBuffer(0)
            {

            }


            ////////////////////////////////////////////////////////////
            RenderTextureImplFBO::~RenderTextureImplFBO()
            {

                // Destroy the depth buffer
                if (m_depthBuffer)
                {
                    GLuint depthBuffer = static_cast<GLuint>(m_depthBuffer);
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                        glCheck(glDeleteRenderbuffers(1, &depthBuffer));
                    else
                        glCheck(glDeleteRenderbuffersEXT(1, &depthBuffer));
                }

                // Destroy the frame buffer
                if (m_frameBuffer)
                {
                    GLuint frameBuffer = static_cast<GLuint>(m_frameBuffer);
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                        glCheck(glDeleteFramebuffers(1, &frameBuffer));
                    else
                        glCheck(glDeleteFramebuffersEXT(1, &frameBuffer));
                }
                //delete m_context;
            }


            ////////////////////////////////////////////////////////////
            bool RenderTextureImplFBO::isAvailable()
            {
                // Make sure that GLEW is initialized
                priv::ensureGlewInit();

                return GLEW_EXT_framebuffer_object != 0;
            }


            ////////////////////////////////////////////////////////////
            bool RenderTextureImplFBO::create(unsigned int width, unsigned int height, window::ContextSettings settings, unsigned int textureId)
            {
                //window::GLResource::TransientContextLock transientLock();

                //m_context = new window::Context(m_settings, m_width, m_height);
                // Create the framebuffer object
                GLuint frameBuffer = 0;
                m_versionMajor = settings.versionMajor;
                m_versionMinor = settings.versionMinor;
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glGenFramebuffers(1, &frameBuffer));
                else
                    glCheck(glGenFramebuffersEXT(1, &frameBuffer));
                m_frameBuffer = static_cast<unsigned int>(frameBuffer);
                if (!m_frameBuffer)
                {
                    std::cerr << "Impossible to create render texture (failed to create the frame buffer object)" << std::endl;
                    return false;
                }
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer));
                else
                    glCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer));
                // Create the depth buffer if requested
                if (settings.depthBits > 0)
                {
                    GLuint depth = 0;
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                        glCheck(glGenRenderbuffers(1, &depth));
                    else
                        glCheck(glGenRenderbuffersEXT(1, &depth));
                    m_depthBuffer = static_cast<unsigned int>(depth);
                    if (!m_depthBuffer)
                    {
                        std::cerr << "Impossible to create render texture (failed to create the attached depth buffer)" << std::endl;
                        return false;
                    }
                    if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                        glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer));
                        glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
                        glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer));
                    } else {
                        glCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer));
                        glCheck(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height));
                        glCheck(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer));
                    }
                }
                glCheck(glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT));
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    glCheck(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureId, 0));

                } else {
                    // Link the texture to the frame buffer
                    glCheck(glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureId, 0));
                }


                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3) {
                    // A final check, just to be sure.
                    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    {
                        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                        std::cerr << "Impossible to create render texture (failed to link the target texture to the frame buffer)" << std::endl;
                        return false;
                    }
                } else {
                    // A final check, just to be sure...
                    if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
                    {
                        glCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
                        std::cerr << "Impossible to create render texture (failed to link the target texture to the frame buffer)" << std::endl;
                        return false;
                    }
                }
                return true;
            }
            //bool RenderTextureImplFBO::activate(bool active) {
                /*if (!m_context)
                    m_context = new window::Context(m_settings, m_width, m_height);*/
                //return m_context->setActive(active);
            //}
            ////////////////////////////////////////////////////////////
            void RenderTextureImplFBO::updateTexture(unsigned int)
            {
                glFlush();
            }
            unsigned int RenderTextureImplFBO::getFramebufferId () {
                return m_frameBuffer;
            }
            void RenderTextureImplFBO::bind() {
                glCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer));
            }
            void RenderTextureImplFBO::selectCubemapFace (int face, int textureID) {
                if (m_versionMajor >= 3 && m_versionMinor >= 3)
                    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer));
                else
                    glCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBuffer));
                if (m_versionMajor > 3 || m_versionMajor == 3 && m_versionMinor >= 3)
                    glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureID, 0));
                else
                    glCheck(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureID, 0));
            }
        }

    } // namespace priv

} // namespace sf
#endif
