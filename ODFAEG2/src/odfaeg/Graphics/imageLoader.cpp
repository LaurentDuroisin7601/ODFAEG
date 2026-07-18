module;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <gli.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vulkan/vulkan_core.h>
//import odfaeg.graphic.imageLoader;
module odfaeg.graphic.imageLoader;
import odfaeg.core.inputStream;
namespace odfaeg{
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
            core::InputStream* stream = static_cast<core::InputStream*>(user);
            return static_cast<int>(stream->read(data, size));
        }
        void skip(void* user, int size)
        {
            core::InputStream* stream = static_cast<core::InputStream*>(user);
            stream->seek(stream->tell() + size);
        }
        int eof(void* user)
        {
            core::InputStream* stream = static_cast<core::InputStream*>(user);
            return stream->tell() >= stream->getSize();
        }

        ////////////////////////////////////////////////////////////
        ImageLoader::ImageLoader()
        {
            m_sizes.push_back(math::Vector2u(0u, 0u));
            dataSizes.push_back(0);
            mipLevels = 1;
        }


        ////////////////////////////////////////////////////////////
        ImageLoader::~ImageLoader()
        {

        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::create(unsigned int width, unsigned int height, const entity::Color& color)
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
                m_pixels[0].swap(newPixels);
                size_t dataSize = m_pixels[0].size();

                // Assign the new size
                dataSizes[0] = dataSize;
            }
            else
            {
                // Dump the pixel buffer
                std::vector<std::uint8_t>().swap(m_pixels[0]);

                // Assign the new size
                m_sizes[0] = math::Vector2u(0u, 0u);
            }
        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::create(unsigned int width, unsigned int height, const std::uint8_t* pixels)
        {

            if (pixels && width && height)
            {

                // Create a new pixel buffer first for exception safety's sake
                std::vector<std::uint8_t> newPixels(pixels, pixels + width * height * 4);

                // Commit the new pixel buffer
                m_pixels[0].swap(newPixels);
                dataSizes[0] = width * height * 4;

                // Assign the new size
                m_sizes[0] = math::Vector2u(width, height);
            }
            else
            {
                // Dump the pixel buffer
                std::vector<std::uint8_t>().swap(m_pixels[0]);

                // Assign the new size
                m_sizes[0] = math::Vector2u(0u, 0u);
            }
        }

        std::string ImageLoader::getExtension(const std::string& filename) {
            std::filesystem::path p(filename);
            std::string ext = p.extension().string();

            // enlever le point ".dds"  "dds"
            if (!ext.empty() && ext[0] == '.')
                ext.erase(0, 1);

            // mettre en minuscule
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            return ext;
        }
        VkFormat ImageLoader::toVkFormat()
        {
            //std::cout<<"format : "<<mFormat<<std::endl;
            switch (mFormat)
            {
                case gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
                    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
                case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
                    return VK_FORMAT_BC3_UNORM_BLOCK;
                case gli::FORMAT_RG_ATI2N_UNORM_BLOCK16:
                    return VK_FORMAT_BC5_UNORM_BLOCK;
                default:
                    throw std::runtime_error("Unsupported DDS format");
            }
        }

        size_t ImageLoader::getMipLevels() {
            return mipLevels;
        }

        ////////////////////////////////////////////////////////////
        bool ImageLoader::loadFromFile(const std::string& filename)
        {
            #ifndef ODFAEG_SYSTEM_ANDROID

            // Clear the array (just in case)


            // Load the image and get a pointer to the pixels in memory
            int width = 0;
            int height = 0;
            int channels = 0;
            unsigned char* ptr = nullptr;

            if (getExtension(filename) == "dds") {
                //std::cout<<"filename : "<<filename<<std::endl;
                gli::texture texture = gli::load(filename);
                mipLevels = texture.levels();
                //std::cout<<"mip level : "<<mipLevels<<std::endl;
                mFormat = texture.format();
                //std::cout<<"gli format : "<<static_cast<int>(mFormat)<<std::endl;
                vkFormat =  toVkFormat();
                compressed = true;
                m_pixels.clear();
                m_pixels.resize(mipLevels);
                dataSizes.clear();
                m_sizes.clear();
                dataSizes.resize(mipLevels);
                m_sizes.resize(mipLevels);
                /*if (vkFormat == VK_FORMAT_UNDEFINED)
                    throw std::runtime_error("Unsupported DDS format");*/
                switch (mFormat) {
                    case gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
                    case gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8:
                        channels = 3;
                        break;

                    case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
                    case gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16:
                        channels = 4;
                        break;

                    case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
                    case gli::FORMAT_RGBA_BP_SRGB_BLOCK16:
                        channels = 4;
                        break;

                    default:
                        channels = gli::detail::bits_per_pixel(mFormat) / 8;
                        break;
                }
                if (texture.empty()) {
                    throw std::runtime_error("Failed to load DDS");
                }
                for (unsigned int i = 0; i < mipLevels; i++) {
                    m_pixels[i].clear();

                    ptr = reinterpret_cast<unsigned char*>(texture.data(0, 0, i));
                    if (ptr) {

                        width = texture.extent(i).x;
                        height = texture.extent(i).y;
                        size_t dataSize = texture.size(i);

                        m_sizes[i] = math::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
                        /*std::cout << "levels   : " << texture.levels() << std::endl;
                        std::cout << "layers   : " << texture.layers() << std::endl;
                        std::cout << "faces    : " << texture.faces() << std::endl;
                        std::cout << "size()   : " << texture.size() << std::endl;
                        std::cout << "size(i)  : " << texture.size(i) << std::endl;*/

                        //std::cout<<"w : "<<width<<", h : "<<height<<std::endl;
                        dataSizes[i] = dataSize;
                        //std::cout<<"width : "<<width<<",height : "<<height<<",dataSize : "<<dataSize<<std::endl;
                        if (width && height)
                        {
                            // Copy the loaded pixels to the pixel buffer

                            m_pixels[i].resize(dataSize);
                            memcpy(m_pixels[i].data(), ptr, dataSize);
                            //std::cout<<"size : "<<m_pixels[i].size()<<",mip level : "<<i<<std::endl;
                        }
                        /*for (int i = 0; i < dataSize; i++) {
                            if ((int)m_pixels[dataSize] == 0)
                                std::cout << "Pixel[" << i << "] = " << (int)m_pixels[dataSize]<< std::endl;
                        }*/
                        //std::cout<<"compressed image loaded"<<std::endl;
                    } else {
                        std::cerr << "Failed to load compressed image \"" << std::endl;
                        return false;
                    }
                }
                return true;
            }
            //std::cout<<"load stb"<<std::endl;
            ptr = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (ptr)
            {
                // Assign the image properties
                m_sizes.clear();
                dataSizes.clear();
                m_sizes.push_back(math::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height)));
                m_pixels.clear();
                m_pixels.resize(1);
                if (width && height)
                {
                    // Copy the loaded pixels to the pixel buffer
                    m_pixels[0].resize(width * height * 4);
                    memcpy(m_pixels[0].data(), ptr, m_pixels[0].size());
                }
                dataSizes.push_back(width * height * 4);
                /*bool empty = true;
                     for (unsigned int i = 0; i < width * height; i++) {
                         if (m_pixels[i*4] != 0 || m_pixels[i*4+1] != 0 || m_pixels[i*4+2] != 0 || m_pixels[i*4+3]!=0)
                             empty = false;

                     }*/
                /*for (int i = 0; i < width*height; i++) {
                         if ((int)m_pixels[i*4 + 3] == 0)
                            std::cout << "A[" << i << "] = " << (int)m_pixels[i*4]<<','<<(int)m_pixels[i*4 + 1]<<","<<(int)m_pixels[i*4 + 2]<< std::endl;
                     }*/

                // Free the loaded pixels (they are now in our own pixel buffer)
                stbi_image_free(ptr);
                //saveToFile(filename);
                //std::cout<<"sucess"<<std::endl;
                return true;
            }
            //std::cout<<"fail : "<<std::endl;
            // Error, failed to load the image
            std::cerr << "Failed to load image \"" << filename << "\". Reason: " << stbi_failure_reason() << std::endl;

            return false;
            #else

            #endif
        }

        bool ImageLoader::isCompressed() const {
            return compressed;
        }
        gli::format ImageLoader::getFormat() const {
            return mFormat;
        }
        size_t ImageLoader::getDataSize(unsigned int mipLevel) const {
            return dataSizes[mipLevel];
        }
        VkFormat ImageLoader::getVkFormat() const {
            return vkFormat;
        }
        ////////////////////////////////////////////////////////////
        bool ImageLoader::loadFromMemory(const void* data, std::size_t size)
        {
            // Check input parameters
            if (data && size)
            {
                // Clear the array (just in case)
                m_pixels.clear();
                m_pixels.resize(1);
                m_pixels[0].clear();
                dataSizes.clear();
                m_sizes.clear();

                // Load the image and get a pointer to the pixels in memory
                int width = 0;
                int height = 0;
                int channels = 0;
                const unsigned char* buffer = static_cast<const unsigned char*>(data);
                unsigned char* ptr = stbi_load_from_memory(buffer, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);

                if (ptr)
                {
                    // Assign the image properties
                    m_sizes.push_back(math::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height)));

                    if (width && height)
                    {
                        // Copy the loaded pixels to the pixel buffer
                        m_pixels[0].resize(width * height * 4);
                        dataSizes.push_back(width * height * 4);
                        memcpy(m_pixels[0].data(), ptr, m_pixels[0].size());
                    }

                    // Free the loaded pixels (they are now in our own pixel buffer)
                    stbi_image_free(ptr);

                    return true;
                }
                else
                {
                    // Error, failed to load the image
                    std::cout << "Failed to load image from memory. Reason: " << stbi_failure_reason() << std::endl;

                    return false;
                }
            }
            else
            {
                std::cout << "Failed to load image from memory, no data provided" << std::endl;
                return false;
            }
        }


        ////////////////////////////////////////////////////////////
        bool ImageLoader::loadFromStream(core::InputStream& stream)
        {
            // Clear the array (just in case)
            m_pixels.resize(1);
            m_pixels[0].clear();

            // Make sure that the stream's reading position is at the beginning
            stream.seek(0);

            // Setup the stb_image callbacks
            stbi_io_callbacks callbacks;
            callbacks.read = &read;
            callbacks.skip = &skip;
            callbacks.eof = &eof;

            // Load the image and get a pointer to the pixels in memory
            int width = 0;
            int height = 0;
            int channels = 0;
            unsigned char* ptr = stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &channels, STBI_rgb_alpha);

            if (ptr)
            {
                // Assign the image properties
                m_sizes.push_back(math::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height)));

                if (width && height)
                {
                    // Copy the loaded pixels to the pixel buffer
                    m_pixels[0].resize(width * height * 4);
                    memcpy(m_pixels[0].data(), ptr, m_pixels.size());
                    dataSizes.push_back(width * height * 4);
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
        bool ImageLoader::saveToFile(const std::string& filename) const
        {
            // Make sure the image is not empty
            if (!m_pixels[0].empty() && (m_sizes[0].x() > 0) && (m_sizes[0].y() > 0))
            {
                // Deduce the image type from its extension

                // Extract the extension
                const std::size_t dot = filename.find_last_of('.');
                const std::string extension = dot != std::string::npos ? toLower(filename.substr(dot + 1)) : "";

                if (extension == "bmp")
                {
                    // BMP format
                    if (stbi_write_bmp(filename.c_str(), m_sizes[0].x(), m_sizes[0].y(), 4, &m_pixels[0][0]))
                        return true;
                }
                else if (extension == "tga")
                {
                    // TGA format
                    if (stbi_write_tga(filename.c_str(), m_sizes[0].x(), m_sizes[0].y(), 4, &m_pixels[0][0]))
                        return true;
                }
                else if (extension == "png")
                {
                    // PNG format
                    if (stbi_write_png(filename.c_str(), m_sizes[0].x(), m_sizes[0].y(), 4, &m_pixels[0][0], 0))
                        return true;
                }
                else if (extension == "jpg" || extension == "jpeg")
                {
                    // JPG format
                    if (stbi_write_jpg(filename.c_str(), m_sizes[0].x(), m_sizes[0].y(), 4, &m_pixels[0][0], 90))
                        return true;
                }
            }

            std::cerr << "Failed to save image \"" << filename << "\"" << std::endl;
            return false;
        }


        ////////////////////////////////////////////////////////////
        math::Vector2u ImageLoader::getSize(size_t mipLevel) const
        {
            return m_sizes[mipLevel];
        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::createMaskFromColor(const entity::Color& color, std::uint8_t alpha)
        {
            // Make sure that the image is not empty
            if (!m_pixels[0].empty())
            {
                // Replace the alpha of the pixels that match the transparent color
                std::uint8_t* ptr = m_pixels[0].data();
                std::uint8_t* end = ptr + m_pixels[0].size();
                while (ptr < end)
                {
                    if ((ptr[0] == color.r) && (ptr[1] == color.g) && (ptr[2] == color.b) && (ptr[3] == color.a))
                        ptr[3] = alpha;
                    ptr += 4;
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::copy(const ImageLoader& source, unsigned int destX, unsigned int destY, const entity::IntRect& sourceRect, bool applyAlpha)
        {
            // Make sure that both images are valid
            if ((source.m_sizes[0].x() == 0) || (source.m_sizes[0].y() == 0) || (m_sizes[0].x() == 0) || (m_sizes[0].y() == 0))
                return;

            // Adjust the source rectangle
            entity::IntRect srcRect = sourceRect;
            if (srcRect.width == 0 || (srcRect.height == 0))
            {
                srcRect.left = 0;
                srcRect.top = 0;
                srcRect.width = source.m_sizes[0].x();
                srcRect.height = source.m_sizes[0].y();
            }
            else
            {
                if (srcRect.left < 0) srcRect.left = 0;
                if (srcRect.top < 0) srcRect.top = 0;
                if (srcRect.width > static_cast<int>(source.m_sizes[0].x())) srcRect.width = source.m_sizes[0].x();
                if (srcRect.height > static_cast<int>(source.m_sizes[0].y())) srcRect.height = source.m_sizes[0].y();
            }

            // Then find the valid bounds of the destination rectangle
            int width = srcRect.width;
            int height = srcRect.height;
            if (destX + width > m_sizes[0].x()) width = m_sizes[0].x() - destX;
            if (destY + height > m_sizes[0].y()) height = m_sizes[0].y() - destY;

            // Make sure the destination area is valid
            if ((width <= 0) || (height <= 0))
                return;

            // Precompute as much as possible
            int          pitch = width * 4;
            int          rows = height;
            int          srcStride = source.m_sizes[0].x() * 4;
            int          dstStride = m_sizes[0].x() * 4;
            const std::uint8_t* srcPixels = &source.m_pixels[0][0] + (srcRect.left + srcRect.top * source.m_sizes[0].x()) * 4;
            std::uint8_t* dstPixels = &m_pixels[0][0] + (destX + destY * m_sizes[0].x()) * 4;

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
                        std::uint8_t* dst = dstPixels + j * 4;

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
            //dataSize = m_pixels[0].size();
        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::setPixel(unsigned int x, unsigned int y, const entity::Color& color)
        {
            std::uint8_t* pixel = &m_pixels[0][(x + y * m_sizes[0].x()) * 4];
            *pixel++ = color.r;
            *pixel++ = color.g;
            *pixel++ = color.b;
            *pixel++ = color.a;
        }
        ////////////////////////////////////////////////////////////
        entity::Color ImageLoader::getPixel(unsigned int x, unsigned int y) const
        {
            const std::uint8_t* pixel = &m_pixels[0][(x + y * m_sizes[0].x()) * 4];
            return entity::Color(pixel[0], pixel[1], pixel[2], pixel[3]);
        }


        ////////////////////////////////////////////////////////////
        const std::uint8_t* ImageLoader::getPixelsPtr(unsigned int mipLevel) const
        {
            //std::cout<<"mip level : "<<mipLevel<<std::endl;
            if (!m_pixels[mipLevel].empty())
            {
                return m_pixels[mipLevel].data();
            }
            else
            {
                std::cerr << "Trying to access the pixels of an empty image" << std::endl;
                return NULL;
            }
        }
        void* ImageLoader::getCompressedDatas() {
            return compressedDatas;
        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::flipHorizontally()
        {
            if (!m_pixels.empty())
            {
                std::size_t rowSize = m_sizes[0].x() * 4;

                for (std::size_t y = 0; y < m_sizes[0].y(); ++y)
                {
                    std::vector<std::uint8_t>::iterator left = m_pixels[0].begin() + y * rowSize;
                    std::vector<std::uint8_t>::iterator right = m_pixels[0].begin() + (y + 1) * rowSize - 4;

                    for (std::size_t x = 0; x < m_sizes[0].x() / 2; ++x)
                    {
                        std::swap_ranges(left, left + 4, right);

                        left += 4;
                        right -= 4;
                    }
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void ImageLoader::flipVertically()
        {
            if (!m_pixels.empty())
            {
                std::size_t rowSize = m_sizes[0].x() * 4;

                std::vector<std::uint8_t>::iterator top = m_pixels[0].begin();
                std::vector<std::uint8_t>::iterator bottom = m_pixels[0].end() - rowSize;

                for (std::size_t y = 0; y < m_sizes[0].y() / 2; ++y)
                {
                    std::swap_ranges(top, top + rowSize, bottom);

                    top += rowSize;
                    bottom -= rowSize;
                }
            }
        }
    }
}
