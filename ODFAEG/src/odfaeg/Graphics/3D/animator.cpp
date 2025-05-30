#include "../../../../include/odfaeg/Graphics/3D/animator.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            Animator::Animator(Animation* animation, EntityFactory& factory) : GameObject (math::Vec3f(0, 0, 0), animation->getSize(),animation->getSize() * 0.5f,"E_BONE_ANIMATION", factory)
            {
                animation->getModel()->setParent(this);
                addChild(animation->getModel());
                m_CurrentTime = 0.0;
                m_CurrentAnimation = animation;

                m_FinalBoneMatrices.reserve(MAX_BONES);

                for (int i = 0; i < MAX_BONES; i++)
                    m_FinalBoneMatrices.push_back(math::Matrix4f());
            }

            void Animator::updateAnimation(float dt)
            {
                m_DeltaTime = dt;
                if (m_CurrentAnimation)
                {
                    m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * dt;
                    m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getDuration());
                    calculateBoneTransform(&m_CurrentAnimation->getRootNode(), math::Matrix4f());
                }
            }

            void Animator::playAnimation(Animation* pAnimation)
            {
                m_CurrentAnimation = pAnimation;
                m_CurrentTime = 0.0f;
            }

            void Animator::calculateBoneTransform(const Animation::AssimpNodeData* node, math::Matrix4f parentTransform)
            {
                std::string nodeName = node->name;
                math::Matrix4f nodeTransform = node->transformation;

                Bone* Bone = m_CurrentAnimation->findBone(nodeName);

                if (Bone)
                {
                    Bone->update(m_CurrentTime);
                    nodeTransform = Bone->getLocalTransform();
                }

                math::Matrix4f globalTransformation = parentTransform * nodeTransform;

                auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
                if (boneInfoMap.find(nodeName) != boneInfoMap.end())
                {
                    int index = boneInfoMap[nodeName].id;
                    math::Matrix4f offset = boneInfoMap[nodeName].offset;
                    m_FinalBoneMatrices[index] = globalTransformation * offset;
                    //std::cout<<"bone matrix : "<<m_FinalBoneMatrices[index]<<std::endl;
                }

                for (int i = 0; i < node->childrenCount; i++)
                    calculateBoneTransform(&node->children[i], globalTransformation);
            }

            std::vector<math::Matrix4f> Animator::getFinalBoneMatrices()
            {
                return m_FinalBoneMatrices;
            }
            Entity* Animator::clone() {
                Animator* a = factory.make_entity<Animator>(m_CurrentAnimation, factory);
                GameObject::copy(a);
                return a;
            }
            Entity* Animator::getCurrentFrame() const {
                return m_CurrentAnimation->getModel();
            }
            Animator::~Animator() {
                delete m_CurrentAnimation;
            }
        }
    }
}
