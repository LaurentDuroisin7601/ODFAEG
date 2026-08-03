#include "iMouse.hpp"
#include "window.hpp"
#if defined(ODFAEG_SYSTEM_WINDOWS)
#include "Windows/win32Mouse.hpp"
typedef odfaeg::window::Win32Mouse MouseType;
#else if defined(ODFAEG_SYSTEM_LINUX)
#include "Linux/x11Mouse.hpp"
typedef odfaeg::window::X11Mouse MouseType;
#endif
#include "iMouse.inl"