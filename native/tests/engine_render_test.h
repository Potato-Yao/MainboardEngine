#ifdef me_engine_render_test
#include "mainboard_engine.h"

#include <unordered_map>
#include <string>
#include <event_message_type.h>
#include <iostream>

int execute() {
    using namespace std;
    ME_Initialize();
    auto window = ME_CreateWindow(0, 100, 100, 800, 600, "Engine Render Test");
    ME_Rect rect;
    ME_GetWindowSize(window, &rect);
    auto map = unordered_map<int, string>();
    map.insert({0, "./native/tests/Ice_Block_(placed).png"});
    map.insert({1, "./native/tests/Cobalt_Brick_(placed).png"});

    for (auto ele: map) {
        int state = ME_LoadBlock(ele.first, ele.second.c_str());
        if (state == 0) {
            cout << "Image not loaded!" << endl;
            return 0;
        }
    }

    int cols = 2;
    int rows = 1;

    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);
    float cellWidth = width / cols;
    float cellHeight = height / rows;

    while (true) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 2; ++j) {
                int col = j % cols;
                int row = j / cols;

                uint16_t viewX = static_cast<uint16_t>(col * cellWidth) + 48 * i;
                uint16_t viewY = static_cast<uint16_t>(row * cellHeight) + 48 * i;

                ME_RenderBlock(j, viewX, viewY);
            }
        }

        int count = ME_RenderFrame(nullptr);
        ME_SetWindowTitle(window, ("Frame count: " + to_string(count)).c_str());
        if (ME_ProcessEvents(window) == ME_QUIT_MESSAGE) {
            break;
        }
    }
    // int state = ME_RenderBlock(0, 100, 10);
    // if (state == 0) {
    //     cout << "Render block 0 failed!" << endl;
    //     return 0;
    // }
    // state = ME_RenderBlock(1, 100, 100);
    // if (state == 0) {
    //     cout << "Render block 1 failed!" << endl;
    //     return 0;
    // }

    return 0;
}

#endif
