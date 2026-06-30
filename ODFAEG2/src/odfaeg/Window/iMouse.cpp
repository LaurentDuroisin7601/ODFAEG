module;
#include <odfaeg/config.hpp>
import odfaeg.window.iMouse;
module odfaeg.window.iMouse;
#if defined (ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32Mouse;
typedef odfaeg::window::Win32Mouse MouseType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11Mouse;
typedef odfaeg::window::X11Mouse MouseType;
#endif
namespace odfaeg {
    namespace window {
        bool IMouse::isButtonPressed(Button button) {
            return MouseType::isButtonPressed(button);
        }
        math::Vector2i IMouse::getPosition() {
            return MouseType::getPosition();
        }
        math::Vector2i IMouse::getPosition(const Window& window) {
            return MouseType::getPosition(window.getImpl().getImplType());
        }
    }
}