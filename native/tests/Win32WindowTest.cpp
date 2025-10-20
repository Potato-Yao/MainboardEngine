#include "../include/mainboard_engine.h"

int main() {
    ME_Initialize();
    ME_HANDLE window = ME_CreateWindow(0, 0, 0, 800, 600, "Hi");

    while (true) {
        if (!ME_ProcessEvents(window)) {
            break;
        }
        ME_RenderFrame(window);
    }
    ME_DestroyWindow(window);

    return 0;
}
