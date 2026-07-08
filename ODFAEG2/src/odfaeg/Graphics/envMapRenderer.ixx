module;
export module odfaeg.Graphics.envMapRenderer;
namespace odfaeg {
    namespace graphic {
        class EnvMapRenderer {
            public:
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
                VertexBuffer fullScreenQuad;
                RenderTexture envMap;
                Shader envMapShader, reflRefrShader;
                CommandPool envMapCmdPool, reflRefrCmdPool, commandPool;
                core::ThreadPool threadPool;
                std::array<core::JobFence, MAX_FRAMES_IN_FLIGHT> jobFence={};
                std::string typesToRenderExpression;
                std::deque<Buffer> nodeCounterBuffer;
                std::deque<Buffer> linkedListBuffer;
                std::deque<Image> headPtrsStorageImage;
                int layer;
                inline static const unsigned int ENV_MAP_SIZE = 1024;
                std::vector<SubMesh> reflRefractSubmeshes;
                // Private members and methods
        };
    }
}