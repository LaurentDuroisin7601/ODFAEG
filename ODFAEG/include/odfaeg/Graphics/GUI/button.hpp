#ifndef ODFAEG_BUTTON_HPP
#define ODFAEG_BUTTON_HPP
#include "../lightComponent.h"
#include "../renderWindow.h"
#include "../rectangleShape.h"
#include "../text.h"
#include "../font.h"
#include "actionListener.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Button : public LightComponent {
                public :
                    Button (math::Vec3f position, math::Vec3f size, const Font* font, std::string t, unsigned int charSize, RenderWindow& rw);
                    void clear();
                    void onDraw(RenderTarget& target, RenderStates states=RenderStates::Default);
                    void setTextSize(unsigned int size);
                    void setTextColor(sf::Color color);
                    std::string getText();
                    void addActionListener(ActionListener* al);
                    bool isMouseInButton();
                    void onEventPushed(window::IEvent event, RenderWindow& window);
                    void onUpdate(RenderWindow* window, window::IEvent& event);
                    void setText(std::string text);
                    void setIcon(const Texture* text);
                    const Texture* getIcon();
                private :
                    math::Vec3f size;
                    Text text;
                    RectangleShape rect;
                    sf::Color background;
                    math::Vec3f mousePos;
            };
        }
    }
}
#endif // ODFAEG_BUTTON_HPP
