#include <SFML/Window/WindowStyle.hpp>
#include "../../../../include/odfaeg/Window/x11Window.hpp"
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include "../../../../include/odfaeg/Window/display.hpp"
#include "../../../../include/odfaeg/Window/iKeyboard.hpp"
#include "../../../../include/odfaeg/Window/iMouse.hpp"
#include "../../../../include/odfaeg/Window/glxContext.hpp"
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
namespace {
    odfaeg::window::X11Window*              fullscreenWindow = NULL;
    std::vector<odfaeg::window::X11Window*> allWindows;
    sf::Mutex                             allWindowsMutex;
    sf::String                            windowManagerName;

    sf::String                            wmAbsPosGood[] = { "Enlightenment", "FVWM", "i3" };

    static const unsigned long            eventMask = FocusChangeMask      | ButtonPressMask     |
                                                      ButtonReleaseMask    | ButtonMotionMask    |
                                                      PointerMotionMask    | KeyPressMask        |
                                                      KeyReleaseMask       | StructureNotifyMask |
                                                      EnterWindowMask      | LeaveWindowMask     |
                                                      VisibilityChangeMask | PropertyChangeMask;

    static const unsigned int             maxTrialsCount = 5;

    // Predicate we use to find key repeat events in processEvent
    struct KeyRepeatFinder
    {
        KeyRepeatFinder(unsigned int keycode, Time time) : keycode(keycode), time(time) {}

        // Predicate operator that checks event type, keycode and timestamp
        bool operator()(const XEvent& event)
        {
            return ((event.type == KeyPress) && (event.xkey.keycode == keycode) && (event.xkey.time - time < 2));
        }

        unsigned int keycode;
        Time time;
    };
    // Filter the events received by windows (only allow those matching a specific window)
    Bool checkEvent(::Display*, XEvent* event, XPointer userData)
    {
        // Just check if the event matches the window
        return event->xany.window == reinterpret_cast< ::Window >(userData);
    }
    // Find the name of the current executable
    std::string findExecutableName()
    {
        // We use /proc/self/cmdline to get the command line
        // the user used to invoke this instance of the application
        int file = ::open("/proc/self/cmdline", O_RDONLY | O_NONBLOCK);

        if (file < 0)
            return "odfaeg";

        std::vector<char> buffer(256, 0);
        std::size_t offset = 0;
        ssize_t result = 0;

        while ((result = read(file, &buffer[offset], 256)) > 0)
        {
            buffer.resize(buffer.size() + result, 0);
            offset += result;
        }

        ::close(file);

        if (offset)
        {
            buffer[offset] = 0;

            // Remove the path to keep the executable name only
            return basename(&buffer[0]);
        }

        // Default fallback name
        return "odfaeg";
    }
     // Check if Extended Window Manager Hints are supported
    bool ewmhSupported()
    {
        static bool checked = false;
        static bool ewmhSupported = false;

        if (checked)
            return ewmhSupported;

        checked = true;
        Atom netSupportingWmCheck = odfaeg::window::Display::getAtom("_NET_SUPPORTING_WM_CHECK", true);
        Atom netSupported = odfaeg::window::Display::getAtom("_NET_SUPPORTED", true);

        if (!netSupportingWmCheck || !netSupported)
            return false;
        ::Display* display = odfaeg::window::Display::openDisplay();

        Atom actualType;
        int actualFormat;
        unsigned long numItems;
        unsigned long numBytes;
        unsigned char* data;

        int result = XGetWindowProperty(display,
                                        DefaultRootWindow(display),
                                        netSupportingWmCheck,
                                        0,
                                        1,
                                        False,
                                        XA_WINDOW,
                                        &actualType,
                                        &actualFormat,
                                        &numItems,
                                        &numBytes,
                                        &data);

        if (result != Success || actualType != XA_WINDOW || numItems != 1)
        {
            if (result == Success)
                XFree(data);
            odfaeg::window::Display::closeDisplay(display);
            return false;
        }

        ::Window rootWindow = *reinterpret_cast< ::Window* >(data);

        XFree(data);

        if (!rootWindow)
        {
            odfaeg::window::Display::closeDisplay(display);
            return false;
        }

        result = XGetWindowProperty(display,
                                    rootWindow,
                                    netSupportingWmCheck,
                                    0,
                                    1,
                                    False,
                                    XA_WINDOW,
                                    &actualType,
                                    &actualFormat,
                                    &numItems,
                                    &numBytes,
                                    &data);

        if (result != Success || actualType != XA_WINDOW || numItems != 1)
        {
            if (result == Success)
                XFree(data);
            odfaeg::window::Display::closeDisplay(display);
            return false;
        }

        ::Window childWindow = *reinterpret_cast< ::Window* >(data);

        XFree(data);

        if (!childWindow)
        {
            odfaeg::window::Display::closeDisplay(display);
            return false;
        }

        // Conforming window managers should return the same window for both queries
        if (rootWindow != childWindow)
        {
            odfaeg::window::Display::closeDisplay(display);
            return false;
        }

        ewmhSupported = true;

        // We try to get the name of the window manager
        // for window manager specific workarounds
        Atom netWmName = odfaeg::window::Display::getAtom("_NET_WM_NAME", true);

        if (!netWmName)
        {
            odfaeg::window::Display::closeDisplay(display);
            return true;
        }

        Atom utf8StringType = odfaeg::window::Display::getAtom("UTF8_STRING");

        if (!utf8StringType)
            utf8StringType = XA_STRING;

        result = XGetWindowProperty(display,
                                    rootWindow,
                                    netWmName,
                                    0,
                                    0x7fffffff,
                                    False,
                                    utf8StringType,
                                    &actualType,
                                    &actualFormat,
                                    &numItems,
                                    &numBytes,
                                    &data);

        if (actualType && numItems)
        {
            // It seems the wm name string reply is not necessarily
            // null-terminated. The work around is to get its actual
            // length to build a proper string
            const char* begin = reinterpret_cast<const char*>(data);
            const char* end = begin + numItems;
            windowManagerName = sf::String::fromUtf8(begin, end);
        }
        if (result == Success)
            XFree(data);
        odfaeg::window::Display::closeDisplay(display);
        return true;
    }

    // Get the parent window.
    ::Window getParentWindow(::Display* disp, ::Window win)
    {
        ::Window root, parent;
        ::Window* children = NULL;
        unsigned int numChildren;

        XQueryTree(disp, win, &root, &parent, &children, &numChildren);

        // Children information is not used, so must be freed.
        if (children != NULL)
            XFree(children);

        return parent;
    }

    // Get the Frame Extents from EWMH WMs that support it.
    bool getEWMHFrameExtents(::Display* disp, ::Window win,
        long& xFrameExtent, long& yFrameExtent)
    {
        if (!ewmhSupported())
            return false;
        Atom frameExtents = odfaeg::window::Display::getAtom("_NET_FRAME_EXTENTS", true);

        if (frameExtents == None)
            return false;

        bool gotFrameExtents = false;
        Atom actualType;
        int actualFormat;
        unsigned long numItems;
        unsigned long numBytesLeft;
        unsigned char* data = NULL;

        int result = XGetWindowProperty(disp,
                                        win,
                                        frameExtents,
                                        0,
                                        4,
                                        False,
                                        XA_CARDINAL,
                                        &actualType,
                                        &actualFormat,
                                        &numItems,
                                        &numBytesLeft,
                                        &data);

        if ((result == Success) && (actualType == XA_CARDINAL) &&
            (actualFormat == 32) && (numItems == 4) && (numBytesLeft == 0) &&
            (data != NULL))
        {
            gotFrameExtents = true;

            long* extents = (long*) data;

            xFrameExtent = extents[0]; // Left.
            yFrameExtent = extents[2]; // Top.
        }

        // Always free data.
        if (data != NULL)
            XFree(data);

        return gotFrameExtents;
    }

