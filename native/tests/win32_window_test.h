#ifdef me_win32_window_test
#include "../include/mainboard_engine.h"
#include "../include/platform.h"
#include "../include/event_message_type.h"

int execute() {
    ME_Initialize();
    ME_HANDLE window = ME_CreateWindow(0, 0, 0, 800, 600, "Hi");
    // auto platform = MainboardEngine::Win32Platform();
    // MainboardEngine::MEWindow *window = platform.CreateWindow(0, 0, 0, 800, 600, "Hi");
    // window->SetTitle("Title set");

    while (true) {
        if (ME_ProcessEvents(window) == ME_QUIT_MESSAGE) {
            break;
        }
        ME_RenderFrame(window);
    }
    ME_DestroyWindow(window);

    return 0;
}

#endif
