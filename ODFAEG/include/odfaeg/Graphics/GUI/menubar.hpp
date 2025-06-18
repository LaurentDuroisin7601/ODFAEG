#ifndef MENU_BAR
#define MENU_BAR
#include "menu.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API MenuBar : public LightComponent {
                public :
                MenuBar(RenderWindow& rw);
                void addMenu(Menu* menu);
                void clear();
                void onDraw(RenderTarget& target, RenderStates states=RenderStates::Default);
                void onEventPushed(window::IEvent event, RenderWindow& window);
            private :
                RectangleShape rect;
                Color background;
                std::vector<Menu*> menus;
            };
        }
    }
}
#endif
