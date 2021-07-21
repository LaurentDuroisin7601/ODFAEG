#ifndef ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
#define ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
#include "GL/glew.h"
#include <SFML/OpenGL.hpp>
#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "world.h"
#include "perPixelLinkedListRenderComponent.hpp"
#include "3D/cube.h"
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
        class ReflectRefractRenderComponent : public HeavyComponent {
        public :
            struct uint64_to_uint128 {
                uint64_t handle;
                uint64_t padding;
            };
            struct Samplers {
                uint64_to_uint128 tex[200];
            };
            ReflectRefractRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
            void loadTextureIndexes();
            std::vector<Entity*> getEntities();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
            bool needToUpdate();
            /**
            * \fn void clearBufferBits()
            * \brief clear the buffer bits of the component.
            */
            void clear ();
            /**
            * \fn void setBackgroundColor(sf::Color color)
            * \brief set the background color of the component. (TRansparent by default)
            * \param sf::Color color : the color.
            */
            void setBackgroundColor(sf::Color color);
            /**
            * \fn void drawNextFrame()
            * \brief draw the next frame of the component.
            */
            void drawNextFrame();
            void setExpression (std::string expression);
            /**
            * \fn draw(Drawable& drawable, RenderStates states = RenderStates::Default);
            * \brief draw a drawable object onto the component.
            * \param Drawable drawable : the drawable object to draw.
            * \param RenderStates states : the render states.
            */
            void draw(Drawable& drawable, RenderStates states = RenderStates::Default);
            /**
            * \fn void draw(RenderTarget& target, RenderStates states)
            * \brief draw the frame on a render target.
            * \param RenderTarget& target : the render target.
            * \param RenderStates states : the render states.
            */
            void draw(RenderTarget& target, RenderStates states);
            std::string getExpression();
            /**
            * \fn int getLayer()
            * \brief get the layer of the component.
            * \return the number of the layer.
            */
            int getLayer();
            /**
            * \fn void setView(View& view)
            * \brief set the view of the component.
            * \param the view of the component.
            */
            /**
            * \fn register an event to the event stack of the component.
            * \param window::IEvent : the event to register.
            * \param Renderwindow : the window generating the event.
            */
            void onVisibilityChanged(bool visible);
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            ~ReflectRefractRenderComponent();
        private :
            RectangleShape quad;
            Batcher batcher, normalBatcher, reflBatcher, reflNormalBatcher, rvBatcher, normalRvBatcher;
            sf::Color backgroundColor; /**> The background color.*/
            std::vector<Instance> m_instances, m_normals, m_reflInstances, m_reflNormals; /**> Instances to draw. (Instanced rendering.) */
            std::vector<Entity*> visibleEntities;
            RenderTexture depthBuffer, alphaBuffer, reflectRefractTex, environmentMap;
            Shader sBuildDepthBuffer, sBuildDepthBufferNormal, sBuildAlphaBuffer, sBuildAlphaBufferNormal, sReflectRefract, sReflectRefractNormal, sLinkedList, sLinkedListNormal, sLinkedList2;
            View view;
            std::string expression;
            bool update, cubeMapCreated;
            unsigned int vboWorldMatrices, atomicBuffer, linkedListBuffer, clearBuf, clearBuf2, clearBuf3, headPtrTex, alphaTex, depthTex, ubo;
            float squareSize;
            Sprite depthBufferSprite, reflectRefractTexSprite, alphaBufferSprite;
            std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
            VertexBuffer vb, vb2;
            std::vector<float> matrices;
            math::Vec3f dirs[6];
        };
        #endif
    }
}
#endif // ODFAEG_REFLECT_REFRACT_RENDER_COMPONENT
