module;
#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>
export module odfaeg.graphic.boneAnimUpdater;
import odfaeg.graphic.animator;
import odfaeg.graphic.buffer;
import odfaeg.core.timer;
import odfaeg.math.vec;
import odfaeg.graphic.commandPool;
import odfaeg.graphic.shader;
import odfaeg.graphic.vertexBuffer;
// Created by laurent on 09/06/2026.
//
namespace odfaeg {
    namespace graphic {
        export class BoneAnimUpdater : public core::Timer {
        public :
            alignas(16) struct AABB {
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
            void addBoneAnim(Animator* boneAnim);
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
            std::vector<Animator*> anims;
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


