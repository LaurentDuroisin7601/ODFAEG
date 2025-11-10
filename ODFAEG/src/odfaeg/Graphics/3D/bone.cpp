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
                    data.position = AssimpHelpers::convertAssimpToODFAEGVec4(aiPosition);

                    data.timeStamp = timeStamp;
                    m_Positions.push_back(data);
                }

                m_NumRotations = channel->mNumRotationKeys;
                for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
                {
                    aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                    float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                    KeyRotation data;
                    data.orientation = AssimpHelpers::convertAssimpToODFAEGQuaternion(aiOrientation);
                    data.timeStamp = timeStamp;
                    m_Rotations.push_back(data);
                }

                m_NumScalings = channel->mNumScalingKeys;
                for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
                {
                    aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                    float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                    KeyScale data;
                    data.scale = AssimpHelpers::convertAssimpToODFAEGVec4(scale);
                    data.timeStamp = timeStamp;
                    m_Scales.push_back(data);
                }
            }
            bool Bone::isBoneMatrixValid(const std::string& boneName, const math::Matrix4f& m) {
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
            void Bone::update(float animationTime)
            {
                math::Matrix4f translation = interpolatePosition(animationTime);
                math::Matrix4f rotation = interpolateRotation(animationTime);
                math::Matrix4f scale = interpolateScaling(animationTime);
                /*std::cout<<"translation : "<<translation<<std::endl;
                std::cout<<"rotation : "<<rotation<<std::endl;
                std::cout<<"scale : "<<scale<<std::endl;*/

                m_LocalTransform = translation * scale * rotation;
                /*if (!isBoneMatrixValid(getBoneName(), m_LocalTransform)) {
                    std::cerr<<"bone matrice not valid"<<std::endl;
                }*/
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
                    auto rotation = m_Rotations[0].orientation.normalize();
                    return rotation.toRotationMatrix();
                }

                int p0Index = getRotationIndex(animationTime);
                int p1Index = p0Index + 1;
                float scaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp,
                    m_Rotations[p1Index].timeStamp, animationTime);
                math::Quaternion finalRotation = m_Rotations[p0Index].orientation.slerp(m_Rotations[p1Index].orientation, scaleFactor);
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
