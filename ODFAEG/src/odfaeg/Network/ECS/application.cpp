#include "../../../../include/odfaeg/Network/ECS/application.hpp"
namespace odfaeg {
    namespace core {
        namespace ecs {
            using namespace sf;
            #ifdef VULKAN
            template <typename A, typename T>
            Clock Application<A, T>::timeClk = Clock();
            #else
            Application* Application::app = nullptr;
            Clock Application::timeClk = Clock();
            #endif
        }
    }
}
