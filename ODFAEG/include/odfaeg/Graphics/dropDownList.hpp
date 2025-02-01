#ifndef DROP_DOWN_LIST
#define DROP_DOWN_LIST
#include "../rectangleShape.h"
#include "../convexShape.h"
#include "../renderWindow.h"
#include "../lightComponent.h"
#include "../../Math/vec4.h"
#include "../text.h"
#include "label.hpp"
#include "../../../../include/odfaeg/Physics/boundingPolyhedron.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class DropDownList : public LightComponent {
                public :
                    DropDownList(RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font* font, std::string text);
                    void clear();
                    void draw (RenderTarget& target, RenderStates states = RenderStates::Default);
                    void onUpdate(RenderWindow* window, sf::Event& event);
                private :
                    math::Vec3f mousePos;
                    RectangleShape rect;
                    ConvexShape shape;
                    physic::BoundingPolyhedron bp;
                    std::vector<Label> items;
                    sf::Color background;
            };
        }
    }
}
#endif
