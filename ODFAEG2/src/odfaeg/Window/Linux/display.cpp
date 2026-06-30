module;
#include <iostream>
#include <map>
#include <X11/Xlib.h>
#include <mutex>
import odfaeg.window.display;
module odfaeg.window.display;
namespace odfaeg {
    namespace window {

        typedef std::map<std::string, Atom> AtomMap;
        ::Display* Display::sharedDisplay = nullptr;
        unsigned int Display::referenceCount = 0;
        std::map<std::string, Atom> Display::atoms = std::map<std::string, Atom>();
        ////////////////////////////////////////////////////////////
        ::Display* Display::openDisplay()
        {
            std::recursive_mutex rec_mutex;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);

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
            std::recursive_mutex rec_mutex;
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);

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
}//
// Created by laurent on 21/05/2026.
//
