package com.potato;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public interface MainboardNativeLibrary extends Library {
    MainboardNativeLibrary INSTANCE = Native.load("mainboard_native", MainboardNativeLibrary.class);

    int ME_Initialize();

    Pointer ME_CreateWindow(int is_full_screen, int x, int y, int width, int height, String title);

    int ME_ProcessEvents(Pointer handle);

    int ME_RenderFrame(Pointer handle);

    int ME_DestroyWindow(Pointer handle);

    Pointer ME_GetMEWindowHandle(Pointer handle);

    int ME_SetWindowSize(Pointer handle, int width, int height);

    int ME_SetWindowTitle(Pointer handle, String title);
}
