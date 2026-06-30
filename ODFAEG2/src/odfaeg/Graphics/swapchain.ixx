module;
#include <vulkan/vulkan.hpp>

export module odfaeg.graphic.swapchain;
import odfaeg.core.nonCopyable;
import odfaeg.graphic.device;
import odfaeg.graphic.image;
namespace odfaeg {
	namespace graphic {
		export  class Swapchain : public core::NonCopyable{
		public :
			Swapchain(Device& device);
			Swapchain(Swapchain&& swapchain) noexcept;
			Swapchain& operator=(Swapchain&& swapchain) noexcept;
			void create(VkSurfaceKHR surface, bool vertSynch, int width, int height);
			void createImageViews();
			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vertSynch);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);
			VkFormat& getSwapchainImageFormat();
			std::vector<Image>& getSwapchainImages();				
			VkExtent2D getSwapchainExtents();
			VkSwapchainKHR getHandle();
			void cleanup();
			~Swapchain();
		private :
			Device& device;
			std::vector<Image> swapchainImages;			
			VkSwapchainKHR swapchain;			
			VkExtent2D swapchainExtents;
			VkFormat swapchainImageFormat;
		};
	}
}
