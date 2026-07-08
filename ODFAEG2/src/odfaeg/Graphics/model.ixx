module;
#include <string>
export module odfaeg.graphic.model;
import odfaeg.graphic.gameObject;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        export class Model : public GameObject {
        public :
            Model();
            Model(math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, GameObject* parent=nullptr);
            bool operator==(GameObject& other);
            GameObject* clone();
            template <typename Archive>
            void vtserialize(Archive& ar) {
                GameObject::vtserialize(ar);
            }
        };
    }
}//
// Created by laurent on 26/05/2026.
//
