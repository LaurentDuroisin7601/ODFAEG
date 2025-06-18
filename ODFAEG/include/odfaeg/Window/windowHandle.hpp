#ifndef ODFAEG_WINDOW_HANDLE_HPP
#define ODFAEG_WINDOW_HANDLE_HPP
#include "../config.hpp"
namespace odfaeg {
    namespace window {
        // Windows' HWND is a typedef on struct HWND__*
        #if defined(ODFAEG_SYSTEM_WINDOWS)
            struct HWND__;
        #endif

        #if defined(ODFAEG_SYSTEM_WINDOWS)

            // Window handle is HWND (HWND__*) on Windows
            typedef HWND__* WindowHandle;

        #elif defined(ODFAEG_SYSTEM_LINUX) || defined(ODFAEG_SYSTEM_FREEBSD) || defined(ODFAEG_SYSTEM_OPENBSD)

            // Window handle is Window (unsigned long) on Unix - X11
            typedef unsigned long WindowHandle;

        #elif defined(ODFAEG_SYSTEM_MACOS)

            // Window handle is NSWindow or NSView (void*) on Mac OS X - Cocoa
            typedef void* WindowHandle;

        #elif defined(ODFAEG_SYSTEM_IOS)

            // Window handle is UIWindow (void*) on iOS - UIKit
            typedef void* WindowHandle;

        #elif defined(ODFAEG_SYSTEM_ANDROID)

            // Window handle is ANativeWindow* (void*) on Android
            typedef void* WindowHandle;
        #endif


    }
}
#endif // ODFAEG_WINDOW_HANDLE_HPP
