#include "../../../../include/odfaeg/Window/Linux/display.hpp"
#include <iostream>
namespace odfaeg {
    namespace window {
        using namespace sf;
        typedef std::map<std::string, Atom> AtomMap;
        ::Display* Display::sharedDisplay = nullptr;
        unsigned int Display::referenceCount = 0;
        std::map<std::string, Atom> Display::atoms = std::map<std::string, Atom>();
        ////////////////////////////////////////////////////////////
        ::Display* Display::openDisplay()
        {
            Mutex mutex;
            Lock lock(mutex);

            if (referenceCount == 0)
            {
                sharedDisplay = XOpenDisplay(NULL);

                // Opening display failed: The best we can do at the moment is to output a meaningful error message
                // and cause an abnormal program termination
                if (!sharedDisplay)
                {
                    std::cerr << "Failed to open X11 display; make sure the DISPLAY environment variable is set correctly" << std::endl;
                    std::abort();
                }
            }

            referenceCount++;
            return sharedDisplay;
        }


        ////////////////////////////////////////////////////////////
        void Display::closeDisplay(::Display* display)
        {
            Mutex mutex;
            Lock lock(mutex);

            assert(display == sharedDisplay);

            referenceCount--;
            if (referenceCount == 0)
                XCloseDisplay(display);
        }
        Atom Display::getAtom(std::string name, bool onlyIfExists) {
            AtomMap::const_iterator iter = atoms.find(name);
            if (iter != atoms.end())
                return iter->second;
            ::Display* display = openDisplay();
            Atom atom = XInternAtom(display, name.c_str(), onlyIfExists ? True : False);
            closeDisplay(display);
            atoms[name] = atom;
            return atom;
        }
    }
}
