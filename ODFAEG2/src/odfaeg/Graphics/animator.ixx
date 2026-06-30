module;
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>
export module odfaeg.graphic.animator;
import odfaeg.graphic.animation;
import odfaeg.math.matrix;
import odfaeg.graphic.gameObject;
namespace odfaeg {
    namespace graphic {
        export class Animator : public GameObject {
        public:
            Animator(Animation* animation);
            void setBoneParent(const Animation::AssimpNodeData* node);
            void updateAnimation(float dt);
            void playAnimation(Animation* pAnimation);
            void calculateBoneTransform(const Animation::AssimpNodeData* node, glm::mat4 parentTransform);
            std::vector<glm::mat4> getFinalBoneMatrices();
            GameObject* clone();
            void attachGameObjectToBone(GameObject* entity, std::string boneName);
            ~Animator();
        private:
            std::vector<glm::mat4> m_FinalBoneMatrices;
            std::vector<glm::mat4> m_FinalBoneGlobalMatrices;
            Animation* m_CurrentAnimation;
            float m_CurrentTime;
            float m_DeltaTime;
        };
    }
}
