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
    Font font(device);
    font.loadFromFile("fonts/FreeSerif.ttf");
    Text text("test", font);
    text.setFillColor(sf::Color::Red);
    text.setBackgroundColor(sf::Color::Green);
    Texture tex(device);
    tex.loadFromFile("tilesets/herbe.png");
    Sprite sprite(tex, Vec3f(0, 0, 0), Vec3f(120, 60, 0), sf::IntRect(0, 0, 50, 25));
    RenderTexture rt(device);
    rt.create(800, 600);
    RectangleShape rs(Vec3f(100, 100, 0));


    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
           if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
           }
        }
        rt.clear();
        rt.draw(rs);
        rt.display();
        window.clear();
        window.draw(text);
        window.display();
    }*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




