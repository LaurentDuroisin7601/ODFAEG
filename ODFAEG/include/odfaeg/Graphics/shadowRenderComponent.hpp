#ifndef ODFAEG_SHADOW_RENDER_COMPONENT_HPP
#define ODFAEG_SHADOW_RENDER_COMPONENT_HPP
#include "GL/glew.h"
#include "renderWindow.h"
#include "renderTexture.h"
#include "sprite.h"
#include "entityManager.h"
#include "heavyComponent.h"
#include "2D/ambientLight.h"
#include "model.h"
#include "rectangleShape.h"
#include "../Physics/particuleSystem.h"
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
        class ODFAEG_GRAPHICS_API ShadowRenderComponent : public HeavyComponent {
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
                ShadowRenderComponent (RenderWindow& window, int layer, std::string expression,window::ContextSettings settings = window::ContextSettings(0, 0, 4, 3, 0));
                void loadTextureIndexes();
                void drawNextFrame();

                void drawInstanced();
                void drawInstancedIndexed();
                void drawNormal();
                void drawNormalIndexed();

                void onVisibilityChanged(bool visible);
                std::vector<Entity*> getEntities();
                void draw(RenderTarget& target, RenderStates states);
                void draw(Drawable& drawable, RenderStates states) {
                }
                void pushEvent(window::IEvent event, RenderWindow& rw);
                bool needToUpdate();
                View& getView();
                int getLayer();
                const Texture& getStencilBufferTexture();
                const Texture& getShadowMapTexture();
                Sprite& getFrameBufferTile ();
                Sprite& getDepthBufferTile();
                void setExpression(std::string expression);
                std::string getExpression();
                void setView(View view);
                bool loadEntitiesOnComponent(std::vector<Entity*> vEntities);
                void clear();
                ~ShadowRenderComponent();
            private :
                Batcher batcher, shadowBatcher, normalBatcher, normalShadowBatcher, batcherIndexed, shadowBatcherIndexed, normalBatcherIndexed, normalShadowBatcherIndexed; /**> A group of faces using the same materials and primitive type.*/
                std::vector<Instance> m_instances, m_normals, m_instancesIndexed, m_normalsIndexed; /**> Instances to draw. (Instanced rendering.) */
                std::vector<Instance> m_shadow_instances, m_shadow_normals, m_shadow_instances_indexed, m_shadow_normalsIndexed;
                std::vector<Entity*> visibleEntities; /**> Entities loaded*/
                RenderTexture stencilBuffer; /**> the stencil buffer.*/
                RenderTexture shadowMap; /**> the shadow map.*/
                RenderTexture depthBuffer;
                RenderTexture alphaBuffer;
                Sprite stencilBufferTile, shadowTile, depthBufferTile, alphaBufferSprite; /**> the stencil and shadow map buffer.*/
                Shader buildShadowMapShader, buildShadowMapNormalShader; /**> the shader to generate the stencil buffer.*/
                Shader perPixShadowShader, perPixShadowShaderNormal; /**> the shader to generate the shadow map.*/
                Shader depthGenShader, depthGenNormalShader;
                Shader sBuildAlphaBufferShader, sBuildAlphaBufferNormalShader;
                View view; /**> the view of the component.*/
                std::string expression;
                bool update;
                unsigned int vboWorldMatrices, vboShadowProjMatrices, ubo, clearBuf, alphaTex, clearBuf2, depthTex, stencilTex, clearBuf3, vboIndirect;
                VertexBuffer vb, vb2;
                std::array<VertexBuffer ,Batcher::nbPrimitiveTypes> vbBindlessTex;
                std::vector<float> matrices, matrices2;
         };
         #endif
    }
}
#endif


