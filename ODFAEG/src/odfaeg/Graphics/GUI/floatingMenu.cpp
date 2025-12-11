#include "../../../../include/odfaeg/Graphics/GUI/floatingMenu.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            FloatingMenu::FloatingMenu() : Transformable(math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0), math::Vec3f(0, 0, 0)) {

            }
            void FloatingMenu::addMenuItem(MenuItem* item) {
                unsigned int yPos = getPosition().y();
                for (unsigned int i = 0; i < items.size(); i++) {
                    yPos += items[i]->getSize().y();
                }
                item->setPosition(math::Vec3f(getPosition().x(), yPos, 0));
                items.push_back(item);
            }
            void FloatingMenu::setPosition(math::Vec3f position) {
                Transformable::setPosition(position);
                unsigned int yPos = getPosition().y();
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->setPosition(math::Vec3f(position.x(), yPos, position.z()));
                    yPos += items[i]->getSize().y();
                }
            }
            void FloatingMenu::setVisible(bool visible) {
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->setVisible(visible);
                    items[i]->setEventContextActivated(visible);
                }
            }
            std::vector<MenuItem*> FloatingMenu::getItems() {
                return items;
            }
            void FloatingMenu::clear() {
                items.clear();
            }
        }
    }
}
