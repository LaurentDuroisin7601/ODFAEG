#ifndef ODFAEG_SFML_MOUSE_HPP
#define ODFAEG_SFML_MOUSE_HPP
#include <SFML/Window/Mouse.hpp>
#include "../iMouse.hpp"
#include "sfmlWindowImpl.hpp"
namespace odfaeg {
    namespace window {
        class SFMLMouse {
            public :
            static sf::Mouse::Button odfaegToSfButton(IMouse::Button button);
            static IMouse::Button sfToODFAEGButton(sf::Mouse::Button button);
            static bool isButtonPressed(IMouse::Button button);
            static sf::Vector2i getPosition();
            static sf::Vector2i getPosition(const SFMLWindowImpl& window);
        };
    }
}
#endif
