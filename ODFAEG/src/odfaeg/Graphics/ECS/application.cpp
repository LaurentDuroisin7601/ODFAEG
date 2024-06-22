#include "../../../../include/odfaeg/Graphics/ECS/application.hpp"
namespace odfaeg {
    namespace core {
        namespace ecs {
            using namespace sf;
            Application* Application::app = nullptr;
            Clock Application::timeClk = Clock();
        }
    }
}
