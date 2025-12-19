#ifndef ODFAEG_MENU_ITEM
#define ODFAEG_MENU_ITEM
#include "../lightComponent.h"
#include "../rectangleShape.h"
#include "../renderWindow.h"
#include "../../Math/vec4.h"
#include "../text.h"
#include "menuItemListener.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API MenuItem : public LightComponent {
            public :
                MenuItem(RenderWindow& rw, const Font* font, std::string text, int eventPriority=-1);
                void clear();
                void setPosition (math::Vec3f position);
                void onDraw(RenderTarget& target, RenderStates states=RenderStates::Default);
                bool isMouseOnMenu();
                void addMenuItemListener (MenuItemListener *mil);
                std::string getText();
                void setText(std::string text);
                void onEventPushed(window::IEvent event, RenderWindow& window);
                bool isContextChanged();
                void setContextChanged(bool contextChanged);
            private :
                RectangleShape rect;
                Text text;
                Color background;
                bool contextChanged;
            };
        }
    }
}
#endif // ODFAEG_MENU_ITEM
