#ifndef ODFAEG_PANEL_HPP
#define ODFAEG_PANEL_HPP
#include "../lightComponent.h"
#include "../renderWindow.h"
#include "../rectangleShape.h"
#include "../sprite.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Panel : public LightComponent {
            public :
                Panel(RenderWindow& window, math::Vec3f position, math::Vec3f size, int priority = 0, LightComponent* parent = nullptr);
                void setBackgroundColor(sf::Color color);
                void clear();
                void onDraw(RenderTarget& target, RenderStates states);
                void drawOn(RenderTarget& target, RenderStates states);
                void addSprite(Sprite sprite);
                void addShape (Shape* shape);
                void removeSprites();
                void onSizeRecomputed();
                void removeAll();
                bool isOnXScroll();
                bool isOnYScroll();
                void moveXItems();
                void moveYItems();
                void setBorderThickness(float thickness);
                void setBorderColor(sf::Color color);
                void updateScrolls();
                void addChild(LightComponent* child);
                void onUpdate(RenderWindow* window, window::IEvent& event);
                bool isPointInside(math::Vec3f point);
                void clearDrawables();
                void setScissorDisable(bool scissor);
                void setMoveComponents(bool moveComponents);
                math::Vec3f getDeltas();
            private :
                bool disableScissor, moveComponents;
                math::Vec3f mousePos, deltas;
                bool scrollX, scrollY;
                RectangleShape rect;
                RectangleShape vertScrollBar, horScrollBar, corner;
                math::Vec3f maxSize;
                int oldMouseX, oldMouseY, mouseDeltaX, mouseDeltaY;
                sf::Color background, border;
                std::vector<Shape*> shapes;
                std::vector<Sprite> sprites;
            };
        }
    }
}
#endif
