#ifndef ODFAEG_OIT_RENDER_COMPONENT_HPP
#define ODFAEG_OIT_RENDER_COMPONENT_HPP
#include "renderWindow.h"
#include "renderTexture.h"
#include "sprite.h"
#include "entityManager.h"
#include "heavyComponent.h"
#include "../Physics/particuleSystem.h"
#include "odfaeg/Graphics/rectangleShape.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file OITRenderComponent.h
          * \class OITRenderComponent
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * \brief represent a component used to render the entities of a scene.
          */
        class ODFAEG_GRAPHICS_API OITRenderComponent : public HeavyComponent {
            public :
            /**
            * \fn OITRenderComponent (RenderWindow& window, int layer)
            * \brief constructor.
            * \param RenderWindow& window : the render window.
            * \param int layer : the layer's position.
            */
            OITRenderComponent (RenderWindow& window, int layer, std::string expression, bool twoLayers=true, sf::ContextSettings settings = sf::ContextSettings(0, 0, 4, 3, 0));
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
            * \fn View& getView()
            * \brief return the view.
            * \return the view.
            */
            View& getView();
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
            void setView(View view);
            /**
            * \fn const Texture& getFrameBufferTexture()
            * \brief get the texture of the frame buffer.
            * \return the texture of the framebuffer.
            */
            const Texture& getFrameBufferTexture();
            /**
            * \fn const Texture& getDepthBufferTexture()
            * \brief get the texture of the depth buffer.
            * \return the texture of the depthbuffer.
            */
            const Texture& getDepthBufferTexture();
            /**
            * \fn const Texture& getNormalMapTexture()
            * \brief get the texture of the normal buffer.
            * \return the texture of the normals.
            */
            const Texture& getFrontBufferTexture();
            /**
            * \fn getFrameBufferSprite ()
            * \brief get the frame buffer tile.
            * \return the tile.
            */
            Sprite& getFrameBufferSprite ();
            Sprite& getFrontBufferSprite();
            /**
            * \fn getDepthBufferSprite ()
            * \brief get the depth buffer tile.
            * \return the tile.
            */
            Sprite& getDepthBufferSprite();
            void changeVisibleEntities(Entity* toRemove, Entity* toAdd, EntityManager* em);
            /**
            * \fn register an event to the event stack of the component.
            * \param sf::Event : the event to register.
            * \param Renderwindow : the window generating the event.
            */
            void pushEvent(sf::Event event, RenderWindow& window);
            void updateParticleSystems();
            void removeEntities(odfaeg::physic::BoundingBox br);
            void insertNewEntities(std::map<physic::BoundingBox, std::vector<Entity*>> entities);
            ~OITRenderComponent();
            private :
            std::vector<std::pair<std::reference_wrapper<Drawable>, RenderStates>> drawables;
            Batcher batcher, newBatcher; /**> A group of faces using the same materials and primitive type.*/
            sf::Color backgroundColor; /**> The background color.*/
            std::vector<Instance> m_instances; /**> Instances to draw. (Instanced rendering.) */
            std::map<physic::BoundingBox, std::pair<unsigned int, std::vector<Instance>>> m_newInstances;
            std::vector<Entity*> visibleEntities; /**> Entities loaded*/
            RenderTexture frontBuffer; /**> the frame buffer.*/
            RenderTexture  depthBuffer; /**> the depth buffer.*/
            RenderTexture  frameBuffer; /**> specular components.*/
            RenderTexture previousFrame;
            Shader depthBufferGenerator; /**> the shader to generate the depth buffer.*/
            Shader depthPeelingGenerator;
            Shader frameBufferGenerator; /**> the shader to generate the frame buffer.*/
            Shader transformFragShader;
            Shader simpleShader;
            RenderStates currentStates; /**> the current render states.*/
            View view, previousView; /**> the view of the component.*/
            Sprite  frameBufferSprite, previousFrameBufferSprite, depthBufferSprite, frontBufferSprite; /**> the frame, depth and normal buffer.*/
            std::string expression;
            bool update, twoLayers;
            bool viewUpdated, firstDraw, secondDraw, firstLoad, animationUpdated;
        };
    }
}
#endif // ODFAEG_FAST_RENDER_COMPONENT_HPP
