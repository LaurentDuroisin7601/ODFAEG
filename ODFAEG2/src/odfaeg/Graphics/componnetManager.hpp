#include <vector>
#include <memory>
#include "iComponent.hpp"
#include "../Window/iEvent.hpp"
namespace odfaeg {
    namespace graphic { 
        class ComponentManager {       
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