    // Check if the current WM is in the list of good WMs that provide
    // a correct absolute position for the window when queried.
    bool isWMAbsolutePositionGood()
    {
        // This can only work with EWMH, to get the name.
        if (!ewmhSupported())
            return false;

        for (size_t i = 0; i < (sizeof(wmAbsPosGood) / sizeof(wmAbsPosGood[0])); i++)
        {
            if (wmAbsPosGood[i] == windowManagerName)
                return true;
        }

        return false;
    }
    odfaeg::window::IKeyboard::Key keysymToODFAEG(KeySym symbol)
    {
        switch (symbol)
        {
            case XK_Shift_L:      return odfaeg::window::IKeyboard::LShift;
            case XK_Shift_R:      return odfaeg::window::IKeyboard::RShift;
            case XK_Control_L:    return odfaeg::window::IKeyboard::LControl;
            case XK_Control_R:    return odfaeg::window::IKeyboard::RControl;
            case XK_Alt_L:        return odfaeg::window::IKeyboard::LAlt;
            case XK_Alt_R:        return odfaeg::window::IKeyboard::RAlt;
            case XK_Super_L:      return odfaeg::window::IKeyboard::LSystem;
            case XK_Super_R:      return odfaeg::window::IKeyboard::RSystem;
            case XK_Menu:         return odfaeg::window::IKeyboard::Menu;
            case XK_Escape:       return odfaeg::window::IKeyboard::Escape;
            case XK_semicolon:    return odfaeg::window::IKeyboard::Semicolon;
            case XK_slash:        return odfaeg::window::IKeyboard::Slash;
            case XK_equal:        return odfaeg::window::IKeyboard::Equal;
            case XK_minus:        return odfaeg::window::IKeyboard::Hyphen;
            case XK_bracketleft:  return odfaeg::window::IKeyboard::LBracket;
            case XK_bracketright: return odfaeg::window::IKeyboard::RBracket;
            case XK_comma:        return odfaeg::window::IKeyboard::Comma;
            case XK_period:       return odfaeg::window::IKeyboard::Period;
            case XK_apostrophe:   return odfaeg::window::IKeyboard::Quote;
            case XK_backslash:    return odfaeg::window::IKeyboard::Backslash;
            case XK_grave:        return odfaeg::window::IKeyboard::Tilde;
            case XK_space:        return odfaeg::window::IKeyboard::Space;
            case XK_Return:       return odfaeg::window::IKeyboard::Enter;
            case XK_KP_Enter:     return odfaeg::window::IKeyboard::Enter;
            case XK_BackSpace:    return odfaeg::window::IKeyboard::Backspace;
            case XK_Tab:          return odfaeg::window::IKeyboard::Tab;
            case XK_Prior:        return odfaeg::window::IKeyboard::PageUp;
            case XK_Next:         return odfaeg::window::IKeyboard::PageDown;
            case XK_End:          return odfaeg::window::IKeyboard::End;
            case XK_Home:         return odfaeg::window::IKeyboard::Home;
            case XK_Insert:       return odfaeg::window::IKeyboard::Insert;
            case XK_Delete:       return odfaeg::window::IKeyboard::Delete;
            case XK_KP_Add:       return odfaeg::window::IKeyboard::Add;
            case XK_KP_Subtract:  return odfaeg::window::IKeyboard::Subtract;
            case XK_KP_Multiply:  return odfaeg::window::IKeyboard::Multiply;
            case XK_KP_Divide:    return odfaeg::window::IKeyboard::Divide;
            case XK_Pause:        return odfaeg::window::IKeyboard::Pause;
            case XK_F1:           return odfaeg::window::IKeyboard::F1;
            case XK_F2:           return odfaeg::window::IKeyboard::F2;
            case XK_F3:           return odfaeg::window::IKeyboard::F3;
            case XK_F4:           return odfaeg::window::IKeyboard::F4;
            case XK_F5:           return odfaeg::window::IKeyboard::F5;
            case XK_F6:           return odfaeg::window::IKeyboard::F6;
            case XK_F7:           return odfaeg::window::IKeyboard::F7;
            case XK_F8:           return odfaeg::window::IKeyboard::F8;
            case XK_F9:           return odfaeg::window::IKeyboard::F9;
            case XK_F10:          return odfaeg::window::IKeyboard::F10;
            case XK_F11:          return odfaeg::window::IKeyboard::F11;
            case XK_F12:          return odfaeg::window::IKeyboard::F12;
            case XK_F13:          return odfaeg::window::IKeyboard::F13;
            case XK_F14:          return odfaeg::window::IKeyboard::F14;
            case XK_F15:          return odfaeg::window::IKeyboard::F15;
            case XK_Left:         return odfaeg::window::IKeyboard::Left;
            case XK_Right:        return odfaeg::window::IKeyboard::Right;
            case XK_Up:           return odfaeg::window::IKeyboard::Up;
            case XK_Down:         return odfaeg::window::IKeyboard::Down;
            case XK_KP_Insert:    return odfaeg::window::IKeyboard::Numpad0;
            case XK_KP_End:       return odfaeg::window::IKeyboard::Numpad1;
            case XK_KP_Down:      return odfaeg::window::IKeyboard::Numpad2;
            case XK_KP_Page_Down: return odfaeg::window::IKeyboard::Numpad3;
            case XK_KP_Left:      return odfaeg::window::IKeyboard::Numpad4;
            case XK_KP_Begin:     return odfaeg::window::IKeyboard::Numpad5;
            case XK_KP_Right:     return odfaeg::window::IKeyboard::Numpad6;
            case XK_KP_Home:      return odfaeg::window::IKeyboard::Numpad7;
            case XK_KP_Up:        return odfaeg::window::IKeyboard::Numpad8;
            case XK_KP_Page_Up:   return odfaeg::window::IKeyboard::Numpad9;
            case XK_a:            return odfaeg::window::IKeyboard::A;
            case XK_b:            return odfaeg::window::IKeyboard::B;
            case XK_c:            return odfaeg::window::IKeyboard::C;
            case XK_d:            return odfaeg::window::IKeyboard::D;
            case XK_e:            return odfaeg::window::IKeyboard::E;
            case XK_f:            return odfaeg::window::IKeyboard::F;
            case XK_g:            return odfaeg::window::IKeyboard::G;
            case XK_h:            return odfaeg::window::IKeyboard::H;
            case XK_i:            return odfaeg::window::IKeyboard::I;
            case XK_j:            return odfaeg::window::IKeyboard::J;
            case XK_k:            return odfaeg::window::IKeyboard::K;
            case XK_l:            return odfaeg::window::IKeyboard::L;
            case XK_m:            return odfaeg::window::IKeyboard::M;
            case XK_n:            return odfaeg::window::IKeyboard::N;
            case XK_o:            return odfaeg::window::IKeyboard::O;
            case XK_p:            return odfaeg::window::IKeyboard::P;
            case XK_q:            return odfaeg::window::IKeyboard::Q;
            case XK_r:            return odfaeg::window::IKeyboard::R;
            case XK_s:            return odfaeg::window::IKeyboard::S;
            case XK_t:            return odfaeg::window::IKeyboard::T;
            case XK_u:            return odfaeg::window::IKeyboard::U;
            case XK_v:            return odfaeg::window::IKeyboard::V;
            case XK_w:            return odfaeg::window::IKeyboard::W;
            case XK_x:            return odfaeg::window::IKeyboard::X;
            case XK_y:            return odfaeg::window::IKeyboard::Y;
            case XK_z:            return odfaeg::window::IKeyboard::Z;
            case XK_0:            return odfaeg::window::IKeyboard::Num0;
            case XK_1:            return odfaeg::window::IKeyboard::Num1;
            case XK_2:            return odfaeg::window::IKeyboard::Num2;
            case XK_3:            return odfaeg::window::IKeyboard::Num3;
            case XK_4:            return odfaeg::window::IKeyboard::Num4;
            case XK_5:            return odfaeg::window::IKeyboard::Num5;
            case XK_6:            return odfaeg::window::IKeyboard::Num6;
            case XK_7:            return odfaeg::window::IKeyboard::Num7;
            case XK_8:            return odfaeg::window::IKeyboard::Num8;
            case XK_9:            return odfaeg::window::IKeyboard::Num9;
        }

        return odfaeg::window::IKeyboard::Unknown;
    }
}
namespace odfaeg {
    namespace window {
        using namespace sf;
        X11Window::X11Window() :
        m_window         (0),
        m_screen         (0),
        m_inputMethod    (NULL),
        m_inputContext   (NULL),
        m_isExternal     (true),
        m_oldVideoMode   (0),
        m_oldRRCrtc      (0),
        m_hiddenCursor   (0),
        m_lastCursor     (None),
        m_keyRepeat      (true),
        m_previousSize   (-1, -1),
        m_useSizeHints   (false),
        m_fullscreen     (false),
        m_cursorGrabbed  (false),
        m_windowMapped   (false),
        m_iconPixmap     (0),
        m_iconMaskPixmap (0),
        m_lastInputTime  (0),
        m_opened(false),
        m_destroyed(true) {
             // Open a connection with the X server
            m_display = Display::openDisplay();

            // Make sure to check for EWMH support before we do anything
            ewmhSupported();
        }
        ////////////////////////////////////////////////////////////
        X11Window::X11Window(sf::WindowHandle handle, const ContextSettings& settings) :
        m_window         (0),
        m_screen         (0),
        m_inputMethod    (NULL),
        m_inputContext   (NULL),
        m_isExternal     (true),
        m_oldVideoMode   (0),
        m_oldRRCrtc      (0),
        m_hiddenCursor   (0),
        m_lastCursor     (None),
        m_keyRepeat      (true),
        m_previousSize   (-1, -1),
        m_useSizeHints   (false),
        m_fullscreen     (false),
        m_cursorGrabbed  (false),
        m_windowMapped   (false),
        m_iconPixmap     (0),
        m_iconMaskPixmap (0),
        m_lastInputTime  (0),
        m_opened(false),
        m_destroyed(true) {
            // Open a connection with the X server
            m_display = Display::openDisplay();

            // Make sure to check for EWMH support before we do anything
            ewmhSupported();

            create(handle, settings);
        }
        X11Window::X11Window(VideoMode mode, const String& title, Uint32 style, const ContextSettings& settings) :
        m_window         (0),
        m_screen         (0),
        m_inputMethod    (NULL),
        m_inputContext   (NULL),
        m_isExternal     (false),
        m_oldVideoMode   (0),
        m_oldRRCrtc      (0),
        m_hiddenCursor   (0),
        m_lastCursor     (None),
        m_keyRepeat      (true),
        m_previousSize   (-1, -1),
        m_useSizeHints   (false),
        m_fullscreen     ((style & Style::Fullscreen) != 0),
        m_cursorGrabbed  (m_fullscreen),
        m_windowMapped   (false),
        m_iconPixmap     (0),
        m_iconMaskPixmap (0),
        m_lastInputTime  (0),
        m_opened(false),
        m_destroyed(true) {
            m_display = Display::openDisplay();
            ewmhSupported();
            create(mode, title, style, settings);
        }
        void X11Window::create(sf::WindowHandle handle, const ContextSettings& settings) {
            m_screen = DefaultScreen(m_display);

            // Save the window handle
            m_window = handle;

            if (m_window)
            {
                // Make sure the window is listening to all the required events
                XSetWindowAttributes attributes;
                attributes.event_mask = eventMask;

                XChangeWindowAttributes(m_display, m_window, CWEventMask, &attributes);

                // Set the WM protocols
                setProtocols();

                // Do some common initializations
                initialize();
            }
        }
        void X11Window::create (VideoMode mode, const String& title, Uint32 style, const ContextSettings& settings) {

            m_screen = DefaultScreen(m_display);
            XVisualInfo vi = GlxContext::selectBestVisual(m_display, mode.bitsPerPixel, settings);
            XSetWindowAttributes swa;
            Colormap cmap;
            swa.colormap = cmap = XCreateColormap(m_display,
                                                   DefaultRootWindow(m_display),
                                                   vi.visual, AllocNone );
            swa.border_pixel      = 0;
            swa.event_mask        = eventMask;
            swa.override_redirect = (m_fullscreen && !ewmhSupported()) ? True : False;
            Vector2i windowPosition;
            if(m_fullscreen)
            {
                windowPosition = getPrimaryMonitorPosition();
            }
            else
            {
                windowPosition.x = (DisplayWidth(m_display, m_screen)  - mode.width) / 2;
                windowPosition.y = (DisplayWidth(m_display, m_screen)  - mode.height) / 2;
            }
            std::cout<<"Creating window"<<std::endl;
            m_window = XCreateWindow(m_display, DefaultRootWindow(m_display),
                                          windowPosition.x, windowPosition.y, mode.width, mode.height, 0, vi.depth, InputOutput,
                                          vi.visual,
                                          CWBorderPixel|CWOverrideRedirect|CWColormap|CWEventMask, &swa );

            setProtocols();
            // Set the WM initial state to the normal state
            XWMHints* hints = XAllocWMHints();
            hints->flags         = StateHint;
            hints->initial_state = NormalState;
            XSetWMHints(m_display, m_window, hints);
            XFree(hints);
            if (!m_fullscreen) {
                // change our window's decorations and functions according to the requested style)
                Atom WMHintsAtom = Display::getAtom("_MOTIF_WM_HINTS", false);
                if (WMHintsAtom)
                {
                    static const unsigned long MWM_HINTS_FUNCTIONS   = 1 << 0;
                    static const unsigned long MWM_HINTS_DECORATIONS = 1 << 1;

                    //static const unsigned long MWM_DECOR_ALL         = 1 << 0;
                    static const unsigned long MWM_DECOR_BORDER      = 1 << 1;
                    static const unsigned long MWM_DECOR_RESIZEH     = 1 << 2;
                    static const unsigned long MWM_DECOR_TITLE       = 1 << 3;
                    static const unsigned long MWM_DECOR_MENU        = 1 << 4;
                    static const unsigned long MWM_DECOR_MINIMIZE    = 1 << 5;
                    static const unsigned long MWM_DECOR_MAXIMIZE    = 1 << 6;

                    //static const unsigned long MWM_FUNC_ALL          = 1 << 0;
                    static const unsigned long MWM_FUNC_RESIZE       = 1 << 1;
                    static const unsigned long MWM_FUNC_MOVE         = 1 << 2;
                    static const unsigned long MWM_FUNC_MINIMIZE     = 1 << 3;
                    static const unsigned long MWM_FUNC_MAXIMIZE     = 1 << 4;
                    static const unsigned long MWM_FUNC_CLOSE        = 1 << 5;

                    struct WMHints
                    {
                        unsigned long flags;
                        unsigned long functions;
                        unsigned long decorations;
                        long          inputMode;
                        unsigned long state;
                    };

                    WMHints hints;
                    std::memset(&hints, 0, sizeof(hints));
                    hints.flags       = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
                    hints.decorations = 0;
                    hints.functions   = 0;

                    if (style & Style::Titlebar)
                    {
                        hints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MINIMIZE | MWM_DECOR_MENU;
                        hints.functions   |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
                    }
                    if (style & Style::Resize)
                    {
                        hints.decorations |= MWM_DECOR_MAXIMIZE | MWM_DECOR_RESIZEH;
                        hints.functions   |= MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;
                    }
                    if (style & Style::Close)
                    {
                        hints.decorations |= 0;
                        hints.functions   |= MWM_FUNC_CLOSE;
                    }

                    XChangeProperty(m_display,
                                    m_window,
                                    WMHintsAtom,
                                    WMHintsAtom,
                                    32,
                                    PropModeReplace,
                                    reinterpret_cast<const unsigned char*>(&hints),
                                    5);
                }
            }
            // This is a hack to force some windows managers to disable resizing
            if (!(style & Style::Resize))
            {
                XSizeHints* sizeHints = XAllocSizeHints();
                sizeHints->flags = PMinSize | PMaxSize | USPosition;
                sizeHints->min_width = sizeHints->max_width = mode.width;
                sizeHints->min_height = sizeHints->max_height = mode.height;
                sizeHints->x = windowPosition.x;
                sizeHints->y = windowPosition.y;
                XSetWMNormalHints(m_display, m_window, sizeHints);
                XFree(sizeHints);
            }

            // Set the window's WM class (this can be used by window managers)
            XClassHint* hint = XAllocClassHint();

            // The instance name should be something unique to this invocation
            // of the application but is rarely if ever used these days.
            // For simplicity, we retrieve it via the base executable name.
            std::string executableName = findExecutableName();
            std::vector<char> windowInstance(executableName.size() + 1, 0);
            std::copy(executableName.begin(), executableName.end(), windowInstance.begin());
            hint->res_name = &windowInstance[0];

            // The class name identifies a class of windows that
            // "are of the same type". We simply use the initial window name as
            // the class name.
            std::string ansiTitle = title;
            std::vector<char> windowClass(ansiTitle.size() + 1, 0);
            std::copy(ansiTitle.begin(), ansiTitle.end(), windowClass.begin());
            hint->res_class = &windowClass[0];

            XSetClassHint(m_display, m_window, hint);

            XFree(hint);
            setTitle(title);
            initialize();
            // Set fullscreen video mode and switch to fullscreen if necessary
            if (m_fullscreen)
            {
                // Disable hint for min and max size,
                // otherwise some windows managers will not remove window decorations
                XSizeHints *sizeHints = XAllocSizeHints();
                long flags = 0;
                XGetWMNormalHints(m_display, m_window, sizeHints, &flags);
                sizeHints->flags &= ~(PMinSize | PMaxSize);
                XSetWMNormalHints(m_display, m_window, sizeHints);
                XFree(sizeHints);

                setVideoMode(mode);
                switchToFullscreen();
            }
        }
        void X11Window::initialize() {
            // Create the input context
            m_inputMethod = XOpenIM(m_display, NULL, NULL, NULL);

            if (m_inputMethod)
            {
                m_inputContext = XCreateIC(m_inputMethod,
                                           XNClientWindow,
                                           m_window,
                                           XNFocusWindow,
                                           m_window,
                                           XNInputStyle,
                                           XIMPreeditNothing | XIMStatusNothing,
                                           reinterpret_cast<void*>(NULL));
            }
            else
            {
                m_inputContext = NULL;
            }

            if (!m_inputContext)
                std::cerr << "Failed to create input context for window -- TextEntered event won't be able to return unicode" << std::endl;

            Atom wmWindowType = Display::getAtom("_NET_WM_WINDOW_TYPE", false);
            Atom wmWindowTypeNormal = Display::getAtom("_NET_WM_WINDOW_TYPE_NORMAL", false);

            if (wmWindowType && wmWindowTypeNormal)
            {
                XChangeProperty(m_display,
                                m_window,
                                wmWindowType,
                                XA_ATOM,
                                32,
                                PropModeReplace,
                                reinterpret_cast<const unsigned char*>(&wmWindowTypeNormal),
                                1);
            }

            // Show the window
            setVisible(true);

            // Raise the window and grab input focus
            grabFocus();

            // Create the hidden cursor
            createHiddenCursor();

            // Flush the commands queue
            XFlush(m_display);

            // Add this window to the global list of windows (required for focus request)
            Lock lock(allWindowsMutex);
            allWindows.push_back(this);
            m_opened = true;
            m_destroyed = false;
        }
        void X11Window::setPosition(const Vector2i& position) {
            XMoveWindow(m_display, m_window, position.x, position.y);
            XFlush(m_display);
        }
        Vector2i X11Window::getPosition() const {
            // Get absolute position of our window relative to root window. This
            // takes into account all information that X11 has, including X11
            // border widths and any decorations. It corresponds to where the
            // window actually is, but not necessarily to where we told it to
            // go using setPosition() and XMoveWindow(). To have the two match
            // as expected, we may have to subtract decorations and borders.
            ::Window child;
            int xAbsRelToRoot, yAbsRelToRoot;

            XTranslateCoordinates(m_display, m_window, DefaultRootWindow(m_display),
                0, 0, &xAbsRelToRoot, &yAbsRelToRoot, &child);

            // CASE 1: some rare WMs actually put the window exactly where we tell
            // it to, even with decorations and such, which get shifted back.
            // In these rare cases, we can use the absolute value directly.
            if (isWMAbsolutePositionGood())
                return Vector2i(xAbsRelToRoot, yAbsRelToRoot);

            // CASE 2: most modern WMs support EWMH and can define _NET_FRAME_EXTENTS
            // with the exact frame size to subtract, so if present, we prefer it and
            // query it first. According to spec, this already includes any borders.
            long xFrameExtent, yFrameExtent;

            if (getEWMHFrameExtents(m_display, m_window, xFrameExtent, yFrameExtent))
            {
                // Get final X/Y coordinates: subtract EWMH frame extents from
                // absolute window position.
                return Vector2i((xAbsRelToRoot - xFrameExtent), (yAbsRelToRoot - yFrameExtent));
            }

            // CASE 3: EWMH frame extents were not available, use geometry.
            // We climb back up to the window before the root and use its
            // geometry information to extract X/Y position. This because
            // re-parenting WMs may re-parent the window multiple times, so
            // we'd have to climb up to the furthest ancestor and sum the
            // relative differences and borders anyway; and doing that to
            // subtract those values from the absolute coordinates of the
            // window is equivalent to going up the tree and asking the
            // furthest ancestor what it's relative distance to the root is.
            // So we use that approach because it's simpler.
            // This approach assumes that any window between the root and
            // our window is part of decorations/borders in some way. This
            // seems to hold true for most reasonable WM implementations.
            ::Window ancestor = m_window;
            ::Window root = DefaultRootWindow(m_display);

            while (getParentWindow(m_display, ancestor) != root)
            {
                // Next window up (parent window).
                ancestor = getParentWindow(m_display, ancestor);
            }

            // Get final X/Y coordinates: take the relative position to
            // the root of the furthest ancestor window.
            int xRelToRoot, yRelToRoot;
            unsigned int width, height, borderWidth, depth;

            XGetGeometry(m_display, ancestor, &root, &xRelToRoot, &yRelToRoot,
                &width, &height, &borderWidth, &depth);

            return Vector2i(xRelToRoot, yRelToRoot);
        }
        Vector2u X11Window::getSize() const
        {
            XWindowAttributes attributes;
            XGetWindowAttributes(m_display, m_window, &attributes);
            return Vector2u(attributes.width, attributes.height);
        }
        void X11Window::setSize(const Vector2u& size)
        {
            // If resizing is disable for the window we have to update the size hints (required by some window managers).
            if (m_useSizeHints)
            {
                XSizeHints* sizeHints = XAllocSizeHints();
                sizeHints->flags = PMinSize | PMaxSize;
                sizeHints->min_width = sizeHints->max_width = size.x;
                sizeHints->min_height = sizeHints->max_height = size.y;
                XSetWMNormalHints(m_display, m_window, sizeHints);
                XFree(sizeHints);
            }

            XResizeWindow(m_display, m_window, size.x, size.y);
            XFlush(m_display);
        }
        void X11Window::setIcon(unsigned int width, unsigned int height, const Uint8* pixels)
        {
            // X11 wants BGRA pixels: swap red and blue channels
            // Note: this memory will be freed by XDestroyImage
            Uint8* iconPixels = static_cast<Uint8*>(std::malloc(width * height * 4));
            for (std::size_t i = 0; i < width * height; ++i)
            {
                iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
                iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
                iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
                iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
            }

            // Create the icon pixmap
            Visual*      defVisual = DefaultVisual(m_display, m_screen);
            unsigned int defDepth  = DefaultDepth(m_display, m_screen);
            XImage* iconImage = XCreateImage(m_display, defVisual, defDepth, ZPixmap, 0, (char*)iconPixels, width, height, 32, 0);
            if (!iconImage)
            {
                err() << "Failed to set the window's icon" << std::endl;
                return;
            }

            if (m_iconPixmap)
                XFreePixmap(m_display, m_iconPixmap);

            if (m_iconMaskPixmap)
                XFreePixmap(m_display, m_iconMaskPixmap);

            m_iconPixmap = XCreatePixmap(m_display, RootWindow(m_display, m_screen), width, height, defDepth);
            XGCValues values;
            GC iconGC = XCreateGC(m_display, m_iconPixmap, 0, &values);
            XPutImage(m_display, m_iconPixmap, iconGC, iconImage, 0, 0, 0, 0, width, height);
            XFreeGC(m_display, iconGC);
            XDestroyImage(iconImage);

            // Create the mask pixmap (must have 1 bit depth)
            std::size_t pitch = (width + 7) / 8;
            std::vector<Uint8> maskPixels(pitch * height, 0);
            for (std::size_t j = 0; j < height; ++j)
            {
                for (std::size_t i = 0; i < pitch; ++i)
                {
                    for (std::size_t k = 0; k < 8; ++k)
                    {
                        if (i * 8 + k < width)
                        {
                            Uint8 opacity = (pixels[(i * 8 + k + j * width) * 4 + 3] > 0) ? 1 : 0;
                            maskPixels[i + j * pitch] |= (opacity << k);
                        }
                    }
                }
            }
            m_iconMaskPixmap = XCreatePixmapFromBitmapData(m_display, m_window, (char*)&maskPixels[0], width, height, 1, 0, 1);

            // Send our new icon to the window through the WMHints
            XWMHints* hints = XAllocWMHints();
            hints->flags       = IconPixmapHint | IconMaskHint;
            hints->icon_pixmap = m_iconPixmap;
            hints->icon_mask   = m_iconMaskPixmap;
            XSetWMHints(m_display, m_window, hints);
            XFree(hints);

            // ICCCM wants BGRA pixels: swap red and blue channels
            // ICCCM also wants the first 2 unsigned 32-bit values to be width and height
            std::vector<unsigned long> icccmIconPixels(2 + width * height, 0);
            unsigned long* ptr = &icccmIconPixels[0];

            *ptr++ = width;
            *ptr++ = height;

            for (std::size_t i = 0; i < width * height; ++i)
            {
                *ptr++ = (pixels[i * 4 + 2] << 0 ) |
                         (pixels[i * 4 + 1] << 8 ) |
                         (pixels[i * 4 + 0] << 16) |
                         (pixels[i * 4 + 3] << 24);
            }

            Atom netWmIcon = Display::getAtom("_NET_WM_ICON");

            XChangeProperty(m_display,
                            m_window,
                            netWmIcon,
                            XA_CARDINAL,
                            32,
                            PropModeReplace,
                            reinterpret_cast<const unsigned char*>(&icccmIconPixels[0]),
                            2 + width * height);

            XFlush(m_display);
        }
        void X11Window::setMouseCursorVisible(bool visible)
        {
            XDefineCursor(m_display, m_window, visible ? m_lastCursor : m_hiddenCursor);
            XFlush(m_display);
        }
        /*void X11Window::setMouseCursor(const sf::Cursor& cursor)
        {
            m_lastCursor = cursor.getImpl().m_cursor;
            XDefineCursor(m_display, m_window, m_lastCursor);
        }*/
        void X11Window::setMouseCursorGrabbed(bool grabbed)
        {
            // This has no effect in fullscreen mode
            if (m_fullscreen || (m_cursorGrabbed == grabbed))
                return;

            if (grabbed)
            {
                // Try multiple times to grab the cursor
                for (unsigned int trial = 0; trial < maxTrialsCount; ++trial)
                {
                    int result = XGrabPointer(m_display, m_window, True, None, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime);

                    if (result == GrabSuccess)
                    {
                        m_cursorGrabbed = true;
                        break;
                    }

                    // The cursor grab failed, trying again after a small sleep
                    sf::sleep(sf::milliseconds(50));
                }

                if (!m_cursorGrabbed)
                    err() << "Failed to grab mouse cursor" << std::endl;
            }
            else
            {
                XUngrabPointer(m_display, CurrentTime);
            }
        }
        void X11Window::setVisible(bool visible) {
            if (visible)
            {
                XMapWindow(m_display, m_window);

                if(m_fullscreen)
                    switchToFullscreen();

                XFlush(m_display);

                // Before continuing, make sure the WM has
                // internally marked the window as viewable
                while (!m_windowMapped && !m_isExternal)
                    processEvents();
            }
            else
            {
                //If the window has just been created we need to do this otherwise the window is not unmapped.
                sf::sleep(sf::seconds(0.5f));
                XUnmapWindow(m_display, m_window);

                XFlush(m_display);

                // Before continuing, make sure the WM has
                // internally marked the window as unviewable
                while (m_windowMapped && !m_isExternal)
                    processEvents();
            }
        }
        void X11Window::setKeyRepeatEnabled(bool enabled)
        {
            m_keyRepeat = enabled;
        }


