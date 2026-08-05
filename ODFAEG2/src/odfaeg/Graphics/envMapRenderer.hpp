#ifndef ODFAEG_ENVMAPRENDERER_HPP
#define ODFAEG_ENVMAPRENDERER_HPP
#include <mutex>
#include <condition_variable>
#include <string>
#include <odfaeg/config.hpp>
#include <deque>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include "../Core/delegate.hpp"
#include "../Core/threadPool.hpp"
#include "../Math/maths.hpp"
#include "../Math/matrix.hpp"
#include "../Window/command.hpp"
#include "gpuContext.hpp"
#include "renderTarget.hpp"
#include "../Physics/boundingBox.hpp"
#include "renderTexture.hpp"
#include "../Entity/gameObject.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "../Entity/vertex.hpp"
#include "blendMode.hpp"
#include "../Entity/primitiveType.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "renderStates.hpp"
#include "iRenderer.hpp"
namespace odfaeg {
    namespace graphic {
        class EnvMapRenderer {
            public:
                struct EnvViewMatrix {
                    math::Matrix4f viewMatrices[6];
                };
                struct EnvMapFragPC {
                    unsigned int maxNodes;
                    int currentImageIndex;
                };   
                struct EnvMapVertPC {
                    math::Matrix4f projMatrix;
                    unsigned int primitiveType;
                    unsigned int currentFrameIndex;                    
                };
                struct ReflRefrVertPC {
                    math::Matrix4f projMatrix;
                    math::Matrix4f viewMatrix;
                    math::Matrix4f modelMatrix;
                    math::Vec2f uvScale;
                    math::Vec2f uvOffset;                   
                    int materialIndex;
                    int currentFrame;
                    int currentImageIndex;
                    int primitiveType;
                };
                struct ReflRefrFragPC {
                    math::Vec4f cameraPos;                    
                    int currentImageIndex;
                };
                EnvMapRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool usethread=true);
                void createCommandPools(); 
                void createDescriptorsAndPipelines();
                void updateDescriptorSets();
                void updateBuffers();
                void clear();
                void drawNextFrame();
                void draw();
                void addReflRefrGameObject(Mesh* gameObject);                
                unsigned int getLayer();
                bool isRendererReady();   
            private:                
                unsigned int maxNodes;
                bool needToUpdateBuffers, needToUpdateDescriptorSets, useThread;
                std::atomic<bool> rendererReady;
                std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> commandBuffersReady={};
                std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> registerFramesJob={};
                std::atomic<bool> stop = {};
                VertexBuffer fullScreenQuad;
                RenderTarget& parentRenderer;
                RenderTexture envMap;
                Shader envMapShader, reflRefrShader, envMapQuadShader;
                CommandPool commandPool;
                std::vector<CommandPool> envMapCmdPools, envMapQuadCmdPools, reflRefrCmdPools;
                core::ThreadPool threadPool;
                std::array<core::JobFence, MAX_FRAMES_IN_FLIGHT> jobFence={};
                std::string typesToRenderExpression;
                std::deque<Buffer> nodeCounterBuffer;
                std::deque<Buffer> linkedListBuffer;
                std::deque<Buffer> viewMatricesBuffer;
                std::deque<Image> headPtrsStorageImage; 
                Buffer staggingViewMatricesBuffer;               
                int layer;
                inline static const unsigned int ENV_MAP_SIZE = 1024;
                std::vector<Mesh*> reflRefrGameObjects;
                std::array<math::Vec3f, 6> dirs, ups;
                std::condition_variable cv;
                std::mutex mtx;
                window::Listener eventListener;
                EnvMapVertPC envMapVertPC;
                EnvMapFragPC envMapFragPC;
                ReflRefrVertPC reflRefrVertPC;
                ReflRefrFragPC reflRefrFragPC;
                // Private members and methods
        };
    }
}
#endif