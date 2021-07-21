#ifndef ODFAEG_MENU_ITEM
#define ODFAEG_MENU_ITEM
#include "../rectangleShape.h"
#include "../renderWindow.h"
#include "../lightComponent.h"
#include "../../Math/vec4.h"
#include "../text.h"
#include "menuItemListener.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class MenuItem : public LightComponent {
            public :
                MenuItem(RenderWindow& rw, const Font* font, std::string text);
                void clear();
                void setPosition (math::Vec3f position);
                void onDraw(RenderTarget& target, RenderStates states=RenderStates::Default);
                bool isMouseOnMenu();
                void addMenuItemListener (MenuItemListener *mil);
                std::string getText();
                void setText(std::string text);
                void onEventPushed(window::IEvent event, RenderWindow& window);
            private :
                RectangleShape rect;
                Text text;
                sf::Color background;
            };
        }
    }
}
#endif // ODFAEG_MENU_ITEM
