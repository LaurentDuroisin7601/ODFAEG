module;
#include <string>
#include <vector>
#include <deque>
#include <odfaeg/config.hpp>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <memory>
export module odfaeg.graphic.shadowRenderer;
import odfaeg.graphic.renderTarget;
import odfaeg.math.matrix;
import odfaeg.math.vec;
import odfaeg.graphic.renderTexture;
import odfaeg.graphic.shader;
import odfaeg.graphic.pipeline;
import odfaeg.graphic.descriptor;
import odfaeg.graphic.commandPool;
import odfaeg.graphic.buffer;
import odfaeg.core.threadPool;
import odfaeg.window.listener;
namespace odfaeg {
    namespace graphic {
        export class ShadowRenderer {
        public :
            struct CascadeData {
                math::Vec4f cascadePlaneDistances[NB_CASCADES+1];
                int cascadeCount;   // number of
                int _pads[3];
            };
            struct ViewProjMatPC {
				math::Matrix4f projMatrix;
				math::Matrix4f viewMatrix;
				int primitiveType;
				int currentFrame;   
                int _pad[2];             
			};  
            struct PushConstantData {
                int primitiveType;
                int currentFrame;
            };         
            struct PushConstantData2 {
                math::Matrix4f view; 
                math::Vec2f _pad;
                alignas(16) math::Vec3f lightDir;                
                float farPlane;
            };
            struct ShadowPassPLVertPC {
                math::Matrix4f lightProjMatrix;
                int primitiveType;
                int currentFrame;
                int _pad[2];
            };
            struct ShadowPassPLFragPC {
                alignas(16) math::Vec3f lightPos;
                float far_plane;
            };
            ShadowRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, bool usethread=true);
            void createCommandPools();
            void createDescriptorsAndPipelines();
            void updateDescriptorSets();
            void clear();
            void drawNextFrame();
            void draw();
            unsigned int getLayer();
            bool isRendererReady();
        private :
            std::vector<float> computeSplits(int cascadeCount, float nearPlane, float farPlane, float lambda);
            math::Matrix4f getLightSpaceMatrix(const float nearPlane, const float farPlane);
            std::array<math::Vec3f, 8> getFrustrumCornersWordlSpace(math::Matrix4f projView);
            std::vector<math::Matrix4f>  fLightSpaceMatrices;
            RenderTarget& parentRenderer;
            RenderTexture shadowMapPL;
            RenderTexture shadowMap;
            
            Shader shadowPassCSMShader, shadowMappingCSMShader, shadowPassPLShader, shadowMappingPLShader;
            std::deque<std::deque<Pipeline>> &shadowPassCSMPipeline, &shadowMappingCSMPipeline, &shadowPassPLPipeline, &shadowMappingPLPipeline;
            std::deque<std::deque<DescriptorSet>> &shadowPassCSMSets, &shadowMappingCSMSets, &shadowPassPLSets, &shadowMappingPLSets;
            
            CommandPool shadowPassCommandPool, shadowMappingCommandPool;            
            std::deque<Buffer> lightSpaceMatricesBuffer, cascadePlaneDistancesBuffer,
            lightViewsPLMatricesBuffer;
            window::Listener eventListener;
            CascadeData cascadeData;
            std::atomic<bool> rendererReady;
            std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> commandBuffersReady={};
            std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> registerFramesJob={};
            std::atomic<bool> stop= {};
            std::condition_variable cv;
            core::ThreadPool threadPool;
            std::array<core::JobFence, MAX_FRAMES_IN_FLIGHT> jobFences={};
            bool useThread, needToUpdateDescriptorSets;
            std::string typesToRenderExpression;
            std::mutex mtx;
            PushConstantData pc;
            PushConstantData2 pc2;            
            ViewProjMatPC viewProjMatPC;
            ShadowPassPLVertPC shadowPassPLVertPC;
            ShadowPassPLFragPC shadowPassPLFragPC;
            std::array<math::Matrix4f, 6> lightViewsPLMatrices;
        };
    }
}

