#ifndef ODFAEG_RENDERGRAPH_HPP
#define ODFAEG_RENDERGRAPH_HPP
#include <map>
#include <string>
#include <vulkan/vulkan.hpp>
#include <map>
#include <iostream>
#include "iComponent.hpp"
#include "iRenderer.hpp"
#include "shadowRenderer.hpp"
#include "linkedListRenderer.hpp"
#include "widget.hpp"
#include "../math/vec.hpp"
#include "renderTarget.hpp"
#include "renderTexture.hpp"
#include "texture.hpp"
namespace odfaeg {
    namespace graphic {
        export class RenderGraph {
            public :                     
            RenderGraph(RenderTexture& output);
            void addOITPass(unsigned int order, unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
            void addShadowPass(unsigned int order,  unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
            void addLightningPass(unsigned int order, unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
            void addRTPass(unsigned int order, unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
            template<typename R>
            void addDirectionnalLight(unsigned int layer, ShadowRenderer::DirLight dirLight) {
                std::map<unsigned int, IRenderer*>::iterator it = renderers.find(layer);
                if (it != renderers.end()) {
                    R* shadowRenderer = static_cast<R*>(it->second);
                    shadowRenderer->addDirectionnalLight(dirLight);
                }
            }
            template<typename R>
            void addPonctualLight(unsigned int layer, ShadowRenderer::PointLight pointLight) {
                std::map<unsigned int, IRenderer*>::iterator it = renderers.find(layer);
                if (it != renderers.end()) {
                    R* shadowRenderer = static_cast<R*>(it->second);
                    shadowRenderer->addPonctualLight(pointLight);
                }
            }
            void drawAllPasses();
            std::vector<IComponent*> getComponents();            
            private :            
            std::map<unsigned int, IRenderer*> renderers;
            std::map<unsigned int, Widget*> widgets;
            RendereTexture& outputTexture;
            RenderTexture csmShadowMap, plShadowMap;
            Texture enviornmentMap;
        };
    }
}
#endif
