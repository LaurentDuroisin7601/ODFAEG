#ifndef ODFAEG_ANIMATION_HPP
#define ODFAEG_ANIMATION_HPP
#include "bone.hpp"
#include "model.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Animation {
            public :
                struct AssimpNodeData
                {
                    math::Matrix4f transformation;
                    std::string name;
                    int childrenCount;
                    std::vector<AssimpNodeData> children;
                };
                Animation() = default;
                Animation(const std::string& animationPath, Entity* model);
                Bone* findBone(const std::string& name);
                float getTicksPerSecond();
                float getDuration();
                const AssimpNodeData& getRootNode();
                const std::map<std::string,Entity::BoneInfo>& getBoneIDMap();
                math::Vec3f getSize();
                ~Animation();
            private :
                math::Matrix4f convertAssimpToODFAEGMatrix(aiMatrix4x4 aiMatrix);
                void readMissingBones(const aiAnimation* animation, Entity& model);
                void readHeirarchyData(AssimpNodeData& dest, const aiNode* src);
                float m_Duration;
                int m_TicksPerSecond;
                std::vector<Bone> m_Bones;
                AssimpNodeData m_RootNode;
                std::map<std::string, Entity::BoneInfo> m_BoneInfoMap;
                math::Vec3f size;
                Entity* model;
            };
        }
    }
}
#endif // ODFAEG_ANIMATION_HPP
