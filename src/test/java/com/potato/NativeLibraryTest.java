package com.potato;

import com.potato.NativeUtils.MainboardNativeLibrary;
import com.sun.jna.Pointer;
import org.junit.jupiter.api.Test;

import java.io.File;

import static com.potato.NativeUtils.MainboardNativeLibrary.*;

class NativeLibraryTest {
    @Test
    void WindowCrateTest() {
        File projectRoot = new File(System.getProperty("user.dir"));
        File nativeDir = new File(projectRoot, "native/MEbuild");

        System.setProperty("jna.library.path", nativeDir.getAbsolutePath());
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
