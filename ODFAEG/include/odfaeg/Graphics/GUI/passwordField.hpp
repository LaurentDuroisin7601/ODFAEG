#ifndef ODFAEG_PASSWORD_FIELD
#define ODFAEG_PASSWORD_FIELD
#include "../lightComponent.h"
#include "../renderWindow.h"
#include "../rectangleShape.h"
#include "../text.h"
#include "../font.h"
#include "focusListener.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class PasswordField : public LightComponent {
            public :
                PasswordField (math::Vec3f position, math::Vec3f size, const Font* font, std::string s, RenderWindow& rw);
                void clear();
                void onDraw(RenderTarget& target, RenderStates states);
                void setTextSize(unsigned int size);
                void setTextColor(sf::Color color);
                void gaignedFocus();
                void lostFocus();
                void onGaignedFocus();
                void onLostFocus();
                bool isMouseInTextArea();
                bool isMouseOutTextArea();
                void onTextEntered(char caracter);
                void onUpdate(RenderWindow* window, window::IEvent& event);
                std::string getText();
                void setText(std::string text);
                void setCursorPos();
                bool hasFocus();
                void onEventPushed(window::IEvent event, RenderWindow& window);
            private :
                unsigned int currentIndex;
                std::string tmp_text, id_text;
                math::Vec3f size;
                Text text;
                RectangleShape rect;
                sf::Color background;
                math::Vec3f cursorPos;
                bool haveFocus;
            };
        }
    }
}
#endif // ODFAEG_TEXT_AREA

