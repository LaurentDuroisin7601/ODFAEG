#ifndef ODFAEG_CHECKBOX_HPP
#define ODFAEG_CHECKBOX_HPP
#include "../lightComponent.h"
#include "../rectangleShape.h"
#include "../renderWindow.h"
#include "../../Math/vec4.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class CheckBox : public LightComponent {
                public :
                CheckBox(RenderWindow& window, math::Vec3f position, math::Vec3f size);
                void clear();
                void onDraw (RenderTarget& target, RenderStates states = RenderStates::Default);
                void setBackgroundColor(sf::Color color);
                sf::Color getBackgroundColor();
                void setCheckedColor(sf::Color color);
                sf::Color getCheckedColor();
                void onUpdate(RenderWindow* window, window::IEvent& event);
                bool isMouseInside();
                void setChecked(bool checked);
                bool isChecked();
                void switchState();
                private :
                math::Vec3f mousePosition;
                RectangleShape rect;
                bool checked;
                sf::Color backgroundColor, checkedColor;
            };
        }
    }
}
#endif // ODFAEG_CHECKBOX_HPP
