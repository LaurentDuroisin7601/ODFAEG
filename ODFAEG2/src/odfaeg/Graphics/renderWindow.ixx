module;
#include <vulkan/vulkan.hpp>
#include <cstdint>
#include <odfaeg/Window/windowHandle.hpp>
#include <deque>
export module odfaeg.graphic.renderWindow;
import odfaeg.window.window;
import odfaeg.window.windowStyle;
import odfaeg.window.videoMode;
import odfaeg.graphic.renderTarget;
import odfaeg.core.string;
import odfaeg.graphic.device;
import odfaeg.graphic.swapchain;
import odfaeg.graphic.frameBuffer;
import odfaeg.graphic.renderPass;
import odfaeg.math.vec;
import odfaeg.entity.color;
import odfaeg.graphic.semaphore;
import odfaeg.graphic.fence;
namespace odfaeg {
	namespace graphic {
		export class RenderWindow : public window::Window, public  RenderTarget {
			public :
                RenderWindow(window::VideoMode mode, const core::String& title,   Device& vkDevice, std::uint32_t style = window::Style::Default, bool useDepth = false, bool useStencil = false);
            explicit RenderWindow(window::WindowHandle handle, Device& vkDevice, bool useDepth=false, bool useStencil=false);
            virtual math::Vector2u getSize() const;
            uint32_t getCurrentFrame();
            void recreateSwapchain();
            void cleanupSwapchain();
            void drawVulkanFrame();
            VkExtent2D getExtents();
            
            Device& getDevice();
            void clear(const entity::Color& color = entity::Color(0, 0, 0, 255));
            uint32_t getImageIndex();

            virtual ~RenderWindow();
            void submit(bool lastSubmit = false, std::vector<VkSemaphore> signalSemaphores = std::vector<VkSemaphore>(),
                                                                                std::vector<VkSemaphore> waitSemaphores = std::vector<VkSemaphore>(), std::vector<VkPipelineStageFlags> waitStages = std::vector<VkPipelineStageFlags>(),
                                                                                std::vector<uint64_t> signalValues = std::vector<uint64_t>(),
                                                                                std::vector<uint64_t> waitValues = std::vector<uint64_t>(), std::vector<VkFence>fences = std::vector<VkFence>(), unsigned int queueIndex = 0, bool resetFence = true, bool resetFences = true, VkFence fenceToSubmit = nullptr);
            
			void cleanup();
			void waitDeviceIdle();
			VkFormat& getImageFormat();
			bool isDepthOnly();
			std::uint32_t& getViewMask();
			void beginRendering(bool secondaryCommandBuffers=false);
			void endRendering();
            void beginRenderPass(bool secondaryCommandBuffers=false);
			void endRenderPass();
            std::uint32_t getSwapchainMinImagesCount();
            std::uint32_t getSwapchainImagesCount();
            RenderPass& getRenderPass(unsigned int renderPassId);
            VkSurfaceKHR getSurface();            
        protected:

            ////////////////////////////////////////////////////////////
            /// \brief Function called after the window has been created
            ///
            /// This function is called so that derived classes can
            /// perform their own specific initialization as soon as
            /// the window is created.
            ///
            ////////////////////////////////////////////////////////////
            virtual void onCreate();

            ////////////////////////////////////////////////////////////
            /// \brief Function called after the window has been resized
            ///
            /// This function is called so that derived classes can
            /// perform custom actions when the size of the window changes.
            ///
            ////////////////////////////////////////////////////////////
            virtual void onResize();
            virtual void onClose();

            std::vector<FrameBuffer>& getFrameBuffers(unsigned int frameBufferId);
            
           
			bool useDepth, useStencil;
        private:

            bool firstSubmit;
            uint32_t imageIndex;

            void createSurface();
            void createSwapchain();
            void createImageViews();            
            void createFrameBuffers();
            void createRenderPass();
            void createSyncObjects();

            VkSurfaceKHR surface;
            Device& device;
            Swapchain swapchain;            
            std::deque<Semaphore> imageAvailableSemaphores;
            std::deque<Semaphore> renderFinishedSemaphores;
            std::deque<Fence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
            std::vector<RenderPass> renderPasses;
            std::vector<std::vector<FrameBuffer>> frameBuffers;
            std::uint32_t currentFrame=0;
            bool framebufferResized = false;
            std::uint32_t viewMask;            
        };			
	}
}