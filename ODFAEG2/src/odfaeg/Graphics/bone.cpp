module;
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
//import odfaeg.graphic.bone;
module odfaeg.graphic.bone;
import odfaeg.graphic.assimpHelper;
import odfaeg.entity.transformMatrix;
import odfaeg.math.vec;
import odfaeg.graphic.gameObject;
namespace odfaeg {
    namespace graphic {
        Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel)
            : GameObject(math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f), "E_BONE"),
              m_Name(name),
              m_ID(ID),
              m_LocalTransform(),
              channel(channel) {
            m_NumPositions = channel->mNumPositionKeys;
            for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
                aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
                float timeStamp = channel->mPositionKeys[positionIndex].mTime;
                KeyPosition data;
                data.position = AssimpHelpers::getGLMVec(aiPosition);

                data.timeStamp = timeStamp;
                m_Positions.push_back(data);
            }
            m_NumRotations = channel->mNumRotationKeys;
            for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
                aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                KeyRotation data;
                data.orientation = AssimpHelpers::getGLMQuat(aiOrientation);
                data.timeStamp = timeStamp;
                m_Rotations.push_back(data);
            }

            m_NumScalings = channel->mNumScalingKeys;
            for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
                aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                KeyScale data;
                data.scale = AssimpHelpers::getGLMVec(scale);
                data.timeStamp = timeStamp;
                m_Scales.push_back(data);
            }
        }

        bool Bone::isBoneMatrixValid(const std::string &boneName, const math::Matrix4f &m) {
            math::Vec3f xAxis(m[0][0], m[1][0], m[2][0]);
            math::Vec3f yAxis(m[0][1], m[1][1], m[2][1]);
            math::Vec3f zAxis(m[0][2], m[1][2], m[2][2]);
            math::Vec3f translation(m[0][3], m[1][3], m[2][3]);

            float scaleX = xAxis.magnitude();
            float scaleY = yAxis.magnitude();
            float scaleZ = zAxis.magnitude();

            float dotXY = std::abs(xAxis.dot(yAxis));
            float dotYZ = std::abs(yAxis.dot(zAxis));
            float dotZX = std::abs(zAxis.dot(xAxis));

            bool isOrtho = (dotXY < 0.01f && dotYZ < 0.01f && dotZX < 0.01f);
            bool isUniformScale = (std::abs(scaleX - 1.0f) < 0.1f &&
                                   std::abs(scaleY - 1.0f) < 0.1f &&
                                   std::abs(scaleZ - 1.0f) < 0.1f);
            bool isTranslationReasonable = translation.magnitude() < 1000.0f;

            std::cout << "Bone: " << boneName << "\n";
            std::cout << "Translation: " << translation << "\n";
            std::cout << "Scale: X=" << scaleX << " Y=" << scaleY << " Z=" << scaleZ << "\n";
            std::cout << "Orthogonality: XY=" << dotXY << " YZ=" << dotYZ << " ZX=" << dotZX << "\n";

            if (!isOrtho) std::cout << " Axes not orthogonal\n";
            if (!isUniformScale) std::cout << "Non-uniform scale\n";
            if (!isTranslationReasonable) std::cout << " Translation too large\n";

            std::cout << "-----------------------------\n";

            return isOrtho && isUniformScale && isTranslationReasonable;
        }

        /*interpolates  b/w positions,rotations & scaling keys based on the curren time of
        the animation and prepares the local transformation matrix by combining all keys
        tranformations*/
        void Bone::update(float animationTime) {
            glm::mat4 translation = interpolatePosition(animationTime);
            glm::mat4 rotation = interpolateRotation(animationTime);
            glm::mat4 scale = interpolateScaling(animationTime);
            /*std::cout<<"translation : "<<translation<<std::endl;
            std::cout<<"rotation : "<<rotation<<std::endl;
            std::cout<<"scale : "<<scale<<std::endl;*/

            m_LocalTransform = translation * rotation * scale;

            //std::cout<<"local transform : "<<m_LocalTransform.transpose()<<std::endl;
            /*if (!isBoneMatrixValid(getBoneName(), m_LocalTransform)) {
                std::cerr<<"bone matrice not valid"<<std::endl;
                int i;
                std::cin>>i;
            }*/
        }

        glm::mat4 Bone::getLocalTransform() { return m_LocalTransform; }
        std::string Bone::getBoneName() const { return m_Name; }
        int Bone::getBoneID() { return m_ID; }


        /* Gets the current index on mKeyPositions to interpolate to based on
        the current animation time*/
        int Bone::getPositionIndex(float animationTime) {
            for (int index = 0; index < m_NumPositions - 1; ++index) {
                if (animationTime < m_Positions[index + 1].timeStamp)
                    return index;
            }
            assert(0);			
        }

        /* Gets the current index on mKeyRotations to interpolate to based on the
        current animation time*/
        int Bone::getRotationIndex(float animationTime) {
            for (int index = 0; index < m_NumRotations - 1; ++index) {
                if (animationTime < m_Rotations[index + 1].timeStamp)
                    return index;
            }
            assert(0);            
        }

        /* Gets the current index on mKeyScalings to interpolate to based on the
        current animation time */
        int Bone::getScaleIndex(float animationTime) {
            for (int index = 0; index < m_NumScalings - 1; ++index) {
                if (animationTime < m_Scales[index + 1].timeStamp)
                    return index;
            }
            assert(0);            
        }


        /* Gets normalized value for Lerp & Slerp*/
        float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
            float scaleFactor = 0.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        /*figures out which position keys to interpolate b/w and performs the interpolation
        and returns the translation matrix*/
        glm::mat4 Bone::interpolatePosition(float animationTime) {
            if (1 == m_NumPositions) {
                return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
            }

            int p0Index = getPositionIndex(animationTime);
            int p1Index = p0Index + 1;
            //std::cout<<"positon index : "<<p0Index<<"\n";

            float scaleFactor = getScaleFactor(m_Positions[p0Index].timeStamp,
                                               m_Positions[p1Index].timeStamp, animationTime);
            glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
            return glm::translate(glm::mat4(1.0f), finalPosition);
        }

        /*figures out which rotations keys to interpolate b/w and performs the interpolation
        and returns the rotation matrix*/
        glm::mat4 Bone::interpolateRotation(float animationTime) {
            if (1 == m_NumRotations) {
                glm::quat rotation = glm::normalize(m_Rotations[0].orientation);
                return glm::toMat4(rotation);
            }

            int p0Index = getRotationIndex(animationTime);
            int p1Index = p0Index + 1;
            //std::cout<<"rotation index : "<<p0Index<<"\n";
            float scaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp,
                                               m_Rotations[p1Index].timeStamp, animationTime);
            glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
                m_Rotations[p1Index].orientation, scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::toMat4(finalRotation);
        }

        /*figures out which scaling keys to interpolate b/w and performs the interpolation
        and returns the scale matrix*/
        glm::mat4 Bone::interpolateScaling(float animationTime) {
            if (1 == m_NumScalings) {
                return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
            }

            int p0Index = getScaleIndex(animationTime);
            int p1Index = p0Index + 1;
            //std::cout<<"scale index : "<<p0Index<<"\n";
            float scaleFactor = getScaleFactor(m_Scales[p0Index].timeStamp,
                                               m_Scales[p1Index].timeStamp, animationTime);
            glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
            , scaleFactor);
            return glm::scale(glm::mat4(1.0f), finalScale);
        }

        GameObject *Bone::clone() {
            Bone *b = new Bone(m_Name, m_ID, channel);
            b->m_NumPositions = m_NumPositions;
            b->m_Rotations = m_Rotations;
            b->m_NumScalings = m_NumScalings;
            b->m_Positions = m_Positions;
            b->m_Rotations = m_Rotations;
            b->m_Scales = m_Scales;
            GameObject::copy(b);
            return b;
        }
    }
}
