#ifndef CUBE
#define CUBE
#include <SFML/Graphics/Color.hpp>
#include "../vertexArray.h"
#include "../gameObject.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Cube : public GameObject {
            public :
                Cube (math::Vec3f position, float w, float h, float d, sf::Color color, EntityFactory& factory);
                bool operator== (Entity& other) {
                    if (!GameObject::operator==(other))
                        return false;
                    return m_color == other.getColor();
                }
                bool isAnimated() const {
                    return false;
                }
                bool isModel() const {
                    return false;
                }
                bool selectable() const {
                    return false;
                }
                bool isLight() const {
                    return false;
                }
                bool isShadow() const {
                    return false;
                }
                bool isLeaf() const {
                    return true;
                }
                void onDraw(RenderTarget &target, RenderStates states);
                void setTexCoords (int face, sf::IntRect texRect);
                Entity* clone();
                sf::Color getColor();
            private:
                sf::Color m_color;
            };
        }
    }
}
#endif // CUBE
