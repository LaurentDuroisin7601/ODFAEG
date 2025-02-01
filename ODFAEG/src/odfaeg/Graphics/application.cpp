#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace core {
        using namespace sf;
        Application* Application::app = nullptr;
        Clock Application::timeClk = Clock();
    }
}
