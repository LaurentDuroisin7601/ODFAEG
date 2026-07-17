module;
#include <vulkan/vulkan.hpp>
export module odfaeg.graphic.renderTexture;
import odfaeg.graphic.device;
import odfaeg.graphic.image;
import odfaeg.graphic.texture;
import odfaeg.graphic.renderPass;
import odfaeg.graphic.frameBuffer;
import odfaeg.graphic.fence;
import odfaeg.graphic.semaphore;
import odfaeg.graphic.color;
import odfaeg.math.vec;
import odfaeg.graphic.renderTarget;
namespace odfaeg {
	namespace graphic {
		export class RenderTexture : public RenderTarget {
        public:            
            inline static const unsigned int NB_SWAPCHAIN_IMAGES = 3;
            inline static const unsigned int RT_MAX_FRAMES_IN_FLIGHT = 2;
            RenderTexture(Device& device, bool useDepthTest = false, bool useStencilTest = false);
            bool create(unsigned int width, unsigned int height, unsigned int depth=1, bool layered=false, bool depthOnly=false);
            bool createCubeMap(unsigned int size, bool depthOnly = false, bool layered = false);
            uint32_t getImageIndex();
            VkSurfaceKHR getSurface();
            VkExtent2D getExtents();
            uint32_t getCurrentFrame();
            Texture& getTexture(unsigned int attachmentPoint=0);
            math::Vector2u getSize() const;
            void createFramebuffers();
            std::vector<FrameBuffer>& getFrameBuffers(unsigned int frameBufferId);
            void createRenderPass();
            RenderPass& getRenderPass(unsigned int renderPassId);
            void clear(const Color& color = Color(0, 0, 0, 255));
            void beginRendering(bool secondaryCommandBuffers=false);
            void endRendering();
            void beginRenderPass(bool secondaryCommandBuffers=false);
			void endRenderPass();
            void submit(bool lastSubmit = false, std::vector<VkSemaphore> signalSemaphores = std::vector<VkSemaphore>(),
                                                                                                       std::vector<VkSemaphore> waitSemaphores = std::vector<VkSemaphore>(), std::vector<VkPipelineStageFlags> waitStages = std::vector<VkPipelineStageFlags>(),
                                                                                                       std::vector<uint64_t> signalValues = std::vector<uint64_t>(),
                                                                                                       std::vector<uint64_t> waitValues = std::vector<uint64_t>(), std::vector<VkFence>fences = std::vector<VkFence>(), unsigned int queueIndex = 0, bool resetFence = true, bool resetFences = true, VkFence fenceToSubmit = nullptr);
            void display();  
            //std::vector<Image>& getImages();            
            VkFormat& getImageFormat();
            std::uint32_t& getViewMask();
            bool isDepthOnly();
            std::vector<Texture>& getTextures();
            std::vector<Semaphore>& getSemaphores();
            std::uint32_t getSwapchainImagesCount();
            ~RenderTexture();
        private:

            void createSyncObjects();
            math::Vector2u m_size;
            std::vector<std::vector<FrameBuffer>> frameBuffers;
            std::vector<RenderPass> renderPasses;
            Device& device;
            std::vector<Texture> m_textures;
            uint32_t currentFrame, imageIndex;            
            std::vector<Fence> inFlightFences;
            std::vector<Semaphore> imageAvailableSemaphores;
            bool firstSubmit, enableDepthTest, enableStencilTest;
            std::uint32_t viewMask;
            VkFormat imageFormat;
            bool depthOnly;
		};
	}
}
