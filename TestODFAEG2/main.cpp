#include <chrono>
#include <iostream>
#include <thread>
#include <vulkan/vulkan.h>
#include <cmath>
#include <bits/random.h>
#include <vector>
import odfaeg.window.videoMode;
import odfaeg.graphic.renderWindow;
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
using namespace odfaeg::window;
using namespace odfaeg::graphic;
using namespace odfaeg::math;
using namespace odfaeg::core;
enum TextureNames {
	WOOD
};
std::random_device device;
std::mt19937 generator = std::mt19937(device());
int main() {
	GPUContext& ctx = GPUContext::instance();	
	RenderWindow window(VideoMode(800, 600), "Test my game", ctx.getDevice(), odfaeg::window::Style::Default, true);
	//window.createDescriptorAndPipelines();
	Camera camera(800, 600, 80, 1, 1000);
	//camera.setUp(Vec3f(0.f, -1.f, 0.f));
	camera.move(0.f, 0.f, 5.f);
	window.setCamera(camera);
	ResourceManager<Texture, TextureNames> textureManager;
	std::tuple<std::reference_wrapper<Device>> args = std::make_tuple(std::ref(ctx.getDevice()));
	textureManager.fromFileWithAlias("tilesets/wood.png", WOOD, args);
	Texture* texWood = textureManager.getResourceByAlias(WOOD);
	texWood->setSamplerAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);	
	Plane plane(Vec3f(-25, -2, -25), Vec3f(50, 0, 50));
	plane.setTexCoords(FloatRect(0, 0, 25, 25));
	plane.setTexture(texWood);
	
	window.addGameObject(&plane);
	std::vector<Cube*> cubes;
	for (unsigned int i = 0; i < 10; i++) {
		
		Cube* cube = new Cube(Vec3f(-1, -1, -1), 2, 2, 2, Color::White);		
		cube->setTexture(textureManager.getResourceByAlias(WOOD));		
		static std::uniform_real_distribution<float> offsetDistribution = std::uniform_real_distribution<float>(-10, 10);
        static std::uniform_real_distribution<float> scaleDistribution = std::uniform_real_distribution<float>(1.0, 2.0);
        static std::uniform_real_distribution<float> rotationDistribution = std::uniform_real_distribution<float>(0, 180);
		cube->move(Vec3f(offsetDistribution(generator), offsetDistribution(generator)+10, offsetDistribution(generator)));
		//cube->move(Vec3f(0, 5, 0));
		//cube->setRotation(45, Vec3f(0, 1, 0));
		cube->setRotation(rotationDistribution(generator), Vec3f(1, 0, 1));
		float scale = scaleDistribution(generator);
		cube->setScale(Vec3f(scale, scale, scale));
		cubes.push_back(cube);
		window.addGameObject(cube);
		//std::cout<<"i : "<<i<<std::endl;
	}
	//std::cout<<"ok"<<std::endl;
	Clock clock;
	unsigned int fps = 0;
	//std::cout<<"widnow adr = "<<&window<<std::endl;
	ShadowRenderer shadowRenderer(window, 0, "*");
	while (window.isOpen()) {
		odfaeg::window::IEvent event;
		while (window.pollEvent(event)) {
			if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
				window.close();
			}
		}
		window.clear();		
		/*window.setTypesToRender("*", window.getCurrentFrame());
		window.draw(Triangles);*/
		shadowRenderer.clear();
		shadowRenderer.draw();	
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