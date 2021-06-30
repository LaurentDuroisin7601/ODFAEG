#include "../../../../include/odfaeg/Graphics/GUI/tabPane.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            TabPane::TabPane(RenderWindow& window, math::Vec3f position, math::Vec3f size) : LightComponent(window,position,size, size * 0.5f) {
                core::Action a (core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Right);
                core::Command cmd1(a, core::FastDelegate<bool>(&TabPane::isOnXScroll, this),core::FastDelegate<void>(&TabPane::moveXItems, this));
                getListener().connect("scrollXMove", cmd1);
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y, 0);
                oldMouseX = mousePos.x;
                maxSizeX = 0;
            }
            void TabPane::addTab (Panel* panel, std::string tabName, const Font& font) {
                if (getParent() != nullptr) {
                    getParent()->recomputeSize();
                }
                float tabXPos = getPosition().x;
                maxSizeX = 0;
                for (unsigned int i = 0; i < getChildren().size(); i++) {
                    if (dynamic_cast<Label*>(getChildren()[i]) != nullptr) {
                        tabXPos += getChildren()[i]->getSize().x;
                        maxSizeX = tabXPos - getPosition().x + 100;
                    }
                }
                //std::cout<<"max size : "<<maxSizeX<<std::endl<<"size : "<<getSize().x<<std::endl;
                if (maxSizeX > getSize().x) {
                    unsigned int scrollXSize = (getSize().x) / maxSizeX * (getSize().x);
                    vertScrollBar = RectangleShape(math::Vec3f(scrollXSize, 10, 0));
                    vertScrollBar.setPosition(math::Vec3f(getPosition().x, getPosition().y + 25, 0));
                    vertScrollBar.setFillColor(sf::Color::Red);
                    scrollX = true;
                } else {
                    scrollX = false;
                }
                Label* lTabName = new Label(getWindow(),math::Vec3f(tabXPos,getPosition().y+10,0),math::Vec3f(100,15,0),&font,tabName,15);
                lTabName->setForegroundColor(sf::Color::Black);
                lTabName->setBorderColor(sf::Color::Red);
                lTabName->setBorderThickness(1.f);
                lTabName->setPriority(-1);
                core::Action a (core::Action::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd (a, core::FastDelegate<bool>(&Label::isMouseInside, lTabName), core::FastDelegate<void>(&TabPane::onTabChanged, this, panel));
                lTabName->getListener().connect("ChangeTab", cmd);
                lTabName->setParent(this);
                panel->setParent(this);
                addChild(lTabName);
                addChild(panel);
                panel->recomputeSize();
                panel->setScissorDisable(false);
                for (unsigned int i = 2; i < getChildren().size(); i++) {
                    if (dynamic_cast<Panel*> (getChildren()[i]) != nullptr) {
                        getChildren()[i]->setVisible(false);
                        getChildren()[i]->setEventContextActivated(false);
                    }
                }
                setName("TAB_PANE");
                selectedTab = static_cast<Label*>(getChildren()[0]);
            }
            bool TabPane::isOnXScroll() {
                math::Vec3f mousePos (window::IMouse::getPosition(getWindow()).x, window::IMouse::getPosition(getWindow()).y, 0);
                mouseDeltaX = mousePos.x - oldMouseX;
                oldMouseX = mousePos.x;
                if (scrollX) {
                    physic::BoundingBox bb(vertScrollBar.getPosition().x, vertScrollBar.getPosition().y, 0, vertScrollBar.getSize().x, vertScrollBar.getSize().y, 0);
                    return bb.isPointInside(mousePos);
                }
                return false;
            }
            void TabPane::moveXItems() {
                if (mouseDeltaX > 0 && vertScrollBar.getPosition().x + vertScrollBar.getSize().x + mouseDeltaX <= getPosition().x + getSize().x) {
                    vertScrollBar.move(math::Vec3f(mouseDeltaX, 0, 0));
                    for (unsigned int i = 0; i < getChildren().size(); i++) {
                        if (dynamic_cast<Label*>(getChildren()[i])) {
                            getChildren()[i]->move(math::Vec3f(-(maxSizeX / (getSize().x) * mouseDeltaX), 0, 0));
                        }
                    }
                } else if (mouseDeltaX < 0 && vertScrollBar.getPosition().x +  mouseDeltaX >= getPosition().x) {
                    vertScrollBar.move(math::Vec3f(mouseDeltaX, 0, 0));
                    for (unsigned int i = 0; i < getChildren().size(); i++) {
                        if (dynamic_cast<Label*>(getChildren()[i])) {
                            getChildren()[i]->move(math::Vec3f(-(maxSizeX / (getSize().x) * mouseDeltaX), 0, 0));
                        }
                    }
                }
            }
            void TabPane::onTabChanged(Panel* panel) {
                panel->setVisible(true);
                panel->setEventContextActivated(true);
                for (unsigned int i = 0; i < getChildren().size(); i++) {
                    if (dynamic_cast<Panel*> (getChildren()[i]) != nullptr && getChildren()[i] != panel) {
                        getChildren()[i]->setVisible(false);
                        getChildren()[i]->setEventContextActivated(false);
                    } else if (getChildren()[i] == panel) {
                        selectedTab = static_cast<Label*>(getChildren()[i-1]);
                    }
                }
            }
            std::string TabPane::getSelectedTab() {
                return selectedTab->getText();
            }
            void TabPane::onDraw(RenderTarget& target, RenderStates states) {


            }
            void TabPane::drawOn(RenderTarget& target, RenderStates states) {
                if (scrollX) {
                    target.draw(vertScrollBar, states);
                }
            }
        }
    }
}
