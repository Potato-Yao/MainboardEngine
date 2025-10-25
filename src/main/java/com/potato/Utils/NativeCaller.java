package com.potato.Utils;

import com.potato.Config;
import com.potato.MainboardNativeLibrary;
import com.potato.Utils.EventMessage;
import com.sun.jna.Pointer;

import java.io.File;
import java.util.ArrayList;

// packing native function calls
public class NativeCaller {
    private boolean isLoaded = false;
    private MainboardNativeLibrary library;
    private Pointer windowHandle;

    public NativeCaller() {
        load();
        library = MainboardNativeLibrary.INSTANCE;
    }

    private void load() {
        if (isLoaded) {
            return;
        }
        File projectRoot = new File(System.getProperty("user.dir"));
        File nativeDir = new File(projectRoot, "native/MEbuild");

        System.setProperty("jna.library.path", nativeDir.getAbsolutePath());

        isLoaded = true;
    }

    public void initializeEngine() {
        if (library.ME_Initialize() == 0) {
            throw new RuntimeException("Failed to initialize the engine.");
        }
    }

    public void createWindow(int isFullScreen, int x, int y, int width, int height, String title) {
        windowHandle = library.ME_CreateWindow(isFullScreen, x, y, width, height, title);
        if (windowHandle == null) {
            throw new RuntimeException("Failed to create window.");
        }
    }

    public void processEvents(ArrayList<EventProcesser> eventProcessers) {
        while (true) {
            // event process of java side
            if (eventProcessers != null) {
                eventProcessers.forEach((processor) -> {
                    processor.process(Config.gameContext, this);
                });
            }

            // event process of native side
            int meg = library.ME_ProcessEvents(windowHandle);
            if (meg == EventMessage.QUIT.getCode()) {
                break;
            }

            if (library.ME_RenderFrame(windowHandle) == 0) {
                throw new RuntimeException("Failed to render frame.");
            }
        }
    }

    public void destroyWindow() {
        if (library.ME_DestroyWindow(windowHandle) == 0) {
            throw new RuntimeException("Failed to destroy window.");
        }
    }

    public void setTitle(String title) {
        library.ME_SetWindowTitle(windowHandle, title);
    }
}
