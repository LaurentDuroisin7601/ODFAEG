#include "../../../../include/odfaeg/Window/x11Mouse.hpp"
#include "../../../../include/odfaeg/Window/display.hpp"
namespace odfaeg {
    namespace window {
        using namespace sf;
        ////////////////////////////////////////////////////////////
        bool X11Mouse::isButtonPressed(IMouse::Button button)
        {
            // Open a connection with the X server
            ::Display* display = Display::openDisplay();
            // we don't care about these but they are required
            ::Window root, child;
            int wx, wy;
            int gx, gy;

            unsigned int buttons = 0;
            XQueryPointer(display, DefaultRootWindow(display), &root, &child, &gx, &gy, &wx, &wy, &buttons);
            Display::closeDisplay(display);
            switch (button)
            {
                case IMouse::Left:     return buttons & Button1Mask;
                case IMouse::Right:    return buttons & Button3Mask;
                case IMouse::Middle:   return buttons & Button2Mask;
                case IMouse::XButton1: return false; // not supported by X
                case IMouse::XButton2: return false; // not supported by X
                default:              return false;
            }

            return false;
        }


        ////////////////////////////////////////////////////////////
        Vector2i X11Mouse::getPosition()
        {
            // Open a connection with the X server
            ::Display* display = Display::openDisplay();
            // we don't care about these but they are required
            ::Window root, child;
            int x, y;
            unsigned int buttons;

            int gx = 0;
            int gy = 0;
            XQueryPointer(display, DefaultRootWindow(display), &root, &child, &gx, &gy, &x, &y, &buttons);
            Display::closeDisplay(display);
            return Vector2i(gx, gy);
        }


        ////////////////////////////////////////////////////////////
        Vector2i X11Mouse::getPosition(const X11Window& relativeTo)
        {
            unsigned long handle = relativeTo.getSystemHandle();
            if (handle)
            {
                // Open a connection with the X server
                ::Display* display = odfaeg::window::Display::openDisplay();
                // we don't care about these but they are required
                ::Window root, child;
                int gx, gy;
                unsigned int buttons;

                int x = 0;
                int y = 0;
                XQueryPointer(display, handle, &root, &child, &gx, &gy, &x, &y, &buttons);
                odfaeg::window::Display::closeDisplay(display);
                return Vector2i(x, y);
            }
            else
            {
                return Vector2i();
            }
        }
    }
}
