#ifndef ODFAEG_RENDERGRAPH_HPP
#define ODFAEG_RENDERGRAPH_HPP
#include <map>
#include <string>
#include <vulkan/vulkan.hpp>
#include <map>
#include <iostream>
#include "iComponent.hpp"
#include "iRenderer.hpp"
#include "opaqueRenderer.hpp"
#include "shadowRenderer.hpp"
#include "linkedListRenderer.hpp"
#include "lightningRenderer.hpp"
#include "rtRenderer.hpp"
#include "widget.hpp"
#include "../math/vec.hpp"
#include "renderTarget.hpp"
#include "renderTexture.hpp"
#include "texture.hpp"
namespace odfaeg {
    namespace graphic {
        class RenderGraph {
            public :                     
            RenderGraph(RenderTexture& output, unsigned int layer);
            void addOpaquePass(unsigned int order, std::string typesToRender, unsigned int windowId);        
            void addOITPass(unsigned int order, std::string typesToRender, unsigned int windowId=-1);
            void addShadowPass(unsigned int order, std::string typesToRender, unsigned int windowId=-1);
            void addLightningPass(unsigned int order, std::string typesToRender, unsigned int windowId=-1);
            void addRTPass(unsigned int order, std::string typesToRender, unsigned int windowId=-1);
            
            void addDirectionnalLight(entity::DirectionnalLight& dirLight);
            void addPonctualLight(entity::PointLight& pointLight);
                
            void drawAllPasses();
            std::vector<IComponent*> getComponents();
            void setEnvironmentMap(Texture& environmentMap);            
            private : 
            unsigned int layer;           
            std::map<unsigned int, IRenderer*> renderers;
            std::map<unsigned int, IRenderer*> rayRenderers; 
            std::map<unsigned int, Widget*> widgets;
            RenderTexture& output;
            RenderTexture csmShadowMap, pointShadowMap, irradianceTexture, prefilterTexture, brdfLut;
            Texture environmentMap;
        };
    }
}
#endif
