  #include "application.hpp"
  using namespace odfaeg::core;
  using namespace odfaeg::graphic::ecs;
  int main() {
      EntityFactory factory;
      ComponentMapping componentMapping(factory);
      sorrok::MyECSAppli app(sf::VideoMode(800, 600), "Test odfaeg", componentMapping);
      return app.exec();
  }
