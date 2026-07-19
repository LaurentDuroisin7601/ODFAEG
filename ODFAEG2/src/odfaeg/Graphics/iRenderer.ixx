module;
export module odfaeg.graphic.iRenderer;
import odfaeg.graphic.iComponent;
import odfaeg.math.vec;
import odfaeg.window.iEvent;
import odfaeg.window.action;
import odfaeg.window.command;
import odfaeg.core.delegate;
namespace odfaeg {
    namespace graphic {
        export class IRenderer : public IComponent {
            public :
                IRenderer(int windowId) : IComponent(windowId) {}  
                template <typename R>
                void connectSwapchainResizeCommand() {
                    window::Action resizedAction(window::Action::RESIZED);
                    window::Command resizedCmd(resizedAction, core::FastDelegate<void>(&R::onSwapchainRecreated, this, core::ph<0, math::Vector2i>()));
                    getEventListener().connect("SwapchainResizeCmd", resizedCmd);
                }              
                virtual void clear() = 0;                
                void update(int windowId, window::IEvent event) {
                    /*Si c'est une fenêtre est liée au renderer, on met à jour la pile d'évènements du listener.*/ 
                    math::Vector2i newSize(event.window.data1, event.window.data2);
                    getEventListener().bindCommandSlotParams("SwapchainResizeCmd", newSize);
                    if (windowId == getWindowId()) {
                        getEventListener().pushEvent(event);
                    }
                }  
        };
    }
}