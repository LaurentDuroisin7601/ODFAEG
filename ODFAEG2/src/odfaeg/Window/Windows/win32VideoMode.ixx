module;
#include <windows.h>
#include <algorithm>
#include <vector>
export module odfaeg.window.win32VideoMode;
import odfaeg.window.videoMode;
namespace odfaeg {
    namespace window {
        export class Win32VideoMode {
        public:
            static std::vector<VideoMode> getFullscreenModes();


            ////////////////////////////////////////////////////////////
            static VideoMode getDesktopMode();
        };        
    }
}