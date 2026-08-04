#ifndef ODFAEG_CUBE_HPP
#define ODFAEG_CUBE_HPP
#include <string>
#include "../Math/vec.hpp"
#include "../Entity/rect.hpp"
#include "../Entity/gameObject.hpp"
namespace odfaeg {
    namespace entity {
        class Cube : public GameObject {
        public :
            Cube (math::Vec3f position, float w, float h, float d, Color color);
            bool operator== (GameObject& other) {
                if (!GameObject::operator==(other))
                    return false;
                return true;
            }
            void setTexCoords (FloatRect texRect);
            GameObject* clone();
            Color getColor();
            void setTexture(std::string textureId);
        private:
            Color m_color;
            FloatRect m_textRect;
            std::string m_textureId;            
        };
    }
}
#endif