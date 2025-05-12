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
    Device device(instance);*/
    RenderWindow window(sf::VideoMode(600, 800), "test"/*, device*/);
    Font font(device);
    font.loadFromFile("fonts/FreeSerif.ttf");
    Text text("test", font, 20);
    text.setFillColor(sf::Color::Red);
    text.setBackgroundColor(sf::Color::Blue);
    CircleShape circle(10);
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
    }
    /*MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();*/
}




