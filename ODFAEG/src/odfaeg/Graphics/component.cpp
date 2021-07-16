#include "../../../include/odfaeg/Graphics/component.h"
#include "../../../include/odfaeg/Graphics/application.h"
namespace odfaeg {
    namespace graphic {
        //int Component::nbComponents = 0;
        Component::Component(RenderWindow& window, math::Vec3f position, math::Vec3f size, math::Vec3f origin, unsigned int priority)
        : Drawable(), Transformable(position, size, origin),
        listener(),
        priority(priority),
        window(window) {
            //std::cout<<"create component"<<std::endl;
            activateEventContext = true;
            visible = true;
            id = -1;
            if (core::Application::app != nullptr)
                id = core::Application::app->getComponentId()-1;
            /*id = nbComponents;
            nbComponents++;*/
            autoResize = false;
            relPosition = false;
        }
        void Component::setEventContextActivated(bool activateEventContext) {
            this->activateEventContext = activateEventContext;
            if (!activateEventContext) {
                /*if (name == "TEXCOORDY")
                    std::cout<<"clear tex coord y event stack"<<std::endl;*/
                getListener().clearEventsStack();
            }
            onEventContextActivated(activateEventContext);
        }
        bool Component::isEventContextActivated() {
            return activateEventContext;
        }
        void Component::setVisible (bool visible) {
            onVisibilityChanged(visible);
            this->visible = visible;
        }
        bool Component::isVisible() {
            return visible;
        }
        void Component::processEvents() {
           if (activateEventContext)
               listener.processEvents();
        }
        void Component::onVisibilityChanged(bool visible) {}
        void Component::onEventContextActivated(bool activate) {}
        View& Component::getView() {
            View view;
            return view;
        }
        void Component::setView(View view) {
        }
        std::string Component::getExpression() {
            return "";
        }
        void Component::setExpression(std::string expression) {
        }
        bool Component::loadEntitiesOnComponent (std::vector<Entity*> entities) {
        }
        void Component::draw(Drawable& drawable, RenderStates states) {
        }
        void Component::loadTextureIndexes() {
        }
    }
}
