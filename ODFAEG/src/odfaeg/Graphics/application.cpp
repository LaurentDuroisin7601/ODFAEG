#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace core {
        using namespace sf;
        Application* Application::app = nullptr;
        Clock Application::timeClk = Clock();
        Application::~Application() {
            std::cout<<"rcm : "<<componentManager.get()<<std::endl;

            stop();
            for (unsigned int i = 0; i < windows.size(); i++) {
                if (windows[i].second)
                    delete windows[i].first;
            }
        }
    }
}
