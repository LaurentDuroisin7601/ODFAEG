#ifdef _WIN32_WINDOWS
    #undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#define WINVER         0x0501
#include "../../../../include/odfaeg/Window/Windows/win32Window.hpp"
#include <SFML/Window/WindowStyle.hpp>
#include <GL/gl.h>
#include <SFML/System/Err.hpp>
#include <SFML/System/Utf.hpp>
// dbt.h is lowercase here, as a cross-compile on linux with mingw-w64
// expects lowercase, and a native compile on windows, whether via msvc
// or mingw-w64 addresses files in a case insensitive manner.
#include <dbt.h>
#include <vector>
#include <cstring>

// MinGW lacks the definition of some Win32 constants
#ifndef XBUTTON1
    #define XBUTTON1 0x0001
#endif
#ifndef XBUTTON2
    #define XBUTTON2 0x0002
#endif
#ifndef WM_MOUSEHWHEEL
    #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef MAPVK_VK_TO_VSC
    #define MAPVK_VK_TO_VSC (0)
#endif

namespace
{
    unsigned int               windowCount      = 0; // Windows owned by SFML
    unsigned int               handleCount      = 0; // All window handles
    const wchar_t*             className        = L"ODFAEG_Window";
    odfaeg::window::Win32Window* fullscreenWindow = NULL;

    const GUID GUID_DEVINTERFACE_HID = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};

    void setProcessDpiAware()
    {
        // Try SetProcessDpiAwareness first
        HINSTANCE shCoreDll = LoadLibrary(L"Shcore.dll");

        if (shCoreDll)
        {
            enum ProcessDpiAwareness
            {
                ProcessDpiUnaware         = 0,
                ProcessSystemDpiAware     = 1,
                ProcessPerMonitorDpiAware = 2
            };

            typedef HRESULT (WINAPI* SetProcessDpiAwarenessFuncType)(ProcessDpiAwareness);
            SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));

            if (SetProcessDpiAwarenessFunc)
            {
                // We only check for E_INVALIDARG because we would get
                // E_ACCESSDENIED if the DPI was already set previously
                // and S_OK means the call was successful
                if (SetProcessDpiAwarenessFunc(ProcessSystemDpiAware) == E_INVALIDARG)
                {
                    sf::err() << "Failed to set process DPI awareness" << std::endl;
                }
                else
                {
                    FreeLibrary(shCoreDll);
                    return;
                }
            }

            FreeLibrary(shCoreDll);
        }

        // Fall back to SetProcessDPIAware if SetProcessDpiAwareness
        // is not available on this system
        HINSTANCE user32Dll = LoadLibrary(L"user32.dll");

        if (user32Dll)
        {
            typedef BOOL (WINAPI* SetProcessDPIAwareFuncType)(void);
            SetProcessDPIAwareFuncType SetProcessDPIAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(GetProcAddress(user32Dll, "SetProcessDPIAware"));

            if (SetProcessDPIAwareFunc)
            {
                if (!SetProcessDPIAwareFunc())
                    sf::err() << "Failed to set process DPI awareness" << std::endl;
            }

            FreeLibrary(user32Dll);
        }
    }
}
namespace odfaeg {
    namespace window {
        using namespace sf;
        Win32Window::Win32Window() :
        m_handle          (NULL),
        m_callback        (0),
        m_cursorVisible   (true), // might need to call GetCursorInfo
        m_lastCursor      (LoadCursor(NULL, IDC_ARROW)),
        m_icon            (NULL),
        m_keyRepeatEnabled(true),
        m_lastSize        (0, 0),
        m_resizing        (false),
        m_surrogate       (0),
        m_mouseInside     (false),
        m_fullscreen      (false),
        m_cursorGrabbed   (false),
        m_opened(false),
        m_destroyed(true) {

        }
        ////////////////////////////////////////////////////////////
        Win32Window::Win32Window(WindowHandle handle, const ContextSettings& settings) :
        m_handle          (handle),
        m_callback        (0),
        m_cursorVisible   (true), // might need to call GetCursorInfo
        m_lastCursor      (LoadCursor(NULL, IDC_ARROW)),
        m_icon            (NULL),
        m_keyRepeatEnabled(true),
        m_lastSize        (0, 0),
        m_resizing        (false),
        m_surrogate       (0),
        m_mouseInside     (false),
        m_fullscreen      (false),
        m_cursorGrabbed   (false),
        m_opened(false),
        m_destroyed(true)
        {
            create(m_handle, settings);
        }


