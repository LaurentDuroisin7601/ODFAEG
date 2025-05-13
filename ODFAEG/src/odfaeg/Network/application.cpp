#include "../../../include/odfaeg/Network/application.h"
namespace odfaeg {
    namespace core {
        using namespace sf;
        #ifdef VULKAN
        template<typename A, typename T>
        Clock Application<A, T>::timeClk = Clock();
        #else
        Application* Application::app = nullptr;
        Clock Application::timeClk = Clock();
        #endif
    }
}
