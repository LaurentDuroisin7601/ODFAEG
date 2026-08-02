#ifndef ODFAEG_MODEL_HPP
#define ODFAEG_MODEL_HPP
#include <string>
#include "gameObject.hpp"
#include "../Math/vec.hpp"
namespace odfaeg {
    namespace entity {
        class Model : public GameObject {
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
#include "model.inl"
#endif
// Created by laurent on 26/05/2026.
//
