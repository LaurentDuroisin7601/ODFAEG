#ifndef ODFAEG_ANIMATOR_HPP
#define ODFAEG_ANIMATOR_HPP
#include "animation.hpp"
namespace odfaeg {
    namespace graphic {
        namespace g3d {
            class ODFAEG_GRAPHICS_API Animator : public GameObject {
            public :
                struct ComputePC {
                    int entityId;
                    int instanced;
                    int padding1;
                    int padding2;
                    math::Matrix4f transform;
                };
                Animator(window::Device& vkDevice, Animation* animation, EntityFactory& factory);
                TransformMatrix& getChildTransform();
                void updateAnimation(float dt);
                void playAnimation(Animation* pAnimation);
                void calculateBoneTransform(const Animation::AssimpNodeData* node, math::Matrix4f parentTransform);
                std::vector<math::Matrix4f> getFinalBoneMatrices();
                void onDraw (RenderTarget &target, RenderStates states) {}
                Entity* clone();
                bool isAnimated() const {
                    return true;
                }
                /**
                * \fn bool isModel() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not an model.
                */
                bool isModel() const {
                    return true;
                }
                /**
                * \fn bool selectable() const
                * \brief redefinition of the method from the base class Entity.
                * \return true, a tile is selectable.
                */
                bool selectable() const {
                    return true;
                }
                /**
                * \fn bool isLight() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not a light.
                */
                bool isLight() const {
                    return false;
                }
                /**
                * \fn bool isShadow() const
                * \brief redefinition of the method from the base class Entity.
                * \return false, a tile is not a shadow.
                */
                bool isShadow() const {
                    return false;
                }
                 /**
                * \fn bool isLeaf() const
                * \brief redefinition of the method from the base class Entity.
                * \return true, a tile can't have children.
                */
                bool isLeaf() const {
                    return false;
                }
                Entity* getCurrentFrame() const;
                void computeParticles(std::mutex* mtx, std::condition_variable* cv2, VertexBuffer& frameVertexBuffer, unsigned int currentFrame, TransformMatrix tm, bool instanced, VkSemaphore computeSemaphore, VkFence computeFence);
                bool isComputeFinished(unsigned int currentFrame);
                void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
                uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
                ~Animator();
                private:
                void compileComputeShader();
                void createCommandPool();
                void createCommandBuffers();
                void createDescriptorPool();
                void createDescriptorSetLayout();
                void allocateDescriptorSets();
                void updateDescriptorSets();
                void createComputePipeline();
                std::vector<math::Matrix4f> m_FinalBoneMatrices;
                Animation* m_CurrentAnimation;
                float m_CurrentTime;
                float m_DeltaTime;
                unsigned int currentFrame=0, maxFinalBonesMatricesSize=0;
                VkSemaphore computeSemaphores;
                window::Device& vkDevice;
                std::optional<std::reference_wrapper<graphic::VertexBuffer>> vertexBuffer;
                VkFence computeFences;
                std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;
                VkCommandPool commandPool=VK_NULL_HANDLE;
                VkDescriptorPool computeDescriptorPool;
                VkDescriptorSetLayout computeDescriptorSetLayout;
                std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> computeDescriptorSets;
                graphic::Shader computeShader;
                ComputePC computeParams;
                VkPipelineLayout computePipelineLayout;
                VkPipeline computePipeline;
                std::condition_variable *cv2;
                std::mutex* mtx=nullptr;
                std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> computeFinished={true, true};
                std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> computeJob={};
                std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> finalBonesMatrices={};
                std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> stagingFinalBonesMatrices={};
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> finalBonesMatricesMemory={};
                std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT> stagingFinalBonesMatricesMemory={};
            };
        }
    }
}
#endif // ODFAEG_ANIMATOR_HPP
