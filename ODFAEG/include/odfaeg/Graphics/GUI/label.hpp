#ifndef LABEL_HPP
#define LABEL_HPP
#include "../rectangleShape.h"
#include "../renderWindow.h"
#include "../lightComponent.h"
#include "../../Math/vec4.h"
#include "../text.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Label : public LightComponent {
                public :
                Label (RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font* font, std::string text, unsigned int charSize);
                void clear();
                void onDraw (RenderTarget& target, RenderStates states = RenderStates::Default);
                void setBackgroundColor(sf::Color color);
                sf::Color getBackgroundColor();
                void setForegroundColor(sf::Color color);
                sf::Color getForegroundColor();
                void onEventPushed(window::IEvent event, RenderWindow& window);
                void setText(std::string text);
                sf::String getText();
                bool isMouseInside();
                void onUpdate(RenderWindow* window, window::IEvent& event);
                void setBorderColor(sf::Color color);
                void setBorderThickness(float thickness);
                unsigned int getCharacterSize() {
                    return text.getCharacterSize();
                }
                private :
                math::Vec3f mousePos;
                RectangleShape rect;
                Text text;
                sf::Color background;
            };
        }
    }
}
#endif
