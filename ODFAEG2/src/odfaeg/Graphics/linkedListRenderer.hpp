#ifndef ODFAEG_LINKEDLIST_RENDERER_HPP
#define ODFAEG_LINKEDLIST_RENDERER_HPP
#include <deque>
#include <string>
#include <odfaeg/config.hpp>
#include <condition_variable>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include "odfaeg/config.hpp"
#include "iRenderer.hpp"
#include "renderTarget.hpp"
#include "../Entity/primitiveType.hpp"
#include "../Entity/vertex.hpp"
#include "renderStates.hpp"
#include "descriptor.hpp"
#include "pipeline.hpp"
#include "../Math/matrix.hpp"
#include "../Math/vec.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "vertexBuffer.hpp"
#include "shader.hpp"
#include "commandPool.hpp"
#include "../Core/threadPool.hpp"
#include "../Window/listener.hpp"
namespace odfaeg {
    namespace graphic {
        class LinkedListRenderer : public IRenderer {
        public :           
            struct LinkedListPC {
                unsigned int maxNodes;
                int currentImageIndex;
            };
            struct ViewProjMatPC {
				math::Matrix4f projMatrix;
				math::Matrix4f viewMatrix;
				int primitiveType;
				int currentFrame;
			};
            LinkedListRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId = -1, bool usethread=true);
            void createCommandPools();
            void createDescriptorsAndPipelines();
            void updateDescriptorSets();            
            void clear();
            void drawNextFrame();
            void draw();
            unsigned int getLayer();
            bool isRendererReady();  
            void onSwapchainResized(math::Vector2i newSize);          
        private :
            VertexBuffer fullScreenQuad;
            std::string typesToRenderExpression;
            std::deque<Buffer> nodeCounterBuffer;
            std::deque<Buffer>& linkedListBuffer;
            std::deque<Image>& headPtrsStorageImage;
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
            LinkedListPC linkedListPC;
            ViewProjMatPC viewProjMatPC;
            bool needToUpdateDescriptorSets;
            CommandPool commandPool;
        };
    }
}
#endif