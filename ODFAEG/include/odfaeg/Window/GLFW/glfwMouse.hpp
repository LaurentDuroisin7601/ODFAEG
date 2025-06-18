#ifndef GLFW_MOUSE_HPP
#define GLFW_MOUSE_HPP
#include "../iMouse.hpp"
#include <GLFW/glfw3.h>
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API GLFWMouse {
            public :
            static int odfaegToGlfwButton(IMouse::Button button);
            static IMouse::Button glfwToODFAEGButton(int button);
            static bool isButtonPressed(GLFWwindow* window, IMouse::Button button);
            static sf::Vector2i getPosition();
            static sf::Vector2i getPosition(GLFWwindow* window);
        };
    }
}
#endif // GLFW_MOUSE_HPP
