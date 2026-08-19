#ifndef ODFAEG_LIGHTNING_RENDERER_HPP
#define ODFAEG_LIGHTNING_RENDERER_HPP
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
#include "renderTexture.hpp"
#include "../Math/matrix.hpp"
#include "../Math/vec.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "vertexBuffer.hpp"
#include "shader.hpp"
#include "commandPool.hpp"
#include "../Core/threadPool.hpp"
#include "../Window/listener.hpp"
#include "../Entity/cube.hpp"
#include "renderStates.hpp"
namespace odfaeg {
    namespace graphic {
       class LightningRenderer {
           public :
               struct PbrVertPC{
                   math::Matrix4f projMatrix;
                   math::Matrix4f viewMatrix;
                   int primitiveType;
                   int currentFrame;
               };
               struct Light {
                   alignas(16) math::Vec3f lightPos;
                   alignas(16) math::Vec3f lightColor; 
               };
               LightningRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId = -1, bool usethread=true);
               void createCommandPools();
               void createDescriptorsAndPipelines();
               void updateDescriptorSets();
               void clear();
               void drawNextFrame();
               void draw();
               unsigned int getLayer();
               bool isRendererReady(); 
               void setEnvironmentMap(Texture& environmentMap);
               void addLight(Light light); 
           private : 
               void updateBuffers();
               void updateTextures();
                std::deque<Buffer> viewsUBO;
                std::deque<Buffer> lightsBuffer;      
                Buffer lightStaggingBuffer;    
                std::string typesToRenderExpression;            
                Shader pbrShader, irradianceShader, prefilterShader, brdfShader, backgroundShader;
                Texture environmentMap;           
                RenderTexture irradianceTexture, prefilterTexture, brdfLUT;
                
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
                bool needToUpdateDescriptorSets, needToUpdateLightsBuffer, needToUpdateTextures;
                CommandPool commandPool, pbrCommandPool;
                VertexBuffer ndcCubeVB;
                VertexBuffer fullScreenQuad;
                PbrVertPC pbrVertPC;
                std::vector<Light> lights;
                window::Listener listener;
       }; 
    }
}
#endif