module;
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
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
#define WIN32_LEAN_AND_MEAN

#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT   0x0501
#ifdef WINNER
#undef WINVER
#define WINVER         0x0501
#endif
#include <windows.h>
#include <dbt.h>
#include <vector>
#include <cstring>
#include <chrono>
#include <iostream>
#include <thread>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>
#include <odfaeg/Window/windowHandle.hpp>
#include <queue>
#include <odfaeg/Window/windowHandle.hpp>
#include <vulkan/vulkan.hpp>
module odfaeg.window.win32Window;
import odfaeg.window.iMouse;
#include "win32Window.inl"