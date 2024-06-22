#ifndef ODFAEG_HEAVY_COMPONENT
#define ODFAEG_HEAVY_COMPONENT
#include "component.h"
#include "entity.h"
namespace odfaeg {
    namespace graphic {
        class EntityManager;
        class HeavyComponent : public Component {
            public :
            HeavyComponent(RenderWindow& window, math::Vec3f position, math::Vec3f size, math::Vec3f origin) :
                Component(window, position, size, origin, position.z) {
                    //std::cout<<"create heavy component"<<std::endl;
                }
            void recomputeSize() {
                float sx, sy, npx, npy, nsx, nsy;
                sx = getSize().x;
                sy = getSize().y;
                npx = getWindow().getSize().x * getRelPosition().x;
                npy = getWindow().getSize().y * getRelPosition().y;
                nsx = getWindow().getSize().x * getRelSize().x;
                nsy = getWindow().getSize().y * getRelSize().y;
                setScale(math::Vec3f(nsx / sx, nsy / sy, 1.f));
                setPosition(math::Vec3f(npx, npy, getPosition().z));
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
            static void addSharedRenderTexture (RenderTexture* rt);
            static bool containsSharedRenderTexture (RenderTexture* rt);
            private :
                static std::vector<RenderTexture*> sharedRenderTextures;
        };
    }
}
#endif // ODFAEG_HEAVY_COMPONENT
