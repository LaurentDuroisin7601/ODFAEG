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
                    glm::mat4 transformation;
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
                Entity* getModel();
                ~Animation();
            private :
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
