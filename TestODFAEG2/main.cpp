#include <chrono>
#include <iostream>
#include <thread>
#include <cmath>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "imgui.h"
#include "external/imgui/backends/imgui_impl_vulkan.h"
/*import odfaeg.graphic.renderWindow;
import odfaeg.window.videoMode;
import odfaeg.graphic.instance;
import odfaeg.graphic.device;
import odfaeg.window.iEvent;
import odfaeg.window.windowStyle;
import odfaeg.entity.color;
import odfaeg.graphic.vertexBuffer;
import odfaeg.entity.primitiveType;
import odfaeg.graphic.vertex;
import odfaeg.entity.tile;
import odfaeg.graphic.renderTarget;
import odfaeg.entity.rect;
import odfaeg.graphic.semaphore;
import odfaeg.core.clock;
import odfaeg.graphic.modelLoader;
import odfaeg.core.resourceManager;
import odfaeg.entity.gameObject;
import odfaeg.graphic.camera;
import odfaeg.graphic.texture;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.particleSystemUpdater;
import odfaeg.entity.emittors;
import odfaeg.entity.particleSystem;
import odfaeg.entity.morphAnim;
import odfaeg.graphic.morphAnimUpdater;
import odfaeg.entity.animation;
import odfaeg.entity.animator;
import odfaeg.graphic.boneAnimUpdater;
import odfaeg.graphic.rectangleShape;
import odfaeg.graphic.linkedListRenderer;
import odfaeg.entity.plane;
import odfaeg.entity.cube;
import odfaeg.graphic.shadowRenderer;
import odfaeg.core.string;
import odfaeg.core.utilities;
import odfaeg.graphic.mesh;
import odfaeg.core.delegate;
import odfaeg.graphic.renderGraph;
import odfaeg.graphic.componentManager;
import odfaeg.graphic.iComponent;
import odfaeg.graphic.renderTexture;*/
#include "odfaeg/Core/core.hpp"
#include "odfaeg/Math/math.hpp"
#include "odfaeg/Entity/ent.hpp"
#include "odfaeg/Window/win.hpp"
#include "odfaeg/Graphics/graphics.hpp"
using namespace odfaeg::core;
using namespace odfaeg::entity;
using namespace odfaeg::window;
using namespace odfaeg::graphic;
using namespace odfaeg::math;

