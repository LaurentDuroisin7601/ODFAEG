#include "imgui.h"
#include "external/imgui/backends/imgui_impl_vulkan.h"
#include <vulkan/vulkan.hpp>
#include <iostream>
import odfaeg.graphic.renderWindow;
import odfaeg.window.videoMode;
import odfaeg.window.iEvent;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.device;
import odfaeg.graphic.commandPool;
import odfaeg.graphic.color;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
static void checkVkResult(VkResult err) {
    if (err == 0)
        return;
   throw std::runtime_error("Vulkan error : "+err);
}
int main() {
    GPUContext& ctx = GPUContext::instance();
    RenderWindow window(VideoMode(800, 600), "Test my game", ctx.getDevice(), odfaeg::window::Style::Default);	
    void(*checkVkResultFn)(VkResult);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    int width, height;
    window.getFramebufferSize(width, height);
    io.DisplaySize.x = width;
    io.DisplaySize.y = height;
  
    ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* user_data) {
        return vkGetInstanceProcAddr((VkInstance)user_data, function_name);
    }, (void*)ctx.getInstance().getInstance());

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 11000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    VkDescriptorPool imguiDescriptorPool;
    vkCreateDescriptorPool(ctx.getDevice().getDevice(), &pool_info, nullptr, &imguiDescriptorPool);
    ImGui_ImplVulkan_InitInfo init_info = {};
    Device::QueueFamilyIndices indices = ctx.getDevice().findQueueFamilies(ctx.getDevice().getPhysicalDevice(), window.getSurface());
    init_info.Instance = ctx.getInstance().getInstance();
    init_info.PhysicalDevice = ctx.getDevice().getPhysicalDevice();
    //std::cout<<"physical device : "<<init_info.PhysicalDevice<<std::endl;
    init_info.Device = ctx.getDevice().getDevice();
    init_info.QueueFamily = indices.graphicsFamily.value();
    init_info.Queue = ctx.getDevice().getQueue(indices.graphicsFamily.value(), 0);
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.MinImageCount = window.getSwapchainMinImagesCount();
    init_info.ImageCount = window.getSwapchainImagesCount();
    init_info.RenderPass = window.getRenderPass(0).getHandle();
    ImGui_ImplVulkan_Init(&init_info);
    
    while (window.isOpen()) {
		odfaeg::window::IEvent event;
		while (window.pollEvent(event)) {
			if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
				window.close();
			}
		}
        
		window.clear(Color::Red);	       	
		ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("ODFAEG Debug");
        ImGui::Text("Hello ODFAEG!");
        ImGui::End();
		ImGui::Render();
        window.beginRenderPass();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, window.getCommandPool().getHandle(window.getCurrentFrame()));
        window.endRenderPass();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        window.submit(true);		
		window.display();
	}
    return 0;
}