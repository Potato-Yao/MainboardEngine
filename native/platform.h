#ifndef NATIVE_PLATFORM_H
#define NATIVE_PLATFORM_H

#include <string>
#include "mainboard_engine.h"

namespace MainboardEngine {
    // platform independent
    class MEPlatform {
    public:
        virtual ~MEPlatform() = default;

        virtual bool Initialize() = 0;

        virtual void Shutdown() = 0;

        virtual class MEWindow *CreateWindow(
            int is_full_screen, int x, int y, int width, int height, const char *title) = 0;

        virtual bool ProcessEvents(ME_HANDLE handle) = 0;

        const std::string className = "MainboardEngineBasedWindow";
    };

    // platform dependent
    class MEWindow {
    public:
        virtual ~MEWindow() = default;

        virtual bool SetSize(int width, int height) = 0;

        virtual bool SetPosition(int x, int y) = 0;

        virtual bool SetTitle(const char *title) = 0;

        virtual void *GetMEWindowHandle() = 0;

        virtual bool IsValid() const = 0;

        virtual bool ShouldClose() const = 0;
    };
}

namespace MainboardEngine {
#ifdef _WIN32
#ifndef ME_WINDOWS_H_INCLUDED
#include <windows.h>
#define ME_WINDOWS_H_INCLUDED
#endif


    // avoid name conflict caused by windows.h
#undef CreateWindow

    class Win32Platform : public MEPlatform {
    public:
        bool Initialize() override;

        void Shutdown() override;

        MEWindow *CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) override;

        bool ProcessEvents(ME_HANDLE handle) override;
    };

    class Win32Window : public MEWindow {
    private:
        HWND m_hwnd;

    public:
        Win32Window(HWND hwnd) : m_hwnd(hwnd) {
        }

        bool SetSize(int width, int height) override;

        bool SetPosition(int x, int y) override;

        bool SetTitle(const char *title) override;

        void *GetMEWindowHandle() override;

        bool IsValid() const override;

        bool ShouldClose() const override;
    };
#elifdef __linux__
    class LinuxPlatform : public MEPlatform {
    public:
        bool Initialize() override;

        void Shutdown() override;

        IWindow *CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) override;

        bool ProcessEvents() override;
    };
#elifdef __APPLE__
    class ApplePlatform : public MEPlatform {
    public:
        bool Initialize() override;

        void Shutdown() override;

        IWindow *CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) override;

        bool ProcessEvents() override;
    };
#endif
}


#endif //NATIVE_PLATFORM_H
