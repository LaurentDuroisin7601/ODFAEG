module;
#include <odfaeg/config.hpp>
#include <vector>
//import odfaeg.window.videoModeImpl;
export module odfaeg.window.videoModeImpl;
/*import odfaeg.window.videoMode;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32VideoMode;
typedef odfaeg::window::Win32VideoMode VideoModeImplType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11VideoMode;
typedef odfaeg::window::X11VideoMode VideoModeImplType;
#endif*/
namespace odfaeg {
    namespace window {
        export class VideoModeImpl
        {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Get the list of all the supported fullscreen video modes
            ///
            /// \return Array filled with the fullscreen video modes
            ///
            ////////////////////////////////////////////////////////////
            static std::vector<VideoMode> getFullscreenModes();

            ////////////////////////////////////////////////////////////
            /// \brief Get the current desktop video mode
            ///
            /// \return Current desktop video mode
            ///
            ////////////////////////////////////////////////////////////
            static VideoMode getDesktopMode();
        };
    }
}
#include "videoModeImpl.inl"
