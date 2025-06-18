#ifndef VIDEO_MODE_IMPL_HPP
#define VIDEO_MODE_IMPL_HPP
#include <vector>
#include "videoMode.hpp"
namespace odfaeg {
    namespace window {
        class VideoModeImpl
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
#endif // VIDEO_MODE_IMPL_HPP
