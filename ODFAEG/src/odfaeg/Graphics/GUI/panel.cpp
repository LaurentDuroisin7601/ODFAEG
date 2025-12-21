#include "../../../../include/odfaeg/Graphics/GUI/panel.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Panel::Panel(RenderWindow& window, math::Vec3f position, math::Vec3f size, int priority, int eventPriority, LightComponent* parent) :
            LightComponent (window, position, size, size * 0.5f, priority, eventPriority, parent) {
                rect = RectangleShape(size);
                background = Color::Black;
                rect.setFillColor(background);
                scrollX = scrollY = false;
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y(), 0);
                oldMouseX = mousePos.x();
                oldMouseY = mousePos.y();
                core::Action a (core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Right);
                core::Command cmd1(a, core::FastDelegate<bool>(&Panel::isOnXScroll, this),core::FastDelegate<void>(&Panel::moveXItems, this));
                getListener().connect("scrollXMove", cmd1);
                core::Command cmd2(a, core::FastDelegate<bool>(&Panel::isOnYScroll, this),core::FastDelegate<void>(&Panel::moveYItems, this));
                getListener().connect("scrollYMove", cmd2);
                maxSize = math::Vec3f(0, 0, 0);
                disableScissor = true;
                moveComponents = true;
                deltas = math::Vec3f(0, 0, 0);
            }
            bool Panel::isOnXScroll() {
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y(), 0);
                mouseDeltaX = mousePos.x() - oldMouseX;
                oldMouseX = mousePos.x();
                if (scrollX) {
                    physic::BoundingBox bb(vertScrollBar.getPosition().x(), vertScrollBar.getPosition().y(), 0, vertScrollBar.getSize().x(), vertScrollBar.getSize().y(), 0);
                    return bb.isPointInside(mousePos);
                }
                return false;
            }
            bool Panel::isOnYScroll() {
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x(), window::IMouse::getPosition(getWindow()).y(), 0);
                mouseDeltaY = mousePos.y() - oldMouseY;
                oldMouseY  = mousePos.y();
                if (scrollY) {
                    physic::BoundingBox bb(horScrollBar.getPosition().x(), horScrollBar.getPosition().y(), 0, horScrollBar.getSize().x(), horScrollBar.getSize().y(), 0);
                    return bb.isPointInside(mousePos);
                }
                return false;
            }
            void Panel::setScrollPosition(math::Vec3f position) {
                // Clamp la position dans les bornes
                float maxX = maxSize.x() /*- getSize().x()*/;
                float maxY = maxSize.y() /*- getSize().y()*/;
                float scrollX = math::Math::clamp(position.x() / maxX, 0.f, 1.f);
                float scrollY = math::Math::clamp(position.y() / maxY, 0.f, 1.f);
                std::cout<<"maxs in % : "<<scrollX<<","<<scrollY<<","<<position.y() / maxY<<std::endl;
                math::Vec2f scrollDeltas;
                scrollDeltas.x() = math::Math::clamp(scrollX * (getSize().x() - 10 - vertScrollBar.getSize().x()), 0.f, (getSize().x() - 10 - vertScrollBar.getSize().x()));
                scrollDeltas.y() = math::Math::clamp(scrollY * (getSize().y() - 10 - horScrollBar.getSize().y()), 0.f, (getSize().y() - 10 - vertScrollBar.getSize().y()));
                //std::cout<<"start positions : "<<vertScrollBar.getPosition()<<std::endl<<horScrollBar.getPosition()<<std::endl;
                // Positionner directement les scrollbars
                vertScrollBar.setPosition(math::Vec3f(getPosition().x() + scrollDeltas.x(), vertScrollBar.getPosition().y(), getPosition().z()+500));
                horScrollBar.setPosition(math::Vec3f(horScrollBar.getPosition().x(), getPosition().y() + scrollDeltas.y(), getPosition().z()+500));

                deltas.x() = math::Math::clamp(position.x() * maxX, 0.f, maxX);
                deltas.y() = math::Math::clamp(position.y() * maxY, 0.f, maxY);
                //std::cout<<"final positions : "<<vertScrollBar.getPosition()<<std::endl<<horScrollBar.getPosition()<<std::endl;
                // Replacer les enfants en fonction de deltas
                float offsetX = -(deltas.x() * maxSize.x() / (getSize().x() - 10));
                float offsetY = -(deltas.y() * maxSize.y() / (getSize().y() - 10));
                for (auto* child : getChildren()) {
                    child->setPosition(child->getBasePosition() + math::Vec3f(offsetX, offsetY, 0));
                }
            }
            void Panel::moveXItems() {
                if (mouseDeltaX > 0 && vertScrollBar.getPosition().x() + vertScrollBar.getSize().x() + mouseDeltaX <= getPosition().x() + getSize().x() - 10) {
                    vertScrollBar.move(math::Vec3f(mouseDeltaX, 0, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(-(maxSize.x() / (getSize().x() - 10) * mouseDeltaX), 0, 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(-(maxSize.x() / (getSize().x() - 10) * mouseDeltaX), 0, 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(-(maxSize.x() / (getSize().x() - 10) * mouseDeltaX), 0, 0));
                    }
                    deltas.x() += (maxSize.x() / (getSize().x() - 10) * mouseDeltaX);
                } else if (mouseDeltaX < 0 && vertScrollBar.getPosition().x() +  mouseDeltaX >= getPosition().x()) {
                    vertScrollBar.move(math::Vec3f(mouseDeltaX, 0, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(-(maxSize.x() / (getSize().x() - 10) * mouseDeltaX), 0, 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(-(maxSize.x() / (getSize().x() - 10) * mouseDeltaX), 0, 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(-(maxSize.x() / (getSize().x() - 10) * mouseDeltaX), 0, 0));
                    }
                    deltas.x() += (maxSize.x() / (getSize().x() - 10) * mouseDeltaX);
                }
            }
            void Panel::moveYItems() {
                if (mouseDeltaY > 0 && horScrollBar.getPosition().y() + horScrollBar.getSize().y() + mouseDeltaY <= getPosition().y() + getSize().y() - 10) {
                    horScrollBar.move(math::Vec3f(0, mouseDeltaY, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(0, -(maxSize.y() / (getSize().y() - 10) * mouseDeltaY), 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(0, -(maxSize.y() / (getSize().y() - 10) * mouseDeltaY), 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(0, -(maxSize.y() / (getSize().y() - 10) * mouseDeltaY), 0));
                    }
                    deltas.y() += (maxSize.y() / (getSize().y() - 10) * mouseDeltaY);
                } else if (mouseDeltaY < 0 && horScrollBar.getPosition().y() +  mouseDeltaY >= getPosition().y()) {
                    horScrollBar.move(math::Vec3f(0, mouseDeltaY, 0));
                    if (moveComponents) {
                        for (unsigned int i = 0; i < getChildren().size(); i++) {
                            getChildren()[i]->move(math::Vec3f(0, -(maxSize.y() / (getSize().y() - 10) * mouseDeltaY), 0));
                        }
                    }
                    for (unsigned int i = 0; i < sprites.size(); i++) {
                        sprites[i].move(math::Vec3f(0, -(maxSize.y() / (getSize().y() - 10) * mouseDeltaY), 0));
                    }
                    for (unsigned int i = 0; i < shapes.size(); i++) {
                        shapes[i]->move(math::Vec3f(0, -(maxSize.y() / (getSize().y() - 10) * mouseDeltaY), 0));
                    }
                    deltas.y() += (maxSize.y() / (getSize().y() - 10) * mouseDeltaY);
                }
            }
            void Panel::setMoveComponents(bool moveComponents) {
                this->moveComponents = moveComponents;
            }
            void Panel::updateScrolls() {

                std::vector<LightComponent*> children = getChildren();
                ////////std::cout<<"update children"<<std::endl;
                float dx = deltas.x();
                float dy = deltas.y();
                float vScrollX = (vertScrollBar.getPosition().x() == 0) ? getPosition().x() : vertScrollBar.getPosition().x();
                float hScrollY = (horScrollBar.getPosition().y() == 0) ? getPosition().y() : horScrollBar.getPosition().y();

                maxSize = math::Vec3f(0, 0, 0);

                for (unsigned int i = 0; i < children.size(); i++) {
                    //std::cout<<"positions : "<<children[i]->getPosition().y()<<","<<getPosition().y()<<","<<dy<<std::endl;
                    if (children[i]->getPosition().y() - getPosition().y() + dy + children[i]->getSize().y() > maxSize.y())
                        maxSize.y() = children[i]->getPosition().y() - getPosition().y() + dy + children[i]->getSize().y();
                    if (children[i]->getPosition().x() - getPosition().x() + dx + children[i]->getSize().x() > maxSize.x())
                        maxSize.x() = children[i]->getPosition().x() - getPosition().x() + dx + children[i]->getSize().x();
                }
                ////////std::cout<<"update sprites"<<std::endl;
                for (unsigned int i = 0; i < sprites.size(); i++) {
                    if (sprites[i].getPosition().y() - getPosition().y() + sprites[i].getSize().y() > maxSize.y())
                        maxSize.y() = sprites[i].getPosition().y() - getPosition().y() + sprites[i].getSize().y();
                    if(sprites[i].getPosition().x() - getPosition().x() + sprites[i].getSize().x() > maxSize.x())
                        maxSize.x() = sprites[i].getPosition().x() - getPosition().x() + sprites[i].getSize().x();
                }
                ////////std::cout<<"update shapes"<<std::endl;
                for (unsigned int i = 0; i < shapes.size(); i++) {
                    if (shapes[i]->getPosition().y() + shapes[i]->getSize().y() > maxSize.y())
                        maxSize.y() = shapes[i]->getPosition().y() + shapes[i]->getSize().y();
                    if(shapes[i]->getPosition().x() + shapes[i]->getSize().x() > maxSize.x())
                        maxSize.x() = shapes[i]->getPosition().x() + shapes[i]->getSize().x();
                }
                ////////std::cout<<"corner"<<std::endl;
                if (maxSize.x() > getSize().x() || maxSize.y() > getSize().y()) {
                    corner = RectangleShape(math::Vec3f(10, 10, 0));
                    corner.setPosition(math::Vec3f(getPosition().x() + getSize().x() - 10 - rect.getOutlineThickness(), getPosition().y() + getSize().y() - 10 - rect.getOutlineThickness(), getPosition().z()+200));
                }
                ////////std::cout<<"vert scroll"<<std::endl;
                if (maxSize.x() > getSize().x()) {
                    unsigned int scrollXSize = (getSize().x() - 10) / maxSize.x() * (getSize().x() - 10);
                    vertScrollBar = RectangleShape(math::Vec3f(scrollXSize, 10, 0));
                    vertScrollBar.setPosition(math::Vec3f(vScrollX, getPosition().y() + getSize().y() - 10 - rect.getOutlineThickness(), getPosition().z()+500));
                    scrollX = true;
                } else {
                    scrollX = false;
                }
                ////////std::cout<<"horizontal scroll"<<std::endl;

                if (maxSize.y() > getSize().y()) {
                    unsigned int scrollYSize = (getSize().y() - 10) / maxSize.y() * (getSize().y() - 10);

                    horScrollBar = RectangleShape(math::Vec3f(10, scrollYSize, 0));
                    horScrollBar.setPosition(math::Vec3f(getPosition().x() + getSize().x() - 10 - rect.getOutlineThickness(), hScrollY, getPosition().z()+500));
                    scrollY = true;
                } else {
                    scrollY = false;
                }
                ////////std::cout<<"end update scroll"<<std::endl;
            }
            void Panel::addChild(LightComponent* child) {
                ////////std::cout<<"add child"<<std::endl;
                LightComponent::addChild(child);
                child->setBasePosition(child->getPosition());
                ////////std::cout<<"recompute size"<<std::endl;
                recomputeSize();
                ////////std::cout<<"update scrolls"<<std::endl;
                updateScrolls();
                ////////std::cout<<"end add child"<<std::endl;
            }
            void Panel::onSizeRecomputed() {

            }
            void Panel::removeAll() {
                LightComponent::removeAll();
                maxSize = math::Vec3f(0, 0, 0);
                vertScrollBar.setPosition(math::Vec3f(getPosition().x(), getPosition().y() + getSize().y() - 10, 0));
                horScrollBar.setPosition(math::Vec3f(getPosition().x() + getSize().x() - 10, getPosition().y(), 0));
                scrollY = scrollX = false;
            }
            void Panel::setBackgroundColor(Color background) {
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
                    if (sprites[i].getPosition().x() - getPosition().x() + sprites[i].getSize().x() > deltas.x())
                        deltas.x() = sprites[i].getPosition().x() - getPosition().x() + sprites[i].getSize().x();
                    if (sprites[i].getPosition().y() - getPosition().y() + sprites[i].getSize().y() > deltas.y())
                        deltas.y() = sprites[i].getPosition().y() - getPosition().y() + sprites[i].getSize().y();
                }
            }
            void Panel::setScissorDisable(bool scissor) {
                disableScissor = scissor;
            }
            void Panel::onDraw(RenderTarget& target, RenderStates states) {
                if (disableScissor) {
                    #ifndef VULKAN
                    glCheck(glEnable(GL_SCISSOR_TEST));
                    glCheck(glScissor(getPosition().x(), getWindow().getSize().y() - (getPosition().y() + getSize().y()), getSize().x(), getSize().y()));
                    #endif
                }
                //std::cout<<"draw panel"<<std::endl;


                rect.setPosition(getPosition());
                rect.setSize(getSize());
                target.draw(rect, states);
                #ifdef VULKAN
                target.submit(false);
                target.getScissors()[0].offset = {getPosition().x(), getPosition().y()};
                //std::cout<<"offsets : "<<target.getScissors()[0].offset.x<<","<<target.getScissors()[0].offset.y<<std::endl;
                target.getScissors()[0].extent = {getSize().x(), getSize().y()};
                #endif
                for (unsigned int i = 0; i < sprites.size(); i++) {
                    target.draw(sprites[i], states);
                }
                for (unsigned int i = 0; i < shapes.size(); i++) {
                    target.draw(*shapes[i], states);
                }
            }
            void Panel::drawOn(RenderTarget& target, RenderStates states) {

                #ifdef VULKAN
                target.getScissors()[0].offset = {0, 0};
                target.getScissors()[0].extent = {target.getSize().x(), target.getSize().y()};
                #endif // VULKAN
                if (scrollX || scrollY) {
                    corner.setFillColor(Color::Red);
                    target.draw(corner, states);
                }
                if (scrollX) {
                    vertScrollBar.setFillColor(Color::Red);
                    target.draw(vertScrollBar, states);
                }
                if (scrollY) {
                    horScrollBar.setFillColor(Color::Red);
                    target.draw(horScrollBar, states);

                }
                #ifdef VULKAN
                if (scrollX || scrollY) {
                    //std::cout<<"submit panel : "<<std::endl;
                    target.submit(false);
                }
                #endif // VULKAN

                if (disableScissor) {
                    #ifndef VULKAN
                    glCheck(glDisable(GL_SCISSOR_TEST));
                    #endif
                }
            }
            void Panel::setBorderThickness(float thickness) {
                rect.setOutlineThickness(thickness);
            }
            void Panel::setBorderColor(Color color) {
                rect.setOutlineColor(color);
            }
            void Panel::addShape(Shape* shape) {
                shapes.push_back(shape);
            }
            bool Panel::isPointInside(math::Vec3f point) {
                physic::BoundingBox bb(getPosition().x(), getPosition().y(), 0, getSize().x(), getSize().y(), 0);
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
                return deltas;
            }
        }
    }
}
