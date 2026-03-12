#include "application.h"
#include "odfaeg/Graphics/graphics.hpp"
#include "odfaeg/Audio/audio.hpp"
#include "odfaeg/Math/math.hpp"
#include "hero.hpp"



#include "odfaeg/Graphics/renderWindow.h"
#include "odfaeg/Graphics/font.h"
#include "odfaeg/Graphics/text.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Window/iEvent.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <iostream>
#include <math.h>
#include <functional>
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;

void f(std::unique_ptr<int> test) {
    std::cout<<"test : "<<*test<<std::endl;
}
void f2(int test, int test2, int test3, int test4, int test5) {
    std::cout<<"test : "<<test<<","<<test2<<","<<test3<<","<<test4<<","<<test2<<std::endl;
}
int main(int argc, char *argv[]) {
    std::unique_ptr<int> i = std::make_unique<int>(5);
    FastDelegate<void> fd(&f, std::move(i));
    FastDelegate<void> fd2(&f2, ph<0, int>(),ph<3, int>(),ph<2, int>(),ph<1, int>(),ph<0, int>());
    fd();
    fd2.bind(5, 6, 7, 8);
    fd2();



    /*VkSettup instance;
    Device device(instance);

    RenderWindow window(VideoMode(800, 600), "test", device, Style::Default, ContextSettings(0, 0, 4, 4, 6));
    Font font(device);
    font.loadFromFile("fonts/FreeSerif.ttf");
    Text text("ODFAEG", font, 15);

    text.setFillColor(Color::Red);
    text.move(Vec3f(0, 0, 200));

    Texture texture(device);
    texture.loadFromFile("tilesets/eau.png");
    Sprite sprite(texture, Vec3f(10, 10, 0), Vec3f(100, 50, 0), IntRect(0, 0, 100, 50));
    RectangleShape rect(Vec3f(50, 50, 0));
    rect.move(Vec3f(0, 0, 100));
    rect.setOutlineThickness(1);
    rect.setOutlineColor(Color::Red);
    RenderTexture rt(device);
    rt.create(800, 600);
    text.setCharacterSize(30);

    window.createDescriptorsAndPipelines();
    rt.createDescriptorsAndPipelines();
    rt.enableDepthTest(true);




    //RenderComponentManager rcm(window);

    while(window.isOpen()) {
        window.clear(Color::Black);
        rt.clear();
        rt.draw(sprite);
        rt.draw(rect);
        rt.draw(text);
        rt.submit(true);
        Sprite sprite2(rt.getTexture(rt.getCurrentFrame()), Vec3f(0, 0, 0), Vec3f(800, 600, 0), IntRect(0, 0, 800, 600));
        sprite2.move(Vec3f(-400, -300, 0));

        window.draw(sprite2);
        window.submit(true);
        rt.display();
        window.display();
        odfaeg::window::IEvent event;

        while (window.pollEvent(event))
        {
            // évènement "fermeture demandée" : on ferme la fenêtre
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED)
                window.close();


        }


    }*/

    MyAppli app(VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




