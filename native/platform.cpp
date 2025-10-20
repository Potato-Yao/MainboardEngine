#include "platform.h"
#include <memory>
#include <codecvt>
#include <locale>

extern "C" {
namespace ME = MainboardEngine;

static std::unique_ptr<ME::MEPlatform> g_platform;

ME_API ME_BOOL ME_Initialize() {
    if (g_platform) {
        return 1;
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

ME_API ME_BOOL ME_ProcessEvents(ME_HANDLE handle) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    return g_platform->ProcessEvents(window);
}

ME_API ME_BOOL ME_RenderFrame(ME_HANDLE handle) {
    return 1;
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
        return 1;
    }

    void Win32Platform::Shutdown() {
    }

    bool Win32Platform::ProcessEvents(ME_HANDLE handle) {
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }


    MEWindow *Win32Platform::CreateWindow(
        int is_full_screen, int x, int y, int width, int height, const char *title) {
        static bool has_registered = false;
        if (!has_registered) {
            has_registered = true;
            WNDCLASSEX wc = {0};
            wc.cbSize = sizeof(wc);
            wc.style = CS_OWNDC;
            wc.lpfnWndProc = WindowProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.hIcon = nullptr;
            wc.hCursor = nullptr;
            wc.hbrBackground = nullptr;
            wc.lpszMenuName = nullptr;
            wc.lpszClassName = this->className.c_str();
            wc.hIconSm = nullptr;
            RegisterClassEx(&wc);
        }

        auto style = WS_OVERLAPPEDWINDOW;
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
        return ME_TRUE;
    }

    bool Win32Window::SetPosition(int x, int y) {
        return ME_TRUE;
    }

    bool Win32Window::SetTitle(const char *title) {
        return ME_TRUE;
    }

    void *Win32Window::GetMEWindowHandle() {
        return nullptr;
    }

    bool Win32Window::IsValid() const {
        return ME_TRUE;
    }

    bool Win32Window::ShouldClose() const {
        return ME_TRUE;
    }
}

#endif
}
