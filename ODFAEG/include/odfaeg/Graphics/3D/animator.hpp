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
                    int layer;
                    int padding2;
                    math::Matrix4f transform;
                };
                Animator(window::Device& vkDevice, Animation* animation, EntityFactory& factory);
                void setBoneParent(const Animation::AssimpNodeData* node);
                void updateAnimation(float dt);
                void playAnimation(Animation* pAnimation);
                void calculateBoneTransform(const Animation::AssimpNodeData* node, math::Matrix4f parentTransform);
                std::vector<math::Matrix4f> getFinalBoneMatrices();
                void onDraw (RenderTarget &target, RenderStates states) {}
                Entity* clone();
                void attachEntityToBone(Entity* entity, std::string boneName);
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
                void computeParticles(std::mutex* mtx, std::condition_variable* cv2, VertexBuffer& frameVertexBuffer, unsigned int currentFrame, TransformMatrix tm, bool instanced, std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> computeSemaphore, std::array<VkFence, MAX_FRAMES_IN_FLIGHT> computeFence, unsigned int layer);
                bool isComputeFinished(unsigned int currentFrame, unsigned int layer);
                void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer cmd);
                uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
                ~Animator();
                private:
                void remapEntityId(Entity* entity);
                void compileComputeShader();
                void createCommandPool();
                void createCommandBuffers(unsigned int oldSize);
                void createDescriptorPool();
                void createDescriptorSetLayout();
                void allocateDescriptorSets();
                void updateDescriptorSets(unsigned int currentFrame);
                void createComputePipeline();
                std::vector<math::Matrix4f> m_FinalBoneMatrices;
                std::vector<math::Matrix4f> m_FinalBoneGlobalMatrices;
                Animation* m_CurrentAnimation;
                float m_CurrentTime;
                float m_DeltaTime;
                std::vector<unsigned int> currentFrame;
                std::vector<std::array<unsigned int, MAX_FRAMES_IN_FLIGHT>> maxFinalBonesMatricesSize;
                std::vector<std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>> computeSemaphores;
                window::Device& vkDevice;
                std::vector<std::optional<std::reference_wrapper<graphic::VertexBuffer>>> vertexBuffer;
                std::vector<std::array<VkFence, MAX_FRAMES_IN_FLIGHT>> computeFences;
                std::vector<std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT>> commandBuffers;
                VkCommandPool commandPool=VK_NULL_HANDLE;
                std::array<VkDescriptorPool, 2> computeDescriptorPool;
                std::array<VkDescriptorSetLayout, 2> computeDescriptorSetLayout;
                std::array<std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>, 2> computeDescriptorSets;
                graphic::Shader computeShader;
                ComputePC computeParams;
                VkPipelineLayout computePipelineLayout;
                VkPipeline computePipeline;
                std::vector<std::condition_variable*> cv2;
                std::vector<std::mutex*> mtx;
                std::deque<std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT>> computeFinished;
                std::deque<std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT>> computeJob;
                std::vector<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>> finalBonesMatrices;
                std::vector<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>> stagingFinalBonesMatrices;
                std::vector<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>> finalBonesMatricesMemory;
                std::vector<std::array<VkDeviceMemory, MAX_FRAMES_IN_FLIGHT>> stagingFinalBonesMatricesMemory;

            };
        }
    }
}
#endif // ODFAEG_ANIMATOR_HPP
