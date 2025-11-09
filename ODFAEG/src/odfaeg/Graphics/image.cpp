#include "../../../include/odfaeg/Graphics/image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <cstring>
namespace odfaeg {
    namespace graphic {
        std::string toLower(std::string str)
        {
            for (std::string::iterator i = str.begin(); i != str.end(); ++i)
                *i = static_cast<char>(std::tolower(*i));
            return str;
        }
        // stb_image callbacks that operate on a sf::InputStream
        int read(void* user, char* data, int size)
        {
            std::istream* stream = static_cast<std::istream*>(user);
            stream->read(data, size);
            return stream->gcount();
        }
        void skip(void* user, int size)
        {
            std::istream* stream = static_cast<std::istream*>(user);
            stream->seekg(stream->tellg() + size, stream->beg);
        }
        int eof(void* user)
        {
            std::istream* stream = static_cast<std::istream*>(user);
            unsigned int offset = stream->tellg();
            stream->seekg(0, std::ios::end);
            unsigned int size = stream->tellg();
            stream->seekg(offset, std::ios::beg);
            return stream->tellg() >= size;
        }

        ////////////////////////////////////////////////////////////
        Image::Image() :
        m_size(0, 0)
        {

        }


        ////////////////////////////////////////////////////////////
        Image::~Image()
        {

        }


