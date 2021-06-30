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
#ifndef VULKAN
#include "GL/glew.h"
#include "glCheck.h"
#endif
#include "../../../include/odfaeg/Graphics/texture.h"
#include "../../../include/odfaeg/Window/window.hpp"

#include "textureSaver.h"
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Err.hpp>
#include <cassert>
#include <cstring>
#include <iostream>
using namespace sf;

namespace
{
    sf::Mutex idMutex;
    sf::Mutex maximumSizeMutex;

    // Thread-safe unique identifier generator,
    // is used for states cache (see RenderTarget)
    sf::Uint64 getUniqueId()
    {
        sf::Lock lock(idMutex);

        static sf::Uint64 id = 1; // start at 1, zero is "no texture"

        return id++;
    }
}


namespace odfaeg {
    namespace graphic {
        unsigned int Texture::nbTextures = 0;
        #ifdef VULKAN
        bool Texture::loadFromImage(const sf::Image& image, const sf::IntRect& area) {
        }
        bool Texture::create(unsigned int width, unsigned int height) {
        }
        sf::Vector2u Texture::getSize() const {
        }
        void Texture::update(const sf::Uint8* pixels, unsigned int width, unsigned int height, unsigned int x, unsigned int y) {
        }
        void Texture::update(const Texture& texture) {
        }
        void Texture::setSmooth(bool smooth) {
        }
        void Texture::swap(Texture& texture) {
        }
        unsigned int Texture::getMaximumSize() {
        }
        #else // VULKAN
        std::vector<Texture*> Texture::allTextures = std::vector<Texture*>();
        ////////////////////////////////////////////////////////////
        Texture::Texture() :
        m_size         (0, 0),
        m_actualSize   (0, 0),
        m_texture      (0),
        m_isSmooth     (false),
        m_isRepeated   (false),
        m_pixelsFlipped(false),
        m_isCubeMap(false),
        m_cacheId      (getUniqueId()),
        m_name(""),
        textureResident(false),
        id(nbTextures+1)
        {
            nbTextures++;
        }


        ////////////////////////////////////////////////////////////
        Texture::Texture(const Texture& copy) :
        m_size         (0, 0),
        m_actualSize   (0, 0),
        m_texture      (0),
        m_isSmooth     (copy.m_isSmooth),
        m_isRepeated   (copy.m_isRepeated),
        m_pixelsFlipped(false),
        m_isCubeMap(copy.m_isCubeMap),
        m_cacheId      (getUniqueId()),
        m_name(copy.m_name),
        textureResident(copy.textureResident),
        id(nbTextures+1)
        {
            if (copy.m_texture)
            {
                if (create(copy.getSize().x, copy.getSize().y))
                {
                    update(copy);

                    // Force an OpenGL flush, so that the texture will appear updated
                    // in all contexts immediately (solves problems in multi-threaded apps)
                    glCheck(glFlush());
                }
                else
                {
                    err() << "Failed to copy texture, failed to create new texture" << std::endl;
                }
            }
            nbTextures++;
        }


