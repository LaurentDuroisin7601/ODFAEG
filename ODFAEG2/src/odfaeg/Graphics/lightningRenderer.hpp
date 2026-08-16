#ifndef ODFAEG_LIGHTNING_RENDERER_HPP
#define ODFAEG_LIGHTNING_RENDERER_HPP
namespace odfaeg {
    namespace graphic {
       class LightingRenderingComponent {
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
               void updateDescriptorSets(unsigned int mipLayerIndex=0);
               void clear();
               void drawNextFrame();
               void draw();
               unsigned int getLayer();
               bool isRendererReady();  
           private : 
            std::queue<Buffer> viewsUBO;
            std::queue<LightBuffer> lightbuffer;      
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
            bool needToUpdateDescriptorSets;
            CommandPool commandPool, pbrCommandPool;
            VertexBuffer ndcCubeVB;
            VertexBuffer fullscreenQuad;
            PbrVertPC pbrVertPC;
            std::vector<Light> lights;
       }; 
    }
}
#endif