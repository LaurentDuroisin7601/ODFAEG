#ifndef ODFAEG_ANIMATOR_HPP
#define ODFAEG_ANIMATOR_HPP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <string>
#include <odfaeg/config.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "gameObject.hpp"
#include "animation.hpp"
namespace odfaeg {
    namespace entity {
        class Animator : public GameObject {
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
#include "animator.inl"
#endif