        ////////////////////////////////////////////////////////////
        void X11Window::requestFocus()
        {
            // Focus is only stolen among SFML windows, not between applications
            // Check the global list of windows to find out whether an SFML window has the focus
            // Note: can't handle console and other non-SFML windows belonging to the application.
            bool sfmlWindowFocused = false;

            {
                Lock lock(allWindowsMutex);
                for (std::vector<X11Window*>::iterator itr = allWindows.begin(); itr != allWindows.end(); ++itr)
                {
                    if ((*itr)->hasFocus())
                    {
                        sfmlWindowFocused = true;
                        break;
                    }
                }
            }

            // Check if window is viewable (not on other desktop, ...)
            // TODO: Check also if minimized
            XWindowAttributes attributes;
            if (XGetWindowAttributes(m_display, m_window, &attributes) == 0)
            {
                sf::err() << "Failed to check if window is viewable while requesting focus" << std::endl;
                return; // error getting attribute
            }

            bool windowViewable = (attributes.map_state == IsViewable);

            if (sfmlWindowFocused && windowViewable)
            {
                // Another SFML window of this application has the focus and the current window is viewable:
                // steal focus (i.e. bring window to the front and give it input focus)
                grabFocus();
            }
            else
            {
                // Otherwise: display urgency hint (flashing application logo)
                // Ensure WM hints exist, allocate if necessary
                XWMHints* hints = XGetWMHints(m_display, m_window);
                if (hints == NULL)
                    hints = XAllocWMHints();

                // Add urgency (notification) flag to hints
                hints->flags |= XUrgencyHint;
                XSetWMHints(m_display, m_window, hints);
                XFree(hints);
            }
        }


