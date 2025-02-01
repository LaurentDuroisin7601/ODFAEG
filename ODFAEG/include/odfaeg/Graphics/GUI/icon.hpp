#ifndef ODFAEG_ICON_HPP
#define ODFAEG_ICON_HPP
#include "../lightComponent.h"
#include "../sprite.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Icon : public LightComponent {
                public :
                Icon(RenderWindow& window, math::Vec3f position, math::Vec3f size, Sprite icon);
                void clear() {}
                void onDraw(RenderTarget& target, RenderStates states);
                bool isMouseInside();
                void onUpdate(RenderWindow* window, window::IEvent& event);
                Sprite getSprite();
                private :
                math::Vec3f mousePos;
                Sprite icon;
            };
        }
    }
}
#endif
