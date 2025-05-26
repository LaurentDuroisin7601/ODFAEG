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
#include "../../../include/odfaeg/Graphics/texture.h"
#ifndef VULKAN
#include "GL/glew.h"
#include "glCheck.h"
#endif

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

        #ifdef VULKAN
        std::vector<Texture*> Texture::allTextures = std::vector<Texture*>();
        unsigned int Texture::nbTextures = 0;
        Texture::Texture(window::Device& vkDevice) : vkDevice(vkDevice), id(0), textureImage(nullptr), textureImageView(nullptr), m_cacheId (getUniqueId()), ct(UNORM), isCubeMap(false) {
            createCommandPool();
        }
        Texture::Texture(const Texture& copy) :
        m_size         (0, 0),
        m_actualSize   (0, 0),
        m_texture      (0),
        m_isSmooth     (copy.m_isSmooth),
        m_isRepeated   (copy.m_isRepeated),
        m_cacheId      (getUniqueId()),
        vkDevice(copy.vkDevice),
        id(nbTextures+1),
        ct(copy.ct)
        {
            if (copy.textureImage)
            {
                if (create(copy.getSize().x, copy.getSize().y))
                {
                    update(copy);
                    // Force an OpenGL flush, so that the texture will appear updated
                    // in all contexts immediately (solves problems in multi-threaded apps)

                }
                else
                {
                    err() << "Failed to copy texture, failed to create new texture" << std::endl;
                }
            }
            nbTextures++;
        }
        Texture::Texture (Texture&& tex) : vkDevice(tex.vkDevice),
        textureImage(std::exchange(tex.textureImage, nullptr)),
        textureImageView(std::exchange(tex.textureImageView, nullptr)),
        textureSampler(std::exchange(tex.textureSampler, nullptr)){
            createCommandPool();
            m_format = std::move(tex.m_format);
            id = std::move(tex.id);
            m_size = std::move(tex.m_size);
            ct = std::move(tex.ct);
        }
        bool Texture::create(uint32_t texWidth, uint32_t texHeight) {
            createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
            createTextureSampler();
            return true;
        }
        bool Texture::createDepthTexture(uint32_t texWidth, uint32_t texHeight) {
            VkFormat depthFormat = findDepthFormat();
            createImage(texWidth, texHeight, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT /*| VK_IMAGE_USAGE_SAMPLED_BIT*/, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            //std::cout<<"dt address : "<<textureImage<<std::endl;
            transitionImageLayout(textureImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            createTextureImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
            createTextureSampler();
            return true;
        }
        bool Texture::loadCubeMapFromFile(std::vector<std::string> filenames, const sf::IntRect& area) {
            for (unsigned int i = 0; i < 6; i++) {
                Image image;
                if (!image.loadFromFile(filenames[i]) || !loadCubeMapFromImage(image, area, i))
                    return false;
            }
            return true;
        }
        bool Texture::loadCubeMapFromImage(const sf::Image& image, const sf::IntRect& area, uint32_t face) {
            const sf::Uint8* pixels = image.getPixelsPtr();
            int texWidth = image.getSize().x;
            int texHeight = image.getSize().y;
            updateCubeMap(pixels, texWidth, texHeight, 0, 0, face);
            allTextures.push_back(this);
            id = nbTextures + 1;
            nbTextures++;
            return true;
        }
        bool Texture::loadFromFile(const std::string& filename, const sf::IntRect& area) {

            Image image;
            return image.loadFromFile(filename) && loadFromImage(image, area);
        }
        bool Texture::loadFromImage(const sf::Image& image, const sf::IntRect& area) {
            const sf::Uint8* pixels = image.getPixelsPtr();
            int texWidth = image.getSize().x;
            int texHeight = image.getSize().y;
            update(pixels, texWidth, texHeight, 0, 0);
            allTextures.push_back(this);
            id = nbTextures + 1;
            nbTextures++;
            return true;
        }
        void Texture::updateCubeMap(const sf::Uint8* pixels, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y, uint32_t face) {
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            if (!pixels) {
                throw std::runtime_error("échec du chargement d'une image!");
            }
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            if (textureImage == nullptr) {
                createCubeMapImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            }
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            void* data;
            vkMapMemory(vkDevice.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(vkDevice.getDevice(), stagingBufferMemory);
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),static_cast<uint32_t>(x), static_cast<uint32_t>(y));
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);
            if (textureImageView == nullptr) {
                createCubeMapTextureImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
                createCubeMapTextureImageSampler();
            }
        }
        void Texture::update(const sf::Uint8* pixels, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y) {
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            if (!pixels) {
                throw std::runtime_error("échec du chargement d'une image!");
            }
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            if (textureImage == nullptr) {
                createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            }
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            void* data;
            vkMapMemory(vkDevice.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(vkDevice.getDevice(), stagingBufferMemory);
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),static_cast<uint32_t>(x), static_cast<uint32_t>(y));
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vkDestroyBuffer(vkDevice.getDevice(), stagingBuffer, nullptr);
            vkFreeMemory(vkDevice.getDevice(), stagingBufferMemory, nullptr);
            if (textureImageView == nullptr) {
                createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
                createTextureSampler();
            }
        }
        void Texture::createImage(uint32_t texWidth, uint32_t texHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
            if (textureImage != nullptr) {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                vkDestroyImageView(vkDevice.getDevice(), textureImageView, nullptr);
                vkDestroyImage(vkDevice.getDevice(), textureImage, nullptr);
                vkFreeMemory(vkDevice.getDevice(), textureImageMemory, nullptr);
            }
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = static_cast<uint32_t>(texWidth);
            imageInfo.extent.height = static_cast<uint32_t>(texHeight);
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.flags = 0; // Optionnel
            if (vkCreateImage(vkDevice.getDevice(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
                throw std::runtime_error("echec de la creation d'une image!");
            }
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(vkDevice.getDevice(), image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
            }

            vkBindImageMemory(vkDevice.getDevice(), image, imageMemory, 0);
            m_size.x = texWidth;
            m_size.y = texHeight;
            m_format = format;
        }
        void Texture::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(vkDevice.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vkDevice.getDevice(), buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(vkDevice.getDevice(), buffer, bufferMemory, 0);
        }
        uint32_t Texture::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(vkDevice.getPhysicalDevice(), &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("aucun type de memoire ne satisfait le buffer!");
        }
        VkFormat Texture::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(vkDevice.getPhysicalDevice(), format, &props);
                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }
            throw std::runtime_error("failed to find supported format!");
        }
        VkFormat Texture::findDepthFormat() {
            return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }
        bool Texture::hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }
        VkCommandBuffer Texture::beginSingleTimeCommands() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(vkDevice.getDevice(), &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;
        }

        void Texture::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(vkDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(vkDevice.getGraphicsQueue());

            vkFreeCommandBuffers(vkDevice.getDevice(), commandPool, 1, &commandBuffer);
        }
        void Texture::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferCopy copyRegion{};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(commandBuffer);
        }
        void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
            imageLayout = newLayout;
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                if (hasStencilComponent(format)) {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            } else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = isCubeMap ? 6 : 1;



            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            } else {
                throw std::invalid_argument("unsupported layout transition!");
            }
            vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

            endSingleTimeCommands(commandBuffer);
        }
        void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t face) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {static_cast<int32_t>(x), static_cast<int32_t>(y), 0};
            region.imageExtent = {
                width,
                height,
                1
            };
            //std::cout<<"offsets : "<<x<<','<<y<<std::endl<<"size : "<<width<<","<<height<<std::endl;

            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            endSingleTimeCommands(commandBuffer);
        }
        void Texture::createTextureImageView(VkFormat format, VkImageAspectFlags aspectFlags) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = textureImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;
            imageAspectFlags = aspectFlags;
            if (vkCreateImageView(vkDevice.getDevice(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
        void Texture::setCoordinatesType(CoordinateType ct) {
            if (ct == UNORM) {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_FALSE;
                samplerInfo.unnormalizedCoordinates = VK_TRUE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = (m_isSmooth) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                m_isRepeated = false;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            } else {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = (m_isSmooth) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }

            }
            this->ct = ct;
        }
        void Texture::createTextureSampler() {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.unnormalizedCoordinates = VK_TRUE;
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 0.0f;
            if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }

        }
        sf::Vector2u Texture::getSize() const {
            return sf::Vector2u(m_size.x, m_size.y);
        }
        void Texture::update(const Texture& texture) {
           update(texture, 0, 0);
        }
        void Texture::update(const Texture& texture, unsigned int x, unsigned int y) {
            VkImageSubresourceLayers subResourceLayers = {
                .aspectMask = imageAspectFlags,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

            blitRegion.srcOffsets[0].x = x;
            blitRegion.srcOffsets[0].y = y;
            blitRegion.srcOffsets[0].z = 0;
            blitRegion.srcOffsets[1].x = texture.m_size.x;
            blitRegion.srcOffsets[1].y = texture.m_size.y;
            blitRegion.srcOffsets[1].z = 1;

            blitRegion.dstOffsets[0].x = x;
            blitRegion.dstOffsets[0].y = y;
            blitRegion.dstOffsets[0].z = 0;
            blitRegion.dstOffsets[1].x = m_size.x;
            blitRegion.dstOffsets[1].y = m_size.y;
            blitRegion.dstOffsets[1].z = 1;

            blitRegion.srcSubresource.aspectMask = imageAspectFlags;
            blitRegion.srcSubresource.baseArrayLayer = 0;
            blitRegion.srcSubresource.layerCount = 1;
            blitRegion.srcSubresource.mipLevel = 0;

            blitRegion.dstSubresource.aspectMask = imageAspectFlags;
            blitRegion.dstSubresource.baseArrayLayer = 0;
            blitRegion.dstSubresource.layerCount = 1;
            blitRegion.dstSubresource.mipLevel = 0;

            VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
            blitInfo.dstImage = textureImage;
            blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            blitInfo.srcImage = texture.textureImage;
            blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            blitInfo.filter = VK_FILTER_LINEAR;
            blitInfo.regionCount = 1;
            blitInfo.pRegions = &blitRegion;
            transitionImageLayout(texture.textureImage, m_format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            transitionImageLayout(textureImage, m_format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            VkCommandBuffer  commandBuffer = beginSingleTimeCommands();
            vkCmdBlitImage2(commandBuffer, &blitInfo);
            endSingleTimeCommands(commandBuffer);
            transitionImageLayout(texture.textureImage, m_format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            transitionImageLayout(textureImage, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        void Texture::setSmooth(bool smooth) {
            m_isSmooth = smooth;
            if (ct == UNORM) {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_FALSE;
                samplerInfo.unnormalizedCoordinates = VK_TRUE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                m_isRepeated = false;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            } else {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = (m_isSmooth) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }

            }
        }
        void Texture::swap(Texture& right) {
            std::swap(id, right.id);
            std::swap(m_size,          right.m_size);
            std::swap(m_actualSize,    right.m_actualSize);
            std::swap(textureImage,       right.textureImage);
            std::swap(m_format, right.m_format);
            std::swap(imageLayout, right.imageLayout);
            std::swap(imageAspectFlags, right.imageAspectFlags);
            std::swap(textureImageView, right.textureImageView);
            std::swap(textureImageMemory, right.textureImageMemory);
            std::swap(textureSampler, right.textureSampler);
            std::swap(m_isSmooth,      right.m_isSmooth);
            std::swap(m_isRepeated,    right.m_isRepeated);
            std::swap(commandPool, right.commandPool);
            std::swap(ct, right.ct);
            m_cacheId = getUniqueId();
            right.m_cacheId = getUniqueId();
        }
        Texture& Texture::operator=(const Texture& right) {
            Texture temp(right);
            swap(temp);
            return *this;
        }
        unsigned int Texture::getMaximumSize() {
            return 10000;
        }
        bool Texture::createCubeMap (unsigned int width, unsigned int height) {
            isCubeMap = true;
            createCubeMapImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            createCubeMapTextureImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
            createCubeMapTextureImageSampler();
            return true;
        }
        void Texture::createCubeMapImage (uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage image, VkDeviceMemory device) {
            VkImageCreateInfo imageCreateInfo = {};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = format;
            imageCreateInfo.mipLevels = 1;
            imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateInfo.extent = { width, height, 1 };
            imageCreateInfo.usage = usage;
            // Cube faces count as array layers in Vulkan
            imageCreateInfo.arrayLayers = 6;
            // This flag is required for cube map images
            imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            if (vkCreateImage(vkDevice.getDevice(), &imageCreateInfo, nullptr, &textureImage) != VK_SUCCESS) {
                throw core::Erreur(0, "Failed to create cubemap image!", 1);
            }
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(vkDevice.getDevice(), textureImage, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(vkDevice.getDevice(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
                throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
            }

            vkBindImageMemory(vkDevice.getDevice(), textureImage, textureImageMemory, 0);
            m_size.x = width;
            m_size.y = height;
            m_format = format;
        }
        void Texture::createCubeMapTextureImageView(VkFormat format, VkImageAspectFlags aspectFlags) {
            VkImageViewCreateInfo view = {};
            view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            // Cube map view type
            view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            view.format = format;
            view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            // 6 array layers (faces)
            view.subresourceRange.layerCount = 6;
            // Set number of mip levels
            view.subresourceRange.levelCount = 1;
            view.image = textureImage;
            if(vkCreateImageView(vkDevice.getDevice(), &view, nullptr, &textureImageView) != VK_SUCCESS) {
                throw core::Erreur(0, "Failed to create image view!", 1);
            }
        }
        void Texture::createCubeMapTextureImageSampler () {
            VkSamplerCreateInfo sampler = {};
            sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler.magFilter = VK_FILTER_LINEAR;
            sampler.minFilter = VK_FILTER_LINEAR;
            sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            sampler.mipLodBias = 0.0f;
            sampler.compareOp = VK_COMPARE_OP_NEVER;
            sampler.minLod = 0.0f;
            sampler.maxLod = 0.0f;
            sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            sampler.maxAnisotropy = 1.0f;
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
            sampler.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            sampler.anisotropyEnable = VK_TRUE;

            if(vkCreateSampler(vkDevice.getDevice(), &sampler, nullptr, &textureSampler) != VK_SUCCESS) {
                throw core::Erreur(0, "failed to create cubemap sampler");
            }
        }
        bool Texture::isSmooth() const {
            return m_isRepeated;
        }
        void Texture::setRepeated(bool repeated) {
            m_isRepeated = repeated;
            if (ct == UNORM) {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_FALSE;
                samplerInfo.unnormalizedCoordinates = VK_TRUE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                m_isRepeated = false;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            } else {
                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeV = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.addressModeW = (m_isRepeated) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(vkDevice.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = 0.0f;
                if (vkCreateSampler(vkDevice.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }

            }
        }
        bool Texture::isRepeated() const {
            return isRepeated();
        }
        bool isCubemap() {
            return false;
        }
        unsigned int Texture::getNativeHandle() {
            return m_texture;
        }
        unsigned int Texture::getId() const {
            return id;
        }
        void Texture::setNativeHandle(unsigned int handle, unsigned int width, unsigned int height) {
            m_texture = handle;
            m_actualSize.x = width;
            m_actualSize.y = height;
        }
        VkImageView Texture::getImageView() const {
            return textureImageView;
        }
        VkSampler Texture::getSampler() const {
            return textureSampler;
        }
        VkFormat Texture::getFormat() {
            return m_format;
        }
        sf::Vector2u Texture::getSize() {
            return m_size;
        }
        VkImage Texture::getImage() {
            return textureImage;
        }
        std::vector<Texture*> Texture::getAllTextures() {
            return allTextures;
        }
        void Texture::createCommandPool() {


            window::Device::QueueFamilyIndices queueFamilyIndices = vkDevice.findQueueFamilies(vkDevice.getPhysicalDevice(), VK_NULL_HANDLE);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optionel
            if (vkCreateCommandPool(vkDevice.getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw core::Erreur(0, "échec de la création d'une command pool!", 1);
            }
        }
        Texture::~Texture() {
            vkDestroyCommandPool(vkDevice.getDevice(), commandPool, nullptr);
            std::cout<<"destroy texture"<<std::endl;
            if (textureImage != VK_NULL_HANDLE) {

                vkDestroySampler(vkDevice.getDevice(), textureSampler, nullptr);
                vkDestroyImageView(vkDevice.getDevice(), textureImageView, nullptr);
                vkDestroyImage(vkDevice.getDevice(), textureImage, nullptr);
                vkFreeMemory(vkDevice.getDevice(), textureImageMemory, nullptr);
            }
        }
        #else // VULKAN
        unsigned int Texture::nbTextures = 0;
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
