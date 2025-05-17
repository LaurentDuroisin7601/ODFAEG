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
    /*VkSettup settup;
    VkDevice device;
    RenderWindow window(sf::VideoMode(800, 600), "test",device);
    window.enableDepthTest(true);
    RectangleShape rect(Vec3f(100, 50, 100));
    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
            }
            window.clear();
            window.draw(rect);
            window.display();
        }
    }*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