        ////////////////////////////////////////////////////////////
        bool X11Window::hasFocus() const
        {
            ::Window focusedWindow = 0;
            int revertToReturn = 0;
            XGetInputFocus(m_display, &focusedWindow, &revertToReturn);

            return (m_window == focusedWindow);
        }


        ////////////////////////////////////////////////////////////
        void X11Window::grabFocus()
        {
            Atom netActiveWindow = None;

            if (ewmhSupported())
                netActiveWindow = Display::getAtom("_NET_ACTIVE_WINDOW");

            // Only try to grab focus if the window is mapped
            XWindowAttributes attr;

            XGetWindowAttributes(m_display, m_window, &attr);

            if (attr.map_state == IsUnmapped)
                return;

            if (netActiveWindow)
            {
                XEvent event;
                std::memset(&event, 0, sizeof(event));

                event.type = ClientMessage;
                event.xclient.window = m_window;
                event.xclient.format = 32;
                event.xclient.message_type = netActiveWindow;
                event.xclient.data.l[0] = 1; // Normal application
                event.xclient.data.l[1] = m_lastInputTime;
                event.xclient.data.l[2] = 0; // We don't know the currently active window

                int result = XSendEvent(m_display,
                                        DefaultRootWindow(m_display),
                                        False,
                                        SubstructureNotifyMask | SubstructureRedirectMask,
                                        &event);

                XFlush(m_display);

                if (!result)
                    err() << "Setting fullscreen failed, could not send \"_NET_ACTIVE_WINDOW\" event" << std::endl;
            }
            else
            {
                XRaiseWindow(m_display, m_window);
                XSetInputFocus(m_display, m_window, RevertToPointerRoot, CurrentTime);
                XFlush(m_display);
            }
        }


