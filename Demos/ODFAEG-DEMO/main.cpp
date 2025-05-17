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
#include "odfaeg/Graphics/renderWindow.h"
#include "odfaeg/Graphics/font.h"
#include "odfaeg/Graphics/text.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Window/iEvent.hpp"


int main(int argc, char *argv[]) {
    /*VkSettup instance;
    Device device(instance);
    RenderWindow window(sf::VideoMode(800, 600), "test", device);
    window.getView().move(400, 300, 0);
    RenderTexture rt1(device), rt2(device);
    rt1.enableDepthTest(true);
    rt1.create(800, 600);
    rt2.enableDepthTest(true);
    rt2.create(800, 600);
    Sprite rtSprite1(rt1.getTexture(), Vec3f(0, 0, 0), Vec3f(800, 600, 0), sf::IntRect(0, 0, 800, 600));
    Sprite rtSprite2(rt2.getTexture(), Vec3f(0, 0, 0), Vec3f(800, 600, 0), sf::IntRect(0, 0 ,800, 600));
    RectangleShape rs(Vec3f(100, 50, 100));
    RectangleShape rs2(Vec3f(100, 50, 100));

    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
           if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
           }
        }
        rt1.clear();
        rt1.draw(rs);
        rt1.display();
        rt2.clear();
        rt2.draw(rs);
        rt2.display();
        window.clear();
        window.draw(rtSprite1);
        window.draw(rtSprite2);
        window.display();
    }*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




