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
                dest.transformation = AssimpHelpers::convertAssimpToODFAEGNodeTransformMatrix(src->mTransformation);
                //std::cout<<"dest transform : "<<dest.transformation<<std::endl;
                dest.childrenCount = src->mNumChildren;

                for (int i = 0; i < src->mNumChildren; i++)
                {
                    AssimpNodeData newData;
                    readHeirarchyData(newData, src->mChildren[i]);
                    dest.children.push_back(newData);
                }
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