        ////////////////////////////////////////////////////////////
        void X11Window::setVideoMode(const VideoMode& mode)
        {
            // Skip mode switching if the new mode is equal to the desktop mode
            if (mode == VideoMode::getDesktopMode())
                return;

            // Check if the XRandR extension is present
            int xRandRMajor, xRandRMinor;
            if (!checkXRandR(xRandRMajor, xRandRMinor))
            {
                // XRandR extension is not supported: we cannot use fullscreen mode
                err() << "Fullscreen is not supported, switching to window mode" << std::endl;
                return;
            }

            // Get root window
            ::Window rootWindow = RootWindow(m_display, m_screen);

            // Get the screen resources
            XRRScreenResources* res = XRRGetScreenResources(m_display, rootWindow);
            if (!res)
            {
                err() << "Failed to get the current screen resources for fullscreen mode, switching to window mode" << std::endl;
                return;
            }

            RROutput output = getOutputPrimary(rootWindow, res, xRandRMajor, xRandRMinor);

            // Get output info from output
            XRROutputInfo* outputInfo = XRRGetOutputInfo(m_display, res, output);
            if (!outputInfo || outputInfo->connection == RR_Disconnected)
            {
                XRRFreeScreenResources(res);

                // If outputInfo->connection == RR_Disconnected, free output info
                if (outputInfo)
                    XRRFreeOutputInfo(outputInfo);

                err() << "Failed to get output info for fullscreen mode, switching to window mode" << std::endl;
                return;
            }

            // Retreive current RRMode, screen position and rotation
            XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(m_display, res, outputInfo->crtc);
            if (!crtcInfo)
            {
                XRRFreeScreenResources(res);
                XRRFreeOutputInfo(outputInfo);
                err() << "Failed to get crtc info for fullscreen mode, switching to window mode" << std::endl;
                return;
            }

            // Find RRMode to set
            bool modeFound = false;
            RRMode xRandMode;

            for (int i = 0; (i < res->nmode) && !modeFound; i++)
            {
                if (crtcInfo->rotation == RR_Rotate_90 || crtcInfo->rotation == RR_Rotate_270)
                    std::swap(res->modes[i].height, res->modes[i].width);

                // Check if screen size match
                if (res->modes[i].width == static_cast<int>(mode.width) &&
                    res->modes[i].height == static_cast<int>(mode.height))
                {
                    xRandMode = res->modes[i].id;
                    modeFound = true;
                }
            }

            if (!modeFound)
            {
                XRRFreeScreenResources(res);
                XRRFreeOutputInfo(outputInfo);
                err() << "Failed to find a matching RRMode for fullscreen mode, switching to window mode" << std::endl;
                return;
            }

            // Save the current video mode before we switch to fullscreen
            m_oldVideoMode = crtcInfo->mode;
            m_oldRRCrtc = outputInfo->crtc;

            // Switch to fullscreen mode
            XRRSetCrtcConfig(m_display,
                             res,
                             outputInfo->crtc,
                             CurrentTime,
                             crtcInfo->x,
                             crtcInfo->y,
                             xRandMode,
                             crtcInfo->rotation,
                             &output,
                             1);

            // Set "this" as the current fullscreen window
            fullscreenWindow = this;

            XRRFreeScreenResources(res);
            XRRFreeOutputInfo(outputInfo);
            XRRFreeCrtcInfo(crtcInfo);
        }


        ////////////////////////////////////////////////////////////
        void X11Window::resetVideoMode()
        {
            if (fullscreenWindow == this)
            {
                // Try to set old configuration
                // Check if the XRandR extension
                int xRandRMajor, xRandRMinor;
                if (checkXRandR(xRandRMajor, xRandRMinor))
                {
                    XRRScreenResources* res = XRRGetScreenResources(m_display, DefaultRootWindow(m_display));
                    if (!res)
                    {
                        err() << "Failed to get the current screen resources to reset the video mode" << std::endl;
                        return;
                    }

                    // Retreive current screen position and rotation
                    XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(m_display, res, m_oldRRCrtc);
                    if (!crtcInfo)
                    {
                        XRRFreeScreenResources(res);
                        err() << "Failed to get crtc info to reset the video mode" << std::endl;
                        return;
                    }

                    RROutput output;

                    // if version >= 1.3 get the primary screen else take the first screen
                    if ((xRandRMajor == 1 && xRandRMinor >= 3) || xRandRMajor > 1)
                    {
                        output = XRRGetOutputPrimary(m_display, DefaultRootWindow(m_display));

                        // Check if returned output is valid, otherwise use the first screen
                        if (output == None)
                            output = res->outputs[0];
                    }
                    else{
                        output = res->outputs[0];
                    }

                    XRRSetCrtcConfig(m_display,
                                     res,
                                     m_oldRRCrtc,
                                     CurrentTime,
                                     crtcInfo->x,
                                     crtcInfo->y,
                                     m_oldVideoMode,
                                     crtcInfo->rotation,
                                     &output,
                                     1);

                    XRRFreeCrtcInfo(crtcInfo);
                    XRRFreeScreenResources(res);
                }

                // Reset the fullscreen window
                fullscreenWindow = NULL;
            }
        }


