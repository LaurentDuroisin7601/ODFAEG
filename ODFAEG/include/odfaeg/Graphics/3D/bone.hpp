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
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Bone {
            public :
                struct KeyPosition
                {
                    glm::vec3 position;
                    float timeStamp;
                };

                struct KeyRotation
                {
                    glm::quat orientation;
                    float timeStamp;
                };

                struct KeyScale
                {
                    glm::vec3 scale;
                    float timeStamp;
                };
                Bone(const std::string& name, int ID, const aiNodeAnim* channel);
                void update(float animationTime);
                glm::mat4 getLocalTransform();
                std::string getBoneName() const;
                int getBoneID();
                int getPositionIndex(float animationTime);
                int getRotationIndex(float animationTime);
                int getScaleIndex(float animationTime);
            private :
                float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
                glm::mat4 interpolatePosition(float animationTime);
                glm::mat4 interpolateRotation(float animationTime);
                glm::mat4 interpolateScaling(float animationTime);
                std::vector<KeyPosition> m_Positions;
                std::vector<KeyRotation> m_Rotations;
                std::vector<KeyScale> m_Scales;
                int m_NumPositions;
                int m_NumRotations;
                int m_NumScalings;

                glm::mat4 m_LocalTransform;
                std::string m_Name;
                int m_ID;
            };
        }
    }
}
#endif // ODFAEG_BONE_HPP
