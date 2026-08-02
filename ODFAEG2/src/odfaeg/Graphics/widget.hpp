#ifndef ODFAEG_WIDGET_HPP
#define ODFAEG_WIDGET_HPP
#include <vector>
#include <memory>
#include "iComponent.hpp"
#include "renderTarget.hpp"
#include "../Window/iEvent.hpp"
namespace odfaeg {
    namespace graphic {
        class Widget : public IComponent {            
        public :
            Widget(RenderTarget& parentTarget, std::string type, unsigned int windowId=-1) : IComponent(windowId), parentTarget(parentTarget), type(type) {

            }
            void addChild(Widget* child) {
                std::unique_ptr<Widget> ptr;
                ptr.reset(child);
                children.push_back(std::move(ptr));
            }
            void enable(bool enabled) {
                this->enabled = enabled;
                getEventListener().enable(enabled);
                for (unsigned int i = 0; i < children.size(); i++) {
                    children[i]->enable(enabled);
                }
            }
            std::string getType() {
                return type;
            }
            void draw() {
                onDraw();
                for (unsigned int i = 0; i < children.size(); i++) {
                    children[i]->draw();
                }                
            }
            void update(int windowId, window::IEvent event) {   
                /*Si c'est une fenêtre est liée au widget, on met à jour la pile d'évènements du listener.*/            
                if (windowId == getWindowId() || windowId == -1) {
                    getEventListener().pushEvent(event);
                }
                for (unsigned int i = 0; i < children.size(); i++) {
                    children[i]->update(windowId, event);
                }
            }       
            virtual void onDraw() = 0;
        private :
            std::vector<std::unique_ptr<Widget>> children;
            bool enabled;
            std::string type;
            RenderTarget& parentTarget;            
        };
    }
}
#endif