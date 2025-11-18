#ifndef NATIVE_PLATFORM_H
#define NATIVE_PLATFORM_H

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "mainboard_engine.h"

#include <bgfx/bgfx.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "platform.h"

// TODO using factory method, make it determined by java side
constexpr int BLOCK_ARRAY_SIZE = 1024;

namespace MainboardEngine {
    class MEWindow;

    struct Block {
        int id;
        std::optional<std::string> type;
        std::optional<bgfx::TextureHandle> texture;
        int width;
        int height;
        int channels;
    };

//     struct Command {
//         virtual ~Command() = default;
//     };
//
//     struct RenderBlockCommand : Command {
//         int x;
//         int y;
//         std::string block_name;
//     };
}

namespace MainboardEngine {
    class MEPlatform {
    public:
        virtual ~MEPlatform() = default;

        virtual bool Initialize() = 0;

        virtual void Shutdown() = 0;

        virtual bool CreateWindow(
            int is_full_screen, int x, int y, int width, int height, const char *title, MEWindow *&window) = 0;

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

    class MEEngine {
        MEWindow *m_window;
        std::optional<Block> m_blocks[BLOCK_ARRAY_SIZE];
        bgfx::VertexBufferHandle m_vbh;
        bgfx::IndexBufferHandle m_ibh;

    public:
        virtual ~MEEngine() = default;

        static bool Start(MEWindow *window);

        void Shutdown();

        static bool RegistryBlock(int id, std::string path);

        bool RenderBlock(int id, int x, int y);

        // bool RegistryRenderBlock(std::string block_name, int x, int y);

        // bool Render();

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

        bool CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title, MEWindow *&window) override;

        ME_MESSAGE_TYPE ProcessEvents(ME_HANDLE handle) override;
    };

    class Win32Window : public MEWindow {
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
