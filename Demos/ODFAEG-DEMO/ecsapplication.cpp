#include "ecsapplication.hpp"
namespace sorrok {
    MyECSAppli::MyECSAppli(sf::VideoMode vm, std::string title) : Application<MyECSAppli>(wm, title, sf::Style::Default, ContextSettings(0, 0, 4, 4, 6)) {
        fpsCounter = 0;
    }
    MyECSAppli::onLoad() {
        TextureManager<> tm;
        tm.fromFileWithAlias("tilesets/herbe.png", "GRASS");
        cache.addResourceManager(tm, "TextureManager");
    }
}
