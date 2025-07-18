#ifndef MENU_HPP
#define MENU_HPP
#include "menuItem.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API Menu : public LightComponent {
            public :
                Menu(RenderWindow& rw, const Font* font, std::string text);
                void addMenuItem(MenuItem* item);
                void clear();
                void setPosition (math::Vec3f position);
                void onDraw(RenderTarget& target, RenderStates states=RenderStates::Default);
                void onClick();
                bool isMouseOnMenu();
                void onEventPushed(window::IEvent event, RenderWindow& window);
            private :
                RectangleShape rect;
                Text text;
                Color background;
                std::vector<MenuItem*> items;
                bool isContextChanged;
            };
        }
    }
}
#endif
