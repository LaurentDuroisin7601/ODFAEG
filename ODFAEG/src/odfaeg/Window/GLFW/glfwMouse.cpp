#include "../../../../include/odfaeg/Window/GLFW/glfwMouse.hpp"
namespace odfaeg {
    namespace window {
        int GLFWMouse::odfaegToGlfwButton(IMouse::Button button) {
            switch (button) {
                case IMouse::Left : return GLFW_MOUSE_BUTTON_LEFT;       ///< The left mouse button
                case IMouse::Right : return GLFW_MOUSE_BUTTON_RIGHT;      ///< The right mouse button
                case IMouse::Middle : return GLFW_MOUSE_BUTTON_MIDDLE;     ///< The middle (wheel) mouse button
                case IMouse::XButton1 : return GLFW_MOUSE_BUTTON_4;   ///< The first extra mouse button
                case IMouse::XButton2 : return GLFW_MOUSE_BUTTON_5;   ///< The second extra mouse button
            }
        }
        IMouse::Button GLFWMouse::glfwToODFAEGButton(int button) {
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT : return IMouse::Left;       ///< The left mouse button
                case GLFW_MOUSE_BUTTON_RIGHT : return IMouse::Right;      ///< The right mouse button
                case GLFW_MOUSE_BUTTON_MIDDLE : return IMouse::Middle;     ///< The middle (wheel) mouse button
                case GLFW_MOUSE_BUTTON_4 : return IMouse::XButton1;   ///< The first extra mouse button
                case GLFW_MOUSE_BUTTON_5 : return IMouse::XButton2;   ///< The second extra mouse button
            }
        }
        bool GLFWMouse::isButtonPressed(GLFWwindow* window, IMouse::Button button) {
            int state = glfwGetMouseButton(window, odfaegToGlfwButton(button));
            if (state == GLFW_PRESS)
                return true;
            return false;
        }
        sf::Vector2i GLFWMouse::getPosition() {
            return sf::Vector2i(0, 0);
        }
        sf::Vector2i GLFWMouse::getPosition(GLFWwindow* window) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            return sf::Vector2i(xpos, ypos);
        }
    }
}
