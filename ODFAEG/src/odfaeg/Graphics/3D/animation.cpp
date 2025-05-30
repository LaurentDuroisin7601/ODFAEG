#include "../../../../include/odfaeg/Graphics/3D/animation.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            Animation::Animation(const std::string& animationPath, Entity* model) : model(model)
            {
                Assimp::Importer importer;
                const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
                assert(scene && scene->mRootNode);
                auto animation = scene->mAnimations[0];
                m_Duration = animation->mDuration;
                m_TicksPerSecond = animation->mTicksPerSecond;
                readHeirarchyData(m_RootNode, scene->mRootNode);
                readMissingBones(animation, *model);
                size = model->getSize();
            }
            math::Vec3f Animation::getSize() {
                return size;
            }

            Bone* Animation::findBone(const std::string& name)
            {
                auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                    [&](const Bone& Bone)
                    {
                        return Bone.getBoneName() == name;
                    }
                );
                if (iter == m_Bones.end()) return nullptr;
                else return &(*iter);
            }


            float Animation::getTicksPerSecond() { return m_TicksPerSecond; }

            float Animation::getDuration() { return m_Duration;}

            const Animation::AssimpNodeData& Animation::getRootNode() { return m_RootNode; }

            const std::map<std::string,Entity::BoneInfo>& Animation::getBoneIDMap()
            {
                return m_BoneInfoMap;
            }

            void Animation::readMissingBones(const aiAnimation* animation, Entity& model)
            {
                int size = animation->mNumChannels;

                auto& boneInfoMap = model.getBoneInfoMap();//getting m_BoneInfoMap from Model class
                int& boneCount = model.getBoneCount(); //getting the m_BoneCounter from Model class

                //reading channels(bones engaged in an animation and their keyframes)
                for (int i = 0; i < size; i++)
                {
                    auto channel = animation->mChannels[i];
                    std::string boneName = channel->mNodeName.data;

                    if (boneInfoMap.find(boneName) == boneInfoMap.end())
                    {
                        boneInfoMap[boneName].id = boneCount;
                        boneCount++;
                    }
                    m_Bones.push_back(Bone(channel->mNodeName.data,
                        boneInfoMap[channel->mNodeName.data].id, channel));
                }

                m_BoneInfoMap = boneInfoMap;
            }

            void Animation::readHeirarchyData(AssimpNodeData& dest, const aiNode* src)
            {
                assert(src);

                dest.name = src->mName.data;
                dest.transformation = convertAssimpToODFAEGMatrix(src->mTransformation);
                dest.childrenCount = src->mNumChildren;

                for (int i = 0; i < src->mNumChildren; i++)
                {
                    AssimpNodeData newData;
                    readHeirarchyData(newData, src->mChildren[i]);
                    dest.children.push_back(newData);
                }
            }
            math::Matrix4f Animation::convertAssimpToODFAEGMatrix(aiMatrix4x4 aiMatrix) {
                math::Matrix4f mat;
                mat.m11 = aiMatrix.a1;
                mat.m12 = aiMatrix.a2;
                mat.m13 = aiMatrix.a3;
                mat.m14 = aiMatrix.a4;

                mat.m21 = aiMatrix.b1;
                mat.m22 = aiMatrix.b2;
                mat.m23 = aiMatrix.b3;
                mat.m24 = aiMatrix.b4;

                mat.m31 = aiMatrix.c1;
                mat.m32 = aiMatrix.c2;
                mat.m33 = aiMatrix.c3;
                mat.m34 = aiMatrix.c4;

                mat.m41 = aiMatrix.d1;
                mat.m42 = aiMatrix.d2;
                mat.m43 = aiMatrix.d3;
                mat.m44 = aiMatrix.d4;
                return mat;
            }
            Entity* Animation::getModel() {
                return model;
            }
            Animation::~Animation() {
                delete model;
            }
        }
    }
}
