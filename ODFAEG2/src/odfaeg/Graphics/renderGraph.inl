namespace odfaeg {
    namespace graphic {
        RenderGraph::RenderGraph(RenderTexture& output, unsigned int layer) : layer(layer), output(output),
        csmShadowMap(GPUContext::instance().getDevice(), true), pointShadowMap(GPUContext::instance().getDevice(), true),
        environmentMap(GPUContext::instance().getDevice()),
        irradianceTexture(GPUContext::instance().getDevice()),
        prefilterTexture(GPUContext::instance().getDevice()),
        brdfLut(GPUContext::instance().getDevice()) {
            environmentMap.createCubeMap(1024);
            ImageLoader imageLoader;
            imageLoader.create(1024, 1024, entity::Color::White);
            //std::cout<<"size : "<<imageLoader.getSize().x()<<std::endl;
            for (unsigned int i = 0; i < 6; i++) {
                environmentMap.loadCubeMapFromImage(imageLoader, i);
            }
            environmentMap.generateMipmaps();
        }
        void RenderGraph::addOpaquePass(unsigned int order, std::string typesToRender, unsigned int windowId) {
            OpaqueRenderer* opaqueRenderer = new OpaqueRenderer(output, layer, typesToRender, windowId);
            renderers.insert(std::make_pair(order, opaqueRenderer));
        }
        void RenderGraph::addOITPass(unsigned int order, std::string typesToRender, unsigned int windowId) {
            LinkedListRenderer* llr = new LinkedListRenderer(output, output, layer, typesToRender, windowId);
            renderers.insert(std::make_pair(order, llr));            
        }
        void RenderGraph::addShadowPass(unsigned int order, std::string typesToRender, unsigned int windowId) {
            
            ShadowRenderer* sr = new ShadowRenderer(output, output, csmShadowMap, pointShadowMap, layer, typesToRender, windowId);
            renderers.insert(std::make_pair(order, sr));
        }
        void RenderGraph::addLightningPass(unsigned int order, std::string typesToRender, unsigned int windowId) {
             LightningRenderer* lightningRenderer = new LightningRenderer(output, environmentMap, irradianceTexture, prefilterTexture, brdfLut, layer, typesToRender, windowId);
             renderers.insert(std::make_pair(order, lightningRenderer));
        }
        void RenderGraph::addRTPass(unsigned int order, std::string typesToRender, unsigned int windowId) {
             RTRenderer* rtRenderer = new RTRenderer(output, environmentMap, output, csmShadowMap, pointShadowMap, layer, typesToRender, windowId);
             renderers.insert(std::make_pair(order, rtRenderer));           
        }
        void RenderGraph::addDirectionnalLight(entity::DirectionnalLight& dirLight) {
            //std::cout<<"add directionnal light"<<std::endl;
            std::map<unsigned int, IRenderer*>::iterator it;
            for (it = renderers.begin(); it != renderers.end(); it++) {
                it->second->addDirectionnalLight(dirLight);
            }
        }
        void RenderGraph::addPonctualLight(entity::PointLight& pointLight) {
            std::map<unsigned int, IRenderer*>::iterator it;
            for (it = renderers.begin(); it != renderers.end(); it++) {
                it->second->addPonctualLight(pointLight);
            }
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
        void RenderGraph::setEnvironmentMap(Texture& envMap) {
            environmentMap.copyFrom(envMap);
        }
        void RenderGraph::drawAllPasses() {
            std::map<unsigned int, IRenderer*>::iterator it;
            
            //inputShadowRT->clear();
            output.clear();
            for (unsigned int i = 0; i < renderers.size(); i++) {
                //std::cout<<"clear"<<std::endl;
                
                //std::cout<<"cleared"<<std::endl;
                //std::cout<<"draw : "<<it->first<<std::endl;
                output.beginRecordCommandBuffer();
                renderers[i]->clear();
                //std::cout<<"draw"<<std::endl;
                renderers[i]->draw();
                output.submit(true);
                //output.display();
                //std::cout<<"drawed"<<std::endl;
            }
        }
    }
}