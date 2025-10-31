#ifdef me_bgfx_test

#include <iostream>
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <mainboard_engine.h>
#include <event_message_type.h>

int execute() {
    using namespace bgfx;
    using namespace std;

    ME_Initialize();
    auto window = ME_CreateWindow(0, 100, 100, 800, 600, "BGFX Window");
    Init bgfx_init;
    bgfx_init.type = RendererType::Count;
    bgfx_init.resolution.width = 800;
    bgfx_init.resolution.height = 600;
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    PlatformData platformData;
    platformData.nwh = ME_GetMEWindowHandle(window);
    bgfx_init.platformData = platformData;

    auto stat = init(bgfx_init);
    cout << "bgfx init state: " << stat << endl;

    if (!stat) {
        cout << "ERROR: bgfx failed to initialize!" << endl;
        ME_DestroyWindow(window);
        return -1;
    }

    const RendererType::Enum renderer = getRendererType();
    cout << "Renderer type: " << getRendererName(renderer) << endl;

    // Set debug flags for more info
    setDebug(BGFX_DEBUG_TEXT);

    // Set clear color to purple/blue
    setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
    setViewRect(0, 0, 0, 800, 600);

    cout << "Starting render loop..." << endl;
    cout << "Window should show a purple background with debug text" << endl;
    cout << "Press ESC or close window to exit" << endl;

    uint32_t frameCount = 0;
    while (true) {
        if (ME_ProcessEvents(window) == ME_QUIT_MESSAGE) {
            cout << "Quit message received after " << frameCount << " frames" << endl;
            break;
        }

        // Use debug text to draw something
        dbgTextClear();
        dbgTextPrintf(0, 1, 0x0f, "BGFX Test - Frame: %d", frameCount);
        dbgTextPrintf(0, 2, 0x0f, "Renderer: %s", getRendererName(renderer));

        // Draw a simple colored pattern using debug text
        for (int y = 5; y < 20; y++) {
            for (int x = 0; x < 40; x++) {
                uint8_t color = 0x01 + (x + y) % 15;
                dbgTextPrintf(x, y, color, "*");
            }
        }

        // Make view 0 active
        touch(0);

        frame();
        frameCount++;

        if (frameCount == 1) {
            cout << "First frame rendered" << endl;
        }

        if (frameCount % 60 == 0) {
            cout << "Frame " << frameCount << " rendered successfully" << endl;
        }
    }

    shutdown();
    ME_DestroyWindow(window);

    return 0;
}

#endif
