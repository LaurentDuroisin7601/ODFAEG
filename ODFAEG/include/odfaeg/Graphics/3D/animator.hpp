#ifndef ODFAEG_ANIMATOR_HPP
#define ODFAEG_ANIMATOR_HPP
#include "animation.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Animator : public GameObject {
            public :
                Animator(Animation* animation, EntityFactory& factory);
                void updateAnimation(float dt);
                void playAnimation(Animation* pAnimation);
                void calculateBoneTransform(const Animation::AssimpNodeData* node, math::Matrix4f parentTransform);
                std::vector<math::Matrix4f> getFinalBoneMatrices();
                void onDraw (RenderTarget &target, RenderStates states) {}
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
                    return false;
                }
                Entity* getCurrentFrame() const;
                ~Animator();
                private:
                std::vector<math::Matrix4f> m_FinalBoneMatrices;
                Animation* m_CurrentAnimation;
                float m_CurrentTime;
                float m_DeltaTime;
            };
        }
    }
}
#endif // ODFAEG_ANIMATOR_HPP
