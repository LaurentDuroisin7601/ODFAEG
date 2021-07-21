#ifndef ODFAEG_GRAPHIC_RAYTRACING_RENDER_COMPONENT
#define ODFAEG_GRAPHIC_RAYTRACING_RENDER_COMPONENT

#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "2D/ambientLight.h"
#include "2D/ponctualLight.h"
#include "../config.hpp"
#ifndef VULKAN
#include "GL/glew.h"
#include <SFML/OpenGL.hpp>
#endif
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
        class RaytracingRenderComponent : public HeavyComponent {
        public :
            struct Triangle {
                math::Matrix4f transform;
                math::Matrix4f textureMatrix;
                math::Vec3f positions[3];
                math::Vec3f colours[3];
                math::Vec3f texCoords[3];
                math::Vec3f normal;
                uint32_t textureIndex;
                uint32_t refractReflect;
                alignas (8) float ratio;
                //float padding;
            };
            struct Light {
                math::Vec3f center;
                math::Vec3f color;
                float radius;
            };
            struct uint64_to_uint128 {
                uint64_t handle;
                uint64_t padding;
            };
            struct Samplers {
                uint64_to_uint128 tex[200];
            };
            RaytracingRenderComponent (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
                 /**
            * \fn bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities)
            * \brief load the given entities onto the component.
            * \param std::vector<Entity*> visibleEntities : the entities to load.
            * \return if the loading was sucessfull.
            */
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
            * \fn int getLayer()C:\Users\Laurent\Windows\ODFAEG\include\odfaeg\Math\triangle.h
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
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            const Texture& getFrameBufferTexture();
            ~RaytracingRenderComponent();
        private :
            VertexBuffer vb;
            odfaeg::graphic::RectangleShape quad;
            sf::Color backgroundColor; /**> The background color.*/
            odfaeg::graphic::RenderTexture frameBuffer;
            Sprite frameBufferSprite;
            unsigned int frameBufferTex, trianglesSSBO, lightsSSBO, clearBuf, ubo;
            unsigned int atomicBuffer, linkedListBuffer, headPtrTex, clearBuf2;
            odfaeg::graphic::Shader rayComputeShader, quadShader;
            std::vector<Triangle> triangles;
            std::vector<Light> lights;
            bool update;
            std::string expression;
            int layer;
            std::vector<odfaeg::graphic::Entity*> visibleEntities;
            View view;
            Texture external;
        };
        #endif
    }
}
#endif // ODFAEG_GRAPHIC_RAYTRACING_RENDER_COMPONENT
