package com.potato;

import com.sun.jna.Pointer;
import org.junit.jupiter.api.Test;
import static com.potato.MainboardNativeLibrary.*;

class NativeLibraryTest {
    @Test
    void WindowCrateTest() {
        NativeLoader.load();
        MainboardNativeLibrary library = INSTANCE;

        library.ME_Initialize();
        Pointer window = library.ME_CreateWindow(0, 0, 0, 800, 600, "hi");

        while (true) {
            if (library.ME_ProcessEvents(window) == 0) {
                break;
            }
            library.ME_RenderFrame(window);
        }

        library.ME_DestroyWindow(window);
    }
}
