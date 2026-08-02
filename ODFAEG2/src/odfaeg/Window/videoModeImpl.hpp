#ifndef ODFAEG_VIDEOMODE_IMPL_HPP
#define ODFAEG_VIDEOMODE_IMPL_HPP
#include <odfaeg/config.hpp>
#include <vector>
//import odfaeg.window.videoModeImpl;
#include "videoMode.hpp";
#if defined(ODFAEG_SYSTEM_WINDOWS)
#include "Windows/win32VideoMode.hpp";
typedef odfaeg::window::Win32VideoMode VideoModeImplType;
#else if defined(ODFAEG_SYSTEM_LINUX)
#include "Linux/x11VideoMode.hpp"
typedef odfaeg::window::X11VideoMode VideoModeImplType;
#endif
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
#endif
