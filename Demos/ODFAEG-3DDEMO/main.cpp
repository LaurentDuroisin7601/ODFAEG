#include "application.h"
#include "odfaeg/Math/triangle.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::core;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
int main (int argv, char* argc[]) {
    /*glm::mat4 lightProjection;
    lightProjection = glm::perspective(Math::toRadians(80.f), 800.f / 600.f, 0.1f, 100.f);

    std::cout<<glm::to_string(lightProjection)<<std::endl;

    View view(800, 600, 80, 0.1f, 100.f);
    std::cout<<view.getProjMatrix().getMatrix().transpose()<<std::endl;*/

    MyAppli appli(Vec2f(800, 600), "test");
    return appli.exec();
}
