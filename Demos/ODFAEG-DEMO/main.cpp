#include "application.h"
/*#include "odfaeg/Graphics/graphics.hpp"
#include "odfaeg/Audio/audio.hpp"
#include "odfaeg/Math/math.hpp"
#include "hero.hpp"*/


/*using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;*/
using namespace sorrok;
/*#include "odfaeg/Graphics/renderWindow.h"
#include "odfaeg/Graphics/font.h"
#include "odfaeg/Graphics/text.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Window/iEvent.hpp"*/


int main(int argc, char *argv[]) {
    /*VkSettup instance;
    Device device(instance);
    RenderWindow window(sf::VideoMode(800, 600), "test", device);
    Font font(device);
    font.loadFromFile("fonts/FreeSerif.ttf");
    Text text("test", font);
    text.setFillColor(sf::Color::Red);
    text.setBackgroundColor(sf::Color::Green);
    sf::Image img;
    img.loadFromFile("tilesets/herbe.png");
    const sf::Uint8* pixels = img.getPixelsPtr();
    Texture tex(device);
    tex.loadFromFile("tilesets/herbe.png");
    sf::Image clear;
    clear.create(img.getSize().x, img.getSize().y, sf::Color::Black);
    tex.update(clear.getPixelsPtr(), img.getSize().x, img.getSize().y, 0, 0);
    tex.update(pixels, 50, 25, 0, 0);
    Sprite sprite(tex, Vec3f(0, 0, 0), Vec3f(120, 60, 0), sf::IntRect(0, 0, 50, 25));
    /*CircleShape circle(10);
    ConvexShape cv(3);
    cv.setPoint(0, sf::Vector3f(25, 0, 0));
    cv.setPoint(1, sf::Vector3f(0, 50, 0));
    cv.setPoint(2, sf::Vector3f(50, 50, 0));
    cv.setOutlineThickness(5);
    cv.setOutlineColor(sf::Color::Red);

    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
           if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
           }
        }
        window.clear();
        window.draw(text);
        window.display();
    }*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




