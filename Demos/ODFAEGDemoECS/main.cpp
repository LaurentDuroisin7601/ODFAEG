  #include "application.hpp"
  using namespace odfaeg::core;
  int main() {
      sorrok::MyECSAppli app(sf::VideoMode(800, 600), "Test odfaeg");
      return app.exec();
  }
