#ifndef ODFAEG_MORPHANIM_UPDATER_HPP
#define ODFAEG_MORPHANIM_UPDATER_HPP
#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <stdexcept>
#include <odfaeg/config.hpp>
#include <vulkan/vulkan_core.h>
#include <iostream>
#include "vk_mem_alloc.h"
#include "../Entity/morphAnim.hpp"
#include "buffer.hpp"
#include "vertexBuffer.hpp"
#include "shader.hpp"
#include "commandPool.hpp"
#include "../Core/timer.hpp"
#include "../Math/vec.hpp"
#include "descriptor.hpp"
#include "gpuContext.hpp"
#include "device.hpp"
#include "../Entity/gameObject.hpp"
#include "../Entity/primitiveType.hpp"
namespace odfaeg {
    namespace graphic {
        class MorphAnimUpdater : public core::Timer {
        public :
            enum BufferID {
                OBJECT_BUFFER, OBJECT_STAGGING_BUFFER, SUBMESHES_BUFFER, SUBMESHES_STAGGING_BUFFER, LOD_BUFFER, LOD_STAGGING_BUFFER, MODEL_DATA_BUFFER,
                STAGGING_MODEL_DATA_BUFFER, MATERIAL_DATA_BUFFER, STAGGING_MATERIAL_DATA_BUFFER, VERTEX_BUFFER
            };
            struct MorphAnimData {
            	int subMeshesOffset;
            	int nbSubMeshes;
            	int offsetInFramesBuffer;
            	int nbFrames;
            	int intPerc;
            	int intLevels;
            	int currentFrameIndex;
            	float passedTime;
            };
            struct FramesAnimData {
                int submeshOffset;
            };
            struct alignas(16) AABB {
                alignas(16) math::Vec3f center; float _pad0; // vec3 + padding
                alignas(16) math::Vec3f size;   float _pad1; // vec3 + padding
            };
            struct DeltaTimePC {
                float dt;
            };
            struct SubMeshData {
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
            static MorphAnimUpdater& instance(std::condition_variable& cv, std::mutex& mtx);
            void setReady(bool r);
            void addMorphAnim(entity::MorphAnim* moprhAnim);
            void setBuffersReady(bool r);
            bool areBuffersReady();
            void setSubmitReady(bool r);
            bool isSubmitReady();
            void onUpdate();
            std::mutex mtx, &mtx2, mtx3;
            std::condition_variable &cv2, cv3;
        private:
            MorphAnimUpdater(std::condition_variable& cv, std::mutex& mtx);
            void updateBuffers();
            void updateDescriptorSets();
            std::vector<entity::MorphAnim*> anims;
            Buffer staggingMorphAnims;
            Buffer staggingFramesAnims;
            Buffer staggingSubmeshesFramesAnims;
            std::deque<Buffer> morphAnims;
            std::deque<Buffer> framesAnims;
            std::deque<Buffer> submeshesFramesAnims;
            std::deque<Buffer> ubo;
            std::deque<VertexBuffer> verticesFramesAnims;
            Shader morphAnimShader, updateAnimIndexShader;
            CommandPool commandPool;
            DeltaTimePC deltaTimePC;
            std::atomic<bool> needToUpdateBuffers, needToUpdateDescriptorSets, ready, buffersReady, submitReady;
        };
    }
}
#endif