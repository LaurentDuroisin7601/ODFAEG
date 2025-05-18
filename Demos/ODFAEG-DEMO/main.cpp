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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_FORCE_LEFT_HANDED

int main(int argc, char *argv[]) {
    /*VkSettup settup;
    Device device(settup);
    RenderWindow window(sf::VideoMode(800, 600), "test",device);
    Vec3f pos(100, 50, 100);
    Matrix4f proj = window.getView().getProjMatrix().getMatrix();
    Matrix4f view = window.getView().getViewMatrix().getMatrix();
    proj.m22 *= -1;
    TransformMatrix tm;
    tm.setTranslation(Vec3f(100, 50, 50));
    tm.setOrigin(Vec3f(50, 25, 0));
    Matrix4f model = tm.getMatrix();

    //std::cout<<"lf : "<<lh<<std::endl;

    Vec3f mvpgpu = proj * view * model * pos;
    std::cout<<mvpgpu<<std::endl;
    Vec3f mvpcpu = proj * view * model * pos;
    std::cout<<mvpcpu<<std::endl;*/
    /*window.enableDepthTest(true);
    RectangleShape rect(Vec3f(100, 50, 100));*/
    /*while (window.isOpen()) {
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




