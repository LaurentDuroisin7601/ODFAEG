#ifndef ODFAEG_TEXT_AREA
#define ODFAEG_TEXT_AREA
#include "../lightComponent.h"
#include "../renderWindow.h"
#include "../rectangleShape.h"
#include "../text.h"
#include "../font.h"
#include "focusListener.hpp"
#include "panel.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ODFAEG_GRAPHICS_API TextArea : public LightComponent {
            public :
                TextArea(math::Vec3f position, math::Vec3f size, const Font* font, std::string s, RenderWindow& rw);
                void clear();
                void onDraw(RenderTarget& target, RenderStates states);
                void setTextSize(unsigned int size);
                void setTextColor(Color color);
                void gaignedFocus();
                void lostFocus();
                void onGaignedFocus();
                void onLostFocus();
                bool isMouseInTextArea();
                bool isMouseOutTextArea();
                void onTextEntered(char caracter);
                void onUpdate(RenderWindow* window, window::IEvent& event);
                std::string getText();
                std::string getSelectedText();
                void setText(std::string text);
                void setCursorPos();
                void setCursorPos2();
                void setSelectedText();
                bool hasFocus();
                bool isTextChanged();
                void onEventPushed(window::IEvent event, RenderWindow& window);
                math::Vec3f getTextSize();
                unsigned int getCharacterIndexAtCursorPos();
                math::Vec3f getCursorPos();
                math::Vec3f getCursorPositionLocal();
                unsigned int getCharacterSize();
                void setCursorPosition(unsigned int currentIndex);
                void onMove(math::Vec3f& t);
            private :
                int scrollX, scrollY;
                int currentIndex, currentIndex2;
                core::String tmp_text, selected_text;
                math::Vec3f size;
                Text text;
                RectangleShape rect;
                Color background;
                math::Vec3f cursorPos;
                bool haveFocus, textChanged;
            };
        }
    }
}
#endif // ODFAEG_TEXT_AREA
