#ifndef ECS_COMPONENT_HPP
#define ECS_COMPONENT_HPP
#include "../drawable.h"
#include "../transformable.h"
#include "../../Window/listener.h"
#include "../renderWindow.h"
#include "../../Core/ecs.hpp"
namespace odfaeg {
    namespace graphic {
        namespace ecs {
            class Component : public Drawable, public Transformable {
            public :
                Component(RenderWindow& window, math::Vec3f position, math::Vec3f size, math::Vec3f origin, unsigned int priority=0);

                /** \fn set the relative position relative to the top-left of the parent's component.
                * if the component haven't any parent, the parent is the window.
                * \param x : the relative position for x axis : 1 = 100%, 0 = 0%.
                * \param y ; the relative position for y axis : 1 = 100%, 0 = 0%.
                */
                void setRelPosition(float x, float y)  {
                    relPosX = x;
                    relPosY = y;
                    autoResize = (autoResize ==  false) ? true : autoResize;
                    relPosition = true;
                }
                /** \fn set the size relative to the size of the parent's component.
                * \param w : the relative width for x axis : 1 = 100%, 0 = 0%
                * \param h : the relative height for y axis : 1 = 100%, 0 = 0%.
                */
                void setRelSize (float w, float h) {
                    relSizeX = w;
                    relSizeY = h;
                    autoResize = (autoResize ==  false) ? true : autoResize;
                    relPosition = true;
                }
                bool isRelPosition() {
                    return relPosition;
                }
                void setRelPosition(bool relPosition) {
                    this->relPosition = relPosition;
                }
                /** \fn get the relative position.
                * \return Vec2f : the relative position.
                */
                math::Vec2f getRelPosition() {
                    return math::Vec2f(relPosX, relPosY);
                }
                /** \fn get the relative size.
                * \return Vec2f the relative size.
                */
                math::Vec2f getRelSize() {
                    return math::Vec2f (relSizeX, relSizeY);
                }
                /**
                * \fn isAutoResized : return if the composant is autoresize of not.
                * \return if the component is autoresized.
                */
                bool isAutoResized () {
                    return autoResize;
                }
                /**
                * \fn recompute the size relative to the parent component if the parent component is resized.
                */
                virtual void recomputeSize() = 0;
                virtual void clear() = 0;
                virtual void pushEvent(window::IEvent event, RenderWindow& window) = 0;
                virtual void onUpdate(RenderWindow* window, window::IEvent& event) {}
                core::Listener& getListener() {
                    return listener;
                }
                void setEventContextActivated(bool activateEventContext);
                bool isEventContextActivated();
                void setVisible (bool visible);
                bool isVisible();
                int getId() {
                    return id;
                }
                unsigned int getPriority() {
                    return priority;
                }
                void setPriority(unsigned int priority) {
                    this->priority = priority;
                }
                virtual void onVisibilityChanged(bool visible);
                virtual void onEventContextActivated(bool activate);
                virtual void processEvents();
                RenderWindow& getWindow() {
                    return window;
                }
                void setAutoResized(bool autoResize) {
                    this->autoResize = autoResize;
                }
                virtual unsigned int getComponentType() const = 0;
                virtual View& getView();
                virtual std::string getExpression();
                virtual void setExpression(std::string expression);
                virtual bool loadEntitiesOnComponent(ComponentMapping& componentMapping, EntityFactory& factory, std::vector<EntityId> entities);
                virtual void setView(View view);
                virtual void draw(Drawable& drawable, RenderStates states);
                virtual void loadTextureIndexes();
            private :
                RenderWindow& window;
                core::Listener listener;
                bool activateEventContext;
                bool visible;
                int id;
                //static int nbComponents;
                unsigned int priority;
                float relPosX, relPosY, relSizeX, relSizeY;
                bool autoResize, relPosition;
            };
        }
    }
}
#endif // ECS_COMPONENT_HPP
