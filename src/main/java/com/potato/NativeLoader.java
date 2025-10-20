package com.potato;

import java.io.File;

public class NativeLoader {
    private static boolean isLoaded = false;

    // To triggering the static block
    public static void load() {
        if (isLoaded) {
            return;
        }

        File projectRoot = new File(System.getProperty("user.dir"));
        File nativeDir = new File(projectRoot, "native/MEbuild");

        System.setProperty("jna.library.path", nativeDir.getAbsolutePath());

        isLoaded = true;
    }
}