        ////////////////////////////////////////////////////////////
        void X11Window::switchToFullscreen()
        {
            grabFocus();

            if (ewmhSupported())
            {
                Atom netWmBypassCompositor = Display::getAtom("_NET_WM_BYPASS_COMPOSITOR");

                if (netWmBypassCompositor)
                {
                    static const unsigned long bypassCompositor = 1;

                    XChangeProperty(m_display,
                                    m_window,
                                    netWmBypassCompositor,
                                    XA_CARDINAL,
                                    32,
                                    PropModeReplace,
                                    reinterpret_cast<const unsigned char*>(&bypassCompositor),
                                    1);
                }

                Atom netWmState = Display::getAtom("_NET_WM_STATE", true);
                Atom netWmStateFullscreen = Display::getAtom("_NET_WM_STATE_FULLSCREEN", true);

                if (!netWmState || !netWmStateFullscreen)
                {
                    err() << "Setting fullscreen failed. Could not get required atoms" << std::endl;
                    return;
                }

                XEvent event;
                std::memset(&event, 0, sizeof(event));

                event.type = ClientMessage;
                event.xclient.window = m_window;
                event.xclient.format = 32;
                event.xclient.message_type = netWmState;
                event.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
                event.xclient.data.l[1] = netWmStateFullscreen;
                event.xclient.data.l[2] = 0; // No second property
                event.xclient.data.l[3] = 1; // Normal window

                int result = XSendEvent(m_display,
                                        DefaultRootWindow(m_display),
                                        False,
                                        SubstructureNotifyMask | SubstructureRedirectMask,
                                        &event);

                if (!result)
                   std::cerr << "Setting fullscreen failed, could not send \"_NET_WM_STATE\" event" << std::endl;
            }
        }


        ////////////////////////////////////////////////////////////
        void X11Window::setProtocols()
        {
            Atom wmProtocols = Display::getAtom("WM_PROTOCOLS");
            Atom wmDeleteWindow = Display::getAtom("WM_DELETE_WINDOW");

            if (!wmProtocols)
            {
                std::cerr << "Failed to request WM_PROTOCOLS atom." << std::endl;
                return;
            }

            std::vector<Atom> atoms;

            if (wmDeleteWindow)
            {
                atoms.push_back(wmDeleteWindow);
            }
            else
            {
                err() << "Failed to request WM_DELETE_WINDOW atom." << std::endl;
            }

            Atom netWmPing = None;
            Atom netWmPid = None;

            if (ewmhSupported())
            {
                netWmPing = Display::getAtom("_NET_WM_PING", true);
                netWmPid = Display::getAtom("_NET_WM_PID", true);
            }

            if (netWmPing && netWmPid)
            {
                const long pid = getpid();

                XChangeProperty(m_display,
                                m_window,
                                netWmPid,
                                XA_CARDINAL,
                                32,
                                PropModeReplace,
                                reinterpret_cast<const unsigned char*>(&pid),
                                1);

                atoms.push_back(netWmPing);
            }

            if (!atoms.empty())
            {
                XChangeProperty(m_display,
                                m_window,
                                wmProtocols,
                                XA_ATOM,
                                32,
                                PropModeReplace,
                                reinterpret_cast<const unsigned char*>(&atoms[0]),
                                atoms.size());
            }
            else
            {
                std::cerr<< "Didn't set any window protocols" << std::endl;
            }
        }
        void X11Window::setTitle(const sf::String& title) {
              // Bare X11 has no Unicode window title support.
              // There is however an option to tell the window manager your Unicode title via hints.

              // Convert to UTF-8 encoding.
              std::basic_string<Uint8> utf8Title;
              Utf32::toUtf8(title.begin(), title.end(), std::back_inserter(utf8Title));

              Atom useUtf8 = Display::getAtom("UTF8_STRING", false);

              // Set the _NET_WM_NAME atom, which specifies a UTF-8 encoded window title.
              Atom wmName = Display::getAtom("_NET_WM_NAME", false);
              XChangeProperty(m_display, m_window, wmName, useUtf8, 8,
                              PropModeReplace, utf8Title.c_str(), utf8Title.size());

              // Set the _NET_WM_ICON_NAME atom, which specifies a UTF-8 encoded window title.
              Atom wmIconName = Display::getAtom("_NET_WM_ICON_NAME", false);
              XChangeProperty(m_display, m_window, wmIconName, useUtf8, 8,
                              PropModeReplace, utf8Title.c_str(), utf8Title.size());

              // Set the non-Unicode title as a fallback for window managers who don't support _NET_WM_NAME.
              #ifdef X_HAVE_UTF8_STRING
              Xutf8SetWMProperties(m_display,
                                   m_window,
                                   title.toAnsiString().c_str(),
                                   title.toAnsiString().c_str(),
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL);
              #else
              XmbSetWMProperties(m_display,
                                 m_window,
                                 title.toAnsiString().c_str(),
                                 title.toAnsiString().c_str(),
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 NULL);
              #endif
        }
        ////////////////////////////////////////////////////////////
        void X11Window::updateLastInputTime(::Time time)
        {
            if (time && (time != m_lastInputTime))
            {
                Atom netWmUserTime = Display::getAtom("_NET_WM_USER_TIME", true);

                if (netWmUserTime)
                {
                    XChangeProperty(m_display,
                                    m_window,
                                    netWmUserTime,
                                    XA_CARDINAL,
                                    32,
                                    PropModeReplace,
                                    reinterpret_cast<const unsigned char*>(&time),
                                    1);
                }

                m_lastInputTime = time;
            }
        }


        ////////////////////////////////////////////////////////////
        void X11Window::createHiddenCursor()
        {
            // Create the cursor's pixmap (1x1 pixels)
            Pixmap cursorPixmap = XCreatePixmap(m_display, m_window, 1, 1, 1);
            GC graphicsContext = XCreateGC(m_display, cursorPixmap, 0, NULL);
            XDrawPoint(m_display, cursorPixmap, graphicsContext, 0, 0);
            XFreeGC(m_display, graphicsContext);

            // Create the cursor, using the pixmap as both the shape and the mask of the cursor
            XColor color;
            color.flags = DoRed | DoGreen | DoBlue;
            color.red = color.blue = color.green = 0;
            m_hiddenCursor = XCreatePixmapCursor(m_display, cursorPixmap, cursorPixmap, &color, &color, 0, 0);

            // We don't need the pixmap any longer, free it
            XFreePixmap(m_display, cursorPixmap);
        }


        ////////////////////////////////////////////////////////////
        void X11Window::cleanup()
        {
            // Restore the previous video mode (in case we were running in fullscreen)
            resetVideoMode();

            // Unhide the mouse cursor (in case it was hidden)
            setMouseCursorVisible(true);
        }

