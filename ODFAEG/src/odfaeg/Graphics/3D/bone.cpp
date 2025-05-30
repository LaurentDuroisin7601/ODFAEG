#include "../../../../include/odfaeg/Graphics/3D/bone.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
                :
                m_Name(name),
                m_ID(ID),
                m_LocalTransform()
            {
                m_NumPositions = channel->mNumPositionKeys;

                for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
                {
                    aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
                    float timeStamp = channel->mPositionKeys[positionIndex].mTime;
                    KeyPosition data;
                    data.position = convertAssimpToODFAEGVec4(aiPosition);
                    data.timeStamp = timeStamp;
                    m_Positions.push_back(data);
                }

                m_NumRotations = channel->mNumRotationKeys;
                for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
                {
                    aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                    float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                    KeyRotation data;
                    data.orientation = convertAssimpToODFAEGQuaternion(aiOrientation);
                    data.timeStamp = timeStamp;
                    m_Rotations.push_back(data);
                }

                m_NumScalings = channel->mNumScalingKeys;
                for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
                {
                    aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                    float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                    KeyScale data;
                    data.scale = convertAssimpToODFAEGVec4(scale);
                    data.timeStamp = timeStamp;
                    m_Scales.push_back(data);
                }
            }
            math::Vec3f Bone::convertAssimpToODFAEGVec4(aiVector3D aiVec) {
                math::Vec3f vec;
                vec.x = aiVec.x;
                vec.y = aiVec.y;
                vec.z = aiVec.z;
                return vec;
            }
            math::Quaternion Bone::convertAssimpToODFAEGQuaternion(aiQuaternion aiQuat) {
                math::Quaternion quat;
                quat.x = aiQuat.x;
                quat.y = aiQuat.y;
                quat.z = aiQuat.z;
                quat.w = aiQuat.w;
                return quat;
            }
            /*interpolates  b/w positions,rotations & scaling keys based on the curren time of
            the animation and prepares the local transformation matrix by combining all keys
            tranformations*/
            void Bone::update(float animationTime)
            {
                math::Matrix4f translation = interpolatePosition(animationTime);
                math::Matrix4f rotation = interpolateRotation(animationTime);
                math::Matrix4f scale = interpolateScaling(animationTime);
                m_LocalTransform = translation * rotation * scale;
            }

            math::Matrix4f Bone::getLocalTransform() { return m_LocalTransform; }
            std::string Bone::getBoneName() const { return m_Name; }
            int Bone::getBoneID() { return m_ID; }


            /* Gets the current index on mKeyPositions to interpolate to based on
            the current animation time*/
            int Bone::getPositionIndex(float animationTime)
            {
                for (int index = 0; index < m_NumPositions - 1; ++index)
                {
                    if (animationTime < m_Positions[index + 1].timeStamp)
                        return index;
                }
                assert(0);
            }

            /* Gets the current index on mKeyRotations to interpolate to based on the
            current animation time*/
            int Bone::getRotationIndex(float animationTime)
            {
                for (int index = 0; index < m_NumRotations - 1; ++index)
                {
                    if (animationTime < m_Rotations[index + 1].timeStamp)
                        return index;
                }
                assert(0);
            }

            /* Gets the current index on mKeyScalings to interpolate to based on the
            current animation time */
            int Bone::getScaleIndex(float animationTime)
            {
                for (int index = 0; index < m_NumScalings - 1; ++index)
                {
                    if (animationTime < m_Scales[index + 1].timeStamp)
                        return index;
                }
                assert(0);
            }


            /* Gets normalized value for Lerp & Slerp*/
            float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
            {
                float scaleFactor = 0.0f;
                float midWayLength = animationTime - lastTimeStamp;
                float framesDiff = nextTimeStamp - lastTimeStamp;
                scaleFactor = midWayLength / framesDiff;
                return scaleFactor;
            }

            /*figures out which position keys to interpolate b/w and performs the interpolation
            and returns the translation matrix*/
            math::Matrix4f Bone::interpolatePosition(float animationTime)
            {
                if (1 == m_NumPositions) {
                    TransformMatrix tm;
                    tm.setTranslation(m_Positions[0].position);
                    return tm.getMatrix();
                }

                int p0Index = getPositionIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = getScaleFactor(m_Positions[p0Index].timeStamp,
                    m_Positions[p1Index].timeStamp, animationTime);
                math::Vec3f finalPosition = m_Positions[p0Index].position.mix(m_Positions[p1Index].position, scaleFactor);
                TransformMatrix tm;
                tm.setTranslation(finalPosition);
                return tm.getMatrix();
            }

            /*figures out which rotations keys to interpolate b/w and performs the interpolation
            and returns the rotation matrix*/
            math::Matrix4f Bone::interpolateRotation(float animationTime)
            {
                if (1 == m_NumRotations)
                {
                    auto rotation = (m_Rotations[0].orientation).normalize();
                    return rotation.toRotationMatrix();
                }

                int p0Index = getRotationIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp,
                    m_Rotations[p1Index].timeStamp, animationTime);
                math::Quaternion finalRotation = m_Rotations[p0Index].orientation.slerp(
                    m_Rotations[p1Index].orientation, scaleFactor);
                finalRotation = finalRotation.normalize();
                return finalRotation.toRotationMatrix();
            }

            /*figures out which scaling keys to interpolate b/w and performs the interpolation
            and returns the scale matrix*/
            math::Matrix4f Bone::interpolateScaling(float animationTime)
            {
                if (1 == m_NumScalings) {
                    TransformMatrix tm;
                    tm.setScale(m_Scales[0].scale);
                    return tm.getMatrix();
                }

                int p0Index = getScaleIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = getScaleFactor(m_Scales[p0Index].timeStamp,
                    m_Scales[p1Index].timeStamp, animationTime);
                math::Vec3f finalScale = m_Scales[p0Index].scale.mix(m_Scales[p1Index].scale
                    , scaleFactor);
                TransformMatrix tm;
                tm.setScale(finalScale);
                return tm.getMatrix();
            }
        }
    }
}
