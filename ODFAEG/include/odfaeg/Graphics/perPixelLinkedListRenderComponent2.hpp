#ifndef ODFAEG_PERPIXEL_LINKEDLIST_RENDER_COMPONENT2_HPP
#define ODFAEG_PERPIXEL_LINKEDLIST_RENDER_COMPONENT2_HPP
#ifndef VULKAN
#include "GL/glew.h"
#include <SFML/OpenGL.hpp>
#endif // VULKAN
#include "heavyComponent.h"
#include "renderTexture.h"
#include "sprite.h"
#include "rectangleShape.h"
#include "3D/skybox.hpp"
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
        class PerPixelLinkedListRenderComponent2 : public HeavyComponent {
            public :
            struct uint64_to_uint128 {
                uint64_t handle;
                uint64_t padding;
            };
            struct Samplers {
                uint64_to_uint128 tex[200];
            };
            struct DrawArraysIndirectCommand {
                unsigned int  count;
                unsigned int  instanceCount;
                unsigned int  firstIndex;
                unsigned int  baseInstance;
            };
            struct EntityData {
                math::Matrix4f transformMatrix;
                unsigned int materialID;
                unsigned int vertexOffset;
            };
            struct MaterialData {
                unsigned int textureId;
            };
            struct VertexData {
                math::Vec3f position;
                math::Vec3f color;
                math::Vec3f texCoords;
            };
            struct EntityIdData {
                unsigned int entityId;
            };
            PerPixelLinkedListRenderComponent2 (RenderWindow& window, int layer, std::string expression, window::ContextSettings settings);
            void preloadEntitiesOnComponent(std::vector<Entity*> entities, EntityFactory& factory);
            void loadTextureIndexes();
            std::vector<Entity*> getEntities();
            bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities);
            void loadSkybox(Entity* skybox);
            bool needToUpdate();
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
            void pushEvent(window::IEvent event, RenderWindow& window);
            void setView(View view);
            View& getView();
            const Texture& getFrameBufferTexture();
            RenderTexture* getFrameBuffer();
            ~PerPixelLinkedListRenderComponent2();
            private :
            GLuint maxNodes;
            sf::Vector3i resolution;
            unsigned int atomicBuffer, linkedListBuffer, clearBuf, headPtrTex, ubo, vboIndirect, entityData, vertexData, materialData, entityIdData;
            void compileShaders();
            Shader perPixelLinkedList, perPixelLinkedListP2, skyboxShader;
            RectangleShape quad;
            std::vector<std::pair<std::reference_wrapper<Drawable>, RenderStates>> drawables;
            std::array<std::vector<Entity*>, Batcher::nbPrimitiveTypes> pVisibleEntities;
            Entity* skybox;
            VertexBuffer quadVBO, skyboxVBO;
            View view;
            std::string expression;
            Sprite frameBufferSprite;
            RenderTexture frameBuffer;
            sf::Color backgroundColor;
            int layer;
            RenderStates currentStates;
            bool update;
        };
        #endif
    }
}
#endif
