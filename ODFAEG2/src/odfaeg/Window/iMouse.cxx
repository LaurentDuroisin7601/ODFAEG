module;
#include <odfaeg/config.hpp>
module odfaeg.window.iMouse;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32Keyboard;
typedef odfaeg::window::Win32Keyboard KeyboardType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11Keyboard;
typedef odfaeg::window::X11Keyboard KeyboardType;
#endif
#include "iMouse.inl"