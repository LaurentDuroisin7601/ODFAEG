module;
#include <vector>
export module odfaeg.window.videoModeImpl;
import odfaeg.window.videoMode;
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
