#ifndef ODFAEG_TOOLBAR_HPP
#define ODFAEG_TOOLBAR_HPP
#include "../lightComponent.h"
#include "../rectangleShape.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API ToolBar : public LightComponent {
            public :
                ToolBar(RenderWindow& rw);
                void addItem(LightComponent* item);
                void clear();
                void onDraw(RenderTarget& target, RenderStates states=RenderStates::Default);
                void onEventPushed(window::IEvent event, RenderWindow& window);
            private :
                RectangleShape rect;
                Color background;
                std::vector<LightComponent*> items;
            };
        }
    }
}
#endif // ODFAEG_TOOLBAR_HPP
