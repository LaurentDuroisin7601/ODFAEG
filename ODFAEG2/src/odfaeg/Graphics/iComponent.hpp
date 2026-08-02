#ifndef ODFAEG_ICOMPONENT_HPP
#define ODFAEG_ICOMPONENT_HPP
#include "../Window/listener.hpp"
#include "../Physics/boundingBox.hpp"
#include "../Window/iEvent.hpp"
namespace odfaeg {
    namespace graphic {
        class IComponent {
        public :
            IComponent(int windowId) : windowId(windowId), relativePosSize(0.f, 0.f, 0.f, 1.f, 1.f, 1.f) {
                relativePosToParent = relativeSizeToParent = false;
                id = nbComponents;
                nbComponents++;
            }
            int getWindowId() {
                return windowId;
            }
            window::Listener& getEventListener() {
                return eventListener;
            } 
            void setRelativePosToParent(bool relative) {
                relativePosToParent = relative;
            }
            void setRelativeSizeToParent(bool relative) {
                relativeSizeToParent = relative; 
            }
            bool isRelativePosToParent() {
                return relativePosToParent;
            }
            bool isRelativeSizeToParent() {
                return relativeSizeToParent;
            }
            physic::BoundingBox getRelativePosSize() {
                return relativePosSize;
            }
            unsigned int getId() {
                return id;
            }
            static unsigned int getNbComponents() {
                return nbComponents;
            }
            virtual void draw() = 0;
            virtual void update(int windowId, window::IEvent event) = 0;                        
        private :
            unsigned int id;
            inline static unsigned int nbComponents = 0;                       
            window::Listener eventListener;
            int windowId;
            physic::BoundingBox relativePosSize;
            bool relativePosToParent, relativeSizeToParent;
        };
    }
}
#endif