        sf::WindowHandle X11Window::getSystemHandle() const {
            return m_window;
        }
        void X11Window::processEvents() {
            XEvent event;
            // Pick out the events that are interesting for this window
            while (XCheckIfEvent(m_display, &event, &checkEvent, reinterpret_cast<XPointer>(m_window)))
                m_xevents.push_back(event);
            // Handle the events for this window that we just picked out
            while (!m_xevents.empty())
            {
                event = m_xevents.front();
                m_xevents.pop_front();
                processEvent(event);
            }
        }
        bool X11Window::processEvent(XEvent& xevent) {
            // Detect repeated key events
            if (xevent.type == KeyRelease)
            {
                // Find the next KeyPress event with matching keycode and time
                std::deque<XEvent>::iterator iter = std::find_if(
                    m_xevents.begin(),
                    m_xevents.end(),
                    KeyRepeatFinder(xevent.xkey.keycode, xevent.xkey.time)
                );

                if (iter != m_xevents.end())
                {
                    // If we don't want repeated events, remove the next KeyPress from the queue
                    if (!m_keyRepeat)
                        m_xevents.erase(iter);

                    // This KeyRelease is a repeated event and we don't want it
                    return false;
                }
            }
            if (xevent.type == DestroyNotify) {
                cleanup();
            }
            if (xevent.type == ClientMessage) {
                static Atom wmProtocols = Display::getAtom("WM_PROTOCOLS");
                if (xevent.xclient.message_type == wmProtocols) {
                    static Atom wmDeleteWindow = Display::getAtom("WM_DELETE_WINDOW");
                    static Atom netWmPing = ewmhSupported() ? Display::getAtom("_NET_WM_PING", true) : None;
                    if ((xevent.xclient.format == 32) && (xevent.xclient.data.l[0]) == static_cast<long>(wmDeleteWindow))
                    {
                        IEvent event;
                        event.type = IEvent::EventType::WINDOW_EVENT;
                        std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                        event.window.type = IEvent::WindowEventID::WINDOW_EVENT_CLOSED;
                        event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                        event.window.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                        pushEvent(event);
                    }  else if (netWmPing && (xevent.xclient.format == 32) && (xevent.xclient.data.l[0]) == static_cast<long>(netWmPing))
                    {
                        // Handle the _NET_WM_PING message, send pong back to WM to show that we are responsive
                        xevent.xclient.window = DefaultRootWindow(m_display);

                        XSendEvent(m_display, DefaultRootWindow(m_display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xevent);
                    }
                }
            }
            if (xevent.type == FocusIn) {
                if (m_inputContext)
                    XSetICFocus(m_inputContext);
                    // Grab cursor
                if (m_cursorGrabbed)
                {
                    // Try multiple times to grab the cursor
                    for (unsigned int trial = 0; trial < maxTrialsCount; ++trial)
                    {
                        int result = XGrabPointer(m_display, m_window, True, None, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime);

                        if (result == GrabSuccess)
                        {
                            m_cursorGrabbed = true;
                            break;
                        }

                        // The cursor grab failed, trying again after a small sleep
                        sf::sleep(sf::milliseconds(50));
                    }

                    if (!m_cursorGrabbed)
                        std::cerr<< "Failed to grab mouse cursor" << std::endl;
                }
                IEvent event;
                event.type = IEvent::EventType::WINDOW_EVENT;
                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                event.window.type = IEvent::WindowEventID::WINDOW_EVENT_FOCUS_GAIGNED;
                event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                event.window.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                pushEvent(event);
            }
            if (xevent.type == FocusOut) {
                if (m_inputContext)
                    XUnsetICFocus(m_inputContext);
                // Release cursor
                if (m_cursorGrabbed)
                    XUngrabPointer(m_display, CurrentTime);
                IEvent event;
                event.type = IEvent::EventType::WINDOW_EVENT;
                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                event.window.type = IEvent::WindowEventID::WINDOW_EVENT_FOCUS_LOST;
                event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                event.window.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                pushEvent(event);
            }
            if (xevent.type == ConfigureNotify) {
                if ((xevent.xconfigure.width != m_previousSize.x) || (xevent.xconfigure.height != m_previousSize.y)) {
                    IEvent event;
                    event.type = IEvent::EventType::WINDOW_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.window.type = IEvent::WindowEventID::WINDOW_EVENT_RESIZED;
                    event.window.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.window.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                    event.window.data1 = xevent.xconfigure.width;
                    event.window.data2 = xevent.xconfigure.height;
                    m_previousSize.x = xevent.xconfigure.width;
                    m_previousSize.y = xevent.xconfigure.height;
                    pushEvent(event);
                }
            }
            if (xevent.type == KeyPress) {
                IKeyboard::Key key = IKeyboard::Unknown;
                // Try each KeySym index (modifier group) until we get a match
                for (int i = 0; i < 4; ++i)
                {
                    // Get the SFML keyboard code from the keysym of the key that has been pressed
                    key = keysymToODFAEG(XLookupKeysym(&xevent.xkey, i));

                    if (key != IKeyboard::Unknown)
                        break;
                }
                IEvent event;
                event.type        = IEvent::EventType::KEYBOARD_EVENT;
                event.keyboard.type    = IEvent::KeyEventID::KEY_EVENT_PRESSED;
                event.keyboard.code    = key;
                event.keyboard.alt     = xevent.xkey.state & Mod1Mask;
                event.keyboard.control = xevent.xkey.state & ControlMask;
                event.keyboard.shift   = xevent.xkey.state & ShiftMask;
                event.keyboard.system  = xevent.xkey.state & Mod4Mask;
                pushEvent(event);
                // Generate a TextEntered event
                if (!XFilterEvent(&xevent, None))
                {
                    #ifdef X_HAVE_UTF8_STRING
                    if (m_inputContext)
                    {
                        Status status;
                        Uint8  keyBuffer[16];

                        int length = Xutf8LookupString(
                            m_inputContext,
                            &xevent.xkey,
                            reinterpret_cast<char*>(keyBuffer),
                            sizeof(keyBuffer),
                            NULL,
                            &status
                        );

                        if (length > 0)
                        {
                            Uint32 unicode = 0;
                            Utf8::decode(keyBuffer, keyBuffer + length, unicode, 0);
                            if (unicode != 0)
                            {                                ;
                                IEvent textEvent;
                                textEvent.type = IEvent::EventType::TEXT_INPUT_EVENT;
                                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                                textEvent.text.type = IEvent::EventType::TEXT_INPUT_EVENT;
                                textEvent.text.timestamp = std::chrono::system_clock::to_time_t(time);
                                textEvent.text.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                                textEvent.text.unicode = static_cast<Uint32>(keyBuffer[0]);
                                pushEvent(textEvent);
                            }
                        }
                    }
                    else
                    #endif
                    {
                        static XComposeStatus status;
                        char keyBuffer[16];
                        if (XLookupString(&xevent.xkey, keyBuffer, sizeof(keyBuffer), NULL, &status))
                        {

                            IEvent textEvent;
                            textEvent.type = IEvent::EventType::TEXT_INPUT_EVENT;
                            std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                            textEvent.text.type = IEvent::EventType::TEXT_INPUT_EVENT;
                            textEvent.text.timestamp = std::chrono::system_clock::to_time_t(time);
                            textEvent.text.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                            textEvent.text.unicode = static_cast<Uint32>(keyBuffer[0]);
                            pushEvent(textEvent);
                        }
                    }
                }
                updateLastInputTime(xevent.xkey.time);
            }
            if (xevent.type == KeyRelease) {
                IKeyboard::Key key = IKeyboard::Unknown;
                // Try each KeySym index (modifier group) until we get a match
                for (int i = 0; i < 4; ++i)
                {
                    // Get the SFML keyboard code from the keysym of the key that has been pressed
                    key = keysymToODFAEG(XLookupKeysym(&xevent.xkey, i));

                    if (key != IKeyboard::Unknown)
                        break;
                }
                IEvent event;
                event.type = IEvent::EventType::KEYBOARD_EVENT;
                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                event.keyboard.type = IEvent::KeyEventID::KEY_EVENT_RELEASED;
                event.keyboard.timestamp = std::chrono::system_clock::to_time_t(time);
                event.keyboard.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                event.keyboard.code = key;
                event.keyboard.alt     = xevent.xkey.state & Mod1Mask;
                event.keyboard.control = xevent.xkey.state & ControlMask;
                event.keyboard.shift   = xevent.xkey.state & ShiftMask;
                event.keyboard.system  = xevent.xkey.state & Mod4Mask;
                pushEvent(event);
            }
            if (xevent.type == ButtonPress) {
                // XXX: Why button 8 and 9?
                // Because 4 and 5 are the vertical wheel and 6 and 7 are horizontal wheel ;)
                unsigned int button = xevent.xbutton.button;
                if ((button == Button1) ||
                    (button == Button2) ||
                    (button == Button3) ||
                    (button == 8) ||
                    (button == 9))
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_PRESSED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                    event.mouseButton.x = xevent.xbutton.x;
                    event.mouseButton.y = xevent.xbutton.y;
                    switch(button)
                    {
                        case Button1: event.mouseButton.button = IMouse::Left;     break;
                        case Button2: event.mouseButton.button = IMouse::Middle;   break;
                        case Button3: event.mouseButton.button = IMouse::Right;    break;
                        case 8:       event.mouseButton.button = IMouse::XButton1; break;
                        case 9:       event.mouseButton.button = IMouse::XButton2; break;
                    }
                    pushEvent(event);
                }
                updateLastInputTime(xevent.xbutton.time);
            }
            if (xevent.type == ButtonRelease) {
                unsigned int button = xevent.xbutton.button;
                if ((button == Button1) ||
                    (button == Button2) ||
                    (button == Button3) ||
                    (button == 8) ||
                    (button == 9))
                {
                    IEvent event;
                    event.type          = IEvent::EventType::MOUSE_BUTTON_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseButton.type = IEvent::MouseEventID::BUTTON_EVENT_RELEASED;
                    event.mouseButton.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseButton.windowID = reinterpret_cast<unsigned long long int>(&m_window);
                    event.mouseButton.x = xevent.xbutton.x;
                    event.mouseButton.y = xevent.xbutton.y;
                    switch(button)
                    {
                        case Button1: event.mouseButton.button = IMouse::Left;     break;
                        case Button2: event.mouseButton.button = IMouse::Middle;   break;
                        case Button3: event.mouseButton.button = IMouse::Right;    break;
                        case 8:       event.mouseButton.button = IMouse::XButton1; break;
                        case 9:       event.mouseButton.button = IMouse::XButton2; break;
                    }
                    pushEvent(event);
                }else if ((button == Button4) || (button == Button5))
                {
                    IEvent event;
                    event.type             = IEvent::EventType::MOUSE_WHEEL_EVENT;
                    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                    event.mouseWheel.type = IEvent::EventType::MOUSE_WHEEL_EVENT;
                    event.mouseWheel.timestamp = std::chrono::system_clock::to_time_t(time);
                    event.mouseWheel.direction = (button == Button4) ? 1 : -1;
                    event.mouseWheel.x     = xevent.xbutton.x;
                    event.mouseWheel.y     = xevent.xbutton.y;
                    pushEvent(event);
                }
            }
            if (xevent.type == MotionNotify) {
                IEvent event;
                event.type = IEvent::EventType::MOUSE_MOTION_EVENT;
                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                event.mouseMotion.type = IEvent::EventType::MOUSE_MOTION_EVENT;
                event.mouseMotion.timestamp = std::chrono::system_clock::to_time_t(time);
                event.mouseMotion.x = xevent.xmotion.x;
                event.mouseMotion.y = xevent.xmotion.y;
                pushEvent(event);
            }
            if (xevent.type == EnterNotify) {
                IEvent event;
                event.type = IEvent::EventType::MOUSE_EVENT_ENTER;
                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                event.mouseMotion.type = IEvent::EventType::MOUSE_EVENT_ENTER;
                pushEvent(event);
            }
            if (xevent.type == LeaveNotify) {
                IEvent event;
                event.type = IEvent::EventType::MOUSE_EVENT_LEAVE;
                std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
                event.mouseMotion.type = IEvent::EventType::MOUSE_EVENT_ENTER;
                pushEvent(event);
            }
            if (xevent.type == UnmapNotify) {
                if (xevent.xunmap.window == m_window)
                    m_windowMapped = false;
            }
            if (xevent.type == VisibilityNotify) {
                // We prefer using VisibilityNotify over MapNotify because
                // some window managers like awesome don't internally flag a
                // window as viewable even after it is mapped but before it
                // is visible leading to certain function calls failing with
                // an unviewable error if called before VisibilityNotify arrives

                // Empirical testing on most widely used window managers shows
                // that mapping a window will always lead to a VisibilityNotify
                // event that is not VisibilityFullyObscured
                if (xevent.xvisibility.window == m_window)
                {
                    if (xevent.xvisibility.state != VisibilityFullyObscured)
                        m_windowMapped = true;
                }
            }
            if (xevent.type == PropertyNotify) {
                if (!m_lastInputTime)
                    m_lastInputTime = xevent.xproperty.time;
            }
            return true;
        }
        bool X11Window::isOpen() const {
            return m_opened;
        }
        void X11Window::close() {
            if (m_opened) {
                m_opened = false;
            }
        }
        unsigned long long int X11Window::getID() {
            return reinterpret_cast<unsigned long long int>(&m_window);
        }
        bool X11Window::popEvent(IEvent& event) {
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
        bool X11Window::pollEvent(IEvent& event) {
            return popEvent(event);
        }
        bool X11Window::checkXRandR(int& xRandRMajor, int& xRandRMinor)
        {
            // Check if the XRandR extension is present
            int version;
            if (!XQueryExtension(m_display, "RANDR", &version, &version, &version))
            {
                err() << "XRandR extension is not supported" << std::endl;
                return false;
            }

            // Check XRandR version, 1.2 required
            if (!XRRQueryVersion(m_display, &xRandRMajor, &xRandRMinor) || xRandRMajor < 1 || (xRandRMajor == 1 && xRandRMinor < 2 ))
            {
                err() << "XRandR is too old" << std::endl;
                return false;
            }

            return true;
        }


        ////////////////////////////////////////////////////////////
        RROutput X11Window::getOutputPrimary(::Window& rootWindow, XRRScreenResources* res, int xRandRMajor, int xRandRMinor)
        {
            // if xRandR version >= 1.3 get the primary screen else take the first screen
            if ((xRandRMajor == 1 && xRandRMinor >= 3) || xRandRMajor > 1)
            {
                RROutput output = XRRGetOutputPrimary(m_display, rootWindow);

                // Check if returned output is valid, otherwise use the first screen
                if (output == None)
                    return res->outputs[0];
                else
                    return output;
            }

            // xRandr version can't get the primary screen, use the first screen
            return res->outputs[0];
        }


        ////////////////////////////////////////////////////////////
        Vector2i X11Window::getPrimaryMonitorPosition()
        {
            Vector2i monitorPosition;

            // Get root window
            ::Window rootWindow = RootWindow(m_display, m_screen);

            // Get the screen resources
            XRRScreenResources* res = XRRGetScreenResources(m_display, rootWindow);
            if (!res)
            {
                err() << "Failed to get the current screen resources for.primary monitor position" << std::endl;
                return monitorPosition;
            }

            // Get xRandr version
            int xRandRMajor, xRandRMinor;
            if (!checkXRandR(xRandRMajor, xRandRMinor))
                xRandRMajor = xRandRMinor = 0;

            RROutput output = getOutputPrimary(rootWindow, res, xRandRMajor, xRandRMinor);

            // Get output info from output
            XRROutputInfo* outputInfo = XRRGetOutputInfo(m_display, res, output);
            if (!outputInfo || outputInfo->connection == RR_Disconnected)
            {
                XRRFreeScreenResources(res);

                // If outputInfo->connection == RR_Disconnected, free output info
                if (outputInfo)
                    XRRFreeOutputInfo(outputInfo);

                err() << "Failed to get output info for.primary monitor position" << std::endl;
                return monitorPosition;
            }

            // Retreive current RRMode, screen position and rotation
            XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(m_display, res, outputInfo->crtc);
            if (!crtcInfo)
            {
                XRRFreeScreenResources(res);
                XRRFreeOutputInfo(outputInfo);
                err() << "Failed to get crtc info for.primary monitor position" << std::endl;
                return monitorPosition;
            }

            monitorPosition.x = crtcInfo->x;
            monitorPosition.y = crtcInfo->y;

            XRRFreeCrtcInfo(crtcInfo);
            XRRFreeOutputInfo(outputInfo);
            XRRFreeScreenResources(res);

            return monitorPosition;
        }
        void X11Window::pushEvent (IEvent& event) {
            m_ievents.push(event);
        }
        void X11Window::destroy() {
            if (!m_destroyed) {
                cleanup();
                // Destroy icon pixmap
                if (m_iconPixmap)
                    XFreePixmap(m_display, m_iconPixmap);

                // Destroy icon mask pixmap
                if (m_iconMaskPixmap)
                    XFreePixmap(m_display, m_iconMaskPixmap);

                // Destroy the cursor
                if (m_hiddenCursor)
                    XFreeCursor(m_display, m_hiddenCursor);

                // Destroy the input context
                if (m_inputContext)
                    XDestroyIC(m_inputContext);

                // Destroy the window
                if (m_window && !m_isExternal)
                {
                    XDestroyWindow(m_display, m_window);
                    XFlush(m_display);
                }

                // Close the input method
                if (m_inputMethod)
                    XCloseIM(m_inputMethod);
                if (m_display) {
                    Display::closeDisplay(m_display);
                }
                // Remove this window from the global list of windows (required for focus request)
                Lock lock(allWindowsMutex);
                allWindows.erase(std::find(allWindows.begin(), allWindows.end(), this));
                m_destroyed = true;
                m_opened = false;
            }
        }
        X11Window::~X11Window() {
            if (!m_destroyed) {
                cleanup();
                // Destroy icon pixmap
                if (m_iconPixmap)
                    XFreePixmap(m_display, m_iconPixmap);

                // Destroy icon mask pixmap
                if (m_iconMaskPixmap)
                    XFreePixmap(m_display, m_iconMaskPixmap);

                // Destroy the cursor
                if (m_hiddenCursor)
                    XFreeCursor(m_display, m_hiddenCursor);

                // Destroy the input context
                if (m_inputContext)
                    XDestroyIC(m_inputContext);

                // Destroy the window
                if (m_window && !m_isExternal)
                {
                    XDestroyWindow(m_display, m_window);
                    XFlush(m_display);
                }

                // Close the input method
                if (m_inputMethod)
                    XCloseIM(m_inputMethod);
                if (m_display) {
                    Display::closeDisplay(m_display);
                }
                // Remove this window from the global list of windows (required for focus request)
                Lock lock(allWindowsMutex);
                allWindows.erase(std::find(allWindows.begin(), allWindows.end(), this));
                m_destroyed = true;
                m_opened = false;
            }
        }
    }
}
//#endif
