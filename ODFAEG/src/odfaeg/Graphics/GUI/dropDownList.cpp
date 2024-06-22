#include "../../../../include/odfaeg/Graphics/GUI/dropDownList.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            DropDownList::DropDownList(RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font* font, std::string t, unsigned int charSize) :
            LightComponent (window, position, size, size * 0.5f, -1),
            font(font) {
                background = sf::Color::Black;
                if (t != "") {
                    Label* label = new Label(window, position, math::Vec3f(size.x - 50, size.y, 0), font, t, charSize);
                    label->setForegroundColor(sf::Color::Black);
                    label->setEventContextActivated(false);
                    items.push_back(label);
                    nbItems = 1;
                    selectedItem = items[0];
                    core::Action a(core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Left);
                    core::Command cmd (a, core::FastDelegate<bool>(&Label::isMouseInside, items[0]), core::FastDelegate<void>(&DropDownList::onItemSelected, this, items[0]));
                    label->getListener().connect(items[0]->getText(), cmd);
                    selectedItemPos = items[0]->getPosition();
                } else {
                    nbItems = 0;
                    selectedItem = nullptr;
                }
                rect = RectangleShape(math::Vec3f(size.x, size.y, 0));
                shape = ConvexShape(3);
                shape.setPoint(0, sf::Vector3f(size.x - 25, size.y, 0));
                shape.setPoint(1, sf::Vector3f(size.x - 50, 0, 0));
                shape.setPoint(2, sf::Vector3f(size.x, 0, 0));
                bp = physic::BoundingPolyhedron(math::Vec3f(position.x + size.x - 25, position.y + 50, 0), math::Vec3f(position.x + size.x - 50, position.y, 0), math::Vec3f(position.x + size.x, position.y, 0),true);
                core::Action a(core::Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd(a, core::FastDelegate<bool> (&DropDownList::isMouseOnTriangle, this), core::FastDelegate<void>(&DropDownList::onTriangleClicked, this));
                getListener().connect("ITEMSELECTED"+t, cmd);
                dropDown = valueChanged = false;
            }
            bool DropDownList::isValueChanged() {
                bool b = valueChanged;
                valueChanged = false;
                return b;
            }
            std::string DropDownList::getSelectedItem() {
                return (selectedItem != nullptr) ? selectedItem->getText() : "";
            }
            void DropDownList::onItemSelected(Label* label) {
                /*if (getName() == "FUNCTION")
                    std::cout<<"function ! "<<std::endl;*/
                if (label != selectedItem)
                    valueChanged = true;
                selectedItem = label;
                selectedItemPos = label->getPosition();
                selectedItem->setPosition(getPosition());
                dropDown = false;
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->setEventContextActivated(false);
                }
            }
            bool DropDownList::isMouseOnTriangle() {
                /*if (getName() == "FUNCTION") {
                    if (bp.isPointInside(mousePos)) {
                        std::cout<<"inside : "<<std::endl;
                    }
                    //std::cout<<"mouse pos : "<<mousePos<<std::endl;
                }*/
                return bp.isPointInside(mousePos);
            }
            void DropDownList::onTriangleClicked() {
                selectedItem->setPosition(selectedItemPos);
                dropDown = true;
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->setEventContextActivated(true);
                }
            }
            void DropDownList::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void DropDownList::addItem(std::string t, unsigned int charSize) {
                /*if (getName() == "POINTERTYPE")
                    std::cout<<"add item : "<<t<<std::endl;*/
                Label* label = new Label (getWindow(), getPosition(), math::Vec3f(getSize().x - 50, getSize().y, 0), font, t, charSize);
                label->setPosition(math::Vec3f(getPosition().x, getPosition().y + getSize().y * nbItems, 0));
                label->setForegroundColor(sf::Color::Black);
                items.push_back(label);
                nbItems++;
                selectedItem = items[0];
                label->setEventContextActivated(false);
                core::Action a(core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Left);
                core::Command cmd (a, core::FastDelegate<bool>(&Label::isMouseInside, items.back()), core::FastDelegate<void>(&DropDownList::onItemSelected, this, items.back()));
                label->getListener().connect(items.back()->getText().toAnsiString(), cmd);
            }
            void DropDownList::removeAllItems() {
                items.clear();
                /*if (getName() == "FUNCTION")
                    std::cout<<"size : "<<items.size()<<std::endl;*/
                selectedItem = nullptr;
            }
            void DropDownList::onDraw(RenderTarget& target, RenderStates states) {
                rect.setPosition(getPosition());
                rect.setSize(getSize());
                shape = ConvexShape(3);
                shape.setPoint(0, sf::Vector3f(getSize().x - 25, getSize().y, 0));
                shape.setPoint(1, sf::Vector3f(getSize().x - 50, 0, 0));
                shape.setPoint(2, sf::Vector3f(getSize().x, 0, 0));
                shape.setPosition(getPosition());
                std::vector<math::Vec3f> points;
                points.resize(shape.getPointCount());
                for (unsigned int i = 0; i < shape.getPointCount(); i++) {
                    sf::Vector3f position = shape.getPoint(i);
                    points[i] = shape.getTransform().transform(math::Vec3f(position.x, position.y, position.z));
                }
                bp = physic::BoundingPolyhedron(points[1], points[0], points[2],true);
                target.draw(rect, states);
                if (!dropDown && selectedItem != nullptr) {
                    /*if (getName() == "FUNCTION")
                        std::cout<<"draw items"<<std::endl;*/
                    selectedItem->setPosition(getPosition());
                    selectedItem->setSize(math::Vec3f(getSize().x - 50, getSize().y, getSize().z));
                    target.draw(*selectedItem, states);
                } else {
                    /*if (getName() == "FUNCTION")
                        std::cout<<"draw items"<<std::endl;*/
                    for (unsigned int i = 0; i < items.size(); i++) {
                        items[i]->setPosition(math::Vec3f(getPosition().x, getPosition().y + getSize().y * i, getPosition().z));
                        items[i]->setSize(math::Vec3f(getSize().x - 50, getSize().y, getSize().z));
                        target.draw(*items[i], states);
                    }
                }
                target.draw(shape, states);

            }
            void DropDownList::onEventPushed(window::IEvent event, RenderWindow& window) {
                if(&window == &getWindow())
                    getListener().pushEvent(event);
            }
            void DropDownList::onUpdate(RenderWindow* window, window::IEvent& event) {
                if (&getWindow() == window
                    && event.type == window::IEvent::MOUSE_BUTTON_EVENT
                    && event.mouseButton.button == window::IEvent::BUTTON_EVENT_PRESSED
                    && event.mouseButton.button == window::IMouse::Left) {
                        mousePos = math::Vec3f(event.mouseButton.x, event.mouseButton.y, 0);
                }
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->onUpdate(window, event);
                }
            }
            void DropDownList::processEvents() {
                if (isEventContextActivated()) {
                    getListener().processEvents();
                    for (unsigned int i = 0; i < items.size(); i++) {
                        if (items[i]->isEventContextActivated()) {
                            items[i]->processEvents();
                        }
                    }
                }
            }
            bool DropDownList::isNotDroppedDown() {
                return !dropDown;
            }
            bool DropDownList::isDroppedDown() {
                return dropDown;
            }
            DropDownList::~DropDownList() {
                for (unsigned int i = 0; i < items.size(); i++) {
                    delete items[i];
                }
            }
        }
    }
}
