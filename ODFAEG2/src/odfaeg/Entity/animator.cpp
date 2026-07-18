module;
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <string>
#include <odfaeg/config.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//import odfaeg.graphic.animator;
module odfaeg.entity.animator;
import odfaeg.entity.bone;
import odfaeg.math.transformMatrix;
import odfaeg.entity.gameObject;
import odfaeg.entity.assimpHelper;
namespace odfaeg {
    namespace entity {
        Animator::Animator(Animation* animation) :
            GameObject (animation->getModel()->getPosition(), animation->getModel()->getSize(), animation->getModel()->getOrigin(),"E_BONE_ANIMATION")
        {
            /*animation->getModel()->setParent(this);
            addChild(animation->getModel());*/
            for (unsigned int i = 0; i < animation->getModel()->getSubMeshesCount(); i++) {
                addSubMesh(animation->getModel()->getSubMeshes()[i]);
            }
            m_CurrentTime = 0.0;
            m_CurrentAnimation = animation;
            setBoneParent(&m_CurrentAnimation->getRootNode());
            m_FinalBoneMatrices.reserve(MAX_BONES);
            m_FinalBoneGlobalMatrices.reserve(MAX_BONES);
            for (int i = 0; i < MAX_BONES; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.f));
            for (int i = 0; i < MAX_BONES; i++)
                m_FinalBoneGlobalMatrices.push_back(glm::mat4(1.f));

        }
        void Animator::setBoneParent(const Animation::AssimpNodeData* node) {
            std::string nodeName = node->name;
            Bone* bone = m_CurrentAnimation->findBone(nodeName);
            if (bone)
            {
                ////////std::cout<<"update"<<std::endl;
                addChild(bone);
                bone->setParent(this);

            }
            for (int i = 0; i < node->childrenCount; i++)
                setBoneParent(&node->children[i]);
        }
        void Animator::updateAnimation(float dt) {
            ////////std::cout<<"update anim"<<std::endl;
            m_DeltaTime = dt;
            if (m_CurrentAnimation)
            {
                m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getDuration());
                calculateBoneTransform(&m_CurrentAnimation->getRootNode(), glm::mat4(1.f));
            }
        }
        void Animator::playAnimation(Animation* pAnimation)
        {
            m_CurrentAnimation = pAnimation;
            m_CurrentTime = 0.0f;
        }
        void Animator::calculateBoneTransform(const Animation::AssimpNodeData* node, glm::mat4 parentTransform)
        {
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;


            Bone* bone = m_CurrentAnimation->findBone(nodeName);
            //std::cout<<"bone : "<<bone<<std::endl;

            if (bone)
            {
                ////////std::cout<<"update"<<std::endl;
                bone->update(m_CurrentTime);
                nodeTransform = bone->getLocalTransform();
                //std::cout<<"node name : "<<nodeName<<std::endl<<"node transform : "<<glm::to_string(nodeTransform)<<std::endl;
                /*int pause;
                std::cin>>pause;*/

            }
            //std::cout<<"bone update done : "<<bone<<std::endl;

            //std::cout<<"node transform : "<<std::endl;
            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            //std::cout<<"get bone info map : "<<m_CurrentAnimation<<std::endl;
            auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
            //std::cout<<"bone info map"<<std::endl;
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                //std::cout<<"index : "<<index<<std::endl;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
                m_FinalBoneGlobalMatrices[index] = globalTransformation;
                //std::cout<<"node name : "<<nodeName<<std::endl<<"offset : "<<glm::to_string(offset)<<"globalTransform : "<<glm::to_string(globalTransformation)<<"final bone transform"<<glm::to_string(m_FinalBoneMatrices[index])<<std::endl;

            }
            //std::cout<<"bone info map done"<<std::endl;
            if (bone) {
                //std::cout<<"bone children : "<<bone->getChildren().size()<<std::endl;
                for (unsigned int i = 0; i < bone->getChildren().size(); i++) {
                    math::TransformMatrix tm =  m_CurrentAnimation->getModel()->getTransform();
                    tm.update();
                    tm.combine(AssimpHelpers::convertGLMToODFAEGMatrix(m_FinalBoneGlobalMatrices[bone->getBoneID()]));
                    bone->getChildren()[i]->setTransform(tm);
                }
                //std::cout<<"bone children transform ok : "<<std::endl;
            }
            for (int i = 0; i < node->childrenCount; i++)
                calculateBoneTransform(&node->children[i], globalTransformation);
        }
        std::vector<glm::mat4> Animator::getFinalBoneMatrices() {
            return m_FinalBoneMatrices;
        }
        GameObject* Animator::clone() {
            Animator* animator = new Animator(m_CurrentAnimation);
            GameObject::copy(animator);
            return animator;
        }
        Animator::~Animator() {

        }
    }
}
