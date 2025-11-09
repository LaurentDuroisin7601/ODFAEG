#ifndef LABEL_HPP
#define LABEL_HPP
#include "../lightComponent.h"
#include "../rectangleShape.h"
#include "../renderWindow.h"
#include "../../Math/vec4.h"
#include "../text.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API Label : public LightComponent {
                public :
                Label (RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font* font, std::string text, unsigned int charSize);
                void clear();
                void onDraw (RenderTarget& target, RenderStates states = RenderStates::Default);
                void setBackgroundColor(Color color);
                Color getBackgroundColor();
                void setForegroundColor(Color color);
                Color getForegroundColor();
                void onEventPushed(window::IEvent event, RenderWindow& window);
                void setText(std::string text);
                core::String getText();
                bool isMouseInside();
                void onUpdate(RenderWindow* window, window::IEvent& event);
                void setBorderColor(Color color);
                void setBorderThickness(float thickness);
                unsigned int getCharacterSize() {
                    return text.getCharacterSize();
                }
                private :
                math::Vec3f mousePos;
                RectangleShape rect;
                Text text;
                Color background;
            };
        }
    }
}
#endif
