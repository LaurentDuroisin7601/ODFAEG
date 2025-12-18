#include "../../../../include/odfaeg/Graphics/GUI/toolBar.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            ToolBar::ToolBar(RenderWindow& rw) :
                LightComponent(rw, math::Vec3f(0, 0, 0), math::Vec3f(rw.getSize().x(), 20, 0), math::Vec3f(0, 0, 0), -2) {
                rect = RectangleShape(math::Vec3f(rw.getSize().x(), 20, 0));
                rect.setPosition(math::Vec3f(0, 20, 0));
                background = Color(50, 50, 50);
            }
            void ToolBar::addItem(LightComponent* item) {
                unsigned int posX = rect.getPosition().x();
                for (unsigned int i = 0; i < items.size(); i++) {
                    posX += items[i]->getSize().x();
                }
                item->setPosition(math::Vec3f(posX, 20, 0));
                items.push_back(item);
            }
            void ToolBar::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void ToolBar::onDraw(RenderTarget& target, RenderStates states) {
                target.draw(rect);
                #ifdef VULKAN
                target.submit(false);
                #endif
            }
            void ToolBar::onEventPushed(window::IEvent event, RenderWindow& window) {
                if (&window == &getWindow()) {
                    getListener().pushEvent(event);
                }
            }
        }
    }
}
