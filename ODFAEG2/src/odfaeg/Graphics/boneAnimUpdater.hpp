//
// Created by laurent on 09/06/2026
module;
#include <vector>
#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept>
#include "odfaeg/config.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <condition_variable>
#include "vk_mem_alloc.h"
#include "gpuContext.hpp"
#include "descriptor.hpp"
#include "../Math/matrix.hpp"
#include "device.hpp"
#include "../Entity/vertex.hpp"
#include "../Entity/primitiveType.hpp"
namespace odfaeg {
    namespace graphic {
        class BoneAnimUpdater : public core::Timer {
        public :
            struct alignas(16) AABB {
                alignas(16) math::Vec3f center; //float _pad0; // vec3 + padding
                alignas(16) math::Vec3f size;   //float _pad1; // vec3 + padding
            };
            enum BufferID {
                OBJECT_BUFFER, OBJECT_STAGGING_BUFFER, SUBMESHES_BUFFER, SUBMESHES_STAGGING_BUFFER, LOD_BUFFER, LOD_STAGGING_BUFFER, MODEL_DATA_BUFFER,
                STAGGING_MODEL_DATA_BUFFER, MATERIAL_DATA_BUFFER, STAGGING_MATERIAL_DATA_BUFFER, VERTEX_BUFFER
            };
            struct BoneAnimData {
                int id;
                int animsSubmeshOffset;
                int subMeshesOffset;
                int nbSubmeshes;
                int subMeshOffset;
            };
            struct SubMesh {
               AABB globalBounds;
               int vertexOffset;
               int indexOffset;
               int primitiveType;
               int materialId;
               int nbVertices;
               int nbIndexes;
               int lodOffset;
               int id;
            };
            static BoneAnimUpdater& instance(std::condition_variable& cv, std::mutex& mtx);
            void setReady(bool r);
            void addBoneAnim(entity::Animator* boneAnim);
            void setBuffersReady(bool r);
            bool areBuffersReady();
            void setSubmitReady(bool r);
            bool isSubmitReady();
            void onUpdate() override;
            std::mutex mtx, &mtx2, mtx3;
            std::condition_variable &cv2, cv3;
        private:
            BoneAnimUpdater(std::condition_variable& cv, std::mutex& mtx);
            void updateBuffers();
            void updateDescriptorSets();
            std::vector<entity::Animator*> anims;
            Buffer staggingBoneAnims;
            Buffer staggingFinalBonesMatrices;
            Buffer staggingAnimsSubmeshes;
            std::deque<Buffer> boneAnims;
            std::deque<Buffer> finalBonesMatrices;
            std::deque<Buffer> animsSubmeshes;
            std::deque<VertexBuffer> verticesIn;
            Shader boneAnimShader;
            CommandPool commandPool;
            std::atomic<bool> needToUpdateBuffers, needToUpdateDescriptorSets, ready, buffersReady, submitReady;
        };
    } // graphic
} // odfaeg
#include "boneAnimUpdater.inl"