enum TextureNames {
	WOOD
};
int main() {	
	GPUContext& ctx = GPUContext::instance();
	//String string("Test my game");	
	RenderWindow window(VideoMode(800, 600), "Test my game", ctx.getDevice(), odfaeg::window::Style::Default, true);
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
	//window.createDescriptorAndPipelines();
	Camera camera(800, 600, 80, 1, 1000);
	//camera.setUp(Vec3f(0.f, -1.f, 0.f));
	camera.move(0.f, 0.f, 5.f);
	Camera imGUICamera = window.getCamera();
	window.setCamera(camera);
	ResourceManager<Texture, TextureNames> textureManager;
	ResourceManager<Texture, std::string> modelTextureManager;
	ModelLoader modelLoader(GPUContext::instance().getDevice(), modelTextureManager);
	/*Mesh* bistroExterior = modelLoader.loadModel("Bistro_v5_2/BistroExterior.fbx");
	bistroExterior->getGameObject()->setScale(Vec3f(1, -1, 1));*/
	//std::cout<<"test"<<std::endl;
	Mesh* bistroExterior = modelLoader.loadModel("car/source/FINAL_MODEL_S4_13/FINAL_MODEL_S4.fbx");
	//GameObject* bistroExterior = modelLoader.loadModel(/*"CubeTest/cube_test.glb"*//**/"carGLTF/scene.gltf"/*"Bistro_v5_2/BistroExterior.fbx"*/);
	//bistroExterior->setRotation(45, Vec3f(0, 1, 0));
	/*std::tuple<std::reference_wrapper<Device>> args = std::make_tuple(std::ref(ctx.getDevice()));
	textureManager.fromFileWithAlias("tilesets/wood.png", WOOD, args);
	Texture* texWood = textureManager.getResourceByAlias(WOOD);
	texWood->setSamplerAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);	
	Plane plane(Vec3f(-25, -2, -25), Vec3f(50, 0, 50));
	plane.setTexCoords(FloatRect(0, 0, 25, 25));
	plane.setTexture(conversionIntString(WOOD));	
	
		
	Cube cube1(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);		
	cube1.setTexture(conversionIntString(WOOD));
	cube1.move(Vec3f(4.0f, -3.5f, 0.0));
    cube1.scale(Vec3f(0.5f, 0.5f, 0.5f));   
	
    Cube cube2(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube2.setTexture(conversionIntString(WOOD));
    cube2.move(Vec3f(2.0f, 3.0f, 1.0));
    cube2.scale(Vec3f(0.75f, 0.75f, 0.75f));
    
    Cube cube3(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube3.setTexture(conversionIntString(WOOD));
    cube3.move(Vec3f(-3.0f, -1.0f, 0.0));
    cube3.scale(Vec3f(0.5f, 0.5f, 0.5f));
    
	Cube cube4(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube4.setTexture(conversionIntString(WOOD));
    cube4.move(Vec3f(-1.5f, 1.0f, 1.5));
    cube4.scale(Vec3f(0.5f, 0.5f, 0.5f));
   
    Cube cube5(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube5.setTexture(conversionIntString(WOOD));
	cube5.move(Vec3f(-1.5f, 2.0f, -3.0));
    cube5.setRotation(60.0f, Vec3f(1.0, 0.0, 1.0));
    cube5.scale(Vec3f(0.75f, 0.75f, 0.75f));
   	
	Mesh planeMesh(&plane);
	Mesh cube1Mesh(&cube1);
	Mesh cube2Mesh(&cube2);
	Mesh cube3Mesh(&cube2);
	Mesh cube4Mesh(&cube3);
	Mesh cube5Mesh(&cube5);
	planeMesh.buildMaterialsFromTextureManager(textureManager);
	cube1Mesh.buildMaterialsFromTextureManager(textureManager);
	cube2Mesh.buildMaterialsFromTextureManager(textureManager);
	cube3Mesh.buildMaterialsFromTextureManager(textureManager);
	cube4Mesh.buildMaterialsFromTextureManager(textureManager);
	cube5Mesh.buildMaterialsFromTextureManager(textureManager);
	window.addGameObject(&planeMesh);
	window.addGameObject(&cube1Mesh);
	window.addGameObject(&cube2Mesh);
	window.addGameObject(&cube3Mesh);
	window.addGameObject(&cube4Mesh);
	window.addGameObject(&cube5Mesh);
	RenderTexture sceneColorTexture(ctx.getDevice());
	sceneColorTexture.create(window.getSize().x(), window.getSize().y());
	sceneColorTexture.setCamera(camera);
	RenderGraph renderGraph;
	renderGraph.addLinkedListPass(sceneColorTexture, 0, "*", window.getId());
	ComponentManager componentManager;
	std::vector<IComponent*> components = renderGraph.getComponents();
	for (unsigned int i = 0; i < components.size(); i++) {
		componentManager.addComponent(components[i]);
	}*/
	window.addGameObject(bistroExterior);	
		//std::cout<<"i : "<<i<<std::endl;*/
	
	//std::cout<<"ok"<<std::endl;
	Clock clock;
	unsigned int fps = 0;
	/*renderGraph.addShadowPass(window, sceneColorTexture, 1, "*", window.getId());
	ShadowRenderer::DirLight dirLight;
	dirLight.dir = Vec3f(20, 50, 20);
	renderGraph.addDirectionnalLight<ShadowRenderer>(1, dirLight);
	ShadowRenderer::PointLight pointLight;
	pointLight.pos = Vec3f(0, 0, 0);
	renderGraph.addPonctualLight<ShadowRenderer>(1, pointLight);*/
	std::string s;	
	while (window.isOpen()) {
		odfaeg::window::IEvent event;
		while (window.pollEvent(event)) {
			if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
				window.close();
			}
			//componentManager.update(window.getId(), event);
		}
		
		
		
		window.setDepthStencil(true, false);
		window.clear();
		window.setTypesToRender("*", window.getCurrentFrame());
		window.setCamera(camera);
		//std::cout<<"draw"<<std::endl;
		window.draw(Triangles);
		//std::cout<<"drawn"<<std::endl;
		/*sceneColorTexture.clear(Color::Red);
		renderGraph.render();*/
		
		
		
		//sceneColorTexture.submit(true);		
		
		
	 	window.submit();
		//std::cout<<"submited"<<std::endl;
		window.beginRecordCommandBuffer();
		window.setCamera(imGUICamera);
		window.setDepthStencil(false, false);
		ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("ODFAEG Debug");
		ImGui::TextUnformatted(s.c_str());
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
		fps++;		
		if (clock.getElapsedTime() >= seconds(1.f)) {
			//std::cout<<"FPS : "<<fps<<std::endl;
			s = "FPS : " + std::to_string(fps);
			fps = 0;
			clock.restart();
		}     
	}
	vkDestroyDescriptorPool(ctx.getDevice().getDevice(), imguiDescriptorPool, nullptr);
	return 0;
}