module;
#include <cassert>
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <memory>
#include <iostream>
#include <glm/glm.hpp>
import odfaeg.graphic.animation;
module odfaeg.graphic.animation;
import odfaeg.graphic.assimpHelper;
namespace odfaeg {
    namespace graphic {
        Animation::Animation(const std::string& animationPath, GameObject* model) : model(model)
        {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
            assert(scene && scene->mRootNode);
            auto animation = scene->mAnimations[0];
            m_Duration = animation->mDuration;
            m_TicksPerSecond = animation->mTicksPerSecond;
            readHeirarchyData(m_RootNode, scene->mRootNode);
            readMissingBones(animation);
        }
        math::Vec3f Animation::getSize() {
            return model->getSize();
        }

        Bone* Animation::findBone(const std::string& name)
        {
            //std::cout<<"begin find bone"<<std::endl;
            auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                [&](const Bone* Bone)
                {
                    return Bone->getBoneName() == name;
                }
            );
            //std::cout<<"end find bone"<<std::endl;
            if (iter == m_Bones.end())
                return nullptr;
            return (*iter);
        }
        float Animation::getTicksPerSecond() { return m_TicksPerSecond; }

        float Animation::getDuration() { return m_Duration;}

        const Animation::AssimpNodeData& Animation::getRootNode() { return m_RootNode; }

        const std::map<std::string,GameObject::BoneInfo>& Animation::getBoneIDMap()
        {
            return m_BoneInfoMap;
        }

        void Animation::readMissingBones(const aiAnimation* animation)
        {
            int size = animation->mNumChannels;
            auto& boneInfoMap = model->getBoneInfoMap();//getting m_BoneInfoMap from Model class
            int& boneCount = model->getBoneCount(); //getting the m_BoneCounter from Model class

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
                Bone* b = new Bone(channel->mNodeName.data,
                    boneInfoMap[channel->mNodeName.data].id, channel);
                m_Bones.push_back(b);
            }
            m_BoneInfoMap = boneInfoMap;
        }

        void Animation::readHeirarchyData(AssimpNodeData& dest, const aiNode* src)
        {
            assert(src);

            dest.name = src->mName.data;
            dest.transformation = AssimpHelpers::convertMatrixToGLMFormat(src->mTransformation);
            //std::cout<<"dest transform : "<<dest.transformation.transpose()<<std::endl;
            dest.childrenCount = src->mNumChildren;

            for (int i = 0; i < src->mNumChildren; i++)
            {
                AssimpNodeData newData;
                readHeirarchyData(newData, src->mChildren[i]);
                dest.children.push_back(newData);
            }
        }
        GameObject* Animation::getModel() {
            return model;
        }
        Animation::~Animation() {

        }
    }
}