        ////////////////////////////////////////////////////////////
        Win32Window::Win32Window(VideoMode mode, const String& title, Uint32 style, const ContextSettings& settings) :
        m_handle          (NULL),
        m_callback        (0),
        m_cursorVisible   (true), // might need to call GetCursorInfo
        m_lastCursor      (LoadCursor(NULL, IDC_ARROW)),
        m_icon            (NULL),
        m_keyRepeatEnabled(true),
        m_lastSize        (0, 0),
        m_resizing        (false),
        m_surrogate       (0),
        m_mouseInside     (false),
        m_fullscreen      (false),
        m_cursorGrabbed   (false),
        m_opened(false),
        m_destroyed(true)
        {
            create(mode, title, style, settings);
        }
        void Win32Window::create(sf::WindowHandle handle, const ContextSettings& settings) {
            // Set that this process is DPI aware and can handle DPI scaling
            setProcessDpiAware();

            if (m_handle)
            {
                // If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
                /*if (handleCount == 0)
                    JoystickImpl::setLazyUpdates(true);*/

                ++handleCount;

                // We change the event procedure of the control (it is important to save the old one)
                SetWindowLongPtrW(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
                m_callback = SetWindowLongPtrW(m_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32Window::globalOnEvent));
            }
            m_opened = true;
            m_destroyed = false;
        }
        void Win32Window::create (VideoMode mode, const String& title, Uint32 style, const ContextSettings& settings) {
            m_fullscreen = (style & Style::Fullscreen) != 0;
            m_cursorGrabbed = m_fullscreen;
            m_lastSize = sf::Vector2u(mode.width, mode.height);
            // Set that this process is DPI aware and can handle DPI scaling
            setProcessDpiAware();

            // Register the window class at first call
            if (windowCount == 0)
                registerWindowClass();

            // Compute position and size
            HDC screenDC = GetDC(NULL);
            int left   = (GetDeviceCaps(screenDC, HORZRES) - static_cast<int>(mode.width))  / 2;
            int top    = (GetDeviceCaps(screenDC, VERTRES) - static_cast<int>(mode.height)) / 2;
            int width  = mode.width;
            int height = mode.height;
            ReleaseDC(NULL, screenDC);

            // Choose the window style according to the Style parameter
            DWORD win32Style = WS_VISIBLE;
            if (style == Style::None)
            {
                win32Style |= WS_POPUP;
            }
            else
            {
                if (style & Style::Titlebar) win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
                if (style & Style::Resize)   win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
                if (style & Style::Close)    win32Style |= WS_SYSMENU;
            }

            // In windowed mode, adjust width and height so that window will have the requested client area
            if (!m_fullscreen)
            {
                RECT rectangle = {0, 0, width, height};
                AdjustWindowRect(&rectangle, win32Style, false);
                width  = rectangle.right - rectangle.left;
                height = rectangle.bottom - rectangle.top;
            }

            // Create the window
            m_handle = CreateWindowW(className, title.toWideString().c_str(), win32Style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);

            // Register to receive device interface change notifications (used for joystick connection handling)
            DEV_BROADCAST_DEVICEINTERFACE deviceInterface = {sizeof(DEV_BROADCAST_DEVICEINTERFACE), DBT_DEVTYP_DEVICEINTERFACE, 0, GUID_DEVINTERFACE_HID, 0};
            RegisterDeviceNotification(m_handle, &deviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

            // If we're the first window handle, we only need to poll for joysticks when WM_DEVICECHANGE message is received
            if (m_handle)
            {
                /*if (handleCount == 0)
                    JoystickImpl::setLazyUpdates(true);*/

                ++handleCount;
            }

            // By default, the OS limits the size of the window the the desktop size,
            // we have to resize it after creation to apply the real size
            setSize(Vector2u(mode.width, mode.height));

            // Switch to fullscreen if requested
            if (m_fullscreen)
                switchToFullscreen(mode);

            // Increment window count
            windowCount++;
            m_opened = true;
            m_destroyed = false;
        }
        void Win32Window::destroy() {
            // TODO should we restore the cursor shape and visibility?

            // Destroy the custom icon, if any
            if (!m_destroyed) {
                if (m_icon)
                    DestroyIcon(m_icon);

                // If it's the last window handle we have to poll for joysticks again
                if (m_handle)
                {
                    --handleCount;

                    /*if (handleCount == 0)
                        JoystickImpl::setLazyUpdates(false);*/
                }

                if (!m_callback)
                {
                    // Destroy the window
                    if (m_handle)
                        DestroyWindow(m_handle);

                    // Decrement the window count
                    windowCount--;

                    // Unregister window class if we were the last window
                    if (windowCount == 0)
                        UnregisterClassW(className, GetModuleHandleW(NULL));
                }
                else
                {
                    // The window is external: remove the hook on its message callback
                    SetWindowLongPtrW(m_handle, GWLP_WNDPROC, m_callback);
                }
                m_destroyed = true;
                m_opened = false;
            }
        }

        ////////////////////////////////////////////////////////////
        Win32Window::~Win32Window()
        {
            // TODO should we restore the cursor shape and visibility?

            // Destroy the custom icon, if any
            if (!m_destroyed) {
                if (m_icon)
                    DestroyIcon(m_icon);

                // If it's the last window handle we have to poll for joysticks again
                if (m_handle)
                {
                    --handleCount;

                    /*if (handleCount == 0)
                        JoystickImpl::setLazyUpdates(false);*/
                }

                if (!m_callback)
                {
                    // Destroy the window
                    if (m_handle)
                        DestroyWindow(m_handle);

                    // Decrement the window count
                    windowCount--;

                    // Unregister window class if we were the last window
                    if (windowCount == 0)
                        UnregisterClassW(className, GetModuleHandleW(NULL));
                }
                else
                {
                    // The window is external: remove the hook on its message callback
                    SetWindowLongPtrW(m_handle, GWLP_WNDPROC, m_callback);
                }
            }
        }

        bool Win32Window::isOpen() const {
            return m_opened;
        }
        void Win32Window::close() {
            if (m_opened) {
                m_opened = false;
            }
        }
        ////////////////////////////////////////////////////////////
        WindowHandle Win32Window::getSystemHandle() const
        {
            return m_handle;
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::processEvents()
        {
            // We process the window events only if we own it
            if (!m_callback)
            {
                MSG message;
                while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&message);
                    DispatchMessageW(&message);
                }
            }
        }


