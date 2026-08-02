module;
#include <vulkan/vulkan.hpp>
#include <cstdint>
#include "../Core/nonCopyable.hpp"
#include "device.hpp"
#include "image.hpp"
namespace odfaeg {
	namespace graphic {
		class Swapchain : public core::NonCopyable{
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
			std::uint32_t getMinImagesCount();
			~Swapchain();			
		private :
			Device& device;
			std::vector<Image> swapchainImages;			
			VkSwapchainKHR swapchain;			
			VkExtent2D swapchainExtents;
			VkFormat swapchainImageFormat;
			uint32_t minImagesCount;
		};
	}
}
#include "swapchain.inl"
