#ifndef ODFAEG_MESH_HPP
#define ODFAEG_MESH_HPP
#include "gameObject.hpp"
namespace odfaeg {
    namespace graphic {
        class Mesh : public GameObject {
            public :
            Mesh(EntityFactory& factory);
            Mesh(math::Vec3f position, math::Vec3f size, std::string type, EntityFactory& factory);
            bool operator==(Entity& other);
            bool isAnimated() const;
            bool isModel() const;
            bool selectable() const;
            bool isLight() const;
            bool isShadow() const;
            bool isLeaf() const;
            void onDraw(RenderTarget& target, RenderStates states);
            Entity* clone();
            template <typename Archive>
            void vtserialize(Archive& ar) {
                GameObject::vtserialize(ar);
            }
        };
    }
}
#endif
