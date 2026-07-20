module;
#include <vulkan/vulkan.hpp>
#include <gli.hpp>
#include <string>
#include <vector>
#include <deque>
export module odfaeg.graphic.texture;
import odfaeg.graphic.image;
import odfaeg.graphic.device;
import odfaeg.math.vec;
import odfaeg.graphic.imageLoader;
import odfaeg.entity.rect;
import odfaeg.graphic.commandPool;
import odfaeg.core.nonCopyable;
import odfaeg.core.resourceManager;
import odfaeg.graphic.buffer;
namespace odfaeg {
	namespace graphic {
		export class Texture : public core::NonCopyable {
		public :
            struct MipInfo {
                uint32_t width;
                uint32_t height;
            };
            Texture(Device& device, unsigned int nbBuffers=1);
            Texture(Texture&& texture) noexcept;
            void copyFrom(Texture& texture);
            void copyFrom(CommandPool& commandPool, Texture& texture);
            Texture& operator=(Texture&& texture) noexcept;
			void createCommandBuffers();
			void setTexType(unsigned int texType);
			void setFormat(VkFormat format);
			void setSize(math::Vector2u size);
			void setSamplerAddressMode(VkSamplerAddressMode wrapU, VkSamplerAddressMode wrapV);
            bool create(uint32_t texWidth, uint32_t texHeight, uint32_t texDepth=1, unsigned int mipLevels = 1, bool layered=false, bool FBOAttachment=false);
            bool createDepthTexture(uint32_t texWidth, uint32_t texHeight, uint32_t depth=1, bool layered=false);
            bool createCubeMap(uint32_t size, bool layered=false, bool FBOAttachment=false);
            bool createDepthCubeMap(uint32_t size, bool layered=false);
            bool loadCubeMapFromFile(std::vector<std::string> filenames, const entity::IntRect& area=entity::IntRect());
            bool loadCubeMapFromImage(const ImageLoader& image, uint32_t face, const entity::IntRect& area=entity::IntRect());
			bool loadFromMemory(const void* data, std::size_t size, const entity::IntRect& area=entity::IntRect());
            bool loadFromFile(const std::string& filename, const entity::IntRect& area=entity::IntRect());
            bool loadFromImage(const ImageLoader& imageLoader, const entity::IntRect& area);
            void updateCubeMap(const std::uint8_t* pixels, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y, uint32_t face);
            void update(const std::uint8_t* pixels, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y);
			void update(CommandPool& commandPool, Buffer& staggingBuffer, unsigned int texWidth, unsigned int texHeight, unsigned int x, unsigned int y, size_t srcStart=0, size_t mipLevel=0);
            static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
            static VkFormat findDepthFormat();
            static bool hasStencilComponent(VkFormat format);
            void toShaderReadOnlyOptimal(unsigned int currentFrame, VkCommandBuffer cmd);
            void toColorAttachmentOptimal(unsigned int currentFrame, VkCommandBuffer cmd);
            static void transitionImageLayout(Image& image, VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int mipLevel=0, unsigned int baseLayer = 0, unsigned int nbLevels = 1, unsigned int nbLayers=1);
            math::Vector2u getSize() const;
            void update(Texture& texture);
            void update(VkCommandBuffer& commandBuffer, Texture& texture, unsigned int imageIndex);
            void updateCubeMap(Texture& texture);
            void update(Texture& texture, unsigned int x, unsigned int y);
            void update(VkCommandBuffer& commandBuffer, Texture& texture, unsigned int x, unsigned int y, unsigned int imageIndex);
            void update(CommandPool& commandPool, Texture& texture, unsigned int x, unsigned int y, size_t mipLevel = 0);
            void updateCubeMap(Texture& texture, unsigned int x, unsigned int y);
            void setSmooth(bool smooth);
            void swap(Texture& right);            
            unsigned int getMaximumSize();
            bool isSmooth() const;
            void setRepeated(bool repeated);
            bool isRepeated() const;
            unsigned int getId() const;
			void setId(unsigned int id);
			std::deque<Image>& getImages();
            Image& getImage(unsigned int currentFrame=0);
            VkFormat& getFormat();
            math::Vector2u getSize();
            unsigned int getNbBuffers() const;
            unsigned int getLayerCount();            
        private :
            unsigned int texType, mipLevels;
            Device& device;
            std::deque<Image> images;
            ImageLoader imageLoader;
            entity::IntRect  m_area;
            math::Vector2u m_size;                     
            unsigned int id, nbBuffers, layerCount;
            VkFormat m_format;
            CommandPool commandPool;            
            bool m_Smooth, m_Repeated, unormalized, isFBOTexture;
            size_t m_DataSize;
            VkImageAspectFlags imageAspectMask;
			VkSamplerAddressMode wrapU, wrapV;
			std::vector<MipInfo> mipsInfos;
		};         
	}    
}
export namespace odfaeg {
    namespace core {
        template <typename I=std::string>
        using TextureManager = ResourceManager<odfaeg::graphic::Texture, I>;
    }
}
