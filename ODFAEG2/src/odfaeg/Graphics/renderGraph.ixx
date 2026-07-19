module;
#include <map>
#include <string>
export module odfaeg.graphic.renderGraph;
import odfaeg.graphic.iComponent;
import odfaeg.graphic.iRenderer;
import odfaeg.graphic.widget;
import odfaeg.math.vec;
import odfaeg.graphic.renderTarget;
import odfaeg.graphic.renderTexture;
import odfaeg.graphic.texture;
namespace odfaeg {
    namespace graphic {
        export class RenderGraph {
            public :
            void addLinkedListPass(RenderTarget& output, unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
            void addShadowPass(RenderTarget& output, RenderTexture& input,  unsigned int layer, std::string typesToRender, unsigned int windowId=-1);
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