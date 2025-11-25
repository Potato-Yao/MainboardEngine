package com.potato.NativeUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

// It should NEVER be called directly. Use `NativeCaller` instead.
public interface MainboardNativeLibrary extends Library {
    MainboardNativeLibrary INSTANCE = Native.load("mainboard_native", MainboardNativeLibrary.class);

    int ME_Initialize();

    Pointer ME_CreateWindow(int is_full_screen, int x, int y, int width, int height, String title);

    int ME_ProcessEvents(Pointer handle);

    int ME_RenderBlock(int block_id, int x, int y);

    int ME_RenderFrame(Pointer handle);

    int ME_ClearView(Pointer handle);

    int ME_DestroyWindow(Pointer handle);

    Pointer ME_GetMEWindowHandle(Pointer handle);

    int ME_SetWindowSize(Pointer handle, int width, int height);

    int ME_GetWindowSize(Pointer handle, WindowRect.ByReference rect);

    int ME_SetWindowTitle(Pointer handle, String title);

    int ME_LoadBlock(int id, String path);

    int ME_ClearBlock();
}
