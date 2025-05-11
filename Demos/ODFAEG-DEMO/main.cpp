#include "application.h"
/*#include "odfaeg/Graphics/graphics.hpp"
#include "odfaeg/Audio/audio.hpp"
#include "odfaeg/Math/math.hpp"
#include "hero.hpp"*/


using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;

int main(int argc, char *argv[]) {
    /*VkSettup instance;
    Device device(instance);
    RenderWindow window(sf::VideoMode(600, 800), "test", device);
    window.getView().move(400, 300, 0);
    Texture texture(device);
    texture.loadFromFile("tilesets/eau.png");
    Sprite sprite(texture, Vec3f(0, 0, 0), Vec3f(100, 50, 0), sf::IntRect(0, 0, 100, 50));
    RenderTexture rt(device);
    rt.create(800, 600);
    Sprite rtSprite(rt.getTexture(), Vec3f(0, 0, 0), Vec3f(800, 600, 0), sf::IntRect(0, 0, 800, 600));
    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
           if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
           }
        }
        rt.clear();
        rt.draw(sprite);
        rt.display();
        window.clear();
        window.draw(rtSprite);
        window.display();
    }*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




