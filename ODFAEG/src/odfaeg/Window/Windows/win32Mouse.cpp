#ifndef ODFAEG_WIN32_MOUSE
#define ODFAEG_WIN32_MOUSE
#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#endif // ODFAEG_WIN32_MOUSE
#include <windows.h>
#include "../../../../include/odfaeg/Window/Windows/win32Mouse.hpp"
namespace odfaeg {
    namespace window {
        using namespace sf;
        ////////////////////////////////////////////////////////////
        bool Win32Mouse::isButtonPressed(IMouse::Button button)
        {
            int vkey = 0;
            switch (button)
            {
                case IMouse::Left:     vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON; break;
                case IMouse::Right:    vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON; break;
                case IMouse::Middle:   vkey = VK_MBUTTON;  break;
                case IMouse::XButton1: vkey = VK_XBUTTON1; break;
                case IMouse::XButton2: vkey = VK_XBUTTON2; break;
                default:              vkey = 0;           break;
            }

            return (GetAsyncKeyState(vkey) & 0x8000) != 0;
        }
        ////////////////////////////////////////////////////////////
        Vector2i Win32Mouse::getPosition()
        {
            POINT point;
            GetCursorPos(&point);
            return Vector2i(point.x, point.y);
        }
        ////////////////////////////////////////////////////////////
        Vector2i Win32Mouse::getPosition(const Win32Window& relativeTo)
        {
            WindowHandle handle = relativeTo.getSystemHandle();
            if (handle)
            {
                POINT point;
                GetCursorPos(&point);
                ScreenToClient(handle, &point);
                return Vector2i(point.x, point.y);
            }
            else
            {
                return Vector2i();
            }
        }
    }
}