        ////////////////////////////////////////////////////////////
        Vector2i Win32Window::getPosition() const
        {
            RECT rect;
            GetWindowRect(m_handle, &rect);

            return Vector2i(rect.left, rect.top);
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setPosition(const Vector2i& position)
        {
            SetWindowPos(m_handle, NULL, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            if(m_cursorGrabbed)
                grabCursor(true);
        }


        ////////////////////////////////////////////////////////////
        Vector2u Win32Window::getSize() const
        {
            RECT rect;
            GetClientRect(m_handle, &rect);

            return Vector2u(rect.right - rect.left, rect.bottom - rect.top);
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setSize(const Vector2u& size)
        {
            // SetWindowPos wants the total size of the window (including title bar and borders),
            // so we have to compute it
            RECT rectangle = {0, 0, static_cast<long>(size.x), static_cast<long>(size.y)};
            AdjustWindowRect(&rectangle, GetWindowLong(m_handle, GWL_STYLE), false);
            int width  = rectangle.right - rectangle.left;
            int height = rectangle.bottom - rectangle.top;

            SetWindowPos(m_handle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setTitle(const String& title)
        {
            SetWindowTextW(m_handle, title.toWideString().c_str());
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setIcon(unsigned int width, unsigned int height, const Uint8* pixels)
        {
            // First destroy the previous one
            if (m_icon)
                DestroyIcon(m_icon);

            // Windows wants BGRA pixels: swap red and blue channels
            std::vector<Uint8> iconPixels(width * height * 4);
            for (std::size_t i = 0; i < iconPixels.size() / 4; ++i)
            {
                iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
                iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
                iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
                iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
            }

            // Create the icon from the pixel array
            m_icon = CreateIcon(GetModuleHandleW(NULL), width, height, 1, 32, NULL, &iconPixels[0]);

            // Set it as both big and small icon of the window
            if (m_icon)
            {
                SendMessageW(m_handle, WM_SETICON, ICON_BIG,   (LPARAM)m_icon);
                SendMessageW(m_handle, WM_SETICON, ICON_SMALL, (LPARAM)m_icon);
            }
            else
            {
                err() << "Failed to set the window's icon" << std::endl;
            }
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setVisible(bool visible)
        {
            ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setMouseCursorVisible(bool visible)
        {
            // Don't call twice ShowCursor with the same parameter value;
            // we don't want to increment/decrement the internal counter
            // more than once.
            if (visible != m_cursorVisible)
            {
                m_cursorVisible = visible;
                ShowCursor(visible);
            }
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setMouseCursorGrabbed(bool grabbed)
        {
            m_cursorGrabbed = grabbed;
            grabCursor(m_cursorGrabbed);
        }




        ////////////////////////////////////////////////////////////
        void Win32Window::setKeyRepeatEnabled(bool enabled)
        {
            m_keyRepeatEnabled = enabled;
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::requestFocus()
        {
            // Allow focus stealing only within the same process; compare PIDs of current and foreground window
            DWORD thisPid       = GetWindowThreadProcessId(m_handle, NULL);
            DWORD foregroundPid = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

            if (thisPid == foregroundPid)
            {
                // The window requesting focus belongs to the same process as the current window: steal focus
                SetForegroundWindow(m_handle);
            }
            else
            {
                // Different process: don't steal focus, but create a taskbar notification ("flash")
                FLASHWINFO info;
                info.cbSize    = sizeof(info);
                info.hwnd      = m_handle;
                info.dwFlags   = FLASHW_TRAY;
                info.dwTimeout = 0;
                info.uCount    = 3;

                FlashWindowEx(&info);
            }
        }


        ////////////////////////////////////////////////////////////
        bool Win32Window::hasFocus() const
        {
            return m_handle == GetForegroundWindow();
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::registerWindowClass()
        {
            WNDCLASSW windowClass;
            windowClass.style         = 0;
            windowClass.lpfnWndProc   = &Win32Window::globalOnEvent;
            windowClass.cbClsExtra    = 0;
            windowClass.cbWndExtra    = 0;
            windowClass.hInstance     = GetModuleHandleW(NULL);
            windowClass.hIcon         = NULL;
            windowClass.hCursor       = 0;
            windowClass.hbrBackground = 0;
            windowClass.lpszMenuName  = NULL;
            windowClass.lpszClassName = className;
            RegisterClassW(&windowClass);
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::switchToFullscreen(const VideoMode& mode)
        {
            DEVMODE devMode;
            devMode.dmSize       = sizeof(devMode);
            devMode.dmPelsWidth  = mode.width;
            devMode.dmPelsHeight = mode.height;
            devMode.dmBitsPerPel = mode.bitsPerPixel;
            devMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

            // Apply fullscreen mode
            if (ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {
                err() << "Failed to change display mode for fullscreen" << std::endl;
                return;
            }

            // Make the window flags compatible with fullscreen mode
            SetWindowLongW(m_handle, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
            SetWindowLongW(m_handle, GWL_EXSTYLE, WS_EX_APPWINDOW);

            // Resize the window so that it fits the entire screen
            SetWindowPos(m_handle, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
            ShowWindow(m_handle, SW_SHOW);

            // Set "this" as the current fullscreen window
            fullscreenWindow = this;
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::cleanup()
        {
            // Restore the previous video mode (in case we were running in fullscreen)
            if (fullscreenWindow == this)
            {
                ChangeDisplaySettingsW(NULL, 0);
                fullscreenWindow = NULL;
            }

            // Unhide the mouse cursor (in case it was hidden)
            setMouseCursorVisible(true);

            // No longer track the cursor
            setTracking(false);

            // No longer capture the cursor
            ReleaseCapture();
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::setTracking(bool track)
        {
            TRACKMOUSEEVENT mouseEvent;
            mouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
            mouseEvent.dwFlags = track ? TME_LEAVE : TME_CANCEL;
            mouseEvent.hwndTrack = m_handle;
            mouseEvent.dwHoverTime = HOVER_DEFAULT;
            TrackMouseEvent(&mouseEvent);
        }


        ////////////////////////////////////////////////////////////
        void Win32Window::grabCursor(bool grabbed)
        {
            if (grabbed)
            {
                RECT rect;
                GetClientRect(m_handle, &rect);
                MapWindowPoints(m_handle, NULL, reinterpret_cast<LPPOINT>(&rect), 2);
                ClipCursor(&rect);
            }
            else
            {
                ClipCursor(NULL);
            }
        }
        void Win32Window::pushEvent(IEvent& event) {
            m_ievents.push(event);
        }
        bool Win32Window::popEvent(IEvent& event) {
            // If the event queue is empty, let's first check if new events are available from the OS
            if (m_ievents.empty())
            {
                // Get events from the system
                processEvents();
            }

            // Pop the first event of the queue, if it is not empty
            if (!m_ievents.empty())
            {
                event = m_ievents.front();
                m_ievents.pop();
                return true;
            }
            return false;
        }
        bool Win32Window::pollEvent(IEvent& event) {
            return popEvent(event);
        }
        ////////////////////////////////////////////////////////////
        void Win32Window::processEvent(UINT message, WPARAM wParam, LPARAM lParam)
        {
            // Don't process any message until window is created
            if (m_handle == NULL)
                return;

            switch (message)
            {
                // Destroy event
                case WM_DESTROY:
                {
                    // Here we must cleanup resources !
                    cleanup();
                    break;
                }

                // Set cursor event
                case WM_SETCURSOR:
                {
                    // The mouse has moved, if the cursor is in our window we must refresh the cursor
                    if (LOWORD(lParam) == HTCLIENT)
                        SetCursor(m_lastCursor);

                    break;
                }

                // Close event
                case WM_CLOSE:
                {
                    IEvent event;
                    event.type = IEvent::EventType::WINDOW_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.window.type = IEvent::WindowEventID::WINDOW_EVENT_CLOSED;
                    event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                    pushEvent(event);
                    break;
                }

                // Resize event
                case WM_SIZE:
                {
                    // Consider only events triggered by a maximize or a un-maximize
                    if (wParam != SIZE_MINIMIZED && !m_resizing && m_lastSize != getSize())
                    {
                        // Update the last handled size
                        m_lastSize = getSize();

                        // Push a resize event
                        IEvent event;
                        event.type = IEvent::EventType::WINDOW_EVENT;
                        std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                        event.window.type = IEvent::WindowEventID::WINDOW_EVENT_RESIZED;
                        event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                        event.window.data1  = m_lastSize.x;
                        event.window.data2 = m_lastSize.y;
                        pushEvent(event);

                        // Restore/update cursor grabbing
                        grabCursor(m_cursorGrabbed);
                    }
                    break;
                }

                // Start resizing
                case WM_ENTERSIZEMOVE:
                {
                    m_resizing = true;
                    grabCursor(false);
                    break;
                }

                // Stop resizing
                case WM_EXITSIZEMOVE:
                {
                    m_resizing = false;

                    // Ignore cases where the window has only been moved
                    if(m_lastSize != getSize())
                    {
                        // Update the last handled size
                        m_lastSize = getSize();

                        // Push a resize event
                        IEvent event;
                        event.type = IEvent::EventType::WINDOW_EVENT;
                        std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                        event.window.type = IEvent::WindowEventID::WINDOW_EVENT_RESIZED;
                        event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                        event.window.data1  = m_lastSize.x;
                        event.window.data2 = m_lastSize.y;
                        pushEvent(event);
                    }

                    // Restore/update cursor grabbing
                    grabCursor(m_cursorGrabbed);
                    break;
                }

                // The system request the min/max window size and position
                case WM_GETMINMAXINFO:
                {
                    // We override the returned information to remove the default limit
                    // (the OS doesn't allow windows bigger than the desktop by default)
                    MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
                    info->ptMaxTrackSize.x = 50000;
                    info->ptMaxTrackSize.y = 50000;
                    break;
                }

                // Gain focus event
                case WM_SETFOCUS:
                {
                    // Restore cursor grabbing
                    grabCursor(m_cursorGrabbed);

                    IEvent event;
                    event.type = IEvent::EventType::WINDOW_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.window.type = IEvent::WindowEventID::WINDOW_EVENT_FOCUS_GAIGNED;
                    event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                    pushEvent(event);
                    break;
                }

                // Lost focus event
                case WM_KILLFOCUS:
                {
                    // Ungrab the cursor
                    grabCursor(false);

                    IEvent event;
                    event.type = IEvent::EventType::WINDOW_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.window.type = IEvent::WindowEventID::WINDOW_EVENT_FOCUS_LOST;
                    event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                    pushEvent(event);
                    break;
                }

                // Text event
                case WM_CHAR:
                {
                    if (m_keyRepeatEnabled || ((lParam & (1 << 30)) == 0))
                    {
                        // Get the code of the typed character
                        Uint32 character = static_cast<Uint32>(wParam);

                        // Check if it is the first part of a surrogate pair, or a regular character
                        if ((character >= 0xD800) && (character <= 0xDBFF))
                        {
                            // First part of a surrogate pair: store it and wait for the second one
                            m_surrogate = static_cast<Uint16>(character);
                        }
                        else
                        {
                            // Check if it is the second part of a surrogate pair, or a regular character
                            if ((character >= 0xDC00) && (character <= 0xDFFF))
                            {
                                // Convert the UTF-16 surrogate pair to a single UTF-32 value
                                Uint16 utf16[] = {m_surrogate, static_cast<Uint16>(character)};
                                sf::Utf16::toUtf32(utf16, utf16 + 2, &character);
                                m_surrogate = 0;
                            }

                            // Send a TextEntered event
                            IEvent textEvent;
                            textEvent.type = IEvent::EventType::TEXT_INPUT_EVENT;
                            std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                            textEvent.text.type = IEvent::EventType::TEXT_INPUT_EVENT;
                            textEvent.text.timestamp = std::chrono::system_clock::to_time_t(time);
                            textEvent.text.unicode = character;
                            pushEvent(textEvent);
                        }
                    }
                    break;
                }

                // Keydown event
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN:
                {
                    if (m_keyRepeatEnabled || ((HIWORD(lParam) & KF_REPEAT) == 0))
                    {
                        IEvent event;
                        event.type        = IEvent::EventType::KEYBOARD_EVENT;
                        event.keyboard.type    = IEvent::KeyEventID::KEY_EVENT_PRESSED;
                        event.keyboard.alt     = HIWORD(GetKeyState(VK_MENU))    != 0;
                        event.keyboard.control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                        event.keyboard.shift   = HIWORD(GetKeyState(VK_SHIFT))   != 0;
                        event.keyboard.system  = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                        event.keyboard.code    = virtualKeyCodeToODFAEG(wParam, lParam);
                        pushEvent(event);
                    }
                    break;
                }

                // Keyup event
                case WM_KEYUP:
                case WM_SYSKEYUP:
                {
                    IEvent event;
                    event.type = IEvent::EventType::KEYBOARD_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.keyboard.type = IEvent::KeyEventID::KEY_EVENT_RELEASED;
                    event.keyboard.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.keyboard.alt     = HIWORD(GetKeyState(VK_MENU))    != 0;
                    event.keyboard.control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                    event.keyboard.shift   = HIWORD(GetKeyState(VK_SHIFT))   != 0;
                    event.keyboard.system  = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                    event.keyboard.code    = virtualKeyCodeToODFAEG(wParam, lParam);
                    pushEvent(event);
                    break;
                }

                // Vertical mouse wheel event
                case WM_MOUSEWHEEL:
                {
                    // Mouse position is in screen coordinates, convert it to window coordinates
                    POINT position;
                    position.x = static_cast<Int16>(LOWORD(lParam));
                    position.y = static_cast<Int16>(HIWORD(lParam));
                    ScreenToClient(m_handle, &position);

                    Int16 delta = static_cast<Int16>(HIWORD(wParam));

                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_WHEEL_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::MOUSE_WHEEL_MOVED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseWheel.direction = delta / 120;
                    event.mouseWheel.x     = position.x;
                    event.mouseWheel.y     = position.y;
                    pushEvent(event);
                    break;
                }

                // Horizontal mouse wheel event
               /* case WM_MOUSEHWHEEL:
                {
                    // Mouse position is in screen coordinates, convert it to window coordinates
                    POINT position;
                    position.x = static_cast<Int16>(LOWORD(lParam));
                    position.y = static_cast<Int16>(HIWORD(lParam));
                    ScreenToClient(m_handle, &position);

                    Int16 delta = static_cast<Int16>(HIWORD(wParam));

                    Event event;
                    event.type                   = Event::MouseWheelScrolled;
                    event.mouseWheelScroll.wheel = Mouse::HorizontalWheel;
                    event.mouseWheelScroll.delta = -static_cast<float>(delta) / 120.f;
                    event.mouseWheelScroll.x     = position.x;
                    event.mouseWheelScroll.y     = position.y;
                    pushEvent(event);
                    break;
                }*/

                // Mouse left button down event
                case WM_LBUTTONDOWN:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_PRESSED;
                    event.mouseButton.button = IMouse::Left;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse left button up event
                case WM_LBUTTONUP:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_RELEASED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = IMouse::Left;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse right button down event
                case WM_RBUTTONDOWN:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_PRESSED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = IMouse::Right;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse right button up event
                case WM_RBUTTONUP:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_RELEASED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = IMouse::Right;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse wheel button down event
                case WM_MBUTTONDOWN:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_PRESSED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = IMouse::Middle;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse wheel button up event
                case WM_MBUTTONUP:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_RELEASED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = IMouse::Middle;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse X button down event
                case WM_XBUTTONDOWN:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_PRESSED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = HIWORD(wParam) == XBUTTON1 ? IMouse::XButton1 : IMouse::XButton2;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse X button up event
                case WM_XBUTTONUP:
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_RELEASED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.button = HIWORD(wParam) == XBUTTON1 ? IMouse::XButton1 : IMouse::XButton2;
                    event.mouseButton.x      = static_cast<Int16>(LOWORD(lParam));
                    event.mouseButton.y      = static_cast<Int16>(HIWORD(lParam));
                    pushEvent(event);
                    break;
                }

                // Mouse leave event
                case WM_MOUSELEAVE:
                {
                    // Avoid this firing a second time in case the cursor is dragged outside
                    if (m_mouseInside)
                    {
                        m_mouseInside = false;

                        // Generate a MouseLeft event
                        IEvent event;
                        event.type = IEvent::EventType::MOUSE_EVENT_LEAVE;
                        std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                        event.mouseMotion.type = IEvent::EventType::MOUSE_EVENT_LEAVE;
                        pushEvent(event);
                    }
                    break;
                }

                // Mouse move event
                case WM_MOUSEMOVE:
                {
                    // Extract the mouse local coordinates
                    int x = static_cast<Int16>(LOWORD(lParam));
                    int y = static_cast<Int16>(HIWORD(lParam));

                    // Get the client area of the window
                    RECT area;
                    GetClientRect(m_handle, &area);

                    // Capture the mouse in case the user wants to drag it outside
                    if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
                    {
                        // Only release the capture if we really have it
                        if (GetCapture() == m_handle)
                            ReleaseCapture();
                    }
                    else if (GetCapture() != m_handle)
                    {
                        // Set the capture to continue receiving mouse events
                        SetCapture(m_handle);
                    }

                    // If the cursor is outside the client area...
                    if ((x < area.left) || (x > area.right) || (y < area.top) || (y > area.bottom))
                    {
                        // and it used to be inside, the mouse left it.
                        if (m_mouseInside)
                        {
                            m_mouseInside = false;

                            // No longer care for the mouse leaving the window
                            setTracking(false);

                            // Generate a MouseLeft event
                            IEvent event;
                            event.type = IEvent::EventType::MOUSE_EVENT_LEAVE;
                            std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                            event.mouseMotion.type = IEvent::EventType::MOUSE_EVENT_LEAVE;
                            pushEvent(event);
                        }
                    }
                    else
                    {
                        // and vice-versa
                        if (!m_mouseInside)
                        {
                            m_mouseInside = true;

                            // Look for the mouse leaving the window
                            setTracking(true);

                            // Generate a MouseEntered event
                           IEvent event;
                            event.type = IEvent::EventType::MOUSE_EVENT_ENTER;
                            std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                            event.mouseMotion.type = IEvent::EventType::MOUSE_EVENT_ENTER;
                            pushEvent(event);
                        }
                    }

                    // Generate a MouseMove event
                    IEvent event;
                    event.type = IEvent::EventType::MOUSE_MOTION_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseMotion.type = IEvent::EventType::MOUSE_MOTION_EVENT;
                    event.mouseMotion.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseMotion.x = x;
                    event.mouseMotion.y = y;
                    pushEvent(event);
                    break;
                }
                /*case WM_DEVICECHANGE:
                {
                    // Some sort of device change has happened, update joystick connections
                    if ((wParam == DBT_DEVICEARRIVAL) || (wParam == DBT_DEVICEREMOVECOMPLETE))
                    {
                        // Some sort of device change has happened, update joystick connections if it is a device interface
                        DEV_BROADCAST_HDR* deviceBroadcastHeader = reinterpret_cast<DEV_BROADCAST_HDR*>(lParam);

                        if (deviceBroadcastHeader && (deviceBroadcastHeader->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE))
                            JoystickImpl::updateConnections();
                    }

                    break;
                }*/
            }
        }


        ////////////////////////////////////////////////////////////
        IKeyboard::Key Win32Window::virtualKeyCodeToODFAEG(WPARAM key, LPARAM flags)
        {
            switch (key)
            {
                // Check the scancode to distinguish between left and right shift
                case VK_SHIFT:
                {
                    static UINT lShift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
                    UINT scancode = static_cast<UINT>((flags & (0xFF << 16)) >> 16);
                    return scancode == lShift ? odfaeg::window::IKeyboard::LShift : odfaeg::window::IKeyboard::RShift;
                }

                // Check the "extended" flag to distinguish between left and right alt
                case VK_MENU : return (HIWORD(flags) & KF_EXTENDED) ? odfaeg::window::IKeyboard::RAlt : odfaeg::window::IKeyboard::LAlt;

                // Check the "extended" flag to distinguish between left and right control
                case VK_CONTROL : return (HIWORD(flags) & KF_EXTENDED) ? odfaeg::window::IKeyboard::RControl : odfaeg::window::IKeyboard::LControl;

                // Other keys are reported properly
                case VK_LWIN:       return odfaeg::window::IKeyboard::LSystem;
                case VK_RWIN:       return odfaeg::window::IKeyboard::RSystem;
                case VK_APPS:       return odfaeg::window::IKeyboard::Menu;
                case VK_OEM_1:      return odfaeg::window::IKeyboard::Semicolon;
                case VK_OEM_2:      return odfaeg::window::IKeyboard::Slash;
                case VK_OEM_PLUS:   return odfaeg::window::IKeyboard::Equal;
                case VK_OEM_MINUS:  return odfaeg::window::IKeyboard::Hyphen;
                case VK_OEM_4:      return odfaeg::window::IKeyboard::LBracket;
                case VK_OEM_6:      return odfaeg::window::IKeyboard::RBracket;
                case VK_OEM_COMMA:  return odfaeg::window::IKeyboard::Comma;
                case VK_OEM_PERIOD: return odfaeg::window::IKeyboard::Period;
                case VK_OEM_7:      return odfaeg::window::IKeyboard::Quote;
                case VK_OEM_5:      return odfaeg::window::IKeyboard::Backslash;
                case VK_OEM_3:      return odfaeg::window::IKeyboard::Tilde;
                case VK_ESCAPE:     return odfaeg::window::IKeyboard::Escape;
                case VK_SPACE:      return odfaeg::window::IKeyboard::Space;
                case VK_RETURN:     return odfaeg::window::IKeyboard::Enter;
                case VK_BACK:       return odfaeg::window::IKeyboard::Backspace;
                case VK_TAB:        return odfaeg::window::IKeyboard::Tab;
                case VK_PRIOR:      return odfaeg::window::IKeyboard::PageUp;
                case VK_NEXT:       return odfaeg::window::IKeyboard::PageDown;
                case VK_END:        return odfaeg::window::IKeyboard::End;
                case VK_HOME:       return odfaeg::window::IKeyboard::Home;
                case VK_INSERT:     return odfaeg::window::IKeyboard::Insert;
                case VK_DELETE:     return odfaeg::window::IKeyboard::Delete;
                case VK_ADD:        return odfaeg::window::IKeyboard::Add;
                case VK_SUBTRACT:   return odfaeg::window::IKeyboard::Subtract;
                case VK_MULTIPLY:   return odfaeg::window::IKeyboard::Multiply;
                case VK_DIVIDE:     return odfaeg::window::IKeyboard::Divide;
                case VK_PAUSE:      return odfaeg::window::IKeyboard::Pause;
                case VK_F1:         return odfaeg::window::IKeyboard::F1;
                case VK_F2:         return odfaeg::window::IKeyboard::F2;
                case VK_F3:         return odfaeg::window::IKeyboard::F3;
                case VK_F4:         return odfaeg::window::IKeyboard::F4;
                case VK_F5:         return odfaeg::window::IKeyboard::F5;
                case VK_F6:         return odfaeg::window::IKeyboard::F6;
                case VK_F7:         return odfaeg::window::IKeyboard::F7;
                case VK_F8:         return odfaeg::window::IKeyboard::F8;
                case VK_F9:         return odfaeg::window::IKeyboard::F9;
                case VK_F10:        return odfaeg::window::IKeyboard::F10;
                case VK_F11:        return odfaeg::window::IKeyboard::F11;
                case VK_F12:        return odfaeg::window::IKeyboard::F12;
                case VK_F13:        return odfaeg::window::IKeyboard::F13;
                case VK_F14:        return odfaeg::window::IKeyboard::F14;
                case VK_F15:        return odfaeg::window::IKeyboard::F15;
                case VK_LEFT:       return odfaeg::window::IKeyboard::Left;
                case VK_RIGHT:      return odfaeg::window::IKeyboard::Right;
                case VK_UP:         return odfaeg::window::IKeyboard::Up;
                case VK_DOWN:       return odfaeg::window::IKeyboard::Down;
                case VK_NUMPAD0:    return odfaeg::window::IKeyboard::Numpad0;
                case VK_NUMPAD1:    return odfaeg::window::IKeyboard::Numpad1;
                case VK_NUMPAD2:    return odfaeg::window::IKeyboard::Numpad2;
                case VK_NUMPAD3:    return odfaeg::window::IKeyboard::Numpad3;
                case VK_NUMPAD4:    return odfaeg::window::IKeyboard::Numpad4;
                case VK_NUMPAD5:    return odfaeg::window::IKeyboard::Numpad5;
                case VK_NUMPAD6:    return odfaeg::window::IKeyboard::Numpad6;
                case VK_NUMPAD7:    return odfaeg::window::IKeyboard::Numpad7;
                case VK_NUMPAD8:    return odfaeg::window::IKeyboard::Numpad8;
                case VK_NUMPAD9:    return odfaeg::window::IKeyboard::Numpad9;
                case 'A':           return odfaeg::window::IKeyboard::A;
                case 'Z':           return odfaeg::window::IKeyboard::Z;
                case 'E':           return odfaeg::window::IKeyboard::E;
                case 'R':           return odfaeg::window::IKeyboard::R;
                case 'T':           return odfaeg::window::IKeyboard::T;
                case 'Y':           return odfaeg::window::IKeyboard::Y;
                case 'U':           return odfaeg::window::IKeyboard::U;
                case 'I':           return odfaeg::window::IKeyboard::I;
                case 'O':           return odfaeg::window::IKeyboard::O;
                case 'P':           return odfaeg::window::IKeyboard::P;
                case 'Q':           return odfaeg::window::IKeyboard::Q;
                case 'S':           return odfaeg::window::IKeyboard::S;
                case 'D':           return odfaeg::window::IKeyboard::D;
                case 'F':           return odfaeg::window::IKeyboard::F;
                case 'G':           return odfaeg::window::IKeyboard::G;
                case 'H':           return odfaeg::window::IKeyboard::H;
                case 'J':           return odfaeg::window::IKeyboard::J;
                case 'K':           return odfaeg::window::IKeyboard::K;
                case 'L':           return odfaeg::window::IKeyboard::L;
                case 'M':           return odfaeg::window::IKeyboard::M;
                case 'W':           return odfaeg::window::IKeyboard::W;
                case 'X':           return odfaeg::window::IKeyboard::X;
                case 'C':           return odfaeg::window::IKeyboard::C;
                case 'V':           return odfaeg::window::IKeyboard::V;
                case 'B':           return odfaeg::window::IKeyboard::B;
                case 'N':           return odfaeg::window::IKeyboard::N;
                case '0':           return odfaeg::window::IKeyboard::Num0;
                case '1':           return odfaeg::window::IKeyboard::Num1;
                case '2':           return odfaeg::window::IKeyboard::Num2;
                case '3':           return odfaeg::window::IKeyboard::Num3;
                case '4':           return odfaeg::window::IKeyboard::Num4;
                case '5':           return odfaeg::window::IKeyboard::Num5;
                case '6':           return odfaeg::window::IKeyboard::Num6;
                case '7':           return odfaeg::window::IKeyboard::Num7;
                case '8':           return odfaeg::window::IKeyboard::Num8;
                case '9':           return odfaeg::window::IKeyboard::Num9;
            }

            return odfaeg::window::IKeyboard::Unknown;
        }


        ////////////////////////////////////////////////////////////
        LRESULT CALLBACK Win32Window::globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
        {
            // Associate handle and Window instance when the creation message is received
            if (message == WM_CREATE)
            {
                // Get Win32Window instance (it was passed as the last argument of CreateWindow)
                LONG_PTR window = (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;

                // Set as the "user data" parameter of the window
                SetWindowLongPtrW(handle, GWLP_USERDATA, window);
            }

            // Get the WindowImpl instance corresponding to the window handle
            Win32Window* window = handle ? reinterpret_cast<Win32Window*>(GetWindowLongPtr(handle, GWLP_USERDATA)) : NULL;

            // Forward the event to the appropriate function
            if (window)
            {
                window->processEvent(message, wParam, lParam);

                if (window->m_callback)
                    return CallWindowProcW(reinterpret_cast<WNDPROC>(window->m_callback), handle, message, wParam, lParam);
            }

            // We don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
            if (message == WM_CLOSE)
                return 0;

            // Don't forward the menu system command, so that pressing ALT or F10 doesn't steal the focus
            if ((message == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
                return 0;

            return DefWindowProcW(handle, message, wParam, lParam);
        }
    }
}
