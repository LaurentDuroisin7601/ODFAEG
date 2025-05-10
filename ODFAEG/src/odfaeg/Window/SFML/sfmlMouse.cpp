#include "../../../../include/odfaeg/Window/SFML/sfmlMouse.hpp"
namespace odfaeg {
    namespace window {
        sf::Mouse::Button SFMLMouse::odfaegToSfButton(IMouse::Button button) {
            switch (button) {
                case IMouse::Left : return sf::Mouse::Left;       ///< The left mouse button
                case IMouse::Right : return sf::Mouse::Right;      ///< The right mouse button
                case IMouse::Middle : return sf::Mouse::Middle;     ///< The middle (wheel) mouse button
                case IMouse::XButton1 : return sf::Mouse::XButton1;   ///< The first extra mouse button
                case IMouse::XButton2 : return sf::Mouse::XButton2;   ///< The second extra mouse button
            }
        }
        IMouse::Button SFMLMouse::sfToODFAEGButton(sf::Mouse::Button button) {
            switch (button) {
                case sf::Mouse::Left : return IMouse::Left;       ///< The left mouse button
                case sf::Mouse::Right : return IMouse::Right;      ///< The right mouse button
                case sf::Mouse::Middle : return IMouse::Middle;     ///< The middle (wheel) mouse button
                case sf::Mouse::XButton1 : return IMouse::XButton1;   ///< The first extra mouse button
                case sf::Mouse::XButton2 : return IMouse::XButton2;   ///< The second extra mouse button
            }
        }
        bool SFMLMouse::isButtonPressed(IMouse::Button button) {
            return sf::Mouse::isButtonPressed(odfaegToSfButton(button));
        }
        sf::Vector2i SFMLMouse::getPosition() {
            return sf::Mouse::getPosition();
        }
        sf::Vector2i SFMLMouse::getPosition(const SFMLWindowImpl& window) {
            return sf::Mouse::getPosition(window);
        }
    }
}
