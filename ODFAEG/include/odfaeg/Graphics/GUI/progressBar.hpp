#ifndef PROGRESS_BAR
#define PROGRESS_BAR
#include "../rectangleShape.h"
#include "../text.h"
#include "../renderWindow.h"
#include "../lightComponent.h"
#include "../../Math/vec4.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class ProgressBar : public LightComponent {
            public :
            ProgressBar(RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font& font, unsigned int charSize);
            void onDraw (RenderTarget& target, RenderStates states = RenderStates::Default);
            void setMaximum (int maxi);
            void setMinimum (int mini);
            void setValue (int value);
            void setColor (sf::Color barColor);
            void clear();
            int getValue();
            private :
            RectangleShape bar;
            Text text;
            int maxi, mini, value;
            sf::Color barColor;
            };
        }
    }
}
#endif // PROGRESS_BAR
