#include "win32Window.hpp"
#include "../iMouse.hpp"
namespace
{
    unsigned int               windowCount = 0; // Windows owned by ODFAEG
    unsigned int               handleCount = 0; // All window handles
    const wchar_t* className = L"ODFAEG_Window";
    odfaeg::window::Win32Window* fullscreenWindow = NULL;

    const GUID GUID_DEVINTERFACE_HID = { 0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };

    void setProcessDpiAware()
    {
        // Try SetProcessDpiAwareness first
        HINSTANCE shCoreDll = LoadLibrary(L"Shcore.dll");

        if (shCoreDll)
        {
            enum ProcessDpiAwareness
            {
                ProcessDpiUnaware = 0,
                ProcessSystemDpiAware = 1,
                ProcessPerMonitorDpiAware = 2
            };

            typedef HRESULT(WINAPI* SetProcessDpiAwarenessFuncType)(ProcessDpiAwareness);
            SetProcessDpiAwarenessFuncType SetProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));

            if (SetProcessDpiAwarenessFunc)
            {
                // We only check for E_INVALIDARG because we would get
                // E_ACCESSDENIED if the DPI was already set previously
                // and S_OK means the call was successful
                if (SetProcessDpiAwarenessFunc(ProcessSystemDpiAware) == E_INVALIDARG)
                {
                    std::cerr << "Failed to set process DPI awareness" << std::endl;
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
            typedef BOOL(WINAPI* SetProcessDPIAwareFuncType)(void);
            SetProcessDPIAwareFuncType SetProcessDPIAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(GetProcAddress(user32Dll, "SetProcessDPIAware"));

            if (SetProcessDPIAwareFunc)
            {
                if (!SetProcessDPIAwareFunc())
                    std::cerr << "Failed to set process DPI awareness" << std::endl;
            }

            FreeLibrary(user32Dll);
        }
    }
}
#include "win32Window.inl"