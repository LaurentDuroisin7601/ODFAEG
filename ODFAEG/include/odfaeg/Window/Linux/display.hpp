#ifndef ODFAEG_DISPLAY_HPP
#define ODFAEG_DISPLAY_HPP
#include <X11/Xlib.h>
#include <string>
#include <map>
#include <ODFAEG/System/Mutex.hpp>
#include <ODFAEG/System/Lock.hpp>
#include <cassert>
#include "../export.hpp"
/**
*RAII Class opening and closing the connexion to the display server.
*/
namespace odfaeg {
    namespace window {
        class ODFAEG_WINDOW_API Display {
            public :
                static ::Display* openDisplay();
                static Atom getAtom(std::string name, bool onlyIfexists=false);
                static void closeDisplay(::Display* display);
            private :
                static ::Display* sharedDisplay;
                static unsigned int referenceCount;
                static std::map<std::string, Atom> atoms;
        };
    }
}
#endif
