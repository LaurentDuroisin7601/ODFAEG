#include "../../../../include/odfaeg/Window/videoModeImpl.hpp"
#include <windows.h>
#include <algorithm>
namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
        {
            std::vector<VideoMode> modes;

            // Enumerate all available video modes for the primary display adapter
            DEVMODE win32Mode;
            win32Mode.dmSize = sizeof(win32Mode);
            win32Mode.dmDriverExtra = 0;
            for (int count = 0; EnumDisplaySettings(NULL, count, &win32Mode); ++count)
            {
                // Convert to sf::VideoMode
                VideoMode mode(win32Mode.dmPelsWidth, win32Mode.dmPelsHeight, win32Mode.dmBitsPerPel);

                // Add it only if it is not already in the array
                if (std::find(modes.begin(), modes.end(), mode) == modes.end())
                    modes.push_back(mode);
            }

            return modes;
        }


        ////////////////////////////////////////////////////////////
        VideoMode VideoModeImpl::getDesktopMode()
        {
            DEVMODE win32Mode;
            win32Mode.dmSize = sizeof(win32Mode);
            win32Mode.dmDriverExtra = 0;
            EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &win32Mode);

            return VideoMode(win32Mode.dmPelsWidth, win32Mode.dmPelsHeight, win32Mode.dmBitsPerPel);
        }
    }
}
