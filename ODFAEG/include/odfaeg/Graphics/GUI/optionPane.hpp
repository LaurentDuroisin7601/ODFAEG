#ifndef ODFAEG_OPTIONPANE_HPP
#define ODFAEG_OPTIONPANE_HPP
#include "../lightComponent.h"
#include "button.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class OptionPane : public LightComponent {
            public :
                enum TYPE {
                    MESSAGE_DIALOG, CONFIRMATION_DIALOG
                };
                enum OPTION {
                    UNDEFINED, NO_OPTION, YES_OPTION
                };
                void clear();
                void onVisibilityChanged(bool visible);
                void setText(std::string text);
                OPTION getOption();
                OptionPane(math::Vec3f position, math::Vec3f size, const Font* font, sf::String t, TYPE type);
                void onDraw(RenderTarget& target, RenderStates states);
                void onNoOption();
                void onYesOption();
                void onEnter();
                void onEventPushed(window::IEvent event, RenderWindow& window);
                ~OptionPane();
            private :
                sf::Color backgroundColor;
                Button* yes;
                Button* no;
                Text text;
                RenderWindow rw;
                OPTION option;
                TYPE type;
            };
        }
    }
}
#endif // ODFAEG_OPTIONPANE_HPP
