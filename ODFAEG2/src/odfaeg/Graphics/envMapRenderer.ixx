module;
export module odfaeg.Graphics.envMapRenderer;
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
                    math::Vec3f cameraPos;                    
                    int currentImageIndex;
                };
                EnvMapRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool usethread=true);
                void createCommandPools(); 
                void createDescriptorsAndPipelines();
                void updateDescriptorSets();
                void clear();
                void drawNextFrame();
                void draw();
                unsigned int getLayer();
                bool isRendererReady();   
            private:
                bool needToUpdateBuffers;
                VertexBuffer fullScreenQuad;
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
                std::vector<GameObject*> reflRefractGameObjects;
                std::array<math::Vec3f, 6> dirs, ups;
                // Private members and methods
        };
    }
}