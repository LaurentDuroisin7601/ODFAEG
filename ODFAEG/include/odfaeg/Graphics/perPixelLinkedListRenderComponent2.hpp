/*#ifndef ODFAEG_PER_PIXEL_LINKED_LIST2
#define ODFAEG_PER_PIXEL_LINKED_LIST2
#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "3D/skybox.hpp"
#include <condition_variable>
#include <mutex>
#include "../Core/clock.h"
#include "../Core/threadPool.hpp"
namespace odfaeg {
    namespace graphic {
        class PerPixelLinkedListDrivenRenderComponent : public HeavyComponent {
        public :
            PerPixelLinkedListRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings, bool useThread = true);
            void compileShaders();
        };
    }
}
#endif // ODFAEG_PERPIXEL_LINKED_LIST2*/
