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
//Make index sequences from an offset.

int main(int argc, char *argv[]) {
    /*VkSettup instance;
    Device device(instance);
    RenderWindow window(VideoMode(800, 600), "test", device);
    Texture texture(device);
    texture.loadFromFile("tilesets/eau.png");
    Sprite sprite(texture, Vec3f(0, 0, 0), Vec3f(100, 50, 0), IntRect(0, 0, 100, 50));
    while (window.isOpen()) {
        window.clear();
        window.draw(sprite);
        window.display();
    }*/



    MyAppli app(VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




