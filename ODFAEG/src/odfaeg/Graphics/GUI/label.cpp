#include "../../../../include/odfaeg/Graphics/GUI/label.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Label::Label (RenderWindow& window, math::Vec3f position, math::Vec3f size, const Font* font, std::string t, unsigned int charSize) :
            LightComponent (window, position, size, size * 0.5f) {
                text.setFont(*font);
                text.setString(t);
                //text.setSize(size);
                text.setCharacterSize(charSize);
                rect = RectangleShape (size);
                background = sf::Color::Black;
                text.setColor(sf::Color::Black);
                rect.setFillColor(background);
                mousePos = math::Vec3f(0, 0, 0);
            }
            void Label::setForegroundColor(sf::Color color) {
                text.setColor(color);
            }
            sf::Color Label::getForegroundColor() {
                return text.getColor();
            }
            void Label::setBackgroundColor(sf::Color color) {
                this->background = color;
            }
            sf::Color Label::getBackgroundColor() {
                return background;
            }
            void Label::clear() {
                if (background != rect.getFillColor()) {
                    rect.setFillColor(background);
                }
            }
            void Label::onDraw(RenderTarget& target, RenderStates states) {
                text.setPosition(getPosition());
                rect.setPosition(getPosition());
                //text.setSize(getSize());
                rect.setSize(getSize());
                //std::cout<<"sizes : "<<text.getSize()/*<<rect.getSize()*/;
                target.draw(rect);
                target.draw(text);
            }
            void Label::setText(std::string t) {
                text.setString(t);
            }
            sf::String Label::getText() {
                return text.getString();
            }
            bool Label::isMouseInside() {
                physic::BoundingBox bb(getPosition().x, getPosition().y, 0, getSize().x, getSize().y, 0);
                return bb.isPointInside(mousePos);
            }
            void Label::setBorderColor(sf::Color color) {
                rect.setOutlineColor(color);
            }
            void Label::setBorderThickness(float tickness) {
                rect.setOutlineThickness(tickness);
            }
            void Label::onUpdate(RenderWindow* window, window::IEvent& event) {
                if (&getWindow() == window) {
                    if (event.type == window::IEvent::MOUSE_MOTION_EVENT)
                        mousePos = math::Vec3f(event.mouseMotion.x, event.mouseMotion.y, 0);
                }
            }
            void Label::onEventPushed (window::IEvent event, RenderWindow& window) {

            }
        }
    }
}
