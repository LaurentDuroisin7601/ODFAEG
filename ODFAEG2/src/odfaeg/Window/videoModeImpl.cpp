module;
#include <odfaeg/config.hpp>
#include <vector>
//import odfaeg.window.videoModeImpl;
module odfaeg.window.videoModeImpl;
import odfaeg.window.videoMode;
#if defined(ODFAEG_SYSTEM_WINDOWS)
import odfaeg.window.win32VideoMode;
typedef odfaeg::window::Win32VideoMode VideoModeImplType;
#else if defined(ODFAEG_SYSTEM_LINUX)
import odfaeg.window.x11VideoMode;
typedef odfaeg::window::X11VideoMode VideoModeImplType;
#endif
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        /// \brief Get the list of all the supported fullscreen video modes
        ///
        /// \return Array filled with the fullscreen video modes
        ///
        ////////////////////////////////////////////////////////////
        std::vector<VideoMode> VideoModeImpl::getFullscreenModes() {
            return VideoModeImplType::getFullscreenModes();
        }

        ////////////////////////////////////////////////////////////
        /// \brief Get the current desktop video mode
        ///
        /// \return Current desktop video mode
        ///
        ////////////////////////////////////////////////////////////
        VideoMode VideoModeImpl::getDesktopMode() {
            return VideoModeImplType::getDesktopMode();
        }
    }
}