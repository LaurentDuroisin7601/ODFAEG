namespace odfaeg {
    namespace graphic {
        RenderGraph::RenderGraph(RenderTarget& output) : output(output),
        csmShadowMap(GPUContext::instance().getDevice(), true), pointShadowMap(GPUContext::instance().getDevice(), true) {
            
        }
        void RenderGraph::addOITPass(unsigned int order, unsigned int layer, std::string typesToRender, unsigned int windowId) {
            LinkedListRenderer* llr = new LinkedListRenderer(output, layer, typesToRender, windowId);
            renderers.insert(std::make_pair(order, llr));            
        }
        void RenderGraph::addShadowPass(unsigned int order, unsigned int layer, std::string typesToRender, unsigned int windowId) {
            
            ShadowRenderer* sr = new ShadowRenderer(output, output, csmShadowMaplayer, plShadowMap, typesToRender, windowId);
            renderers.insert(std::make_pair(layer, sr));
        }
        void RenderGraph::addRTPass(unsigned int order, unsigned int layer, std::string typesToRender, unsigned int windowId) {
                        
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
        void RenderGraph::drawAllPasses() {
            std::map<unsigned int, IRenderer*>::iterator it;
            
            //inputShadowRT->clear();
            for (it = renderers.begin(); it != renderers.end(); it++) {
                //std::cout<<"clear"<<std::endl;
                
                //std::cout<<"cleared"<<std::endl;
                //std::cout<<"draw : "<<it->first<<std::endl;
                it->second->clear();
                //std::cout<<"draw"<<std::endl;
                it->second->draw();
                //std::cout<<"drawed"<<std::endl;
            }   
        }
    }
}