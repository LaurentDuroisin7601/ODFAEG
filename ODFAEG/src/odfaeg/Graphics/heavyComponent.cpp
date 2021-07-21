#include "../../../include/odfaeg/Graphics/heavyComponent.h"
namespace odfaeg {
    namespace graphic {
        std::vector<RenderTexture*> HeavyComponent::sharedRenderTextures = std::vector<RenderTexture*>();
        bool HeavyComponent::containsSharedRenderTexture(RenderTexture* rt) {
            for (unsigned int i = 0; i < sharedRenderTextures.size(); i++) {
                if (sharedRenderTextures[i] == rt)
                    return true;
            }
            return false;
        }
        void HeavyComponent::addSharedRenderTexture (RenderTexture* sharedRenderTexture) {
            if (!containsSharedRenderTexture(sharedRenderTexture)) {
                sharedRenderTextures.push_back(sharedRenderTexture);
            }
        }
        unsigned int HeavyComponent::getComponentType() const {
            return 0;
        }
    }
}
