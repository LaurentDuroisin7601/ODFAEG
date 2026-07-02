module;
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <iostream>
import odfaeg.graphic.image;
module odfaeg.graphic.image;
namespace odfaeg {
	namespace graphic {          
        using namespace std;
        Image::Image(Device& device) : device(device), imageView(device), sampler(device) {
            image = VK_NULL_HANDLE; 
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
            isSwapchainImage = false;
        }    
        Image::Image(Image&& other) noexcept
            : device(other.device),
            layout(other.layout),
            image(other.image),
            imageView(std::move(other.imageView)),
            sampler(std::move(other.sampler)),
	        memory(other.memory)
        {
            other.image = VK_NULL_HANDLE;
            other.layout = VK_IMAGE_LAYOUT_UNDEFINED;
            other.memory = VK_NULL_HANDLE;
            isSwapchainImage = other.isSwapchainImage;
        }
        Image& Image::operator=(Image&& other) noexcept {
            if (this != &other) {
                cleanup(); // d�truit image, imageView, sampler
               
                layout = other.layout;
                image = other.image;
                imageView = std::move(other.imageView);
                sampler = std::move(other.sampler);
                memory = other.memory;

                other.image = VK_NULL_HANDLE;
                other.layout = VK_IMAGE_LAYOUT_UNDEFINED;
                other.memory = VK_NULL_HANDLE;
                isSwapchainImage = other.isSwapchainImage;
            }
            return *this;
        }
        void Image::create(uint32_t width, uint32_t height, uint32_t depth, VkImageType type, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, unsigned int mipLevels, unsigned int arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling,VkImageCreateFlags flags) {
            if (image != VK_NULL_HANDLE) {
                cleanup();
            }
            VkImageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.imageType = type;
            info.format = format;
            info.extent = { width, height, depth };
            info.mipLevels = mipLevels;
            info.arrayLayers = arrayLayers;
            info.samples = samples;
            info.tiling = tiling;
            info.usage = usage;
            info.flags = flags;
            VmaAllocationCreateInfo alloc{};
            alloc.usage = memoryUsage;
            vmaCreateImage(device.getAllocator(), &info, &alloc, &image, &memory, nullptr);
            m_format = format;
            //std::cout<<"size : "<<width<<","<<height<<"mip levels : "<<mipLevels<<",image : "<<image<<std::endl;
        } 
        void Image::createSampler(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV, unsigned int mipLevels, bool smooth, bool unormalized) {
            sampler.create(wrapU, wrapV, mipLevels, smooth, unormalized);
        }
        void Image::setHandle(VkImage handle) {
            isSwapchainImage = true;
            image = handle;
        }
        VkImageAspectFlags Image::getImageAspectFlags() {
            return aspectFlags;
        }
	    VkFormat Image::getFormat() {
            return m_format;
        }
        VkImage Image::getHandle() {
            return image;
        }
        VkImageLayout Image::getLayout() {
            return layout;
        }
        ImageView& Image::getImageView() {
            return imageView;
        }
        Sampler& Image::getSampler() {
            return sampler;
        }
        void Image::cleanup() {
            if (image != VK_NULL_HANDLE && !isSwapchainImage) {
                vmaDestroyImage(device.getAllocator(), image, memory);
                image = VK_NULL_HANDLE;
                imageView.cleanup();
                sampler.cleanup();
            }
        }
        void Image::createImageView(VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t levelCount, uint32_t layerCount) {
            this->aspectFlags = aspectFlags;
            imageView.create(image, viewType, format, aspectFlags, baseMipLevel, baseArrayLayer, levelCount, layerCount);
        }
        void Image::copyBufferToImage(VkCommandBuffer cmd, Buffer& buffer, uint32_t width, uint32_t height, uint32_t x, uint32_t y, size_t srcStart, size_t mipLevel, uint32_t face) {
            VkBufferImageCopy region{};
            region.bufferOffset = srcStart;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { static_cast<int32_t>(x), static_cast<int32_t>(y), 0 };
            region.imageExtent = {
                width,
                height,
                1
            };
            //std::cout<<"Copy: extent=%u %u, dataSize=%u"<<width<<","<<height<<","<<srcStart<<std::endl;
            vkCmdCopyBufferToImage(cmd, buffer.getHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        }
        void Image::setLayout(VkImageLayout newLayout) {
            layout = newLayout;
        }
        Image::~Image() {
            cleanup();
        }
        ImageView::ImageView(Device& device) : device(device) {
            imageView = VK_NULL_HANDLE;
        }
        ImageView::ImageView(ImageView&& other) noexcept : device(other.device) {            
            imageView = other.imageView;
            other.imageView = VK_NULL_HANDLE;
        }
        ImageView& ImageView::operator=(ImageView&& other) noexcept {
            if (this != &other) {
                cleanup();
                imageView = other.imageView;
                other.imageView = VK_NULL_HANDLE;
            }
            return *this;
        }
        void ImageView::create(VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t levelCount, uint32_t layerCount) {
            if (imageView != VK_NULL_HANDLE) {
                cleanup();
            }
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = viewType;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
            viewInfo.subresourceRange.levelCount = levelCount;
            viewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
            viewInfo.subresourceRange.layerCount = layerCount;                    
            if (vkCreateImageView(device.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }   
        VkImageView ImageView::getHandle() {
            return imageView;
        }
        void ImageView::cleanup() {
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(device.getDevice(), imageView, nullptr);
                imageView = VK_NULL_HANDLE;
            }
        }
        ImageView::~ImageView() {
            cleanup();
        }
        Sampler::Sampler(Device& device) : device(device) {
            sampler = VK_NULL_HANDLE;
        }
        Sampler::Sampler(Sampler&& other) noexcept : device(other.device) {            
            sampler = other.sampler;
            other.sampler = VK_NULL_HANDLE;
        }
        Sampler& Sampler::operator= (Sampler&& other) noexcept {
            if (this != &other) {
                cleanup();
                sampler = other.sampler;
                other.sampler = VK_NULL_HANDLE;
            }
            return *this;
        }
        void Sampler::create(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV, unsigned int mipLevels, bool smooth, bool unormalized) {
            
            if (sampler != VK_NULL_HANDLE) {
                cleanup();
            }
            if (unormalized) {
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = wrapU;
                samplerInfo.addressModeV = wrapV;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_FALSE;
                samplerInfo.unnormalizedCoordinates = VK_TRUE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = static_cast<float>(mipLevels - 1);
                if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            }
            else {
                /*if (wrapU == VK_SAMPLER_ADDRESS_MODE_REPEAT && wrapV == VK_SAMPLER_ADDRESS_MODE_REPEAT)
                    std::cout<<"create sampler repeat!"<<std::endl;*/
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = wrapU;
                samplerInfo.addressModeV = wrapV;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                samplerInfo.anisotropyEnable = VK_TRUE;
                samplerInfo.unnormalizedCoordinates = VK_FALSE;
                VkPhysicalDeviceProperties properties{};
                vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
                samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
                samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                samplerInfo.compareEnable = VK_FALSE;
                samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo.mipLodBias = 0.0f;
                samplerInfo.minLod = 0.0f;
                samplerInfo.maxLod = static_cast<float>(mipLevels - 1);
                if (vkCreateSampler(device.getDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture sampler!");
                }
            }
        }
        void Sampler::cleanup() {
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(device.getDevice(), sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
        }
        VkSampler Sampler::getHandle() {
            return sampler;
        }
        Sampler::~Sampler() {
            cleanup();
        }
	}
}