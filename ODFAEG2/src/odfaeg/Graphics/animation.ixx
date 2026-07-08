module;
#include <map>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
export module odfaeg.graphic.animation;
import odfaeg.graphic.bone;
import odfaeg.math.matrix;
import odfaeg.graphic.gameObject;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        export class Animation {
        public :
            struct AssimpNodeData
            {
                glm::mat4 transformation;
                std::string name;
                int childrenCount;
                std::vector<AssimpNodeData> children;
            };
            Animation(const std::string& animationPath, GameObject* model);
            Bone* findBone(const std::string& name);
            float getTicksPerSecond();
            float getDuration();
            const AssimpNodeData& getRootNode();
            const std::map<std::string,GameObject::BoneInfo>& getBoneIDMap();
            math::Vec3f getSize();
            GameObject* getModel();
            ~Animation();
        private :
            void readMissingBones(const aiAnimation* animation);
            void readHeirarchyData(AssimpNodeData& dest, const aiNode* src);
            float m_Duration;
            int m_TicksPerSecond;
            std::vector<Bone*> m_Bones;
            AssimpNodeData m_RootNode;
            std::map<std::string, GameObject::BoneInfo> m_BoneInfoMap;
            GameObject* model;
        };
    }
}
