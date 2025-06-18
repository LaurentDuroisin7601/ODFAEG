#ifndef GLFW_KEYBOARD_HPP
#define GLFW_KEYBOARD_HPP
#include "../iKeyboard.hpp"
#include <GLFW/glfw3.h>
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API GLFWKeyboard {
            public :
            static IKeyboard::Key glfwToODFAEGKey(int key);
            static int odfaegToGlfwKey(IKeyboard::Key key);
            static bool isKeyPressed(GLFWwindow* window, IKeyboard::Key key);
        };
    }
}
#endif // GLFW_KEYBOARD_HPP