        ////////////////////////////////////////////////////////////
        Texture::~Texture()
        {
            // Destroy the OpenGL texture
            if (m_texture)
            {
                //ensureGlContext();

                GLuint texture = static_cast<GLuint>(m_texture);
                glCheck(glDeleteTextures(1, &texture));
            }
        }
        uint64_t Texture::getTextureHandle() {
            GLuint64 handle_texture = glGetTextureHandleARB(m_texture);
            return handle_texture;
        }
        void Texture::makeTextureResident(uint64_t handle_texture) {
            if (!textureResident) {
               glCheck(glMakeTextureHandleResidentARB(handle_texture));
               textureResident = true;
            }
        }
        bool Texture::isTextureResident() {
            return textureResident;
        }
        ////////////////////////////////////////////////////////////
        bool Texture::create(unsigned int width, unsigned int height, GLenum precision, GLenum format, GLenum type)
        {
            // Check if texture parameters are valid before creating it
            if ((width == 0) || (height == 0))
            {
                err() << "Failed to create texture, invalid size (" << width << "x" << height << ")" << std::endl;
                return false;
            }

            // Compute the internal texture dimensions depending on NPOT textures support
            Vector2u actualSize(getValidSize(width), getValidSize(height));

            // Check the maximum texture size
            unsigned int maxSize = getMaximumSize();
            if ((actualSize.x > maxSize) || (actualSize.y > maxSize))
            {
                err() << "Failed to create texture, its internal size is too high "
                      << "(" << actualSize.x << "x" << actualSize.y << ", "
                      << "maximum is " << maxSize << "x" << maxSize << ")"
                      << std::endl;
                return false;
            }

            // All the validity checks passed, we can store the new texture settings
            m_size.x        = width;
            m_size.y        = height;
            m_actualSize    = actualSize;
            m_pixelsFlipped = false;
            //ensureGlContext();

            // Create the OpenGL texture if it doesn't exist yet
            if (!m_texture)
            {
                GLuint texture;
                glCheck(glGenTextures(1, &texture));
                m_texture = static_cast<unsigned int>(texture);
                //glCheck(glBindImageTextures(0, 1, &m_texture));
            }

            // Make sure that the current texture binding will be preserved
            priv::TextureSaver save;

            // Initialize the texture
            glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
            glCheck(glTexImage2D(GL_TEXTURE_2D, 0, precision, m_actualSize.x, m_actualSize.y, 0, format, type, NULL));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
            m_cacheId = getUniqueId();
            m_type = type;
            m_format = format;
            m_precision = precision;
            allTextures.push_back(this);
            return true;
        }
        bool Texture::createCubeMap(unsigned int width, unsigned int height) {
            m_isCubeMap = true;
            // Check if texture parameters are valid before creating it
            if ((width == 0) || (height == 0))
            {
                err() << "Failed to create texture, invalid size (" << width << "x" << height << ")" << std::endl;
                return false;
            }

            // Compute the internal texture dimensions depending on NPOT textures support
            Vector2u actualSize(getValidSize(width), getValidSize(height));

            // Check the maximum texture size
            unsigned int maxSize = getMaximumSize();
            if ((actualSize.x > maxSize) || (actualSize.y > maxSize))
            {
                err() << "Failed to create texture, its internal size is too high "
                      << "(" << actualSize.x << "x" << actualSize.y << ", "
                      << "maximum is " << maxSize << "x" << maxSize << ")"
                      << std::endl;
                return false;
            }
            // All the validity checks passed, we can store the new texture settings
            m_size.x        = width;
            m_size.y        = height;
            m_actualSize    = actualSize;
            m_pixelsFlipped = false;
            if (!m_texture)
            {
                GLuint texture;
                glCheck(glGenTextures(1, &texture));
                m_texture = static_cast<unsigned int>(texture);
                //glCheck(glBindImageTextures(0, 1, &m_texture));
            }
            glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture));
            for (unsigned int i = 0; i < 6; i++) {
                glCheck(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                     0,
                                     GL_RGBA,
                                     width,
                                     height,
                                     0,
                                     GL_RGBA,
                                     GL_UNSIGNED_BYTE,
                                     nullptr)
                );
            }
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
            m_cacheId = getUniqueId();
            allTextures.push_back(this);
            return true;
        }
        bool Texture::createCubeMap(unsigned int width, unsigned int height, std::vector<sf::Image> images) {
            m_isCubeMap = true;
            // Check if texture parameters are valid before creating it
            if ((width == 0) || (height == 0))
            {
                err() << "Failed to create texture, invalid size (" << width << "x" << height << ")" << std::endl;
                return false;
            }

            // Compute the internal texture dimensions depending on NPOT textures support
            Vector2u actualSize(getValidSize(width), getValidSize(height));

            // Check the maximum texture size
            unsigned int maxSize = getMaximumSize();
            if ((actualSize.x > maxSize) || (actualSize.y > maxSize))
            {
                err() << "Failed to create texture, its internal size is too high "
                      << "(" << actualSize.x << "x" << actualSize.y << ", "
                      << "maximum is " << maxSize << "x" << maxSize << ")"
                      << std::endl;
                return false;
            }
            // All the validity checks passed, we can store the new texture settings
            m_size.x        = width;
            m_size.y        = height;
            m_actualSize    = actualSize;
            m_pixelsFlipped = false;
            if (!m_texture)
            {
                GLuint texture;
                glCheck(glGenTextures(1, &texture));
                m_texture = static_cast<unsigned int>(texture);
                //glCheck(glBindImageTextures(0, 1, &m_texture));
            }
            glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture));
            for (unsigned int i = 0; i < 6; i++) {
                glCheck(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                     0,
                                     GL_RGBA,
                                     width,
                                     height,
                                     0,
                                     GL_RGBA,
                                     GL_UNSIGNED_BYTE,
                                     images[i].getPixelsPtr())
                );
            }
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            glCheck(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            m_cacheId = getUniqueId();
            allTextures.push_back(this);
            return true;
        }
        std::vector<Texture*> Texture::getAllTextures() {
            return allTextures;
        }
        ////////////////////////////////////////////////////////////
        bool Texture::loadFromFile(const std::string& filename, const IntRect& area)
        {
            Image image;
            return image.loadFromFile(filename) && loadFromImage(image, area);
        }


        ////////////////////////////////////////////////////////////
        bool Texture::loadFromMemory(const void* data, std::size_t size, const IntRect& area)
        {
            Image image;
            return image.loadFromMemory(data, size) && loadFromImage(image, area);
        }


        ////////////////////////////////////////////////////////////
        bool Texture::loadFromStream(InputStream& stream, const IntRect& area)
        {
            Image image;
            return image.loadFromStream(stream) && loadFromImage(image, area);
        }


        ////////////////////////////////////////////////////////////
        bool Texture::loadFromImage(const Image& image, const IntRect& area)
        {
            // Retrieve the image size
            int width = static_cast<int>(image.getSize().x);
            int height = static_cast<int>(image.getSize().y);

            // Load the entire image if the source area is either empty or contains the whole image
            if (area.width == 0 || (area.height == 0) ||
               ((area.left <= 0) && (area.top <= 0) && (area.width >= width) && (area.height >= height)))
            {
                //std::cout<<"width  : "<<width<<" height : "<<height<<std::endl;
                // Load the entire image
                if (create(image.getSize().x, image.getSize().y))
                {
                    update(image);

                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // Load a sub-area of the image

                // Adjust the rectangle to the size of the image
                IntRect rectangle = area;
                if (rectangle.left   < 0) rectangle.left = 0;
                if (rectangle.top    < 0) rectangle.top  = 0;
                if (rectangle.left + rectangle.width > width)  rectangle.width  = width - rectangle.left;
                if (rectangle.top + rectangle.height > height) rectangle.height = height - rectangle.top;

                // Create the texture and upload the pixels
                if (create(rectangle.width, rectangle.height))
                {

                    // Make sure that the current texture binding will be preserved
                    priv::TextureSaver save;

                    // Copy the pixels to the texture, row by row
                    const Uint8* pixels = image.getPixelsPtr() + 4 * (rectangle.left + (width * rectangle.top));
                    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                    for (int i = 0; i < rectangle.height; ++i)
                    {
                        glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, rectangle.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
                        pixels += 4 * width;
                    }

                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));


                    // Force an OpenGL flush, so that the texture will appear updated
                    // in all contexts immediately (solves problems in multi-threaded apps)
                    glCheck(glFlush());

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }


        ////////////////////////////////////////////////////////////
        Vector2u Texture::getSize() const
        {
            return m_size;
        }


        ////////////////////////////////////////////////////////////
        Image Texture::copyToImage() const
        {
            // Easy case: empty texture
            if (!m_texture)
                return Image();

            //ensureGlContext();

            // Make sure that the current texture binding will be preserved
            priv::TextureSaver save;

            // Create an array of pixels
            std::vector<Uint8> pixels(m_size.x * m_size.y * 4);

            if ((m_size == m_actualSize) && !m_pixelsFlipped)
            {
                // Texture is not padded nor flipped, we can use a direct copy
                glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                glCheck(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]));
                for (unsigned int i = 0; i < m_size.x * m_size.y * 4; i++)
                    std::cout<<"pixel : "<<(int) pixels[i]<<std::endl;
            }
            else
            {
                // Texture is either padded or flipped, we have to use a slower algorithm

                // All the pixels will first be copied to a temporary array
                std::vector<Uint8> allPixels(m_actualSize.x * m_actualSize.y * 4);
                glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                glCheck(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &allPixels[0]));

                // Then we copy the useful pixels from the temporary array to the final one
                const Uint8* src = &allPixels[0];
                Uint8* dst = &pixels[0];
                int srcPitch = m_actualSize.x * 4;
                int dstPitch = m_size.x * 4;

                // Handle the case where source pixels are flipped vertically
                if (m_pixelsFlipped)
                {
                    src += srcPitch * (m_size.y - 1);
                    srcPitch = -srcPitch;
                }

                for (unsigned int i = 0; i < m_size.y; ++i)
                {
                    std::memcpy(dst, src, dstPitch);
                    src += srcPitch;
                    dst += dstPitch;
                }
            }

            // Create the image
            Image image;
            image.create(m_size.x, m_size.y, &pixels[0]);

            return image;
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(const Uint8* pixels)
        {
            // Update the whole texture
            update(pixels, m_size.x, m_size.y, 0, 0);
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(const Uint8* pixels, unsigned int width, unsigned int height, unsigned int x, unsigned int y)
        {
            assert(x + width <= m_size.x);
            assert(y + height <= m_size.y);

            if (pixels && m_texture)
            {
                //ensureGlContext();

                // Make sure that the current texture binding will be preserved
                priv::TextureSaver save;
                /*for (unsigned int i = 0; i < width * height * 4; i++) {
                    std::cout<<"update pixel : "<<(int) pixels[i]<<std::endl;
                }*/
                // Copy pixels from the given array to the texture
                glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
                m_pixelsFlipped = false;
                m_cacheId = getUniqueId();
            }
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(const Image& image)
        {
            // Update the whole texture
            update(image.getPixelsPtr(), image.getSize().x, image.getSize().y, 0, 0);
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(const Image& image, unsigned int x, unsigned int y)
        {
            update(image.getPixelsPtr(), image.getSize().x, image.getSize().y, x, y);
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(window::Window& window)
        {
            update(window, 0, 0);
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(window::Window& window, unsigned int x, unsigned int y)
        {
            assert(x + window.getSize().x <= m_size.x);
            assert(y + window.getSize().y <= m_size.y);

            if (m_texture && window.setActive(true))
            {
                // Make sure that the current texture binding will be preserved
                priv::TextureSaver save;

                // Copy pixels from the back-buffer to the texture
                glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                glCheck(glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 0, 0, window.getSize().x, window.getSize().y));
                m_pixelsFlipped = true;
                m_cacheId = getUniqueId();
            }
        }


        ////////////////////////////////////////////////////////////
        void Texture::setSmooth(bool smooth)
        {
            if (smooth != m_isSmooth)
            {
                m_isSmooth = smooth;

                if (m_texture)
                {
                    //ensureGlContext();

                    // Make sure that the current texture binding will be preserved
                    priv::TextureSaver save;

                    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        bool Texture::isSmooth() const
        {
            return m_isSmooth;
        }


        ////////////////////////////////////////////////////////////
        void Texture::setRepeated(bool repeated)
        {
            if (repeated != m_isRepeated)
            {
                m_isRepeated = repeated;

                if (m_texture)
                {
                    //ensureGlContext();

                    // Make sure that the current texture binding will be preserved
                    priv::TextureSaver save;

                    glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_isRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        bool Texture::isRepeated() const
        {
            return m_isRepeated;
        }


        ////////////////////////////////////////////////////////////
        void Texture::bind(const Texture* texture, CoordinateType coordinateType)
        {
            //ensureGlContext();

            if (texture && texture->m_texture)
            {
                // Bind the texture
                if (!texture->m_isCubeMap) {
                    glCheck(glBindTexture(GL_TEXTURE_2D, texture->m_texture));
                } else {
                    glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, texture->m_texture));
                }
                // Check if we need to define a special texture matrix
                if ((coordinateType == Pixels) || texture->m_pixelsFlipped)
                {
                    GLfloat matrix[16] = {1.f, 0.f, 0.f, 0.f,
                                          0.f, 1.f, 0.f, 0.f,
                                          0.f, 0.f, 1.f, 0.f,
                                          0.f, 0.f, 0.f, 1.f};

                    // If non-normalized coordinates (= pixels) are requested, we need to
                    // setup scale factors that convert the range [0 .. size] to [0 .. 1]
                    if (coordinateType == Pixels)
                    {
                        matrix[0] = 1.f / texture->m_actualSize.x;
                        matrix[5] = 1.f / texture->m_actualSize.y;
                    }

                    // If pixels are flipped we must invert the Y axis
                    if (texture->m_pixelsFlipped)
                    {
                        matrix[5] = -matrix[5];
                        matrix[13] = static_cast<float>(texture->m_size.y / texture->m_actualSize.y);
                    }

                    // Load the matrix
                    glCheck(glMatrixMode(GL_TEXTURE));
                    glCheck(glLoadMatrixf(matrix));

                    // Go back to model-view mode (sf::RenderTarget relies on it)
                    glCheck(glMatrixMode(GL_MODELVIEW));
                }
            }
            else
            {
                // Bind no texture
                glCheck(glBindTexture(GL_TEXTURE_2D, 0));
                //glCheck(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
            }
        }
        math::Matrix4f Texture::getTextureMatrix() const {
            math::Matrix4f matrix(1.f, 0.f, 0.f, 0.f,
                       0.f, 1.f, 0.f, 0.f,
                       0.f, 0.f, 1.f, 0.f,
                       0.f, 0.f, 0.f, 1.f);
            matrix.m11 = 1.f / m_actualSize.x;
            matrix.m22 = 1.f / m_actualSize.y;
            /*if (m_name == "CUBE") {
                std::cout<<"actual size : "<<m_actualSize.x<<","<<m_actualSize.y<<std::endl;
            }*/
           /* if (m_pixelsFlipped)
            {
                matrix.m22 = -matrix.m22;
                matrix.m42 = 1.f;
            }*/
            return matrix;
        }
        ////////////////////////////////////////////////////////////
        unsigned int Texture::getMaximumSize()
        {
            //ensureGlContext();

            GLint size;
            glCheck(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size));
            return static_cast<unsigned int>(size);
        }


        ////////////////////////////////////////////////////////////
        Texture& Texture::operator =(const Texture& right)
        {
            Texture temp(right);

            std::swap(m_size,          temp.m_size);
            std::swap(m_actualSize,    temp.m_actualSize);
            std::swap(m_texture,       temp.m_texture);
            std::swap(m_isSmooth,      temp.m_isSmooth);
            std::swap(m_isRepeated,    temp.m_isRepeated);
            std::swap(m_pixelsFlipped, temp.m_pixelsFlipped);
            m_cacheId = getUniqueId();

            return *this;
        }


        ////////////////////////////////////////////////////////////
        unsigned int Texture::getValidSize(unsigned int size)
        {
            //ensureGlContext();

            // Make sure that GLEW is initialized
            priv::ensureGlewInit();

            if (GLEW_ARB_texture_non_power_of_two)
            {
                // If hardware supports NPOT textures, then just return the unmodified size
                return size;
            }
            else
            {
                // If hardware doesn't support NPOT textures, we calculate the nearest power of two
                unsigned int powerOfTwo = 1;
                while (powerOfTwo < size)
                    powerOfTwo *= 2;

                return powerOfTwo;
            }
        }
        void Texture::clear() {
            glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, m_precision, 0, 0, m_size.x, m_size.y, m_format,
            m_type, NULL));
        }
        void Texture::onSave(std::vector<sf::Uint8>& vPixels) {
            glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
            const std::size_t size = 4 * m_size.x * m_size.y;
            sf::Uint8* pixels = new sf::Uint8[size];
            glCheck(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
            vPixels.assign(pixels, pixels + size);
        }
        void Texture::onLoad(std::vector<sf::Uint8>& vPixels) {
            sf::Uint8* pixels = &vPixels[0];
            create(m_size.x, m_size.y);
            glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
            glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.x, m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
            glCheck(glFlush());
        }
        const sf::Image& Texture::getImage() const {
            return m_image;
        }
        unsigned int Texture::getNativeHandle() const {
            return m_texture;
        }
        bool Texture::isCubemap() {
            return m_isCubeMap;
        }
        void Texture::setName (std::string name) {
            m_name = name;
        }
        ////////////////////////////////////////////////////////////
        void Texture::update(const Texture& texture)
        {
            // Update the whole texture
            update(texture, 0, 0);
        }


        ////////////////////////////////////////////////////////////
        void Texture::update(const Texture& texture, unsigned int x, unsigned int y)
        {
            GLint readFramebuffer = 0;
            GLint drawFramebuffer = 0;

            glCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer));
            glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer));

            // Create the framebuffers
            GLuint sourceFrameBuffer = 0;
            GLuint destFrameBuffer = 0;
            glCheck(glGenFramebuffers(1, &sourceFrameBuffer));
            glCheck(glGenFramebuffers(1, &destFrameBuffer));

            if (!sourceFrameBuffer || !destFrameBuffer)
            {
                err() << "Cannot copy texture, failed to create a frame buffer object" << std::endl;
                return;
            }

            // Link the source texture to the source frame buffer
            glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFrameBuffer));
            glCheck(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.m_texture, 0));

            // Link the destination texture to the destination frame buffer
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFrameBuffer));
            glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0));

            // A final check, just to be sure...
            GLenum sourceStatus;
            glCheck(sourceStatus = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

            GLenum destStatus;
            glCheck(destStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

            if ((sourceStatus == GL_FRAMEBUFFER_COMPLETE) && (destStatus == GL_FRAMEBUFFER_COMPLETE))
            {
                // Blit the texture contents from the source to the destination texture
                glCheck(glBlitFramebuffer(
                    0, texture.m_pixelsFlipped ? texture.m_size.y : 0, texture.m_size.x, texture.m_pixelsFlipped ? 0 : texture.m_size.y, // Source rectangle, flip y if source is flipped
                    x, y, x + texture.m_size.x, y + texture.m_size.y, // Destination rectangle
                    GL_COLOR_BUFFER_BIT, GL_NEAREST
                ));
            }
            else
            {
                err() << "Cannot copy texture, failed to link texture to frame buffer" << std::endl;
            }

            // Restore previously bound framebuffers
            glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer));
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer));

            // Delete the framebuffers
            glCheck(glDeleteFramebuffers(1, &sourceFrameBuffer));
            glCheck(glDeleteFramebuffers(1, &destFrameBuffer));

            // Make sure that the current texture binding will be preserved
            priv::TextureSaver save;

            // Set the parameters of this texture
            glCheck(glBindTexture(GL_TEXTURE_2D, m_texture));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_isSmooth ? GL_LINEAR : GL_NEAREST));
            m_pixelsFlipped = false;
            m_cacheId = getUniqueId();

            // Force an OpenGL flush, so that the texture data will appear updated
            // in all contexts immediately (solves problems in multi-threaded apps)
            glCheck(glFlush());

            return;
            assert(x + texture.m_size.x <= m_size.x);
            assert(y + texture.m_size.y <= m_size.y);

            if (!m_texture || !texture.m_texture)
                return;
            update(texture.copyToImage(), x, y);
        }
        ////////////////////////////////////////////////////////////
        void Texture::setNativeHandle(unsigned int handle, unsigned int width, unsigned int height) {
            m_texture = handle;
            m_actualSize.x = width;
            m_actualSize.y = height;
        }
        unsigned int Texture::getId() const {
            return id;
        }
        void Texture::swap(Texture& right)
        {
            std::swap(m_size,          right.m_size);
            std::swap(m_actualSize,    right.m_actualSize);
            std::swap(m_texture,       right.m_texture);
            std::swap(m_isSmooth,      right.m_isSmooth);
            std::swap(m_isRepeated,    right.m_isRepeated);
            std::swap(m_pixelsFlipped, right.m_pixelsFlipped);
            m_cacheId = getUniqueId();
            right.m_cacheId = getUniqueId();
        }
        #endif
    }
} // namespace sf
