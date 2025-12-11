#ifndef FLOATING_MENU_HPP
#define FLOATING_MENU_HPP
#include "menuItem.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API FloatingMenu : public Transformable  {
                public :
                FloatingMenu();
                void addMenuItem(MenuItem* item);
                void setPosition (math::Vec3f position);
                void setVisible(bool visible);
                void clear();
                std::vector<MenuItem*> getItems();
            private :
                std::vector<MenuItem*> items;
            };
        }
    }
}
#endif // FLOATING_MENU_HPP
