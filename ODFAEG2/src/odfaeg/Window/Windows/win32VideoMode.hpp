#ifndef ODFAEG_WIN32VIDEOMODE_HPP
#define ODFAEG_WIN32VIDEOMODE_HPP
#include "../videoMode.hpp"
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
#endif