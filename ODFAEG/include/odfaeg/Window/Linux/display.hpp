#ifndef ODFAEG_DISPLAY_HPP
#define ODFAEG_DISPLAY_HPP
#include <X11/Xlib.h>
#include <string>
#include <map>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <cassert>
/**
*RAII Class opening and closing the connexion to the display server.
*/
namespace odfaeg {
    namespace window {
        class Display {
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
