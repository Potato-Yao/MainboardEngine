#define me_win32_window_test
 // #define me_bgfx_test
// #define me_wayland_window_test
// #define me_window_test
#include <win32_window_test.h>
#include <bgfx_test.h>

#ifdef me_wayland_window_test
#include <wayland_window_test.h>
#endif

#include <window_test.h>
#include <iostream>

int main() {
    int state = execute();
    std::cout << state << std::endl;

    return 0;
}
