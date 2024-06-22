#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
//using namespace odfaeg::audio;
using namespace sorrok;
/*class Mere {
public :
    template <class Self>
    void testMere(this Self&& self) {
        std::cout<<"je suis appelé par la classe mère"<<std::endl;
        self.testFille();
    }
};
class Fille : public Mere {
public :
    void testFille() {
        std::cout<<"je suis appelé par la classe fille"<<std::endl;
    }
};*/
int main() {
    /*Mere* mere = new Fille();
    mere->testMere();
    delete mere;*/
    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




