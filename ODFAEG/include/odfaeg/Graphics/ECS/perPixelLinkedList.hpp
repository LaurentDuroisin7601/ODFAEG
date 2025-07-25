#ifndef ECS_PER_PIXEL_LINKED_LIST
#define ECS_PER_PIXEL_LINKED_LIST

#include "heavyComponent.hpp"
#include "../renderTexture.h"
#include "../sprite.h"
#include "../rectangleShape.h"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            #ifdef VULKAN
            #else
                class ODFAEG_GRAPHICS_API PerPixelLinkedListRenderComponent : public HeavyComponent {
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
                struct DrawElementsIndirectCommand {
                        unsigned index_count;
                        unsigned instance_count;
                        unsigned first_index;       // cf parametre offset de glDrawElements()
                        unsigned vertex_base;
                        unsigned instance_base;
                };
                struct ModelData {
                    math::Matrix4f worldMat;
                };
                struct MaterialData {
                    unsigned int textureIndex;
                    unsigned int materialType;
                };
                PerPixelLinkedListRenderComponent (RenderWindow& window, int layer, std::string expression, ComponentMapping& componentMapping, window::ContextSettings settings);
                void onVisibilityChanged(bool visible);
                void loadTextureIndexes();

                     /**
                * \fn bool loadEntitiesOnComponent(std::vector<Entity*> visibleEntities)
                * \brief load the given entities onto the component.
                * \param std::vector<Entity*> visibleEntities : the entities to load.
                * \return if the loading was sucessfull.
                */
                std::vector<EntityId> getEntities();
                bool loadEntitiesOnComponent(ComponentMapping& componentMapping, std::vector<EntityId> visibleEntities);
                void loadSkybox(EntityId skybox);
                bool needToUpdate();
                /**
                * \fn void clearBufferBits()
                * \brief clear the buffer bits of the component.
                */
                void clear ();
                /**
                * \fn void setBackgroundColor(Color color)
                * \brief set the background color of the component. (TRansparent by default)
                * \param Color color : the color.
                */
                void setBackgroundColor(Color color);
                /**
                * \fn void drawNextFrame()
                * \brief draw the next frame of the component.
                */
                void drawNextFrame();
                void drawInstances();
                void drawInstancesIndexed();

                void drawSelectedInstances();
                void drawSelectedInstancesIndexed();
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
                ~PerPixelLinkedListRenderComponent();
                private :
                void compileShaders();
                RectangleShape quad;
                std::vector<std::pair<std::reference_wrapper<Drawable>, RenderStates>> drawables;
                Batcher batcher, batcherIndexed, normalBatcher, normalBatcherIndexed,
                selectedInstanceBatcher, selectedInstanceScaleBatcher, selectedInstanceIndexBatcher, selectedInstanceIndexScaleBatcher,
                selectedBatcher, selectedScaleBatcher, selectedIndexBatcher, selectedIndexScaleBatcher, skyboxBatcher; /**> A group of faces using the same materials and primitive type.*/
                Color backgroundColor; /**> The background color.*/
                std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed,
                m_selectedScale, m_selected, m_selectedScaleIndexed, m_selectedIndexed,
                m_selectedScaleInstance, m_selectedInstance, m_selectedScaleInstanceIndexed, m_selectedInstanceIndexed, m_skyboxInstance; /**> Instances to draw. (Instanced rendering.) */

                std::vector<EntityId> visibleEntities;
                //std::vector<std::unique_ptr<EntityId>> visibleSelectedScaleEntities; /**> Entities loaded*/
                RenderTexture frameBuffer; /**> the frame buffer.*/
                Shader perPixelLinkedListP2, indirectRenderingShader, skyboxShader;
                RenderStates currentStates; /**> the current render states.*/
                View view; /**> the view of the component.*/
                std::string expression;
                bool update;
                GLuint maxNodes;
                math::Vec3f resolution;
                unsigned int atomicBuffer, linkedListBuffer, clearBuf, headPtrTex, modelDataBuffer, materialDataBuffer, ubo, vboIndirect;
                Sprite frameBufferSprite;
                VertexBuffer vb;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
                int layer;
                EntityId skybox;
                core::Clock timeClock;
                ComponentMapping& componentMapping;
            };
            #endif
        }
    }
}
#endif // ECS_PER_PIXEL_LINKED_LIST
