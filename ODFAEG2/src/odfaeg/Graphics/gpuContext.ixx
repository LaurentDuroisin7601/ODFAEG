module;
#include <deque>
#include <memory>
export module odfaeg.graphic.gpuContext;
import odfaeg.graphic.pipeline;
import odfaeg.graphic.descriptor;
import odfaeg.graphic.image;
import odfaeg.graphic.buffer;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.texture;
import odfaeg.graphic.instance;
import odfaeg.graphic.device;
import odfaeg.entity.primitiveType;
import odfaeg.graphic.shader;
import odfaeg.graphic.blendMode;
import odfaeg.graphic.fence;
import odfaeg.graphic.semaphore;
namespace odfaeg {
    namespace graphic {
        export class GPUContext {
        public :
            static GPUContext& instance();
            std::deque<std::deque<Pipeline>>& getGraphicsPipeline(Shader& shader);
            Pipeline& getGraphicsPipeline(entity::PrimitiveType primType, Shader& shader, BlendMode blendMode, unsigned int depthStencilInfoId);
            Pipeline& getComputePipeline(Shader& shader);
            DescriptorPool& getDescriptorPool(Shader& shader, unsigned int nbShaderBindings, unsigned int setBinding=0);
            DescriptorSetLayout& getDescriptorSetLayout(Shader& shader, unsigned int nbShaderBindings, bool bindless=false, unsigned int setBinding=0);
            std::deque<DescriptorSetLayout>& getDescriptorSetLayout(Shader& shader);
            std::deque<DescriptorSet>& getDescriptorSets(Shader& shader, unsigned int nbShaderBindings, unsigned int nbDescriptorSetsPerFrame, unsigned int setBinding=0);
            std::deque<std::deque<DescriptorSet>>& getDescriptorSets(Shader& shader);
            std::deque<Buffer>& getSharedBuffers(unsigned int bufferID);
            std::deque<Texture>& getSharedTextures(unsigned int textureID);
            std::deque<Image>& getSharedImage(unsigned int imageID);
            std::deque<VertexBuffer>& getSharedVertexBuffer(unsigned int vertexBufferID);
            std::deque<Fence>& getSharedFence(unsigned int fenceID);
            std::deque<Semaphore>& getSharedSemaphore(unsigned int semaphoreID);
            Device& getDevice();
            Instance& getInstance();
        private:
            GPUContext();
            Instance inst;
            Device device;
            std::deque<std::deque<Buffer>> sharedBuffers;
            std::deque<std::deque<Texture>> sharedTextures;
            std::deque<std::deque<Image>> sharedImages;
            std::deque<std::deque<VertexBuffer>> sharedVertexBuffers;
            std::deque<std::deque<DescriptorPool>> descriptorPools;
            std::deque<std::deque<DescriptorSetLayout>> descriptorSetLayouts;
            std::deque<std::deque<std::deque<DescriptorSet>>> descriptorSets;
            std::deque<std::deque<std::deque<Pipeline>>> graphicsPipeline;
            std::deque<Pipeline> computePipelines;
            std::deque<std::deque<Fence>> sharedFences;
            std::deque<std::deque<Semaphore>> sharedSemaphores;
        };
    }
}
