module;
export module odfaeg.graphic.iRenderer;
import odfaeg.graphic.iComponent;
import odfaeg.math.vec;
import odfaeg.window.iEvent;
namespace odfaeg {
    namespace graphic {
        export class IRenderer : public IComponent {
            public :
                IRenderer(int windowId) : IComponent(windowId) {}                
                virtual void clear() = 0;
                void update(int windowId, window::IEvent event) {
                    /*Si c'est une fenêtre est liée au renderer, on met à jour la pile d'évènements du listener.*/ 
                    if (windowId == getWindowId()) {
                        getEventListener().pushEvent(event);
                    }
                }  
        };
    }
}