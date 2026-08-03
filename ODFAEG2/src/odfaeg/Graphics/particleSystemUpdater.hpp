#ifndef ODFAEG_PARTICLESYSTEMUPDATER_HPP
#define ODFAEG_PARTICLESYSTEMUPDATER_HPP
#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <entt.hpp>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <mutex>
#include <condition_variable>
#include <future>
#include <iostream>
#include <odfaeg/config.hpp>
#include "vk_mem_alloc.h"
#include "../Entity/particleSystem.hpp"
#include "../Entity/particle.hpp"
#include "../core/timer.hpp"
#include "buffer.hpp"
#include "pipeline.hpp"
#include "fence.hpp"
#include "semaphore.hpp"
#include "../Core/timer.hpp"
#include "vertexBuffer.hpp"
#include "commandPool.hpp"
#include "shader.hpp"
#include "../Entity/vertex.hpp"
#include "../Entity/emittors.hpp"
#include "camera.hpp"
#include "../Math/vec.hpp"
#include "gpuContext.hpp"
#include "descriptor.hpp"
#include "../Entity/primitiveType.hpp"
#include "../Entity/emittors.hpp"
#include "device.hpp"
#include "../Core/clock.hpp"
namespace odfaeg {
    namespace graphic {
        class ParticleSystemUpdater : public core::Timer {
        typedef std::array<entity::Vertex, 4> Quad;
        struct alignas(16) AABB {
            alignas(16) math::Vec3f center; float _pad0; // vec3 + padding
            alignas(16) math::Vec3f size;   float _pad1; // vec3 + padding
        };
        struct ParticlesSystemData {
            unsigned int id;
            unsigned int offsetInAliveCountBuffer;
            unsigned int offsetInSubMeshBuffer;
            unsigned int offsetInParticleBuffer;
            unsigned int offsetInQuadBuffer;
            unsigned int subMeshCount;
        };
        enum BufferID {
            OBJECT_BUFFER, OBJECT_STAGGING_BUFFER, SUBMESHES_BUFFER, SUBMESHES_STAGGING_BUFFER, LOD_BUFFER, LOD_STAGGING_BUFFER, MODEL_DATA_BUFFER,
            STAGGING_MODEL_DATA_BUFFER, MATERIAL_DATA_BUFFER, STAGGING_MATERIAL_DATA_BUFFER, VERTEX_BUFFER
        };
        struct DeltaTimePC {
            float dt;
        };
        public :
            std::mutex mtx, &mtx2, mtx3;
            std::condition_variable &cv2, cv3;
            static ParticleSystemUpdater& instance(std::condition_variable& cv, std::mutex& mtx);
            void onUpdate();
            void setCamera(Camera camera);
            void setReady(bool r);
            void addParticleSystem(entity::ParticleSystem* particleSystem);
            void setBuffersReady(bool r);
            bool areBuffersReady();
            void setSubmitReady(bool r);
            bool isSubmitReady();
            bool isReady();
        private :
            ParticleSystemUpdater(std::condition_variable& cv, std::mutex& mtx);
            void updateDescriptorSets();
            void updateBuffers();
            std::vector<entity::ParticleSystem*> particlesSystems;
            std::deque<Buffer> particlesSystemsBuffer;
            Buffer particlesSystemsStaggingBuffer;
            Buffer aliveCountStaggingBuffer;
            Buffer particlesStaggingBuffer;
            std::deque<Buffer> particlesBuffer;
            std::deque<Buffer> particlesQuadsBuffer;
            Buffer particlesQuadsStaggingBuffer;
            std::deque<Buffer> ubo;
            std::deque<Buffer> aliveCountBuffer;
            std::deque<Buffer> particlesEmittorsBuffer;
            Buffer particlesEmittorsStaggingBuffer;
            CommandPool commandPool;
            Shader particlesEmittorShader, particlesUpdaterShader, particlesVerticesShader;
            bool needToUpdateBuffers, needToUpdateDescriptorSets;
            DeltaTimePC deltaTimePC;
            inline static std::atomic<bool> ready, buffersReady, submitReady;
            unsigned int maxParticles;
            Camera camera;
            AABB cullingInfo;
            std::vector<entity::UniversalEmittor> emittors;
        };
    }
}
#include "particleSystemUpdater.inl"
#endif