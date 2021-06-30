#ifndef ODFAEG_LIGHT_RENDER_COMPONENT_HPP
#define ODFAEG_LIGHT_RENDER_COMPONENT_HPP
#include "renderWindow.h"
#include "renderTexture.h"
#include "sprite.h"
#include "entityManager.h"
#include "heavyComponent.h"
#include "2D/ambientLight.h"
#include "../Physics/particuleSystem.h"
#include "rectangleShape.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        #else
        /**
          * \file OITRenderComponent.h
          * \class OITRenderComponent
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * \brief represent a component used to render the entities of a scene.
          */
        class ODFAEG_GRAPHICS_API LightRenderComponent : public HeavyComponent {
            public :
                struct uint64_to_uint128 {
                    uint64_t handle;
                    uint64_t padding;
                };
                struct Samplers {
                    uint64_to_uint128 tex[200];
                };
                LightRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0));
                void loadTextureIndexes();
                void onVisibilityChanged(bool visible);
                void pushEvent(window::IEvent event, RenderWindow& rw);
                bool needToUpdate();
                std::string getExpression();
                void clear();
                Sprite& getNormalMapTile ();
                Sprite& getDepthBufferTile();
                Sprite& getspecularTile ();
                Sprite& getBumpTile();
                Sprite& getLightTile();
                const Texture& getDepthBufferTexture();
                const Texture& getnormalMapTexture();
                const Texture& getSpecularTexture();
                const Texture& getbumpTexture();
                const Texture& getLightMapTexture();
                bool loadEntitiesOnComponent(std::vector<Entity*> vEntities);
                void setView(View view);
                void setExpression(std::string expression);
                void drawNextFrame();
                std::vector<Entity*> getEntities();
                void draw(RenderTarget& target, RenderStates states);
                void draw(Drawable& drawable, RenderStates states) {
                }
                View& getView();
                int getLayer();
                ~LightRenderComponent();
            private :
                Batcher batcher, lightBatcher, normalBatcher; /**> A group of faces using the same materials and primitive type.*/
                std::vector<Instance> m_instances, m_normals; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Instance> m_light_instances; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Entity*> visibleEntities; /**> Entities loaded*/
                RenderTexture depthBuffer; /**> the stencil buffer.*/
                RenderTexture normalMap; /**> the shadow map.*/
                RenderTexture bumpTexture;
                RenderTexture specularTexture;
                RenderTexture lightMap;
                Sprite  depthBufferTile, normalMapTile, bumpMapTile, specularBufferTile, lightMapTile; /**> the stencil and shadow map buffer.*/
                Shader depthBufferGenerator, depthBufferNormalGenerator; /**> the shader to generate the stencil buffer.*/
                Shader normalMapGenerator; /**> the shader to generate the shadow map.*/
                Shader specularTextureGenerator;
                Shader bumpTextureGenerator;
                Shader lightMapGenerator;
                View view; /**> the view of the component.*/
                std::string expression;
                bool update;
                unsigned int vboWorldMatrices, ubo;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
                VertexBuffer vb;
                std::vector<float> matrices;
        };
        #endif
    }
}
#endif
