#ifndef ODFAEG_WIN32VIDEOMODE_HPP
#define ODFAEG_WIN32VIDEOMODE_HPP
#include "../videoMode.hpp"
#include <windows.h>
#include <algorithm>
#include <vector>
namespace odfaeg {
    namespace window {
        class Win32VideoMode {
        public:
            static std::vector<VideoMode> getFullscreenModes();


            ////////////////////////////////////////////////////////////
            static VideoMode getDesktopMode();
        };        
    }
}
#include "win32VideoMode.inl"
#endif