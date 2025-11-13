#ifndef NATIVE_PLATFORM_H
#define NATIVE_PLATFORM_H

#include <cstdint>
#include <memory>
#include <string>
#include "mainboard_engine.h"
#include "event_message_type.h"
#include "platform.h"
#include "platform.h"

namespace MainboardEngine {
    class MEPlatform {
    public:
        virtual ~MEPlatform() = default;

        virtual bool Initialize() = 0;

        virtual void Shutdown() = 0;

        virtual class MEWindow *CreateWindow(
            int is_full_screen, int x, int y, int width, int height, const char *title) = 0;

        virtual ME_MESSAGE_TYPE ProcessEvents(ME_HANDLE handle) = 0;

        const std::string className = "MainboardEngineBasedWindow";
    };

    class MEWindow {
    public:
        virtual ~MEWindow() = default;

        virtual bool SetSize(int width, int height) = 0;

        virtual ME_Rect GetSize() = 0;

        virtual bool SetPosition(int x, int y) = 0;

        virtual bool SetTitle(const char *title) = 0;

        virtual void *GetMEWindowHandle() = 0;
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
        Win32Platform() = default;

        bool Initialize() override;

        void Shutdown() override;

        MEWindow *CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) override;

        ME_MESSAGE_TYPE ProcessEvents(ME_HANDLE handle) override;
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

        ME_Rect GetSize() override;

        void *GetMEWindowHandle() override;
    };
#endif

#ifdef __linux__
    class LinuxPlatform : public MEPlatform {
        std::unique_ptr<LinuxPlatform> m_platform;

    public:
        ~LinuxPlatform() override = default;

        bool Initialize() override;

        void Shutdown() override;

        MEWindow *CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) override;

        int ProcessEvents(ME_HANDLE handle) override;
    };

    class LinuxWindow : public MEWindow {
        std::unique_ptr<LinuxWindow> m_window;

    public:
        ~LinuxWindow() override = default;

        bool SetSize(int width, int height) override;

        ME_Rect GetSize() override;

        bool SetPosition(int x, int y) override;

        bool SetTitle(const char *title) override;

        void *GetMEWindowHandle() override;
    };

#ifdef __ME_USE_WAYLAND__
    class WaylandPlatform : public LinuxPlatform {
        void *display;
        void *compositor;
        void *shm;
        void *xdg_wm_base;

        static void registry_global_handler(void *data, void *registry, uint32_t name, const char *interface,
                                            uint32_t version);

        static void registry_global_remove_handler(void *data, void *registry, uint32_t name);

    public:
        WaylandPlatform() : display(nullptr), compositor(nullptr), shm(nullptr), xdg_wm_base(nullptr) {}
        ~WaylandPlatform() override;

        bool Initialize() override;

        void Shutdown() override;

        MEWindow *CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) override;

        int ProcessEvents(ME_HANDLE handle) override;
    };

    class WaylandWindow : public MEWindow {
    public:
        bool SetSize(int width, int height) override;

        ME_Rect GetSize() override;

        bool SetPosition(int x, int y) override;

        bool SetTitle(const char *title) override;

        void *GetMEWindowHandle() override;
    };
#endif

#endif

#ifdef __APPLE__
    class ApplePlatform : public MEPlatform {
    public:
    };
#endif
}


#endif //NATIVE_PLATFORM_H
