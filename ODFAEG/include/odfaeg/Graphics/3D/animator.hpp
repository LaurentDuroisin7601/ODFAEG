#ifndef ODFAEG_ANIMATOR_HPP
#define ODFAEG_ANIMATOR_HPP
#include "animation.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Animator : public GameObject {
            public :
                Animator(Animation* Animation);
                void updateAnimation(float dt);
                void playAnimation(Animation* pAnimation);
                void calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
                std::vector<math::Matrix4f> getFinalBoneMatrices();
                Entity* clone();
                bool isAnimated() const {
                    return true;
                }
                /**
                * \fn bool isModel() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not an model.
                */
                bool isModel() const {
                    return true;
                }
                /**
                * \fn bool selectable() const
                * \brief redefinition of the method from the base class Entity.
                * \return true, a tile is selectable.
                */
                bool selectable() const {
                    return true;
                }
                /**
                * \fn bool isLight() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not a light.
                */
                bool isLight() const {
                    return false;
                }
                /**
                * \fn bool isShadow() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not a shadow.
                */
                bool isShadow() const {
                    return false;
                }
                 /**
                * \fn bool isLeaf() const
                * \brief redefinition of the method from the base class Entity.
                * \return true, a tile can't have children.
                */
                bool isLeaf() const {
                    return true;
                }
            };
        }
    }
}
#endif // ODFAEG_ANIMATOR_HPP
