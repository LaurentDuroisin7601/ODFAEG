module;
#include <X11/Xlib.h>
#include <string>
#include <map>
#include <cassert>
export module odfaeg.window.display;
/**
*RAII Class opening and closing the connexion to the display server.
*/
namespace odfaeg {
    namespace window {
        export class Display {
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
