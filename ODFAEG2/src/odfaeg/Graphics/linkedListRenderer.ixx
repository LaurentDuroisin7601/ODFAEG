module;
#include <deque>
#include <string>
#include <odfaeg/config.hpp>
#include <condition_variable>
#include <memory>
export module odfaeg.graphic.linkedListRenderer;
import odfaeg.graphic.renderTarget;
import odfaeg.entity.primitiveType;
import odfaeg.graphic.renderStates;
import odfaeg.graphic.descriptor;
import odfaeg.graphic.pipeline;
import odfaeg.math.matrix;
import odfaeg.math.vec;
import odfaeg.graphic.buffer;
import odfaeg.graphic.image;
import odfaeg.graphic.vertexBuffer;
import odfaeg.graphic.shader;
import odfaeg.graphic.commandPool;
import odfaeg.core.threadPool;
import odfaeg.window.listener;
namespace odfaeg {
    namespace graphic {
        export class LinkedListRenderer {
        public :
            struct LinkedListPC {
                unsigned int maxNodes;
                int currentImageIndex;
            };
            LinkedListRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool usethread=true);
            void createCommandPools();
            void createDescriptorsAndPipelines();
            void updateDescriptorSets();
            void clear();
            void drawNextFrame();
            void draw();
            unsigned int getLayer();
            bool isRendererReady();            
        private :
            VertexBuffer fullScreenQuad;
            std::string typesToRenderExpression;
            std::deque<Buffer> nodeCounterBuffer;
            std::deque<Buffer> linkedListBuffer;
            std::deque<Image> headPtrsStorageImage;
            Shader linkedListShader, quadLinkedListShader;
            CommandPool linkedListCmdPool, quadLinkedListCommandPool;
            core::ThreadPool threadPool;
            std::array<core::JobFence, MAX_FRAMES_IN_FLIGHT> jobFence={};
            int layer;
            unsigned int maxNodes;
            RenderTarget& parentRenderer;
            std::mutex mtx;
            std::condition_variable cv;
            std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> commandBuffersReady={};
            std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> registerFramesJob={};
            std::atomic<bool> stop= {};
            std::atomic<bool> rendererReady;
            window::Listener eventListener;
            LinkedListPC linkedListPC;
            bool needToUpdateDescriptorSets;
            CommandPool commandPool;
        };
    }
}