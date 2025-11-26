#include "include/platform.h"
#include <codecvt>
#include <locale>
#include <string>
#include <fstream>
#include <iostream>
// #include <direct.h>

#include  "include/event_message_type.h"

extern "C" {
namespace ME = MainboardEngine;

static std::unique_ptr<ME::MEPlatform> g_platform;
static std::unique_ptr<ME::MEEngine> g_engine;

ME_API ME_BOOL ME_Initialize() {
    if (g_platform) {
        return ME_TRUE;
    }

#if defined(_WIN32)
    g_platform = std::make_unique<MainboardEngine::Win32Platform>();
#elif defined(__linux__)
    g_platform = std::make_unique<MainboardEngine::LinuxPlatform>();
#elif defined(__APPLE__)
    g_platform = std::make_unique<ApplePlatform>();
#endif

    return g_platform->Initialize();
}

ME_API ME_HANDLE ME_CreateWindow(
    int is_full_screen, int x, int y, int width, int height, const char *title) {
    ME::MEWindow *window = nullptr;
    bool state = g_platform->CreateWindow(is_full_screen, x, y, width, height, title, window);
    if (!state) {
        return nullptr;
    }

    return window;
}

ME_API ME_MESSAGE_TYPE ME_ProcessEvents(ME_HANDLE handle) {
    auto *window = static_cast<ME::MEWindow *>(handle);
    return g_platform->ProcessEvents(window);
}

ME_API ME_BOOL ME_RenderBlock(int block_id, int x, int y) {
    return g_engine->RenderBlock(block_id, x, y);
}

ME_API int ME_RenderFrame(ME_HANDLE handle) {
    return g_engine->Render();
}

ME_API ME_BOOL ME_ClearView(ME_HANDLE handle) {
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

ME_API ME_BOOL ME_LoadBlock(int id, const char *path) {
    return MainboardEngine::MEEngine::RegistryBlock(id, path);
}


ME_API ME_BOOL ME_ClearBlock() {
    return MainboardEngine::MEEngine::ClearBlock();
}

namespace MainboardEngine {
    static int GetRectWidth(ME_Rect *rect) {
        return rect->right - rect->left;
    }

    static int GetRectHeight(ME_Rect *rect) {
        return rect->bottom - rect->top;
    }
}

namespace MainboardEngine {
    static bgfx::ShaderHandle loadShader(const char *filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return BGFX_INVALID_HANDLE;
        }

        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        const bgfx::Memory *mem = bgfx::alloc(static_cast<uint32_t>(size + 1));
        file.read(reinterpret_cast<char *>(mem->data), size);
        mem->data[size] = '\0';
        file.close();

        return bgfx::createShader(mem);
    }

    bool MEEngine::Start(MEWindow *window) {
        using namespace bgfx;

        auto temp_engine = new MEEngine();
        temp_engine->m_window = window;

        for (int i = 0; i < BLOCK_ARRAY_SIZE; ++i) {
            temp_engine->m_blocks[i] = std::nullopt;
        }

        ME_Rect window_rect = temp_engine->m_window->GetSize();
        Init init;
        init.type = RendererType::Count;
        init.resolution.width = GetRectWidth(&window_rect);
        init.resolution.height = GetRectHeight(&window_rect);
        init.resolution.reset = BGFX_RESET_VSYNC;
        PlatformData platformData;
        platformData.nwh = temp_engine->m_window->GetMEWindowHandle();
        init.platformData = platformData;

        auto stat = bgfx::init(init);
        if (!stat) {
            return false;
        }

        setViewRect(0, 0, 0, init.resolution.width, init.resolution.height);

        struct PosTexCoord {
            float x, y, z;
            float u, v;
        };

        static PosTexCoord quadVertices[] = {
            {-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
            {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, -1.0f, 0.0f, 1.0f, 1.0f}
        };

        VertexLayout layout;
        layout.begin()
                .add(Attrib::Position, 3, AttribType::Float)
                .add(Attrib::TexCoord0, 2, AttribType::Float)
                .end();
        VertexBufferHandle vbh = createVertexBuffer(makeRef(quadVertices, sizeof(quadVertices)), layout);
        static const uint16_t quadIndices[] = {0, 1, 2, 1, 3, 2};
        IndexBufferHandle ibh = createIndexBuffer(makeRef(quadIndices, sizeof(quadIndices)));
        temp_engine->m_vbh = vbh;
        temp_engine->m_ibh = ibh;

        UniformHandle s_tex = createUniform("s_tex", UniformType::Sampler);
        UniformHandle u_resolution = createUniform("u_resolution", UniformType::Vec4);
        temp_engine->m_s_tex = s_tex;
        temp_engine->m_u_resolution = u_resolution;

        ShaderHandle vsh = BGFX_INVALID_HANDLE;
        ShaderHandle fsh = BGFX_INVALID_HANDLE;

        auto renderer = getRendererType();
        const char *shaderDir = nullptr;

        switch (renderer) {
            case RendererType::Direct3D11:
            case RendererType::Direct3D12:
                shaderDir = "dx11";
                break;
            case RendererType::OpenGL:
                shaderDir = "glsl";
                break;
            case RendererType::Vulkan:
                shaderDir = "spirv";
                break;
            default:
                shaderDir = "dx11";
                break;
        }

        std::string vsPath = std::string("./shader/") + shaderDir + "/vs_fullscreen.bin";
        std::string fsPath = std::string("./shader/") + shaderDir + "/fs_tiled.bin";

        // // auto path = _getcwd(nullptr, 0);

        vsh = loadShader(vsPath.c_str());
        fsh = loadShader(fsPath.c_str());
        ProgramHandle program = BGFX_INVALID_HANDLE;

        if (isValid(vsh) && isValid(fsh)) {
            program = createProgram(vsh, fsh, true);
            temp_engine->m_program = program;
            temp_engine->m_vsh = vsh;
            temp_engine->m_fsh = fsh;
        } else {
            return false;
        }
        setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);

        g_engine = std::unique_ptr<MEEngine>(temp_engine);

        return true;
    }


    bool MEEngine::RegistryBlock(int id, std::string path) {
        if (g_engine->m_blocks[id] != std::nullopt) {
            return false;
        }

        Block block = {};
        block.id = id;

        auto data = stbi_load(path.c_str(), &block.width, &block.height, &block.channels, 4);
        if (!data) {
            return false;
        }
        auto texture = bgfx::createTexture2D(block.width, block.height, false, 1, bgfx::TextureFormat::RGBA8,
                                             BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
                                             bgfx::copy(data, block.width * block.height * 4));
        // TODO how the hell can i know if the texture is created successfully
        block.texture = texture;
        stbi_image_free(data);

        g_engine->m_blocks[id] = block;

        return true;
    }

    bool MEEngine::ClearBlock() {
        for (int i = 0; i < BLOCK_ARRAY_SIZE; ++i) {
            if (g_engine->m_blocks[i] != std::nullopt) {
                auto block = &g_engine->m_blocks[i].value();
                if (bgfx::isValid(block->texture.value())) {
                    bgfx::destroy(block->texture.value());
                }
                g_engine->m_blocks[i] = std::nullopt;
            }
        }

        return true;
    }


    // bool MEEngine::RegistryRenderBlock(std::string block_name, int x, int y) {
    //     RenderBlockCommand command = {};
    //     command.block_name = block_name;
    //     command.x = x;
    //     command.y = y;
    //     m_commands.push_back(command);
    //
    //     return true;
    // }
    //
    bool MEEngine::RenderBlock(int id, int x, int y) {
        if (m_blocks[id] == std::nullopt) {
            return false;
        }
        auto block = &m_blocks[id].value();
        // bgfx::ViewId view_id = static_cast<bgfx::ViewId>(block->id);

        // bgfx::setViewRect(0, 0, 0, GetRectWidth(&window_rect), GetRectHeight(&window_rect));

        if (!bgfx::isValid(m_program)) {
            return false;
        }
        if (!bgfx::isValid(m_blocks[id].value().texture.value())) {
            return false;
        }

        auto window_rect = m_window->GetSize();
        uint16_t screenW = static_cast<uint16_t>(GetRectWidth(&window_rect));
        uint16_t screenH = static_cast<uint16_t>(GetRectHeight(&window_rect));

        uint16_t viewX = static_cast<uint16_t>(x);
        uint16_t viewY = static_cast<uint16_t>(y);
        uint16_t viewWidth = static_cast<uint16_t>(block->width);
        uint16_t viewHeight = static_cast<uint16_t>(block->height);
        // bgfx::setViewRect(block->id, viewX, viewY, viewWidth, viewHeight);
        bgfx::setScissor(viewX, viewY, viewWidth, viewHeight);
        // bgfx::setViewRect(view_id, viewX, viewY, block->width, block->height);
        // std::cout << "Rendering block ID " << block->id << " at (" << x << ", " << y << ") with size ("
        // << block->width << "x" << block->height << ")" << std::endl;

        // if (block->id == 0) {
        //     bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        // }

        float resolution[4] = {
            static_cast<float>(screenW),
            static_cast<float>(screenH),
            static_cast<float>(block->width),
            static_cast<float>(block->height)
        };
        bgfx::setUniform(m_u_resolution, resolution);
        bgfx::setVertexBuffer(0, m_vbh);
        bgfx::setIndexBuffer(m_ibh);
        bgfx::setTexture(0, m_s_tex, block->texture.value());
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        bgfx::submit(0, m_program);

        return true;
    }

    int MEEngine::Render() {
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
        bgfx::touch(0);
        int frame_num = bgfx::frame();
        return frame_num;
    }


    //
    // bool MEEngine::Render() {
    //     for (Command &command : m_commands) {
    //         try {
    //             auto render_block_command = dynamic_cast<RenderBlockCommand &>(command);
    //
    //         } catch (std::bad_cast cast) {
    //         }
    //     }
    //     m_commands.clear();
    //
    //     return true;
    // }
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


    bool Win32Platform::CreateWindow(
        int is_full_screen, int x, int y, int width, int height, const char *title, MEWindow *&window) {
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
            return false;
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(new Win32Window(hwnd)));

        auto temp_window = reinterpret_cast<Win32Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        bool state = MEEngine::Start(temp_window);
        // bool state = true;

        window = temp_window;

        return state;
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

#ifdef __linux__
namespace MainboardEngine {
    bool LinuxPlatform::Initialize() {
        if (m_platform) {
            return true;
        }
#ifdef __ME_USE_WAYLAND__
m_platform= std::make_unique<WaylandPlatform>();
#endif
if (!m_platform) {
            return false;
        }
        return m_platform->Initialize();
    }

void LinuxPlatform::Shutdown() {
    m_platform->Shutdown();
}

MEWindow *LinuxPlatform::CreateWindow(int is_full_screen, int x, int y, int width, int height, const char *title) {
    return m_platform->CreateWindow(is_full_screen, x, y, width, height, title);
}

int LinuxPlatform::ProcessEvents(ME_HANDLE handle) {
    return m_platform->ProcessEvents(handle);
}
}

namespace MainboardEngine {
    bool LinuxWindow::SetSize(int width, int height) {
        return m_window->SetSize(width, height);
    }

    ME_Rect LinuxWindow::GetSize() {
        return m_window->GetSize();
    }

    bool LinuxWindow::SetPosition(int x, int y) {
        return m_window->SetPosition(x, y);
    }

    bool LinuxWindow::SetTitle(const char *title) {
        return m_window->SetTitle(title);
    }

    void *LinuxWindow::GetMEWindowHandle() {
        return m_window->GetMEWindowHandle();
    }
}

#ifdef __ME_USE_WAYLAND__

extern "C" {
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
}

#include <cstring>

namespace MainboardEngine {
    void WaylandPlatform::registry_global_handler(void *data, void *registry_ptr, uint32_t name,
                                                  const char *interface, uint32_t version) {
        WaylandPlatform *platform = static_cast<WaylandPlatform *>(data);
        if (!platform) return;

        wl_registry *registry = static_cast<wl_registry *>(registry_ptr);

        if (strcmp(interface, wl_compositor_interface.name) == 0) {
            platform->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
        } else if (strcmp(interface, wl_shm_interface.name) == 0) {
            platform->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
        } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
            platform->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        }
    }

    void WaylandPlatform::registry_global_remove_handler(void *data, void *registry_ptr, uint32_t name) {
    }

    bool WaylandPlatform::Initialize() {
        display = wl_display_connect(nullptr);
        if (!display) {
            return false;
        }

        wl_registry *registry = wl_display_get_registry(static_cast<wl_display *>(display));

        static const wl_registry_listener registry_listener = {
            .global = reinterpret_cast<void (*)(void *, wl_registry *, uint32_t, const char *, uint32_t)>(
                registry_global_handler),
            .global_remove = reinterpret_cast<void (*)(void *, wl_registry *, uint32_t)>(registry_global_remove_handler)
        };

        wl_registry_add_listener(registry, &registry_listener, this);
        wl_display_roundtrip(static_cast<wl_display *>(display));

        if (!compositor || !shm || !xdg_wm_base) {
            return false;
        }

        return true;
    }

    void WaylandPlatform::Shutdown() {
    }

    MEWindow *WaylandPlatform::CreateWindow(int is_full_screen, int x, int y, int width, int height,
                                            const char *title) {
        return nullptr;
    }

    int WaylandPlatform::ProcessEvents(ME_HANDLE handle) {
        return ME_NO_EVENT_MESSAGE;
    }

    WaylandPlatform::~WaylandPlatform() = default;
}

namespace MainboardEngine {
    bool WaylandWindow::SetSize(int width, int height) {
        return false; // TODO: implement
    }

    ME_Rect WaylandWindow::GetSize() {
        return {}; // TODO: implement
    }

    bool WaylandWindow::SetPosition(int x, int y) {
        return false; // TODO: implement
    }

    void *WaylandWindow::GetMEWindowHandle() {
        return nullptr; // TODO: implement
    }

    bool WaylandWindow::SetTitle(const char *title) {
        return false; // TODO: implement
    }
}

#endif

#endif
