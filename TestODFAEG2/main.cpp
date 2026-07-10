#include <chrono>
#include <iostream>
#include <thread>
#include <cmath>
#include <vector>
#include <vulkan/vulkan.hpp>
import odfaeg.graphic.renderWindow;
import odfaeg.window.videoMode;
import odfaeg.graphic.instance;
import odfaeg.graphic.device;
import odfaeg.window.iEvent;
import odfaeg.window.windowStyle;
import odfaeg.graphic.color;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.primitiveType;
import odfaeg.graphic.vertex;
import odfaeg.math.vec;
import odfaeg.graphic.tile;
import odfaeg.graphic.renderTarget;
import odfaeg.graphic.rect;
import odfaeg.graphic.semaphore;
import odfaeg.core.clock;
import odfaeg.graphic.modelLoader;
import odfaeg.core.resourceManager;
import odfaeg.graphic.gameObject;
import odfaeg.graphic.camera;
import odfaeg.graphic.texture;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.particleSystemUpdater;
import odfaeg.graphic.emittors;
import odfaeg.graphic.particleSystem;
import odfaeg.graphic.morphAnim;
import odfaeg.graphic.morphAnimUpdater;
import odfaeg.graphic.animation;
import odfaeg.graphic.animator;
import odfaeg.graphic.boneAnimUpdater;
import odfaeg.graphic.rectangleShape;
import odfaeg.graphic.linkedListRenderer;
import odfaeg.graphic.plane;
import odfaeg.graphic.cube;
import odfaeg.graphic.shadowRenderer;
import odfaeg.core.string;
using namespace odfaeg::window;
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::core;
enum TextureNames {
	WOOD
};
int main() {
	GPUContext& ctx = GPUContext::instance();
	//String string("Test my game");	
	RenderWindow window(VideoMode(800, 600), "Test my game", ctx.getDevice(), odfaeg::window::Style::Default, true);
	//window.createDescriptorAndPipelines();
	Camera camera(800, 600, 80, 1, 1000);
	//camera.setUp(Vec3f(0.f, -1.f, 0.f));
	camera.move(0.f, 0.f, 5.f);
	window.setCamera(camera);
	ResourceManager<Texture, TextureNames> textureManager;
	ResourceManager<Texture, std::string> modelTextureManager;
	ModelLoader modelLoader(GPUContext::instance().getDevice(), modelTextureManager);
	GameObject* bistroExterior = modelLoader.loadModel(/*"CubeTest/cube_test.glb"*/"car/source/FINAL_MODEL_S4_13/FINAL_MODEL_S4.fbx"/*"Bistro_v5_2/BistroExterior.fbx"*/);
	//bistroExterior->setRotation(45, Vec3f(0, 1, 0));
	/*std::tuple<std::reference_wrapper<Device>> args = std::make_tuple(std::ref(ctx.getDevice()));
	textureManager.fromFileWithAlias("tilesets/wood.png", WOOD, args);
	Texture* texWood = textureManager.getResourceByAlias(WOOD);
	texWood->setSamplerAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);	
	Plane plane(Vec3f(-25, -2, -25), Vec3f(50, 0, 50));
	plane.setTexCoords(FloatRect(0, 0, 25, 25));
	plane.setTexture(texWood);
	
	window.addGameObject(&plane);
		
	Cube cube(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);		
	cube.setTexture(textureManager.getResourceByAlias(WOOD));
	cube.move(Vec3f(4.0f, -3.5f, 0.0));
    cube.scale(Vec3f(0.5f, 0.5f, 0.5f));   
	window.addGameObject(&cube);
    Cube cube2(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube2.setTexture(textureManager.getResourceByAlias(WOOD));
    cube2.move(Vec3f(2.0f, 3.0f, 1.0));
    cube2.scale(Vec3f(0.75f, 0.75f, 0.75f));
    window.addGameObject(&cube2);
    Cube cube3(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube3.setTexture(textureManager.getResourceByAlias(WOOD));
    cube3.move(Vec3f(-3.0f, -1.0f, 0.0));
    cube3.scale(Vec3f(0.5f, 0.5f, 0.5f));
    window.addGameObject(&cube3);
	Cube cube4(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube4.setTexture(textureManager.getResourceByAlias(WOOD));
    cube4.move(Vec3f(-1.5f, 1.0f, 1.5));
    cube4.scale(Vec3f(0.5f, 0.5f, 0.5f));
    window.addGameObject(&cube4);
    Cube cube5(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);
	cube5.setTexture(textureManager.getResourceByAlias(WOOD));
	cube5.move(Vec3f(-1.5f, 2.0f, -3.0));
    cube5.setRotation(60.0f, Vec3f(1.0, 0.0, 1.0));
    cube5.scale(Vec3f(0.75f, 0.75f, 0.75f));
   	window.addGameObject(&cube5);*/
	window.addGameObject(bistroExterior);	
		//std::cout<<"i : "<<i<<std::endl;
	
	//std::cout<<"ok"<<std::endl;
	Clock clock;
	unsigned int fps = 0;
	//std::cout<<"widnow adr = "<<&window<<std::endl;	
	/*ShadowRenderer shadowRenderer(window, 0, "*");
	ShadowRenderer::DirLight dirLight;
	dirLight.dir = Vec3f(20, 50, 20);
	shadowRenderer.addDirectionnalLight(dirLight);
	ShadowRenderer::PointLight pointLight;
	pointLight.pos = Vec3f(0, 0, 0);
	shadowRenderer.addPonctualLight(pointLight);*/
	while (window.isOpen()) {
		odfaeg::window::IEvent event;
		while (window.pollEvent(event)) {
			if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
				window.close();
			}
		}
		window.clear();		
		window.setTypesToRender("*", window.getCurrentFrame());
		//window.applyCullingAndBatching();
		window.draw(Triangles);
		/*shadowRenderer.clear();
		//shadowRenderer.drawNextFrame();
		shadowRenderer.draw();*/
		window.submit(true);
		window.display();
		fps++;
		if (clock.getElapsedTime() >= seconds(1.f)) {
			std::cout<<"FPS : "<<fps<<std::endl;
			fps = 0;
			clock.restart();
		}
	}
	return 0;
}