module;
#include <odfaeg/config.hpp>
//import odfaeg.window.iKeyboard;
module odfaeg.window.iKeyboard;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32Keyboard;
typedef odfaeg::window::Win32Keyboard KeyboardType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11Keyboard;
typedef odfaeg::window::X11Keyboard KeyboardType;
#endif
namespace odfaeg {
	namespace window {
		bool IKeyboard::isKeyPressed(Key key) {
			return KeyboardType::isKeyPressed(key);
		}
	}
}