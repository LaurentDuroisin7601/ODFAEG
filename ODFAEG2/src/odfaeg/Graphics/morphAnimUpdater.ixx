module;
#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>
export module odfaeg.graphic.morphAnimUpdater;
import odfaeg.graphic.morphAnim;
import odfaeg.graphic.buffer;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.shader;
import odfaeg.graphic.commandPool;
import odfaeg.core.timer;
import odfaeg.math.vec;
namespace odfaeg {
    namespace graphic {
        export class MorphAnimUpdater : public core::Timer {
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
            alignas(16) struct AABB {
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
            static MorphAnimUpdater& instance();
            void setReady(bool r);
            void addMorphAnim(MorphAnim* moprhAnim);
            void areBuffersReady(bool r);
            void onUpdate();
            std::mutex mtx, mtx2;
            std::condition_variable cv2;
        private:
            MorphAnimUpdater();
            void updateBuffers();
            void updateDescriptorSets();
            std::vector<MorphAnim*> anims;
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
            std::atomic<bool> needToUpdateBuffers, needToUpdateDescriptorSets, ready, buffersReady;
        };
    }
}