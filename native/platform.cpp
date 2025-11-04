#include "include/platform.h"
#include <memory>
#include <codecvt>
#include <locale>

extern "C" {
namespace ME = MainboardEngine;

static std::unique_ptr<ME::MEPlatform> g_platform;

ME_API ME_BOOL ME_Initialize() {
    if (g_platform) {
        return ME_TRUE;
    }

#if defined(_WIN32)
    g_platform = std::make_unique<MainboardEngine::Win32Platform>();
#elif defined(__linux__)
    g_platform = std::make_unique<LinuxPlatform>();
#elif defined(__APPLE__)
    g_platform = std::make_unique<ApplePlatform>();
#endif

    return g_platform->Initialize();
}

ME_API ME_HANDLE ME_CreateWindow(
    int is_full_screen, int x, int y, int width, int height, const char *title) {
    ME::MEWindow *window = g_platform->CreateWindow(is_full_screen, x, y, width, height, title);

    return window;
}

ME_API ME_MESSAGE_TYPE ME_ProcessEvents(ME_HANDLE handle) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    return g_platform->ProcessEvents(window);
}

ME_API ME_BOOL ME_RenderFrame(ME_HANDLE handle) {
    return ME_TRUE;
}

ME_API ME_BOOL ME_DestroyWindow(ME_HANDLE handle) {
    auto *windows = static_cast<ME::MEWindow *>(handle);
    delete windows; // MEWindow has developer defined deconstructor function
    return ME_TRUE;
}

ME_API ME_HANDLE ME_GetMEWindowHandle(ME_HANDLE handle) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    return window->GetMEWindowHandle();
}


ME_API ME_BOOL ME_SetWindowSize(ME_HANDLE handle, int width, int height) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    return window->SetSize(width, height);
}

ME_API int ME_GetWindowSize(ME_HANDLE handle, ME_Rect *rect) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    *rect = window->GetSize();

    return ME_TRUE;
}

ME_API ME_BOOL ME_SetWindowTitle(ME_HANDLE handle, const char *title) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    return window->SetTitle(title);
}

#ifdef _WIN32
namespace MainboardEngine {
#ifndef ME_WINDOWS_H_INCLUDED
#include <windows.h>
#define ME_WINDOWS_H_INCLUDED
#endif

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Win32Window *window = reinterpret_cast<Win32Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        switch (msg) {
            case WM_DESTROY:
            case WM_CLOSE:
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }

    bool Win32Platform::Initialize() {
        return true;
    }

    void Win32Platform::Shutdown() {
    }

    ME_MESSAGE_TYPE Win32Platform::ProcessEvents(ME_HANDLE handle) {
        MSG msg = {};
        if (!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            return ME_NO_EVENT_MESSAGE; // 0 means no message
        }

        ME_MESSAGE_TYPE mes;
        switch (msg.message) {
            case WM_QUIT:
                mes = ME_QUIT_MESSAGE;
                break;
            default:
                mes = ME_NO_EVENT_MESSAGE;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        return mes;
    }


    MEWindow *Win32Platform::CreateWindow(
        int is_full_screen, int x, int y, int width, int height, const char *title) {
        static bool has_registered = false;
        if (!has_registered) {
            has_registered = true;
            WNDCLASSEX wc = {};
            wc.cbSize = sizeof(wc);
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wc.lpfnWndProc = WindowProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.hIcon = nullptr;
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = nullptr;
            wc.lpszMenuName = nullptr;
            wc.lpszClassName = this->className.c_str();
            wc.hIconSm = nullptr;
            RegisterClassEx(&wc);
        }

        auto style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        RECT wr = {0, 0, width, height};
        AdjustWindowRect(&wr, style, FALSE);

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
        HWND hwnd = CreateWindowW(converter.from_bytes(this->className).c_str(), converter.from_bytes(title).c_str(),
                                  style, x, y, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr,
                                  GetModuleHandle(nullptr), nullptr);
        if (!hwnd) {
            return nullptr;
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(new Win32Window(hwnd)));

        return reinterpret_cast<Win32Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
}

namespace MainboardEngine {
#ifndef ME_WINDOWS_H_INCLUDED
#include <windows.h>
#define ME_WINDOWS_H_INCLUDED
#endif

    bool Win32Window::SetSize(int width, int height) {
        RECT rect = {};
        GetWindowRect(m_hwnd, &rect);
        SetWindowPos(m_hwnd, nullptr, rect.left, rect.top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
        return true;
    }

    ME_Rect Win32Window::GetSize() {
        RECT rect = {};
        GetWindowRect(m_hwnd, &rect);
        ME_Rect me_rect = {};
        me_rect.left = rect.left;
        me_rect.right = rect.right;
        me_rect.top = rect.top;
        me_rect.bottom = rect.bottom;
        return me_rect;
    }

    bool Win32Window::SetPosition(int x, int y) {
        RECT rect = {};
        GetWindowRect(m_hwnd, &rect);
        SetWindowPos(m_hwnd, nullptr, x, y, rect.right - rect.left, rect.top - rect.bottom,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        return true;
    }

    bool Win32Window::SetTitle(const char *title) {
        return SetWindowText(this->m_hwnd, title) != 0;
    }

    void *Win32Window::GetMEWindowHandle() {
        return this->m_hwnd;
    }
}

#endif
}
