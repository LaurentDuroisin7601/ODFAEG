module;
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
export module odfaeg.graphic.image;
import odfaeg.core.nonCopyable;
import odfaeg.graphic.device;
import odfaeg.graphic.buffer;
namespace odfaeg {
	namespace graphic {
        export  class ImageView : public core::NonCopyable {
        public:
            ImageView(Device& device);
            ImageView(ImageView&& ImageView) noexcept ;
            ImageView& operator=(ImageView&& imageView) noexcept;
            void create(VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t levelCount, uint32_t layerCount);
            void cleanup();
            VkImageView getHandle();
            ~ImageView();
        private:
            Device& device;
            VkImageView imageView;
        };
        export class Sampler : public core::NonCopyable {
        public:
            Sampler(Device& device);
            Sampler(Sampler&& sampler) noexcept;
            Sampler& operator=(Sampler&& sampler) noexcept;
            void create(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV, unsigned int mipLevels, bool smooth = false, bool unormalizedCoordinates = false);
            VkSampler getHandle();
            void cleanup();
            ~Sampler();
        private:
            Device& device;
            VkSampler sampler;
        };
		export class Image : public core::NonCopyable {
        public:
            Image(Device& device);
            Image(Image&& image) noexcept;
            Image& operator= (Image&& image) noexcept;
            void setHandle(VkImage image);
            void create(uint32_t width, uint32_t height, uint32_t depth, VkImageType type, VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
                unsigned int mipLevels, unsigned int arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling);                
            void createSampler(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV, unsigned int mipLevels, bool smooth, bool unormalized);
            ImageView& getImageView();
            Sampler& getSampler();
            void cleanup();
            void createImageView(VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t baseMipLevel, uint32_t baseArrayLayer, uint32_t levelCount, uint32_t layerCount);
            void copyBufferToImage(VkCommandBuffer cmd, Buffer& buffer, uint32_t width, uint32_t height, uint32_t x, uint32_t y, size_t srcStart=0, size_t mipLevel = 0, uint32_t face=0);
            VkImage getHandle();
            void setLayout(VkImageLayout newLayout);
            VkImageLayout getLayout();
            VkFormat getFormat();
            VkImageAspectFlags getImageAspectFlags();
            ~Image();
        private:
		    bool isSwapchainImage;
            Device& device;
            VkImage image;
            VkImageLayout layout;
            ImageView imageView;
            Sampler sampler;
            VmaAllocation memory;
            VkFormat m_format;
            VkImageAspectFlags aspectFlags;
		}; 
	}
}