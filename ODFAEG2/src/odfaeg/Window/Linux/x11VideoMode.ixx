module;
#include <X11/Xlib.h>
#include <algorithm>
#include <vector>
export module odfaeg.window.x11VideoMode;
import odfaeg.window.videoMode;
namespace odfaeg {
    namespace window {
        export class X11VideoMode {
        public:
            static std::vector<VideoMode> getFullscreenModes();


            ////////////////////////////////////////////////////////////
            static VideoMode getDesktopMode();
        };
    }
}//
// Created by laurent on 21/05/2026.
//
