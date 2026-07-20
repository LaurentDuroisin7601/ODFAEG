module;
#include <string>
#include <vulkan/vulkan.hpp>
#include <map>
#include <iostream>
module odfaeg.graphic.renderGraph;
namespace odfaeg {
    namespace graphic {
        RenderGraph::RenderGraph() {
            inputShadowRT = nullptr;
        }
        void RenderGraph::addLinkedListPass(RenderTarget& output, unsigned int layer, std::string typesToRender, unsigned int windowId) {
            LinkedListRenderer* llr = new LinkedListRenderer(output, layer, typesToRender, windowId);
            renderers.insert(std::make_pair(layer, llr));            
        }
        void RenderGraph::addShadowPass(RenderTarget& output, RenderTexture& input,  unsigned int layer, std::string typesToRender, unsigned int windowId) {
            llSMTransitionPoint = layer;
            ShadowRenderer* sr = new ShadowRenderer(output, input, layer, typesToRender, windowId);
            renderers.insert(std::make_pair(layer, sr));
            inputShadowRT = &input;
        }
        std::vector<IComponent*> RenderGraph::getComponents() {
            std::vector<IComponent*> components;
            std::map<unsigned int, IRenderer*>::iterator it;
            for (it = renderers.begin(); it != renderers.end(); it++) {
                components.push_back(it->second);
            }
            std::map<unsigned int, Widget*>::iterator it2;
            for (it2 = widgets.begin(); it2 != widgets.end(); it2++) {
                components.push_back(it2->second);
            }
            return components;
        }
        void RenderGraph::render() {
            std::map<unsigned int, IRenderer*>::iterator it;
            for (it = renderers.begin(); it != renderers.end(); it++) {
                //std::cout<<"clear"<<std::endl;
                
                //std::cout<<"cleared"<<std::endl;
                if (inputShadowRT != nullptr && it->first == llSMTransitionPoint) {
                    Texture::transitionImageLayout(inputShadowRT->getTexture().getImage(inputShadowRT->getImageIndex()), inputShadowRT->getCommandPool().getHandle(inputShadowRT->getCurrentFrame()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
                    inputShadowRT->submit(true);
                }
                it->second->clear();
                //std::cout<<"draw"<<std::endl;
                it->second->draw();
                //std::cout<<"drawed"<<std::endl;
            }
            if (inputShadowRT != nullptr) {
                inputShadowRT->beginRecordCommandBuffer();
                Texture::transitionImageLayout(inputShadowRT->getTexture().getImage(inputShadowRT->getImageIndex()), inputShadowRT->getCommandPool().getHandle(inputShadowRT->getCurrentFrame()), VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);   
                //inputShadowRT->submit(true);
            }         
        }
    }
}