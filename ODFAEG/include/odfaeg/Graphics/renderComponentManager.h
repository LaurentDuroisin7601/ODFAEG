#ifndef ODFAEG_FAST_RENDER_COMPONENT_MANAGER_HPP
#define ODFAEG_FAST_RENDER_COMPONENT_MANAGER_HPP
#include "renderTexture.h"
#include "renderWindow.h"
#include "shader.h"
#include "../Graphics/tile.h"
#include "../../../include/odfaeg/Graphics/heavyComponent.h"
#include "../../../include/odfaeg/Graphics/lightComponent.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file OITRenderComponentManger.h
          * \class RenderComponentManager
          * \brief Store every components.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  Represent manager of components.
          */
        class ODFAEG_GRAPHICS_API RenderComponentManager {

        public :
            /**
            * \fn RenderComponentManager(RenderWindow& window)
            * \brief constructor.
            * \param window : the window
            */
            RenderComponentManager(graphic::RenderWindow& window);
            /**
            * \fn void addRenderComponent(g2d::OITRenderComponent* component)
            * \brief add a render component.
            * \param component : the render component to add.
            */
            void addComponent(graphic::Component* component);
            void addWindow(RenderWindow& window);
            void setEventContextActivated(bool activated, RenderWindow& window);
            bool removeComponent(unsigned int layer);
            /**
            * \fn RenderWindow& getWindow()
            * \brief get the window.
            * \return the window.
            */
            RenderWindow& getWindow();
            /**
            * \fn void initComponents();
            * \brief init the components.
            */
            void initComponents();
            /**
            * \fn void clearComponents();
            * \brief clear the components.
            */
            void clearComponents();
            /**
            * \fn void drawComponents();
            * \brief draw the components into the window.
            */
            void drawRenderComponents ();
            void drawGuiComponents();
            /**
            * \fn sf::Color getClrColor();
            * \brief get the clear color.
            * \return the clear color.
            */
            sf::Color getClrColor();
            /**
            * \fn bool isComponentCreated(int layer)
            * \brief check if a component has been created on the following layer.
            * \param the layer.
            * \return if the component has been created.
            */
            bool isComponentAdded(unsigned int layer);
            /**
            * \fn g2d::OITRenderComponent* getRenderComponent(int layer)
            * \brief get the render component on the following layer.
            * \param layer : the number of the layer.
            * \return the render component.
            */
            Component* getRenderComponent(unsigned int layer);
            Component* getGuiComponent(unsigned int layer);
            Component* getComponent(unsigned int layer);
            std::vector<Component*> getComponents();
            std::vector<Component*> getRenderComponents();
            /**
            * \fn ~RenderComponentManager();
            * \brief destructor.
            */
            ~RenderComponentManager();
            /**
            * \fn unsigned int getNbComponents();
            * \brief get the number of components.
            * \return the number of components.
            */
            unsigned int getNbComponents();
            void updateComponents();
        protected :
            std::multimap<int, std::unique_ptr<Component>, std::greater<int>> components; /**> the components.*/
            std::vector<RenderWindow*> windows; /**> the window.*/
        };
    }
}
#endif // FAST_RENDER_COMPONENT_HPP
