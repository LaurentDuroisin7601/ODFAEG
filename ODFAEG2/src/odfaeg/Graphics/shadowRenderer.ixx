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
            struct ShadowMappingVertPC {
				math::Matrix4f projMatrix;
				math::Matrix4f viewMatrix;
				int primitiveType;
				int currentFrame;   
                int _pad[2];             
			}; 
            struct ShadowMappingFragPC  {
               math::Matrix4f view; 
               unsigned int nbDirLights;
               unsigned int nbPointLights;
            }; 
            struct ShadowPassCSMVertPC {
                int primitiveType;
                int currentFrame;
            };  
            struct ShadowPassPLVertPC {
                math::Matrix4f lightProjMatrix;
                int primitiveType;
                int currentFrame;
                int _pad[2];
            };
            struct ShadowPassPLFragPC {  
                math::Vec3f lightPos;
                float pad;                
                float far_plane;
            };
            struct DirLight {               
                math::Vec3f dir;  
                float pad;
                float far_plane;              
            };
            struct PointLight {
                math::Vec3f pos;
                float pad;
                float far_plane;
            };
            struct LightSpaceMatrix {
                math::Matrix4f lightSpaceMatrices[NB_CASCADES+1];
            };
            struct ViewPLMatrix {
                math::Matrix4f viewsPLMatrices[6];
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
            void addDirectionnalLight(DirLight dirLight);
            void addPonctualLight(PointLight pointLight);
        private :                    
            void computeLightMatrices();
            std::vector<float> computeSplits(int cascadeCount, float nearPlane, float farPlane, float lambda);
            math::Matrix4f getLightSpaceMatrix(math::Vec3f lightDir, const float nearPlane, const float farPlane);
            std::array<math::Vec3f, 8> getFrustrumCornersWordlSpace(math::Matrix4f projView);
            std::vector<LightSpaceMatrix>  fLightSpaceMatrices;
            RenderTarget& parentRenderer;
            RenderTexture shadowMapPL;
            RenderTexture shadowMap;
            
            Shader shadowPassCSMShader, shadowPassPLShader, shadowMappingShader;            
            
            CommandPool shadowPassCommandPool, shadowPassPLCommandPool, shadowMappingCommandPool;            
            std::deque<Buffer> lightSpaceMatricesBuffer, lightSpaceMatricesBufferFinal, cascadePlaneDistancesBuffer,
            lightViewsPLMatricesBuffer, dirLightsBufferFinal, pointLightsBufferFinal;
            Buffer lightSpaceMatricesStaggingBuffer, lightViewsPLMatricesStaggingBuffer,
            dirLightsStaggingBufferFinal, pointLightsStaggingBufferFinal, lightSpaceMatricesStaggingBufferFinal;
            window::Listener eventListener;
            CascadeData cascadeData;
            std::atomic<bool> rendererReady;
            std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> commandBuffersReady={};
            std::array<std::atomic<bool>, MAX_FRAMES_IN_FLIGHT> registerFramesJob={};
            std::atomic<bool> stop= {};
            std::condition_variable cv;
            core::ThreadPool threadPool;
            std::array<core::JobFence, MAX_FRAMES_IN_FLIGHT> jobFences={};
            bool useThread, needToUpdateDescriptorSets, needToUpdateLightsMatrices;
            std::string typesToRenderExpression;
            std::mutex mtx;                       
            ShadowMappingVertPC shadowMappingVertPC;
            ShadowMappingFragPC shadowMappingFragPC;
            ShadowPassCSMVertPC shadowPassCSMVertPC;
            ShadowPassPLVertPC shadowPassPLVertPC;
            ShadowPassPLFragPC shadowPassPLFragPC;
            std::vector<ViewPLMatrix> lightViewsPLMatrices;
            CommandPool commandPool;
            std::vector<DirLight> dirLights;
            std::vector<PointLight> pointLights; 
            std::vector<float> shadowCascadeLevels;                           
        };
    }
}

