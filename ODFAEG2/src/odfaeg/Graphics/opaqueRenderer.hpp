#ifndef ODFAEG_OPAQUE_RENDERER_HPP
#define ODFAEG_OPAQUE_RENDERER_HPP
#include "renderTexture.hpp"
namespace odfaeg {
    namespace graphic {
        class OpaqueRenderer {
            OpaqueRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId, bool useThread);
            void createDescriptorAndPipelines();
            void updateDescriptorSets();
            void drawNextFrame();
            void clear();
            unsigned int getLayer();
            void draw();
        private :
            unsigned int layer;
            bool needToUpdateDescriptorSets;
            Shader hzShader;            
            RenderTarget& parentRenderer;
            RenderTarget::ViewProjMatPC hzVertPC;
        };
    }
}
#include "opaqueRenderer.inl"
#endif