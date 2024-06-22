#include "../../../../include/odfaeg/Graphics/GUI/panel.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Panel::Panel(RenderWindow& window, math::Vec3f position, math::Vec3f size, int priority, LightComponent* parent) :
            LightComponent (window, position, size, size * 0.5f, priority, parent) {
                rect = RectangleShape(size);
                background = sf::Color::Black;
                rect.setFillColor(background);
                scrollX = scrollY = false;
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y, 0);
                oldMouseX = mousePos.x;
                oldMouseY = mousePos.y;
                core::Action a (core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Right);
                core::Command cmd1(a, core::FastDelegate<bool>(&Panel::isOnXScroll, this),core::FastDelegate<void>(&Panel::moveXItems, this));
                getListener().connect("scrollXMove", cmd1);
                core::Command cmd2(a, core::FastDelegate<bool>(&Panel::isOnYScroll, this),core::FastDelegate<void>(&Panel::moveYItems, this));
                getListener().connect("scrollYMove", cmd2);
                maxSize = math::Vec2f(0, 0);
                disableScissor = true;
                moveComponents = true;
                deltas = math::Vec3f(0, 0, 0);
            }
            bool Panel::isOnXScroll() {
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y, 0);
                mouseDeltaX = mousePos.x - oldMouseX;
                oldMouseX = mousePos.x;
                if (scrollX) {
                    physic::BoundingBox bb(vertScrollBar.getPosition().x, vertScrollBar.getPosition().y, 0, vertScrollBar.getSize().x, vertScrollBar.getSize().y, 0);
                    return bb.isPointInside(mousePos);
                }
                return false;
            }
            bool Panel::isOnYScroll() {
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y, 0);
                mouseDeltaY = mousePos.y - oldMouseY;
                oldMouseY  = mousePos.y;
                if (scrollY) {
                    physic::BoundingBox bb(horScrollBar.getPosition().x, horScrollBar.getPosition().y, 0, horScrollBar.getSize().x, horScrollBar.getSize().y, 0);
                    return bb.isPointInside(mousePos);
                }
                return false;
            }
            void Panel::moveXItems() {
                if (mouseDeltaX > 0 && vertScrollBar.getPosition().x + vertScrollBar.getSize().x + math::Math::abs(mouseDeltaX) <= getPosition().x + getSize().x - 10) {
                    vertScrollBar.move(math::Vec3f(mouseDeltaX, 0, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(-(maxSize.x / (getSize().x - 10) * mouseDeltaX), 0, 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(-(maxSize.x / (getSize().x - 10) * mouseDeltaX), 0, 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(-(maxSize.x / (getSize().x - 10) * mouseDeltaX), 0, 0));
                    }
                } else if (mouseDeltaX < 0 && vertScrollBar.getPosition().x +  math::Math::abs(mouseDeltaX) >= getPosition().x) {
                    vertScrollBar.move(math::Vec3f(mouseDeltaX, 0, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(-(maxSize.x / (getSize().x - 10) * mouseDeltaX), 0, 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(-(maxSize.x / (getSize().x - 10) * mouseDeltaX), 0, 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(-(maxSize.x / (getSize().x - 10) * mouseDeltaX), 0, 0));
                    }
                }
            }
            void Panel::moveYItems() {
                if (mouseDeltaY > 0 && horScrollBar.getPosition().y + horScrollBar.getSize().y + math::Math::abs(mouseDeltaY) <= getPosition().y + getSize().y - 10) {
                    horScrollBar.move(math::Vec3f(0, mouseDeltaY, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(0, -(maxSize.y / (getSize().y - 10) * mouseDeltaY), 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(0, -(maxSize.y / (getSize().y - 10) * mouseDeltaY), 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(0, -(maxSize.y / (getSize().y - 10) * mouseDeltaY), 0));
                    }
                } else if (mouseDeltaY < 0 && horScrollBar.getPosition().y +  math::Math::abs(mouseDeltaY) >= getPosition().y) {
                    horScrollBar.move(math::Vec3f(0, mouseDeltaY, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(0, -(maxSize.y / (getSize().y - 10) * mouseDeltaY), 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(0, -(maxSize.y / (getSize().y - 10) * mouseDeltaY), 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(0, -(maxSize.y / (getSize().y - 10) * mouseDeltaY), 0));
                    }
                }
            }
            void Panel::setMoveComponents(bool moveComponents) {
                this->moveComponents = moveComponents;
            }
            void Panel::updateScrolls() {
                maxSize = math::Vec3f(0, 0, 0);
                std::vector<LightComponent*> children = getChildren();
                for (unsigned int i = 0; i < children.size(); i++) {
                    if (children[i]->getPosition().y - getPosition().y + children[i]->getSize().y > maxSize.y)
                        maxSize.y = children[i]->getPosition().y - getPosition().y + children[i]->getSize().y;
                    if (children[i]->getPosition().x - getPosition().x + children[i]->getSize().x > maxSize.x)
                        maxSize.x = children[i]->getPosition().x - getPosition().x + children[i]->getSize().x;
                }
                for (unsigned int i = 0; i < sprites.size(); i++) {
                    if (sprites[i].getPosition().y + sprites[i].getSize().y > maxSize.y)
                        maxSize.y = sprites[i].getPosition().y + sprites[i].getSize().y;
                    if(sprites[i].getPosition().x + sprites[i].getSize().x > maxSize.x)
                        maxSize.x = sprites[i].getPosition().x + sprites[i].getSize().x;
                }
                for (unsigned int i = 0; i < shapes.size(); i++) {
                    if (shapes[i]->getPosition().y + shapes[i]->getSize().y > maxSize.y)
                        maxSize.y = shapes[i]->getPosition().y + shapes[i]->getSize().y;
                    if(shapes[i]->getPosition().x + shapes[i]->getSize().x > maxSize.x)
                        maxSize.x = shapes[i]->getPosition().x + shapes[i]->getSize().x;
                }
                if (maxSize.x > getSize().x || maxSize.y > getSize().y) {
                    corner = RectangleShape(math::Vec3f(10, 10, 0));
                    corner.setPosition(math::Vec3f(getPosition().x + getSize().x - 10, getPosition().y + getSize().y - 10, 0));
                }
                if (maxSize.x > getSize().x) {
                    unsigned int scrollXSize = (getSize().x - 10) / maxSize.x * (getSize().x - 10);
                    vertScrollBar = RectangleShape(math::Vec3f(scrollXSize, 10, 0));
                    vertScrollBar.setPosition(math::Vec3f(getPosition().x, getPosition().y + getSize().y - 10, 0));
                    scrollX = true;
                } else {
                    scrollX = false;
                }
                if (maxSize.y > getSize().y) {
                    unsigned int scrollYSize = (getSize().y - 10) / maxSize.y * (getSize().y - 10);
                    horScrollBar = RectangleShape(math::Vec3f(10, scrollYSize, 0));
                    horScrollBar.setPosition(math::Vec3f(getPosition().x + getSize().x - 10, getPosition().y, 0));
                    scrollY = true;
                } else {
                    scrollY = false;
                }
            }
            void Panel::addChild(LightComponent* child) {
                //std::cout<<"add child"<<std::endl;
                LightComponent::addChild(child);
                //std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                //std::cout<<"update scrolls"<<std::endl;
                updateScrolls();
            }
            void Panel::onSizeRecomputed() {

            }
            void Panel::removeAll() {
                LightComponent::removeAll();
                maxSize = math::Vec3f(0, 0, 0);
                vertScrollBar.setPosition(math::Vec3f(getPosition().x, getPosition().y + getSize().y - 10, 0));
                horScrollBar.setPosition(math::Vec3f(getPosition().x + getSize().x - 10, getPosition().y, 0));
                scrollY = scrollX = false;
            }
            void Panel::setBackgroundColor(sf::Color background) {
                this->background = background;
            }
            void Panel::clear() {
                if (background != rect.getFillColor()) {
                    rect.setFillColor(background);
                }
                for (unsigned int i = 0; i < getChildren().size(); i++) {
                    getChildren()[i]->clear();
                }
            }
            void Panel::removeSprites() {
                sprites.clear();
            }
            void Panel::addSprite(Sprite sprite) {
                sprites.push_back(sprite);
                for (unsigned int i = 0; i < sprites.size(); i++) {
                    if (sprites[i].getPosition().x - getPosition().x > deltas.x)
                        deltas.x = sprites[i].getPosition().x - getPosition().x;
                    if (sprites[i].getPosition().y - getPosition().y > deltas.y)
                        deltas.y = sprites[i].getPosition().y - getPosition().y;
                }
            }
            void Panel::setScissorDisable(bool scissor) {
                disableScissor = scissor;
            }
            void Panel::onDraw(RenderTarget& target, RenderStates states) {
                if (disableScissor) {
                    #ifndef VULKAN
                    glCheck(glEnable(GL_SCISSOR_TEST));
                    glCheck(glScissor(getPosition().x, getWindow().getSize().y - (getPosition().y + getSize().y), getSize().x, getSize().y));
                    #endif
                }
                rect.setPosition(getPosition());
                rect.setSize(getSize());
                target.draw(rect, states);
                for (unsigned int i = 0; i < sprites.size(); i++) {
                    target.draw(sprites[i], states);
                }
                for (unsigned int i = 0; i < shapes.size(); i++) {
                    target.draw(*shapes[i], states);
                }
            }
            void Panel::drawOn(RenderTarget& target, RenderStates states) {
                if (scrollX || scrollY) {
                    corner.setFillColor(sf::Color::Red);
                    target.draw(corner, states);
                }
                if (scrollX) {
                    vertScrollBar.setFillColor(sf::Color::Red);
                    target.draw(vertScrollBar, states);
                }
                if (scrollY) {
                    horScrollBar.setFillColor(sf::Color::Red);
                    target.draw(horScrollBar, states);
                }
                if (disableScissor) {
                    #ifndef VULKAN
                    glCheck(glDisable(GL_SCISSOR_TEST));
                    #endif
                }
            }
            void Panel::setBorderThickness(float thickness) {
                rect.setOutlineThickness(thickness);
            }
            void Panel::setBorderColor(sf::Color color) {
                rect.setOutlineColor(color);
            }
            void Panel::addShape(Shape* shape) {
                shapes.push_back(shape);
            }
            bool Panel::isPointInside(math::Vec3f point) {
                physic::BoundingBox bb(getPosition().x, getPosition().y, 0, getSize().x, getSize().y, 0);
                return bb.isPointInside(point);
            }
            void Panel::onUpdate(RenderWindow* window, window::IEvent& event) {
                LightComponent::onUpdate(window, event);
                if (&getWindow() == window) {
                    if (event.type == window::IEvent::MOUSE_MOTION_EVENT)
                        mousePos = math::Vec3f(event.mouseMotion.x, event.mouseMotion.y, 0);
                }
            }
            void Panel::clearDrawables() {
                sprites.clear();
                shapes.clear();
            }
            math::Vec3f Panel::getDeltas() {
                if(moveComponents && getChildren().size() > 0) {
                    return math::Vec3f(getChildren()[0]->getPosition().x - getPosition().x, getChildren()[0]->getPosition().y - getPosition().y, 0);
                } else if (sprites.size() > 0) {
                    //std::cout<<"sprite pos : "<<sprites[0].getPosition()<<"pan pos : "<<getPosition()<<"delta : "<<deltas<<std::endl;
                    return math::Vec3f(math::Math::abs(sprites[0].getPosition().x - getPosition().x - deltas.x), math::Math::abs(sprites[0].getPosition().y - getPosition().y - deltas.y), 0);
                } else if (shapes.size() > 0) {
                    return math::Vec3f(shapes[0]->getPosition().x - getPosition().x, shapes[0]->getPosition().y - getPosition().y, 0);
                } else {
                    return math::Vec3f(0, 0, 0);
                }
            }
        }
    }
}