        ////////////////////////////////////////////////////////////
        void Image::create(unsigned int width, unsigned int height, const Color& color)
        {
            if (width && height)
            {
                // Create a new pixel buffer first for exception safety's sake
                std::vector<std::uint8_t> newPixels(width * height * 4);

                // Fill it with the specified color
                std::uint8_t* ptr = &newPixels[0];
                std::uint8_t* end = ptr + newPixels.size();
                while (ptr < end)
                {
                    *ptr++ = color.r;
                    *ptr++ = color.g;
                    *ptr++ = color.b;
                    *ptr++ = color.a;
                }

                // Commit the new pixel buffer
                m_pixels.swap(newPixels);

                // Assign the new size
                m_size[0] = width;
                m_size[1] = height;
            }
            else
            {
                // Dump the pixel buffer
                std::vector<std::uint8_t>().swap(m_pixels);

                // Assign the new size
                m_size[0] = 0;
                m_size[1] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        void Image::create(unsigned int width, unsigned int height, const std::uint8_t* pixels)
        {

            if (pixels && width && height)
            {

                // Create a new pixel buffer first for exception safety's sake
                std::vector<std::uint8_t> newPixels(pixels, pixels + width * height * 4);

                // Commit the new pixel buffer
                m_pixels.swap(newPixels);

                // Assign the new size
                m_size[0] = width;
                m_size[1] = height;
            }
            else
            {
                // Dump the pixel buffer
                std::vector<std::uint8_t>().swap(m_pixels);

                // Assign the new size
                m_size[0] = 0;
                m_size[1] = 0;
            }
        }


        ////////////////////////////////////////////////////////////
        bool Image::loadFromFile(const std::string& filename)
        {
            #ifndef ODFAEG_SYSTEM_ANDROID

                // Clear the array (just in case)
                m_pixels.clear();

                // Load the image and get a pointer to the pixels in memory
                int width = 0;
                int height = 0;
                int channels = 0;
                unsigned char* ptr = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

                if (ptr)
                {
                    // Assign the image properties
                    m_size[0] = width;
                    m_size[1] = height;

                    if (width && height)
                    {
                        // Copy the loaded pixels to the pixel buffer
                        m_pixels.resize(width * height * 4);
                        memcpy(&m_pixels[0], ptr, m_pixels.size());
                    }
                    /*bool empty = true;
                    for (unsigned int i = 0; i < width * height; i++) {
                        if (m_pixels[i*4] != 0 || m_pixels[i*4+1] != 0 || m_pixels[i*4+2] != 0 || m_pixels[i*4+3]!=0)
                            empty = false;

                    }
                    //////std::cout<<"empty : "<<empty<<std::endl;*/
                    // Free the loaded pixels (they are now in our own pixel buffer)
                    stbi_image_free(ptr);
                    //saveToFile(filename);
                    return true;
                }
                else
                {
                    // Error, failed to load the image
                    std::cerr << "Failed to load image \"" << filename << "\". Reason: " << stbi_failure_reason() << std::endl;

                    return false;
                }

            #else

            #endif
        }


        ////////////////////////////////////////////////////////////
        bool Image::loadFromMemory(const void* data, std::size_t size)
        {
            // Check input parameters
            if (data && size)
            {
                // Clear the array (just in case)
                m_pixels.clear();

                // Load the image and get a pointer to the pixels in memory
                int width = 0;
                int height = 0;
                int channels = 0;
                const unsigned char* buffer = static_cast<const unsigned char*>(data);
                unsigned char* ptr = stbi_load_from_memory(buffer, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);

                if (ptr)
                {
                    // Assign the image properties
                    m_size[0] = width;
                    m_size[1] = height;

                    if (width && height)
                    {
                        // Copy the loaded pixels to the pixel buffer
                        m_pixels.resize(width * height * 4);
                        memcpy(&m_pixels[0], ptr, m_pixels.size());
                    }

                    // Free the loaded pixels (they are now in our own pixel buffer)
                    stbi_image_free(ptr);

                    return true;
                }
                else
                {
                    // Error, failed to load the image
                    std::cerr << "Failed to load image from memory. Reason: " << stbi_failure_reason() << std::endl;

                    return false;
                }
            }
            else
            {
                std::cerr << "Failed to load image from memory, no data provided" << std::endl;
                return false;
            }
        }


        ////////////////////////////////////////////////////////////
        bool Image::loadFromStream(std::istream& stream)
        {
            // Clear the array (just in case)
            m_pixels.clear();

            // Make sure that the stream's reading position is at the beginning
            stream.seekg(0, stream.beg);

            // Setup the stb_image callbacks
            stbi_io_callbacks callbacks;
            callbacks.read = &read;
            callbacks.skip = &skip;
            callbacks.eof  = &eof;

            // Load the image and get a pointer to the pixels in memory
            int width = 0;
            int height = 0;
            int channels = 0;
            unsigned char* ptr = stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &channels, STBI_rgb_alpha);

            if (ptr)
            {
                // Assign the image properties
                m_size[0] = width;
                m_size[1] = height;

                if (width && height)
                {
                    // Copy the loaded pixels to the pixel buffer
                    m_pixels.resize(width * height * 4);
                    memcpy(&m_pixels[0], ptr, m_pixels.size());
                }

                // Free the loaded pixels (they are now in our own pixel buffer)
                stbi_image_free(ptr);

                return true;
            }
            else
            {
                // Error, failed to load the image
                std::cerr << "Failed to load image from stream. Reason: " << stbi_failure_reason() << std::endl;

                return false;
            }
        }


        ////////////////////////////////////////////////////////////
        bool Image::saveToFile(const std::string& filename) const
        {
            // Make sure the image is not empty
            if (!m_pixels.empty() && (m_size.x() > 0) && (m_size.y() > 0))
            {
                // Deduce the image type from its extension

                // Extract the extension
                const std::size_t dot = filename.find_last_of('.');
                const std::string extension = dot != std::string::npos ? toLower(filename.substr(dot + 1)) : "";

                if (extension == "bmp")
                {
                    // BMP format
                    if (stbi_write_bmp(filename.c_str(), m_size.x(), m_size.y(), 4, &m_pixels[0]))
                        return true;
                }
                else if (extension == "tga")
                {
                    // TGA format
                    if (stbi_write_tga(filename.c_str(), m_size.x(), m_size.y(), 4, &m_pixels[0]))
                        return true;
                }
                else if (extension == "png")
                {
                    // PNG format
                    if (stbi_write_png(filename.c_str(), m_size.x(), m_size.y(), 4, &m_pixels[0], 0))
                        return true;
                }
                else if (extension == "jpg" || extension == "jpeg")
                {
                    // JPG format
                    if (stbi_write_jpg(filename.c_str(), m_size.x(), m_size.y(), 4, &m_pixels[0], 90))
                        return true;
                }
            }

            std::cerr << "Failed to save image \"" << filename << "\"" << std::endl;
            return false;
        }


        ////////////////////////////////////////////////////////////
        math::Vector2u Image::getSize() const
        {
            return m_size;
        }


        ////////////////////////////////////////////////////////////
        void Image::createMaskFromColor(const Color& color, std::uint8_t alpha)
        {
            // Make sure that the image is not empty
            if (!m_pixels.empty())
            {
                // Replace the alpha of the pixels that match the transparent color
                std::uint8_t* ptr = &m_pixels[0];
                std::uint8_t* end = ptr + m_pixels.size();
                while (ptr < end)
                {
                    if ((ptr[0] == color.r) && (ptr[1] == color.g) && (ptr[2] == color.b) && (ptr[3] == color.a))
                        ptr[3] = alpha;
                    ptr += 4;
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Image::copy(const Image& source, unsigned int destX, unsigned int destY, const IntRect& sourceRect, bool applyAlpha)
        {
            // Make sure that both images are valid
            if ((source.m_size.x() == 0) || (source.m_size.y() == 0) || (m_size.x() == 0) || (m_size.y() == 0))
                return;

            // Adjust the source rectangle
            IntRect srcRect = sourceRect;
            if (srcRect.width == 0 || (srcRect.height == 0))
            {
                srcRect.left   = 0;
                srcRect.top    = 0;
                srcRect.width  = source.m_size.x();
                srcRect.height = source.m_size.y();
            }
            else
            {
                if (srcRect.left   < 0) srcRect.left = 0;
                if (srcRect.top    < 0) srcRect.top  = 0;
                if (srcRect.width  > static_cast<int>(source.m_size.x())) srcRect.width  = source.m_size.x();
                if (srcRect.height > static_cast<int>(source.m_size.y())) srcRect.height = source.m_size.y();
            }

            // Then find the valid bounds of the destination rectangle
            int width  = srcRect.width;
            int height = srcRect.height;
            if (destX + width  > m_size.x()) width  = m_size.x() - destX;
            if (destY + height > m_size.y()) height = m_size.y() - destY;

            // Make sure the destination area is valid
            if ((width <= 0) || (height <= 0))
                return;

            // Precompute as much as possible
            int          pitch     = width * 4;
            int          rows      = height;
            int          srcStride = source.m_size.x() * 4;
            int          dstStride = m_size.x() * 4;
            const std::uint8_t* srcPixels = &source.m_pixels[0] + (srcRect.left + srcRect.top * source.m_size.x()) * 4;
            std::uint8_t*       dstPixels = &m_pixels[0] + (destX + destY * m_size.x()) * 4;

            // Copy the pixels
            if (applyAlpha)
            {
                // Interpolation using alpha values, pixel by pixel (slower)
                for (int i = 0; i < rows; ++i)
                {
                    for (int j = 0; j < width; ++j)
                    {
                        // Get a direct pointer to the components of the current pixel
                        const std::uint8_t* src = srcPixels + j * 4;
                        std::uint8_t*       dst = dstPixels + j * 4;

                        // Interpolate RGBA components using the alpha value of the source pixel
                        std::uint8_t alpha = src[3];
                        dst[0] = (src[0] * alpha + dst[0] * (255 - alpha)) / 255;
                        dst[1] = (src[1] * alpha + dst[1] * (255 - alpha)) / 255;
                        dst[2] = (src[2] * alpha + dst[2] * (255 - alpha)) / 255;
                        dst[3] = alpha + dst[3] * (255 - alpha) / 255;
                    }

                    srcPixels += srcStride;
                    dstPixels += dstStride;
                }
            }
            else
            {
                // Optimized copy ignoring alpha values, row by row (faster)
                for (int i = 0; i < rows; ++i)
                {
                    std::memcpy(dstPixels, srcPixels, pitch);
                    srcPixels += srcStride;
                    dstPixels += dstStride;
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Image::setPixel(unsigned int x, unsigned int y, const Color& color)
        {
            std::uint8_t* pixel = &m_pixels[(x + y * m_size.x()) * 4];
            *pixel++ = color.r;
            *pixel++ = color.g;
            *pixel++ = color.b;
            *pixel++ = color.a;
        }


        ////////////////////////////////////////////////////////////
        Color Image::getPixel(unsigned int x, unsigned int y) const
        {
            const std::uint8_t* pixel = &m_pixels[(x + y * m_size.x()) * 4];
            return Color(pixel[0], pixel[1], pixel[2], pixel[3]);
        }


        ////////////////////////////////////////////////////////////
        const std::uint8_t* Image::getPixelsPtr() const
        {
            if (!m_pixels.empty())
            {
                return &m_pixels[0];
            }
            else
            {
                std::cerr << "Trying to access the pixels of an empty image" << std::endl;
                return NULL;
            }
        }


        ////////////////////////////////////////////////////////////
        void Image::flipHorizontally()
        {
            if (!m_pixels.empty())
            {
                std::size_t rowSize = m_size.x() * 4;

                for (std::size_t y = 0; y < m_size.y(); ++y)
                {
                    std::vector<std::uint8_t>::iterator left = m_pixels.begin() + y * rowSize;
                    std::vector<std::uint8_t>::iterator right = m_pixels.begin() + (y + 1) * rowSize - 4;

                    for (std::size_t x = 0; x < m_size.x() / 2; ++x)
                    {
                        std::swap_ranges(left, left + 4, right);

                        left += 4;
                        right -= 4;
                    }
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Image::flipVertically()
        {
            if (!m_pixels.empty())
            {
                std::size_t rowSize = m_size.x() * 4;

                std::vector<std::uint8_t>::iterator top = m_pixels.begin();
                std::vector<std::uint8_t>::iterator bottom = m_pixels.end() - rowSize;

                for (std::size_t y = 0; y < m_size.y() / 2; ++y)
                {
                    std::swap_ranges(top, top + rowSize, bottom);

                    top += rowSize;
                    bottom -= rowSize;
                }
            }
        }
    }
}
