module;
#include <vector>
#include <memory>
export module odfaeg.graphic.componentManager;
import odfaeg.graphic.iComponent;
import odfaeg.window.iEvent;
namespace odfaeg {
    namespace graphic { 
        export class ComponentManager {       
            public :        
            void addComponent(IComponent* component) {
                std::unique_ptr<IComponent> ptr;
                ptr.reset(component);
                components.push_back(std::move(ptr));
            }    
            void update(int windowId, window::IEvent event) {
                for (unsigned int i = 0; i < components.size(); i++) {
                    components[i]->update(windowId, event);
                }
            } 
            private :
            std::vector<std::unique_ptr<IComponent>> components;
        };
    }
}