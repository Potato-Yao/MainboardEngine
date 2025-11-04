#ifdef me_win32_window_test
#include "../include/mainboard_engine.h"
#include "../include/platform.h"
#include "../include/event_message_type.h"

#include <string>

int execute() {
    ME_Initialize();
    ME_HANDLE window = ME_CreateWindow(0, 0, 0, 800, 600, "Hi");
    // auto platform = MainboardEngine::Win32Platform();
    // MainboardEngine::MEWindow *window = platform.CreateWindow(0, 0, 0, 800, 600, "Hi");
    // window->SetTitle("Title set");

    int count = 0;
    while (true) {
        if (ME_ProcessEvents(window) == ME_QUIT_MESSAGE) {
            break;
        }
        int width = 800 + count % 200;
        int height = 600 + count % 200;
        ME_SetWindowTitle(window, (std::to_string(width) + " x " + std::to_string(height)).c_str());
        ME_SetWindowSize(window, width, height);

        ME_RenderFrame(window);
        ++count;
    }
    ME_DestroyWindow(window);

    return 0;
}

#endif
