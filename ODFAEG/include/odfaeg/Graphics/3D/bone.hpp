#ifndef ODFAEG_BONE_HPP
#define ODFAEG_BONE_HPP
#include "../../Math/quaternion.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../export.hpp"
#include "../../Math/transformMatrix.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "../assimp_helper.hpp"
#include "../gameObject.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Bone : public GameObject {
            public :
                struct KeyPosition
                {
                    math::Vec3f position;
                    float timeStamp;
                };

                struct KeyRotation
                {
                    math::Quaternion orientation;
                    float timeStamp;
                };

                struct KeyScale
                {
                    math::Vec3f scale;
                    float timeStamp;
                };
                Bone(const std::string& name, int ID, const aiNodeAnim* channel, EntityFactory& factory);
                Bone(Bone&&) noexcept = default;
                Bone& operator=(Bone&&) noexcept = default;
                void update(float animationTime);
                math::Matrix4f getLocalTransform();
                std::string getBoneName() const;
                int getBoneID();
                int getPositionIndex(float animationTime);
                int getRotationIndex(float animationTime);
                int getScaleIndex(float animationTime);
                 /**
                * \fn bool isAnimated() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not an animation.
                */
                bool isAnimated() const {
                    return false;
                }
                /**
                * \fn bool isModel() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not an model.
                */
                bool isModel() const {
                    return false;
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
                /**
                * \fn void onDraw (RenderTarget &target, RenderStates states) const;
                * \brief draw the tile on the given render target with the given render states.
                * \param target : the render target.
                * \param states : the render states.
                */
                void onDraw (RenderTarget &target, RenderStates states);
                Entity* clone();
            private :
                bool isBoneMatrixValid(const std::string& boneName, const math::Matrix4f& m);
                float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
                math::Matrix4f interpolatePosition(float animationTime);
                math::Matrix4f interpolateRotation(float animationTime);
                math::Matrix4f interpolateScaling(float animationTime);
                std::vector<KeyPosition> m_Positions;
                std::vector<KeyRotation> m_Rotations;
                std::vector<KeyScale> m_Scales;
                int m_NumPositions;
                int m_NumRotations;
                int m_NumScalings;

                math::Matrix4f m_LocalTransform;
                std::string m_Name;
                int m_ID;
                EntityFactory& factory;
                const aiNodeAnim* channel;

            };
        }
    }
}
#endif // ODFAEG_BONE_HPP
