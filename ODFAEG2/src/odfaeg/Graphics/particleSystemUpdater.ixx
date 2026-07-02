module;
#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>
#include <mutex>
export module odfaeg.graphic.particleSystemUpdater;
import odfaeg.graphic.particleSystem;
import odfaeg.graphic.particle;
import odfaeg.core.timer;
import odfaeg.graphic.buffer;
import odfaeg.graphic.pipeline;
import odfaeg.graphic.fence;
import odfaeg.graphic.semaphore;
import odfaeg.core.timer;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.commandPool;
import odfaeg.graphic.shader;
import odfaeg.graphic.vertex;
import odfaeg.graphic.emittors;
import odfaeg.graphic.camera;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        export class ParticleSystemUpdater : public core::Timer {
        typedef std::array<Vertex, 4> Quad;
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
            void addParticleSystem(ParticleSystem* particleSystem);
            void setBuffersReady(bool r);
            bool areBuffersReady();
            void setSubmitReady(bool r);
            bool isSubmitReady();
            bool isReady();
        private :
            ParticleSystemUpdater(std::condition_variable& cv, std::mutex& mtx);
            void updateDescriptorSets();
            void updateBuffers();
            std::vector<ParticleSystem*> particlesSystems;
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
            std::vector<UniversalEmittor> emittors;
        };
    }
}