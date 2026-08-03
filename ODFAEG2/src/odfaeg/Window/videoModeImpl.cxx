module;
#include <odfaeg/config.hpp>
module odfaeg.window.videoModeImpl;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32VideoMode;
typedef odfaeg::window::Win32VideoMode VideoModeImplType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11VideoMode;
typedef odfaeg::window::X11VideoMode VideoModeImplType;
#endif
#include "videoModeImpl.hpp"