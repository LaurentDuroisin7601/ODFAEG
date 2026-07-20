module;
#include <map>
#include <string>
export module odfaeg.graphic.renderGraph;
import odfaeg.graphic.iComponent;
import odfaeg.graphic.iRenderer;
import odfaeg.graphic.shadowRenderer;
import odfaeg.graphic.linkedListRenderer;
import odfaeg.graphic.widget;
import odfaeg.math.vec;
import odfaeg.graphic.renderTarget;
import odfaeg.graphic.renderTexture;
import odfaeg.graphic.texture;
namespace odfaeg {
    namespace graphic {
        export class RenderGraph {
            public :
            RenderGraph();
            void addLinkedListPass(RenderTarget& output, unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
            void addShadowPass(RenderTarget& output, RenderTexture& input,  unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
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
            void render();
            std::vector<IComponent*> getComponents();            
            private :
            unsigned int llSMTransitionPoint;
            std::map<unsigned int, IRenderer*> renderers;
            std::map<unsigned int, Widget*> widgets;
            RenderTexture* inputShadowRT;
        };
    }
}