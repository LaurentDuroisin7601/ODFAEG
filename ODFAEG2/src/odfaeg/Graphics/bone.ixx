module;
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
export module odfaeg.graphic.bone;
import odfaeg.math.vec;
import odfaeg.math.matrix;
import odfaeg.math.quaternion;
import odfaeg.graphic.gameObject;
namespace odfaeg {
    namespace graphic {
        export class Bone : public GameObject {
            public:
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
            Bone(Bone&&) noexcept = default;
            Bone& operator=(Bone&&) noexcept = default;
            void update(float animationTime);
            glm::mat4 getLocalTransform();
            std::string getBoneName() const;
            int getBoneID();
            int getPositionIndex(float animationTime);
            int getRotationIndex(float animationTime);
            int getScaleIndex(float animationTime);
            GameObject* clone();
        private :
            bool isBoneMatrixValid(const std::string& boneName, const math::Matrix4f& m);
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
            const aiNodeAnim* channel;
        };
    }
}