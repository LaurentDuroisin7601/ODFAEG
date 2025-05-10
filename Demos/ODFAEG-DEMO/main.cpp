#include "application.h"
using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;
/*#include <iostream>
#include <memory>
#include <vector>
class RenderTarget {
public :
    RenderTarget() {}
    virtual ~RenderTarget() {
        std::cout<<"destroy render target"<<std::endl;
    }
};
class RenderTexture : public RenderTarget{
public :
    RenderTexture() {}
    ~RenderTexture() {
        std::cout<<"destroy render texture"<<std::endl;
    }
};
class Component {
    public :
    Component() {}
    virtual ~Component() {
        std::cout<<"destroy component"<<std::endl;
    }
};
class PerPixelLinkedListRenderComponent : public Component {
    public :
    PerPixelLinkedListRenderComponent() {
    }
    ~PerPixelLinkedListRenderComponent() {
        std::cout<<"destroy per pixel linked list render component"<<std::endl;
    }
    private :
        RenderTarget rt;
};
class RenderComponentManager {
    public :
    RenderComponentManager () {}
    void addComponent(Component* component) {
        std::unique_ptr<Component> ptr;
        ptr.reset(component);
        components.push_back(std::move(ptr));
    }
    ~RenderComponentManager() {
        std::cout<<"destroy component manager"<<std::endl;
    }
    private :
    std::vector<std::unique_ptr<Component>> components;
};
class Application {
    public :
    Application() {
        componentManager = std::make_unique<RenderComponentManager>();
    }
    ~Application() {
        std::cout<<"destroy application"<<std::endl;
    }
    RenderComponentManager& getComponentManager() {
        return *componentManager;
    }
    private :
    std::unique_ptr<RenderComponentManager> componentManager;
};
class MyApp : public Application {
public :
    MyApp() : Application() {}
};*/
int main() {
    /*MyApp app;
    PerPixelLinkedListRenderComponent* ppllrc = new PerPixelLinkedListRenderComponent();
    app.getComponentManager().addComponent(ppllrc);*/

    MyAppli app(sf::VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




