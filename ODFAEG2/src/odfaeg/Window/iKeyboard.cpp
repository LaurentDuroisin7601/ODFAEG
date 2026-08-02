#include "iKeyboard.hpp"
#if defined(ODFAEG_SYSTEM_WINDOWS)
#include "Windows/win32Keyboard.hpp"
typedef odfaeg::window::Win32Keyboard KeyboardType;
#else if defined(ODFAEG_SYSTEM_LINUX)
#include "Linux/x11Keyboard.hpp"
typedef odfaeg::window::X11Keyboard KeyboardType;
#endif
#include "iKeyboard.inl"