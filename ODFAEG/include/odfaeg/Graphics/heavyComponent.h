#ifndef ODFAEG_HEAVY_COMPONENT
#define ODFAEG_HEAVY_COMPONENT
#include "component.h"
#include "entity.h"
namespace odfaeg {
    namespace graphic {
        class EntityManager;
        class ODFAEG_GRAPHICS_API HeavyComponent : public Component {
            public :
            HeavyComponent(RenderWindow& window, math::Vec3f position, math::Vec3f size, math::Vec3f origin) :
                Component(window, position, size, origin, position.z()) {
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (!sharedValues[i].has_value()) {
                        sharedValues[i] = 0;
                    }
                }
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (!valuesToWait[i].has_value()) {
                        valuesToWait[i] = 0;
                    }
                }
                VkSemaphoreCreateInfo semaphoreInfo{};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                VkSemaphoreTypeCreateInfo timelineCreateInfo{};
                timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
                timelineCreateInfo.pNext = nullptr;
                timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

                semaphoreInfo.pNext = &timelineCreateInfo;
                for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                    if (sharedTimeline[i] == VK_NULL_HANDLE) {
                        timelineCreateInfo.initialValue = sharedValues[i].value();
                        if (vkCreateSemaphore(window.getDevice().getDevice(), &semaphoreInfo, nullptr, &sharedTimeline[i]) != VK_SUCCESS) {
                            throw std::runtime_error("failed to create semaphore");
                        }
                    }
                }
                nbHeavyComponents++;
            }
            void recomputeSize() {
                float sx, sy, npx, npy, nsx, nsy;
                sx = getSize().x();
                sy = getSize().y();
                npx = getWindow().getSize().x() * getRelPosition().x();
                npy = getWindow().getSize().y() * getRelPosition().y();
                nsx = getWindow().getSize().x() * getRelSize().x();
                nsy = getWindow().getSize().y() * getRelSize().y();
                setScale(math::Vec3f(nsx / sx, nsy / sy, 1.f));
                setPosition(math::Vec3f(npx, npy, getPosition().z()));
                setAutoResized(false);
            }
            virtual void loadShaders() {}
            virtual void updateTransformMatrices() {}
            virtual void updateSceneVertices() {}
            virtual bool loadEntitiesOnComponent(std::vector<Entity*> entity) = 0;
            virtual std::vector<Entity*> getEntities() = 0;
            virtual std::string getExpression() = 0;
            virtual void setExpression(std::string expression) = 0;
            virtual void drawNextFrame() = 0;
            virtual void draw(Drawable& drawable, RenderStates states) = 0;
            virtual void setView(View view) = 0;
            virtual View& getView() = 0;
            virtual void loadTextureIndexes() = 0;
            unsigned int getComponentType() const;
            ~HeavyComponent() {
                nbHeavyComponents--;
                if (nbHeavyComponents == 0) {
                    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                        vkDestroySemaphore(getWindow().getDevice().getDevice(), sharedTimeline[i], nullptr);
                    }
                }
            }
            static void addSharedRenderTexture (RenderTexture* rt);
            static bool containsSharedRenderTexture (RenderTexture* rt);
            static VkSemaphore getSharedTimeline(unsigned int currentFrame);
            static unsigned int getSharedValue(unsigned int currentFrame);
            static void increaseSharedValue(unsigned int currentFrame);
            static void increaseValueToWait(unsigned int currentFrame, unsigned int value);
            static unsigned int getValueToWait(unsigned int currentFrame);
            private :
                static std::vector<RenderTexture*> sharedRenderTextures;
                static std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> sharedTimeline;
                static std::array<std::optional<unsigned int>, MAX_FRAMES_IN_FLIGHT> sharedValues;
                static std::array<std::optional<unsigned int>, MAX_FRAMES_IN_FLIGHT> valuesToWait;
                static unsigned int nbHeavyComponents;
        };
    }
}
#endif // ODFAEG_HEAVY_COMPONENT
