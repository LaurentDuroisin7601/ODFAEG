#ifndef ODFAEG_WALL_HPP
#define ODFAEG_WALL_HPP
#include <string>
#include "gameObject.hpp"
#include "../Math/vec.hpp"
namespace odfaeg {
    namespace entity {
        class Wall : public GameObject {
        public :
            enum Type {
                TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, TOP_BOTTOM, RIGHT_LEFT, T_TOP, T_RIGHT, T_LEFT, T_BOTTOM, X, NB_WALL_TYPES
            };
            Wall(Type type);
            Wall(Type type, math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, GameObject* parent=nullptr);
            bool operator==(GameObject& other);
            GameObject* clone();
            template <typename Archive>
            void vtserialize(Archive& ar) {
                GameObject::vtserialize(ar);
            }
            Type getWallType();
        private :
            Type type;
        };
#endif