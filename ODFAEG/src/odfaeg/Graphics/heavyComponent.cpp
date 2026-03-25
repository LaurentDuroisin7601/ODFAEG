#include "../../../include/odfaeg/Graphics/heavyComponent.h"
namespace odfaeg {
    namespace graphic {
        std::vector<RenderTexture*> HeavyComponent::sharedRenderTextures = std::vector<RenderTexture*>();
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> HeavyComponent::sharedTimeline = std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>{};
        std::array<std::optional<unsigned int>, MAX_FRAMES_IN_FLIGHT> HeavyComponent::sharedValues = std::array<std::optional<unsigned int>, MAX_FRAMES_IN_FLIGHT>{};
        std::array<std::optional<unsigned int>, MAX_FRAMES_IN_FLIGHT> HeavyComponent::valuesToWait = std::array<std::optional<unsigned int>, MAX_FRAMES_IN_FLIGHT>{};
        unsigned int HeavyComponent::nbHeavyComponents = 0;
        VkSemaphore HeavyComponent::getSharedTimeline(unsigned int currentFrame) {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            return sharedTimeline[currentFrame];
        }
        unsigned int HeavyComponent::getSharedValue(unsigned int currentFrame) {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            return sharedValues[currentFrame].value();
        }
        unsigned int HeavyComponent::getValueToWait(unsigned int currentFrame) {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            return valuesToWait[currentFrame].value();
        }
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
        void HeavyComponent::increaseSharedValue(unsigned int currentFrame) {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            sharedValues[currentFrame].value()++;
        }
        void HeavyComponent::increaseValueToWait(unsigned int currentFrame, unsigned int value) {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            valuesToWait[currentFrame].value() += value;
        }
    }
}
