#include "../../../../include/odfaeg/Graphics/GUI/dropDownList.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            DropDownList::DropDownList(RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font* font, std::string t, unsigned int charSize) :
            LightComponent (window, position, size, size * 0.5f, -1),
            font(font) {
                background = Color::Black;
                if (t != "") {
                    Label* label = new Label(window, math::Vec3f(getPosition().x(), getPosition().y() + getSize().y(), getPosition().z()), math::Vec3f(size.x() - 50, size.y(), 0), font, t, charSize);
                    label->setForegroundColor(Color::Black);
                    label->setEventContextActivated(false);
                    items.push_back(label);
                    filteredItems.push_back(label);
                    nbItems = 1;
                    selectedItem = new TextArea(math::Vec3f(getPosition().x(), getPosition().y(), getPosition().z()), math::Vec3f(size.x() - 50, size.y(), 0), font, items[0]->getText(), window);
                    core::Action a(core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Left);
                    core::Command cmd (a, core::FastDelegate<bool>(&Label::isMouseInside, items[0]), core::FastDelegate<void>(&DropDownList::onItemSelected, this, items[0]));
                    label->getListener().connect(items[0]->getText(), cmd);
                    selectedItemPos = items[0]->getPosition();
                    core::Command cmd2(core::FastDelegate<bool>(&TextArea::isTextChanged, selectedItem),core::FastDelegate<void>(&DropDownList::onTextChanged, this));
                    selectedItem->getListener().connect("DropDownTextChanged", cmd2);
                    selectedItem->setName("DropDownTA");
                } else {
                    nbItems = 0;
                    selectedItem = nullptr;
                }
                rect = RectangleShape(math::Vec3f(size.x(), size.y(), 0));
                shape = ConvexShape(3);
                shape.setPoint(0, math::Vec3f(size.x() - 25, size.y(), 0));
                shape.setPoint(1, math::Vec3f(size.x() - 50, 0, 0));
                shape.setPoint(2, math::Vec3f(size.x(), 0, 0));
                bp = physic::BoundingPolyhedron(math::Vec3f(position.x() + size.x() - 25, position.y() + 50, 0), math::Vec3f(position.x() + size.x() - 50, position.y(), 0), math::Vec3f(position.x() + size.x(), position.y(), 0),true);
                core::Action a(core::Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, window::IMouse::Left);
                core::Command cmd(a, core::FastDelegate<bool> (&DropDownList::isMouseOnTriangle, this), core::FastDelegate<void>(&DropDownList::onTriangleClicked, this));
                getListener().connect("ITEMSELECTED"+t, cmd);

                dropDown = valueChanged = stateChanged = false;
                scrollableSize = physic::BoundingBox(getPosition().x(), getPosition().y() + getSize().y(), getPosition().z()+150, size.x() - 50, size.y(), 0);
                //getListener().launch();
            }
            bool DropDownList::isValueChanged() {
                bool b = valueChanged;
                if (valueChanged)
                    //////std::cout<<"value changed"<<std::endl;
                valueChanged = false;
                return b;
            }
            std::string DropDownList::getSelectedItem() {
                return (selectedItem != nullptr) ? selectedItem->getText() : "";
            }
            void DropDownList::onItemSelected(Label* label) {
                /*if (getName() == "FUNCTION")
                    //////std::cout<<"function ! "<<std::endl;*/

                if (label->getText() != selectedItem->getText()) {
                    //////std::cout<<"change value changed"<<std::endl;
                    valueChanged = true;
                }
                selectedItem->setText(label->getText());
                selectedItemPos = label->getPosition();
                dropDown = false;
                for (unsigned int i = 0; i < items.size(); i++) {
                    filteredItems[i]->setEventContextActivated(false);
                }
            }
            bool DropDownList::isMouseOnTriangle() {
                /*if (getName() == "FUNCTION") {
                    if (bp.isPointInside(mousePos)) {
                        //////std::cout<<"inside : "<<std::endl;
                    }
                    ////////std::cout<<"mouse pos : "<<mousePos<<std::endl;
                }*/
                return bp.isPointInside(mousePos);
            }
            void DropDownList::onTriangleClicked() {
                selectedItem->setPosition(selectedItemPos);
                dropDown = true;
                filteredItems.clear();
                for (unsigned int i = 0; i < items.size(); i++) {
                    items[i]->setEventContextActivated(true);
                    filteredItems.push_back(items[i]);
                    filteredItems[i]->setPosition(math::Vec3f(getPosition().x(), getPosition().y() + getSize().y() * (i+1), getPosition().z()));
                }
            }
            void DropDownList::clear() {
                if (background != rect.getFillColor())
                    rect.setFillColor(background);
            }
            void DropDownList::addItem(std::string t, unsigned int charSize) {
                /*if (getName() == "POINTERTYPE")
                    //////std::cout<<"add item : "<<t<<std::endl;*/
                nbItems++;
                Label* label = new Label (getWindow(), math::Vec3f(getPosition().x(), getPosition().y() + getSize().y() * nbItems, getPosition().z()), math::Vec3f(getSize().x() - 50, getSize().y(), 0), font, t, charSize);

                label->setForegroundColor(Color::Black);
                items.push_back(label);
                filteredItems.push_back(label);
                if (selectedItem == nullptr) {
                    selectedItem = new TextArea(math::Vec3f(getPosition().x(), getPosition().y(), getPosition().z()), math::Vec3f(getSize().x() - 50, getSize().y(), 0), font, items[0]->getText(), getWindow());
                    core::Command cmd2(core::FastDelegate<bool>(&TextArea::isTextChanged, selectedItem),core::FastDelegate<void>(&DropDownList::onTextChanged, this));
                    selectedItem->getListener().connect("DropDownTextChanged", cmd2);
                    selectedItem->setName("DropDownTA");
                }
                label->setEventContextActivated(false);
                core::Action a(core::Action::EVENT_TYPE::MOUSE_BUTTON_HELD_DOWN, window::IMouse::Left);
                core::Command cmd (a, core::FastDelegate<bool>(&Label::isMouseInside, items.back()), core::FastDelegate<void>(&DropDownList::onItemSelected, this, items.back()));
                label->getListener().connect(items.back()->getText().toAnsiString(), cmd);
                scrollableSize.setSize(scrollableSize.getSize().x(), scrollableSize.getSize().y()+getSize().y(), 0);
            }
            void DropDownList::setSelectedItem(std::string t) {
                for (unsigned int i = 0; i < items.size(); i++) {
                    if (items[i]->getText() == t) {
                        selectedItem->setText(items[i]->getText());
                    }
                }
            }
            void DropDownList::removeAllItems() {
                items.clear();
                /*if (getName() == "FUNCTION")
                    //////std::cout<<"size : "<<items.size()<<std::endl;*/
                selectedItem = nullptr;
            }
            void DropDownList::onDraw(RenderTarget& target, RenderStates states) {

                rect.setPosition(getPosition());
                rect.setSize(getSize());
                shape = ConvexShape(3);
                shape.setPoint(0, math::Vec3f(getSize().x() - 25, getSize().y(), getPosition().z()+100));
                shape.setPoint(1, math::Vec3f(getSize().x() - 50, 0, getPosition().z()+100));
                shape.setPoint(2, math::Vec3f(getSize().x(), 0, getPosition().z()+100));
                shape.setPosition(getPosition());
                std::vector<math::Vec3f> points;
                points.resize(shape.getPointCount());
                for (unsigned int i = 0; i < shape.getPointCount(); i++) {
                    math::Vec3f position = math::Vec3f(shape.getPoint(i).x(), shape.getPoint(i).y(), 0);
                    points[i] = shape.getTransform().transform(math::Vec3f(position.x(), position.y(), position.z()));
                }
                bp = physic::BoundingPolyhedron(points[1], points[0], points[2],true);
                target.draw(rect, states);

                if (!dropDown && selectedItem != nullptr) {
                    /*if (getName() == "FUNCTION")
                        //////std::cout<<"draw items"<<std::endl;*/
                    selectedItem->setPosition(math::Vec3f(getPosition().x(), getPosition().y(), getPosition().z()));
                    selectedItem->setSize(math::Vec3f(getSize().x() - 50, getSize().y(), getSize().z()));
                    target.draw(*selectedItem, states);
                } else {
                    /*if (getName() == "FUNCTION")
                        //////std::cout<<"draw items"<<std::endl;*/
                    selectedItem->setPosition(math::Vec3f(getPosition().x(), getPosition().y(), getPosition().z()));
                    selectedItem->setSize(math::Vec3f(getSize().x() - 50, getSize().y(), getSize().z()));
                    target.draw(*selectedItem, states);
                    for (unsigned int i = 0; i < filteredItems.size(); i++) {
                        /*filteredItems[i]->setPosition(math::Vec3f(getPosition().x(), getPosition().y() + getSize().y() * (i + 1), getPosition().z()+150));
                        filteredItems[i]->setSize(math::Vec3f(getSize().x() - 50, getSize().y(), getSize().z()));*/
                        target.draw(*filteredItems[i], states);
                    }
                }
                target.draw(shape, states);
                #ifdef VULKAN
                target.submit(false);
                #endif
            }
            void DropDownList::onEventPushed(window::IEvent event, RenderWindow& window) {
                if(&window == &getWindow())
                    getListener().pushEvent(event);
                if (selectedItem != nullptr && selectedItem->isEventContextActivated()) {
                    selectedItem->getListener().pushEvent(event);
                    selectedItem->onUpdate(&window, event);
                }
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
                LightComponent::processEvents();
                for (unsigned int i = 0; i < filteredItems.size(); i++) {
                    if (filteredItems[i]->isEventContextActivated()) {
                        filteredItems[i]->processEvents();
                    }
                }
                if (selectedItem != nullptr && selectedItem->isEventContextActivated()) {
                    //std::cout<<"process events"<<std::endl;
                    selectedItem->processEvents();
                }
            }
            bool DropDownList::isNotDroppedDown() {
                bool b = stateChanged;
                if (!dropDown)
                    stateChanged = false;
                return !dropDown && b;
            }
            bool DropDownList::isDroppedDown() {
                bool b = stateChanged;
                if (dropDown)
                    stateChanged = true;
                return dropDown && !b;
            }
            void DropDownList::onTextChanged() {
                filteredItems.clear();
                unsigned int position = 1;
                for (unsigned int i = 0; i < items.size(); i++) {
                    if (items[i]->getText().find(selectedItem->getText()) != std::string::npos) {
                        filteredItems.push_back(items[i]);
                        filteredItems[i]->setPosition(math::Vec3f(getPosition().x(), getPosition().y() + getSize().y() * position, getPosition().z()));
                        position++;
                    }
                }
            }
            DropDownList::~DropDownList() {
                //getListener().stop();
                for (unsigned int i = 0; i < items.size(); i++) {
                    delete items[i];
                }
            }
        }
    }
}
