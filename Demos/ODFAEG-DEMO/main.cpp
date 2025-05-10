#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
//using namespace odfaeg::audio;
using namespace sorrok;
/*class Test : public Drawable, Transformable {
public :
    Test(Device& vkDevice) : rt(vkDevice), rect(Vec3f(100, 50, 0)) {
        rt.create(800, 600);
        rtSprite = Sprite(rt.getTexture(), Vec3f(0, 0, 0), Vec3f(800, 600, 0), sf::IntRect(0, 0, 800, 600));

    }
    void drawNextFrame() {
        rt.clear();
        rt.draw(rect);
        rt.display();
    }
    void draw(RenderTarget& target, RenderStates states) {
        target.draw(rtSprite, states);
    }
private :
    RenderTexture rt;
    Sprite rtSprite;
    RectangleShape rect;
};*/
int main() {
    /*VkSettup settup;
    Device device(settup);
    RenderWindow window(sf::VideoMode(800, 600), "test", device);
    window.getView().move(400, 300, 0);

    Test test1(device), test2(device);
    RectangleShape rect(Vec3f(100, 50, 0));
    while (window.isOpen()) {
        IEvent event;
        while (window.pollEvent(event)) {
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
                window.close();
            }
        }
        test1.drawNextFrame();
        test2.drawNextFrame();
        window.clear();
        window.draw(test1);
        window.draw(test2);
        window.display();
    }*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




