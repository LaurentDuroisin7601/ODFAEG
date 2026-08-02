#ifndef ODFAEG_IRENDERER_HPP
#define ODFAEG_IRENDERER_HPP
#include "iComponent.hpp"
#include "../Math/vec.hpp"
#include "../Window/iEvent.hpp"
#include "../Window/action.hpp"
#include "../Window/command.hpp"
#include "../Core/delegate.hpp"
namespace odfaeg {
    namespace graphic {
        class IRenderer : public IComponent {
            public :
                IRenderer(int windowId) : IComponent(windowId) {}  
                template <typename R>
                void connectSwapchainResizedCommand() {
                    window::Action resizedAction(window::Action::RESIZED);
                    window::Command resizedCmd(resizedAction, core::FastDelegate<void>(&R::onSwapchainResized, static_cast<R*>(this), core::ph<0, math::Vector2i>()));
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
#endif